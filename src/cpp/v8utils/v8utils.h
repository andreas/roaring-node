#ifndef __V8UTILS__H__
#define __V8UTILS__H__

#include <node.h>
#include <node_buffer.h>
#include <uv.h>

#if NODE_MAJOR_VERSION > 14
#  define NEW_LITERAL_V8_STRING(isolate, str, type) v8::String::NewFromUtf8Literal(isolate, str, type)
#else
#  define NEW_LITERAL_V8_STRING(isolate, str, type) v8::String::NewFromUtf8(isolate, str, type).ToLocalChecked()
#endif

class JSTypes {
 public:
  static v8::Eternal<v8::Object> Uint32Array;
  static v8::Eternal<v8::Function> Uint32Array_from;

  static void initJSTypes(v8::Isolate * isolate, const v8::Local<v8::Object> & global);
};

namespace v8utils {

  uint32_t getCpusCount();

  template <typename T>
  inline void ignoreMaybeResult(v8::Maybe<T>) {}

  template <typename T>
  inline void ignoreMaybeResult(v8::MaybeLocal<T>) {}

  template <int N>
  void throwError(v8::Isolate * isolate, const char (&message)[N]) {
    isolate->ThrowException(v8::Exception::Error(NEW_LITERAL_V8_STRING(isolate, message, v8::NewStringType::kInternalized)));
  }

  template <int N>
  void throwTypeError(v8::Isolate * isolate, const char (&message)[N]) {
    isolate->ThrowException(
      v8::Exception::TypeError(NEW_LITERAL_V8_STRING(isolate, message, v8::NewStringType::kInternalized)));
  }

  void throwError(v8::Isolate * isolate, const char * message);
  void throwTypeError(v8::Isolate * isolate, const char * message);
  void throwTypeError(v8::Isolate * isolate, const char * context, const char * message);

  template <int N>
  void defineHiddenField(
    v8::Isolate * isolate, v8::Local<v8::Object> target, const char (&literal)[N], v8::Local<v8::Value> value) {
    v8::HandleScope scope(isolate);
    v8::PropertyDescriptor propertyDescriptor(value, false);
    propertyDescriptor.set_configurable(false);
    propertyDescriptor.set_enumerable(false);

    auto name = NEW_LITERAL_V8_STRING(isolate, literal, v8::NewStringType::kInternalized);
    ignoreMaybeResult(target->DefineProperty(isolate->GetCurrentContext(), name, propertyDescriptor));
  }

  template <int N>
  void defineReadonlyField(
    v8::Isolate * isolate, v8::Local<v8::Object> target, const char (&literal)[N], v8::Local<v8::Value> value) {
    v8::HandleScope scope(isolate);
    v8::PropertyDescriptor propertyDescriptor(value, false);
    propertyDescriptor.set_configurable(false);
    propertyDescriptor.set_enumerable(true);

    auto name = NEW_LITERAL_V8_STRING(isolate, literal, v8::NewStringType::kInternalized);
    ignoreMaybeResult(target->DefineProperty(isolate->GetCurrentContext(), name, propertyDescriptor));
  }

  template <int N>
  void defineHiddenFunction(
    v8::Isolate * isolate, v8::Local<v8::Object> target, const char (&literal)[N], v8::FunctionCallback callback) {
    v8::HandleScope scope(isolate);
    v8::Local<v8::FunctionTemplate> t = v8::FunctionTemplate::New(isolate, callback);

    auto name = NEW_LITERAL_V8_STRING(isolate, literal, v8::NewStringType::kInternalized);
    t->SetClassName(name);

    auto fn = t->GetFunction(isolate->GetCurrentContext());
    v8::Local<v8::Function> fnLocal;
    if (fn.ToLocal(&fnLocal)) {
      v8::PropertyDescriptor propertyDescriptor(fnLocal, false);
      propertyDescriptor.set_configurable(false);
      propertyDescriptor.set_enumerable(false);
      ignoreMaybeResult(target->DefineProperty(isolate->GetCurrentContext(), name, propertyDescriptor));
    }
  }

  template <typename T>
  struct TypedArrayContent {
    size_t length;
    T * data;
    v8::Local<v8::ArrayBuffer> arrayBuffer;
#if NODE_MAJOR_VERSION > 13
    std::shared_ptr<v8::BackingStore> backingStore;
#endif

    inline TypedArrayContent() : length(0), data(nullptr) {}

    inline TypedArrayContent(TypedArrayContent<T> & copy) : length(copy.length), data(copy.data) {}

    template <typename Q>
    inline explicit TypedArrayContent(v8::Local<Q> from) {
      set(from);
    }

    template <typename Q>
    inline explicit TypedArrayContent(v8::MaybeLocal<Q> from) {
      set(from);
    }

    inline void reset() {
      this->length = 0;
      this->data = nullptr;
#if NODE_MAJOR_VERSION > 13
      this->backingStore = nullptr;
#endif
      this->arrayBuffer.Clear();
    }

    template <typename Q>
    inline bool set(v8::MaybeLocal<Q> from) {
      v8::Local<Q> local;
      if (from.ToLocal(&local)) return this->set(local);
      this->length = 0;
      this->data = nullptr;
      return false;
    }

    template <typename Q>
    inline bool set(v8::Local<Q> from) {
      if (!from.IsEmpty() && from->IsArrayBufferView()) {
        auto array = v8::Local<v8::ArrayBufferView>::Cast(from);
        this->length = array->ByteLength() / sizeof(T);
        this->arrayBuffer = array->Buffer();
#if NODE_MAJOR_VERSION > 13
        this->backingStore = this->arrayBuffer->GetBackingStore();
        this->data = (T *)((uint8_t *)(this->backingStore->Data()) + array->ByteOffset());
#else
        this->data = (T *)((uint8_t *)(this->arrayBuffer->GetContents().Data()) + array->ByteOffset());
#endif
        return true;
      }

      this->length = 0;
      this->data = nullptr;
      return false;
    }
  };

  namespace ObjectWrap {
    template <class T>
    static T * Unwrap(v8::Local<v8::Object> object) {
      return (T *)(object->GetAlignedPointerFromInternalField(0));
    }

    template <class T>
    static T * TryUnwrap(const v8::Local<v8::Value> & value, v8::Isolate * isolate) {
      v8::Local<v8::Object> obj;
      if (!value->ToObject(isolate->GetCurrentContext()).ToLocal(&obj)) {
        return nullptr;
      }
      return (T *)(obj->GetAlignedPointerFromInternalField(0));
    }

    template <class T>
    static T * TryUnwrap(
      const v8::Local<v8::Value> & value, const v8::Local<v8::FunctionTemplate> & ctorTemplate, v8::Isolate * isolate) {
      return !ctorTemplate.IsEmpty() && ctorTemplate->HasInstance(value) ? ObjectWrap::TryUnwrap<T>(value, isolate)
                                                                         : nullptr;
    }

    template <class T>
    static T * TryUnwrap(
      const v8::Local<v8::Value> & value, const v8::Persistent<v8::FunctionTemplate> & ctorTemplate, v8::Isolate * isolate) {
      return ObjectWrap::TryUnwrap<T>(value, ctorTemplate.Get(isolate), isolate);
    }

    template <class T>
    static T * TryUnwrap(
      const v8::Local<v8::Value> & value, const v8::Eternal<v8::FunctionTemplate> & ctorTemplate, v8::Isolate * isolate) {
      return ObjectWrap::TryUnwrap<T>(value, ctorTemplate.Get(isolate), isolate);
    }

    template <class T>
    static T * TryUnwrap(
      const v8::FunctionCallbackInfo<v8::Value> & info,
      int argumentIndex,
      const v8::Local<v8::FunctionTemplate> & ctorTemplate) {
      return info.Length() <= argumentIndex ? nullptr
                                            : ObjectWrap::TryUnwrap<T>(info[argumentIndex], ctorTemplate, info.GetIsolate());
    }

    template <class T>
    static T * TryUnwrap(
      const v8::FunctionCallbackInfo<v8::Value> & info,
      int argumentIndex,
      const v8::Persistent<v8::FunctionTemplate> & ctorTemplate) {
      return info.Length() <= argumentIndex ? nullptr
                                            : ObjectWrap::TryUnwrap<T>(info[argumentIndex], ctorTemplate, info.GetIsolate());
    }

    template <class T>
    static T * TryUnwrap(
      const v8::FunctionCallbackInfo<v8::Value> & info,
      int argumentIndex,
      const v8::Eternal<v8::FunctionTemplate> & ctorTemplate) {
      return info.Length() <= argumentIndex ? nullptr
                                            : ObjectWrap::TryUnwrap<T>(info[argumentIndex], ctorTemplate, info.GetIsolate());
    }
  }  // namespace ObjectWrap

  typedef const char * const_char_ptr_t;

  class AsyncWorker {
   public:
    v8::Isolate * const isolate;

    explicit AsyncWorker(v8::Isolate * isolate);

    virtual ~AsyncWorker();

    bool setCallback(v8::Local<v8::Value> callback);

    inline bool hasError() const { return this->_error != nullptr; }

    inline void setError(const_char_ptr_t error) {
      if (error != nullptr && this->_error == nullptr) {
        this->_error = error;
      }
    }

    inline void clearError() { this->_error = nullptr; }

    static v8::Local<v8::Value> run(AsyncWorker * worker);

   protected:
    // Called in a thread to execute the workload
    virtual void work() = 0;

    // Called after the thread completes without errors.
    virtual v8::Local<v8::Value> done();

   private:
    uv_work_t _task{};
    volatile const_char_ptr_t _error;
    volatile bool _completed;
    v8::Persistent<v8::Function> _callback;
    v8::Persistent<v8::Promise::Resolver> _resolver;

    v8::Local<v8::Value> _invokeDone();
    virtual bool _start();
    static void _complete(AsyncWorker * worker);
    static void _resolveOrReject(AsyncWorker * worker);
    static void _work(uv_work_t * request);
    static void _done(uv_work_t * request, int status);

    friend class ParallelAsyncWorker;
  };

  class ParallelAsyncWorker : public AsyncWorker {
   public:
    uint32_t loopCount;
    uint32_t concurrency;

    explicit ParallelAsyncWorker(v8::Isolate * isolate);
    virtual ~ParallelAsyncWorker();

   protected:
    void work() override;

    virtual void parallelWork(uint32_t index) = 0;

   private:
    uv_work_t * _tasks;
    volatile int32_t _pendingTasks;
    volatile uint32_t _currentIndex;

    bool _start() override;

    static void _parallelWork(uv_work_t * request);
    static void _parallelDone(uv_work_t * request, int status);
  };

}  // namespace v8utils

#endif
