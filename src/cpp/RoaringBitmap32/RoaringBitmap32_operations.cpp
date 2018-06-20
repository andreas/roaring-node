#include "../RoaringBitmap32Iterator/RoaringBitmap32Iterator.h"
#include "RoaringBitmap32.h"

void RoaringBitmap32::add(const v8::FunctionCallbackInfo<v8::Value> & info) {
  v8::Isolate * isolate = info.GetIsolate();
  v8::HandleScope scope(isolate);

  if (info.Length() < 1 || !info[0]->IsUint32())
    return v8utils::throwTypeError("RoaringBitmap32::add - 32 bit unsigned integer expected");

  RoaringBitmap32 * self = v8utils::ObjectWrap::Unwrap<RoaringBitmap32>(info.Holder());
  roaring_bitmap_add(&self->roaring, info[0]->Uint32Value());
  return info.GetReturnValue().Set(info.Holder());
}

void RoaringBitmap32::tryAdd(const v8::FunctionCallbackInfo<v8::Value> & info) {
  v8::Isolate * isolate = info.GetIsolate();
  v8::HandleScope scope(isolate);

  if (info.Length() < 1 || !info[0]->IsUint32())
    return info.GetReturnValue().Set(false);

  RoaringBitmap32 * self = v8utils::ObjectWrap::Unwrap<RoaringBitmap32>(info.Holder());

  auto oldSize = roaring_bitmap_get_cardinality(&self->roaring);
  roaring_bitmap_add(&self->roaring, info[0]->Uint32Value());
  return info.GetReturnValue().Set(oldSize != roaring_bitmap_get_cardinality(&self->roaring));
}

template <typename TArg>
void roaringAddMany(v8::Isolate * isolate, RoaringBitmap32 * self, const TArg & arg) {
  if (arg->IsUint32Array() || arg->IsInt32Array()) {
    const v8utils::TypedArrayContent<uint32_t> typedArray(isolate, arg);
    roaring_bitmap_add_many(&self->roaring, typedArray.length, typedArray.data);
    return;
  }

  if (arg->IsObject() && !arg->IsNull()) {
    if (RoaringBitmap32::constructorTemplate.Get(isolate)->HasInstance(arg)) {
      RoaringBitmap32 * other = v8utils::ObjectWrap::Unwrap<RoaringBitmap32>(arg->ToObject());
      roaring_bitmap_or_inplace(&self->roaring, &other->roaring);
    } else {
      v8::Local<v8::Value> argv[] = {arg};
      auto t = TypedArrays::Uint32Array_from.Get(isolate)->Call(TypedArrays::Uint32Array.Get(isolate), 1, argv);
      if (!t.IsEmpty()) {
        const v8utils::TypedArrayContent<uint32_t> typedArray(isolate, t);
        roaring_bitmap_add_many(&self->roaring, typedArray.length, typedArray.data);
      }
    }

    return;
  }

  return v8utils::throwTypeError("Uint32Array, RoaringBitmap32 or Iterable<number> expected");
}

void RoaringBitmap32::copyFrom(const v8::FunctionCallbackInfo<v8::Value> & info) {
  v8::Isolate * isolate = info.GetIsolate();
  v8::HandleScope scope(isolate);

  if (info.Length() > 0) {
    auto const & arg = info[0];

    RoaringBitmap32 * self = v8utils::ObjectWrap::Unwrap<RoaringBitmap32>(info.Holder());

    if (arg->IsUint32Array() || arg->IsInt32Array()) {
      const v8utils::TypedArrayContent<uint32_t> typedArray(isolate, arg);
      RoaringBitmap32::clear(info);
      roaring_bitmap_add_many(&self->roaring, typedArray.length, typedArray.data);
      return;
    }

    if (arg->IsObject() && !arg->IsNull()) {
      if (RoaringBitmap32::constructorTemplate.Get(isolate)->HasInstance(arg)) {
        RoaringBitmap32 * other = v8utils::ObjectWrap::Unwrap<RoaringBitmap32>(arg->ToObject());

        if (self == other) {
          return;
        }

        ra_clear(&self->roaring.high_low_container);
        bool is_ok = ra_copy(&other->roaring.high_low_container, &self->roaring.high_low_container, other->roaring.copy_on_write);
        if (!is_ok) {
          return v8utils::throwError("RoaringBitmap32::copyFrom failed to copy");
        }

        self->roaring.copy_on_write = other->roaring.copy_on_write;
      } else {
        v8::Local<v8::Value> argv[] = {arg};
        const v8utils::TypedArrayContent<uint32_t> typedArray(
            isolate, TypedArrays::Uint32Array_from.Get(isolate)->Call(TypedArrays::Uint32Array.Get(isolate), 1, argv));
        RoaringBitmap32::clear(info);
        roaring_bitmap_add_many(&self->roaring, typedArray.length, typedArray.data);
      }
    }
    return;
  }

  return v8utils::throwTypeError("Uint32Array, RoaringBitmap32 or Iterable<number> expected");
}

void RoaringBitmap32::addMany(const v8::FunctionCallbackInfo<v8::Value> & info) {
  v8::Isolate * isolate = info.GetIsolate();
  v8::HandleScope scope(isolate);

  if (info.Length() > 0) {
    RoaringBitmap32 * self = v8utils::ObjectWrap::Unwrap<RoaringBitmap32>(info.Holder());
    roaringAddMany(info.GetIsolate(), self, info[0]);
    return info.GetReturnValue().Set(info.Holder());
  }
  return v8utils::throwTypeError("Uint32Array, RoaringBitmap32 or Iterable<number> expected");
}

void RoaringBitmap32::removeMany(const v8::FunctionCallbackInfo<v8::Value> & info) {
  v8::Isolate * isolate = info.GetIsolate();
  v8::HandleScope scope(isolate);

  if (info.Length() > 0) {
    auto const & arg = info[0];
    RoaringBitmap32 * self = v8utils::ObjectWrap::Unwrap<RoaringBitmap32>(info.Holder());
    if (RoaringBitmap32::constructorTemplate.Get(info.GetIsolate())->HasInstance(arg)) {
      RoaringBitmap32 * other = v8utils::ObjectWrap::Unwrap<RoaringBitmap32>(arg->ToObject());
      roaring_bitmap_andnot_inplace(&self->roaring, &other->roaring);
      return info.GetReturnValue().Set(info.Holder());
    } else {
      RoaringBitmap32 tmp;
      roaringAddMany(info.GetIsolate(), &tmp, arg);
      roaring_bitmap_andnot_inplace(&self->roaring, &tmp.roaring);
      return info.GetReturnValue().Set(info.Holder());
    }
  }

  return v8utils::throwTypeError("Uint32Array, RoaringBitmap32 or Iterable<number> expected");
}

void RoaringBitmap32::andInPlace(const v8::FunctionCallbackInfo<v8::Value> & info) {
  v8::Isolate * isolate = info.GetIsolate();
  v8::HandleScope scope(isolate);

  if (info.Length() > 0) {
    auto const & arg = info[0];
    RoaringBitmap32 * self = v8utils::ObjectWrap::Unwrap<RoaringBitmap32>(info.Holder());
    if (RoaringBitmap32::constructorTemplate.Get(info.GetIsolate())->HasInstance(arg)) {
      RoaringBitmap32 * other = v8utils::ObjectWrap::Unwrap<RoaringBitmap32>(arg->ToObject());
      roaring_bitmap_and_inplace(&self->roaring, &other->roaring);
      return info.GetReturnValue().Set(info.Holder());
    } else {
      RoaringBitmap32 tmp;
      roaringAddMany(info.GetIsolate(), &tmp, arg);
      roaring_bitmap_and_inplace(&self->roaring, &tmp.roaring);
      return info.GetReturnValue().Set(info.Holder());
    }
  }

  return v8utils::throwTypeError("Uint32Array, RoaringBitmap32 or Iterable<number> expected");
}

void RoaringBitmap32::xorInPlace(const v8::FunctionCallbackInfo<v8::Value> & info) {
  v8::Isolate * isolate = info.GetIsolate();
  v8::HandleScope scope(isolate);

  if (info.Length() > 0) {
    auto const & arg = info[0];
    RoaringBitmap32 * self = v8utils::ObjectWrap::Unwrap<RoaringBitmap32>(info.Holder());
    if (RoaringBitmap32::constructorTemplate.Get(info.GetIsolate())->HasInstance(arg)) {
      RoaringBitmap32 * other = v8utils::ObjectWrap::Unwrap<RoaringBitmap32>(arg->ToObject());
      roaring_bitmap_xor_inplace(&self->roaring, &other->roaring);
      return info.GetReturnValue().Set(info.Holder());
    } else {
      RoaringBitmap32 tmp;
      roaringAddMany(info.GetIsolate(), &tmp, arg);
      roaring_bitmap_xor_inplace(&self->roaring, &tmp.roaring);
      return info.GetReturnValue().Set(info.Holder());
    }
  }

  return v8utils::throwTypeError("Uint32Array, RoaringBitmap32 or Iterable<number> expected");
}

void RoaringBitmap32::remove(const v8::FunctionCallbackInfo<v8::Value> & info) {
  v8::Isolate * isolate = info.GetIsolate();
  v8::HandleScope scope(isolate);

  if (info.Length() >= 1 && info[0]->IsUint32()) {
    RoaringBitmap32 * self = v8utils::ObjectWrap::Unwrap<RoaringBitmap32>(info.Holder());
    roaring_bitmap_remove(&self->roaring, info[0]->Uint32Value());
  }
}

void RoaringBitmap32::removeChecked(const v8::FunctionCallbackInfo<v8::Value> & info) {
  v8::Isolate * isolate = info.GetIsolate();
  v8::HandleScope scope(isolate);

  if (info.Length() < 1 || !info[0]->IsUint32()) {
    info.GetReturnValue().Set(false);
  } else {
    RoaringBitmap32 * self = v8utils::ObjectWrap::Unwrap<RoaringBitmap32>(info.Holder());
    auto oldSize = roaring_bitmap_get_cardinality(&self->roaring);
    if (oldSize == 0) {
      info.GetReturnValue().Set(false);
    } else {
      roaring_bitmap_remove(&self->roaring, info[0]->Uint32Value());
      info.GetReturnValue().Set(oldSize != roaring_bitmap_get_cardinality(&self->roaring));
    }
  }
}

void RoaringBitmap32::clear(const v8::FunctionCallbackInfo<v8::Value> & info) {
  v8::Isolate * isolate = info.GetIsolate();
  v8::HandleScope scope(isolate);

  RoaringBitmap32 * self = v8utils::ObjectWrap::Unwrap<RoaringBitmap32>(info.Holder());
  roaring_bitmap_t newRoaring;
  if (!ra_init(&newRoaring.high_low_container)) {
    return v8utils::throwError("RoaringBitmap32::clear - failed to initialize a new roaring container");
  }
  ra_clear(&self->roaring.high_low_container);
  self->roaring.high_low_container = std::move(newRoaring.high_low_container);
}

inline static bool getRangeOperationParameters(const v8::FunctionCallbackInfo<v8::Value> & info, uint64_t & minInteger, uint64_t & maxInteger) {
  if (info.Length() < 2 || !info[0]->IsNumber() || !info[1]->IsNumber()) {
    return false;
  }

  double minimum = info[0]->NumberValue();
  if (std::isnan(minimum)) {
    return false;
  }

  double maximum = info[1]->NumberValue();
  if (std::isnan(maximum)) {
    return false;
  }

  if (minimum < 0) {
    minimum = 0;
  }

  if (maximum < 0) {
    maximum = 0;
  } else if (maximum > 4294967296) {
    maximum = 4294967296;
  }

  minInteger = (uint64_t)minimum;
  maxInteger = (uint64_t)maximum;

  return minInteger < maxInteger;
}

void RoaringBitmap32::flipRange(const v8::FunctionCallbackInfo<v8::Value> & info) {
  v8::Isolate * isolate = info.GetIsolate();
  v8::HandleScope scope(isolate);

  uint64_t minInteger, maxInteger;
  if (getRangeOperationParameters(info, minInteger, maxInteger)) {
    RoaringBitmap32 * self = v8utils::ObjectWrap::Unwrap<RoaringBitmap32>(info.Holder());
    roaring_bitmap_flip_inplace(&self->roaring, minInteger, maxInteger);
  }
}

void RoaringBitmap32::addRange(const v8::FunctionCallbackInfo<v8::Value> & info) {
  v8::Isolate * isolate = info.GetIsolate();
  v8::HandleScope scope(isolate);

  uint64_t minInteger, maxInteger;
  if (getRangeOperationParameters(info, minInteger, maxInteger)) {
    RoaringBitmap32 * self = v8utils::ObjectWrap::Unwrap<RoaringBitmap32>(info.Holder());
    roaring_bitmap_add_range_closed(&self->roaring, (uint32_t)minInteger, (uint32_t)(maxInteger - 1));
  }
}

void RoaringBitmap32::swapStatic(const v8::FunctionCallbackInfo<v8::Value> & info) {
  v8::Isolate * isolate = info.GetIsolate();
  v8::HandleScope scope(isolate);

  if (info.Length() < 2)
    return v8utils::throwTypeError("RoaringBitmap32::swap expects 2 arguments");

  auto constructorTemplate = RoaringBitmap32::constructorTemplate.Get(isolate);
  if (!constructorTemplate->HasInstance(info[0]))
    return v8utils::throwTypeError("RoaringBitmap32::swap first argument must be a RoaringBitmap32");

  if (!constructorTemplate->HasInstance(info[1]))
    return v8utils::throwTypeError("RoaringBitmap32::swap second argument must be a RoaringBitmap32");

  RoaringBitmap32 * a = v8utils::ObjectWrap::Unwrap<RoaringBitmap32>(info[0]->ToObject());
  RoaringBitmap32 * b = v8utils::ObjectWrap::Unwrap<RoaringBitmap32>(info[1]->ToObject());

  if (a != b) {
    std::swap(a->roaring, b->roaring);
  }
}

void RoaringBitmap32::andStatic(const v8::FunctionCallbackInfo<v8::Value> & info) {
  v8::Isolate * isolate = info.GetIsolate();
  v8::HandleScope scope(isolate);

  if (info.Length() < 2)
    return v8utils::throwTypeError("RoaringBitmap32::and expects 2 arguments");
  if (!RoaringBitmap32::constructorTemplate.Get(info.GetIsolate())->HasInstance(info[0]))
    return v8utils::throwTypeError("RoaringBitmap32::and first argument must be a RoaringBitmap32");
  if (!RoaringBitmap32::constructorTemplate.Get(info.GetIsolate())->HasInstance(info[1]))
    return v8utils::throwTypeError("RoaringBitmap32::and second argument must be a RoaringBitmap32");
  RoaringBitmap32 * a = v8utils::ObjectWrap::Unwrap<RoaringBitmap32>(info[0]->ToObject());
  RoaringBitmap32 * b = v8utils::ObjectWrap::Unwrap<RoaringBitmap32>(info[1]->ToObject());

  v8::Local<v8::Function> cons = constructor.Get(isolate);

  auto resultMaybe = cons->NewInstance(isolate->GetCurrentContext(), 0, nullptr);
  if (resultMaybe.IsEmpty())
    return;

  auto result = resultMaybe.ToLocalChecked();
  auto self = v8utils::ObjectWrap::Unwrap<RoaringBitmap32>(result);

  roaring_bitmap_t * r = roaring_bitmap_and(&a->roaring, &b->roaring);
  if (r == nullptr)
    return v8utils::throwTypeError("RoaringBitmap32::and failed materalization");

  ra_clear(&self->roaring.high_low_container);
  self->roaring.high_low_container = std::move(r->high_low_container);

  info.GetReturnValue().Set(result);
}

void RoaringBitmap32::orStatic(const v8::FunctionCallbackInfo<v8::Value> & info) {
  v8::Isolate * isolate = info.GetIsolate();
  v8::HandleScope scope(isolate);

  if (info.Length() < 2)
    return v8utils::throwTypeError("RoaringBitmap32::or expects 2 arguments");
  if (!RoaringBitmap32::constructorTemplate.Get(info.GetIsolate())->HasInstance(info[0]))
    return v8utils::throwTypeError("RoaringBitmap32::or first argument must be a RoaringBitmap32");
  if (!RoaringBitmap32::constructorTemplate.Get(info.GetIsolate())->HasInstance(info[1]))
    return v8utils::throwTypeError("RoaringBitmap32::or second argument must be a RoaringBitmap32");

  RoaringBitmap32 * a = v8utils::ObjectWrap::Unwrap<RoaringBitmap32>(info[0]->ToObject());
  RoaringBitmap32 * b = v8utils::ObjectWrap::Unwrap<RoaringBitmap32>(info[1]->ToObject());

  v8::Local<v8::Function> cons = constructor.Get(isolate);

  auto resultMaybe = cons->NewInstance(isolate->GetCurrentContext(), 0, nullptr);
  if (resultMaybe.IsEmpty())
    return;

  auto result = resultMaybe.ToLocalChecked();
  auto self = v8utils::ObjectWrap::Unwrap<RoaringBitmap32>(result);

  roaring_bitmap_t * r = roaring_bitmap_or(&a->roaring, &b->roaring);
  if (r == nullptr)
    return v8utils::throwTypeError("RoaringBitmap32::or failed materalization");

  ra_clear(&self->roaring.high_low_container);
  self->roaring.high_low_container = std::move(r->high_low_container);

  info.GetReturnValue().Set(result);
}

void RoaringBitmap32::xorStatic(const v8::FunctionCallbackInfo<v8::Value> & info) {
  v8::Isolate * isolate = info.GetIsolate();
  v8::HandleScope scope(isolate);

  if (info.Length() < 2)
    return v8utils::throwTypeError("RoaringBitmap32::xor expects 2 arguments");
  if (!RoaringBitmap32::constructorTemplate.Get(info.GetIsolate())->HasInstance(info[0]))
    return v8utils::throwTypeError("RoaringBitmap32::xor first argument must be a RoaringBitmap32");
  if (!RoaringBitmap32::constructorTemplate.Get(info.GetIsolate())->HasInstance(info[1]))
    return v8utils::throwTypeError("RoaringBitmap32::xor second argument must be a RoaringBitmap32");

  RoaringBitmap32 * a = v8utils::ObjectWrap::Unwrap<RoaringBitmap32>(info[0]->ToObject());
  RoaringBitmap32 * b = v8utils::ObjectWrap::Unwrap<RoaringBitmap32>(info[1]->ToObject());

  v8::Local<v8::Function> cons = constructor.Get(isolate);

  auto resultMaybe = cons->NewInstance(isolate->GetCurrentContext(), 0, nullptr);
  if (resultMaybe.IsEmpty())
    return;

  auto result = resultMaybe.ToLocalChecked();
  auto self = v8utils::ObjectWrap::Unwrap<RoaringBitmap32>(result);

  roaring_bitmap_t * r = roaring_bitmap_xor(&a->roaring, &b->roaring);
  if (r == nullptr)
    return v8utils::throwTypeError("RoaringBitmap32::xor failed materalization");

  ra_clear(&self->roaring.high_low_container);
  self->roaring.high_low_container = std::move(r->high_low_container);

  info.GetReturnValue().Set(result);
}

void RoaringBitmap32::andNotStatic(const v8::FunctionCallbackInfo<v8::Value> & info) {
  v8::Isolate * isolate = info.GetIsolate();
  v8::HandleScope scope(isolate);

  if (info.Length() < 2)
    return v8utils::throwTypeError("RoaringBitmap32::andnot expects 2 arguments");
  if (!RoaringBitmap32::constructorTemplate.Get(info.GetIsolate())->HasInstance(info[0]))
    return v8utils::throwTypeError("RoaringBitmap32::andnot first argument must be a RoaringBitmap32");
  if (!RoaringBitmap32::constructorTemplate.Get(info.GetIsolate())->HasInstance(info[1]))
    return v8utils::throwTypeError("RoaringBitmap32::andnot second argument must be a RoaringBitmap32");

  RoaringBitmap32 * a = v8utils::ObjectWrap::Unwrap<RoaringBitmap32>(info[0]->ToObject());
  RoaringBitmap32 * b = v8utils::ObjectWrap::Unwrap<RoaringBitmap32>(info[1]->ToObject());

  v8::Local<v8::Function> cons = constructor.Get(isolate);

  auto resultMaybe = cons->NewInstance(isolate->GetCurrentContext(), 0, nullptr);
  if (resultMaybe.IsEmpty())
    return;

  auto result = resultMaybe.ToLocalChecked();
  auto self = v8utils::ObjectWrap::Unwrap<RoaringBitmap32>(result);

  roaring_bitmap_t * r = roaring_bitmap_andnot(&a->roaring, &b->roaring);
  if (r == nullptr)
    return v8utils::throwTypeError("RoaringBitmap32::andnot failed materalization");

  ra_clear(&self->roaring.high_low_container);
  self->roaring.high_low_container = std::move(r->high_low_container);

  info.GetReturnValue().Set(result);
}

template <typename T, typename TLen>
void orManyStaticImpl(const v8::FunctionCallbackInfo<v8::Value> & info, T & array, TLen length) {
  v8::Isolate * isolate = info.GetIsolate();
  v8::HandleScope scope(isolate);

  v8::Local<v8::FunctionTemplate> ctorType = RoaringBitmap32::constructorTemplate.Get(isolate);
  v8::Local<v8::Function> cons = RoaringBitmap32::constructor.Get(isolate);

  if (length == 1) {
    if (!ctorType->HasInstance(info[0])) {
      return v8utils::throwTypeError("RoaringBitmap32::orMany accepts only RoaringBitmap32 instances");
    }

    v8::Local<v8::Value> argv[] = {info[0]};
    auto v = cons->NewInstance(isolate->GetCurrentContext(), 1, argv);
    if (!v.IsEmpty()) {
      info.GetReturnValue().Set(v.ToLocalChecked());
    }
    return;
  }

  for (TLen i = 0; i < length; ++i) {
    if (!ctorType->HasInstance(info[i])) {
      return v8utils::throwTypeError("RoaringBitmap32::orMany accepts only RoaringBitmap32 instances");
    }
  }

  const roaring_bitmap_t ** x = (const roaring_bitmap_t **)malloc(length * sizeof(roaring_bitmap_t *));
  if (x == nullptr) {
    return v8utils::throwTypeError("RoaringBitmap32::orMany failed allocation");
  }

  for (TLen i = 0; i < length; ++i) {
    auto p = v8utils::ObjectWrap::Unwrap<RoaringBitmap32>(info[i]->ToObject());
    if (p == nullptr) {
      free(x);
      return v8utils::throwTypeError("RoaringBitmap32::orMany accepts only RoaringBitmap32 instances");
    }
    x[i] = &p->roaring;
  }

  auto resultMaybe = cons->NewInstance(isolate->GetCurrentContext(), 0, nullptr);
  if (resultMaybe.IsEmpty()) {
    free(x);
    return;
  }

  auto result = resultMaybe.ToLocalChecked();
  auto self = v8utils::ObjectWrap::Unwrap<RoaringBitmap32>(result);

  ra_clear(&self->roaring.high_low_container);

  roaring_bitmap_t * r = roaring_bitmap_or_many(length, x);
  if (r == nullptr) {
    free(x);
    return v8utils::throwTypeError("RoaringBitmap32::orMany failed roaring allocation");
  }

  self->roaring.high_low_container = std::move(r->high_low_container);

  info.GetReturnValue().Set(result);
}

void RoaringBitmap32::orManyStatic(const v8::FunctionCallbackInfo<v8::Value> & info) {
  v8::Isolate * isolate = info.GetIsolate();
  v8::HandleScope scope(isolate);

  int length = info.Length();

  v8::Local<v8::FunctionTemplate> ctorType = RoaringBitmap32::constructorTemplate.Get(isolate);
  v8::Local<v8::Function> cons = RoaringBitmap32::constructor.Get(isolate);

  if (length == 0) {
    auto v = cons->NewInstance(isolate->GetCurrentContext(), 0, nullptr);
    if (!v.IsEmpty()) {
      info.GetReturnValue().Set(v.ToLocalChecked());
    }
    return;
  }

  if (length == 1) {
    if (info[0]->IsArray()) {
      auto array = v8::Local<v8::Array>::Cast(info[0]);

      size_t arrayLength = array->Length();

      if (arrayLength == 0) {
        auto v = cons->NewInstance(isolate->GetCurrentContext(), 0, nullptr);
        if (!v.IsEmpty()) {
          info.GetReturnValue().Set(v.ToLocalChecked());
        }
        return;
      }

      if (arrayLength == 1) {
        v8::Local<v8::Value> item = array->Get(0);
        if (!ctorType->HasInstance(item)) {
          return v8utils::throwTypeError("RoaringBitmap32::orMany accepts only RoaringBitmap32 instances");
        }

        v8::Local<v8::Value> argv[] = {item};
        auto v = cons->NewInstance(isolate->GetCurrentContext(), 1, argv);
        if (!v.IsEmpty()) {
          info.GetReturnValue().Set(v.ToLocalChecked());
        }
        return;
      }

      const roaring_bitmap_t ** x = (const roaring_bitmap_t **)malloc(arrayLength * sizeof(roaring_bitmap_t *));
      if (x == nullptr) {
        return v8utils::throwTypeError("RoaringBitmap32::orMany failed allocation");
      }

      for (size_t i = 0; i < arrayLength; ++i) {
        v8::Local<v8::Value> item = array->Get(i);
        if (!ctorType->HasInstance(item)) {
          return v8utils::throwTypeError("RoaringBitmap32::orMany accepts only RoaringBitmap32 instances");
        }
        auto p = v8utils::ObjectWrap::Unwrap<RoaringBitmap32>(item->ToObject());
        if (p == nullptr) {
          free(x);
          return v8utils::throwTypeError("RoaringBitmap32::orMany accepts only RoaringBitmap32 instances");
        }
        x[i] = &p->roaring;
      }

      auto resultMaybe = cons->NewInstance(isolate->GetCurrentContext(), 0, nullptr);
      if (resultMaybe.IsEmpty()) {
        free(x);
        return;
      }

      auto result = resultMaybe.ToLocalChecked();
      auto self = v8utils::ObjectWrap::Unwrap<RoaringBitmap32>(result);

      ra_clear(&self->roaring.high_low_container);

      roaring_bitmap_t * r = roaring_bitmap_or_many(arrayLength, x);
      if (r == nullptr) {
        free(x);
        return v8utils::throwTypeError("RoaringBitmap32::orMany failed roaring allocation");
      }

      self->roaring.high_low_container = std::move(r->high_low_container);

      info.GetReturnValue().Set(result);

    } else {
      if (!ctorType->HasInstance(info[0])) {
        return v8utils::throwTypeError("RoaringBitmap32::orMany accepts only RoaringBitmap32 instances");
      }

      v8::Local<v8::Value> argv[] = {info[0]};
      auto v = cons->NewInstance(isolate->GetCurrentContext(), 1, argv);
      if (!v.IsEmpty()) {
        info.GetReturnValue().Set(v.ToLocalChecked());
      }
    }
  } else {
    const roaring_bitmap_t ** x = (const roaring_bitmap_t **)malloc(length * sizeof(roaring_bitmap_t *));
    if (x == nullptr) {
      return v8utils::throwTypeError("RoaringBitmap32::orMany failed allocation");
    }

    for (int i = 0; i < length; ++i) {
      if (!ctorType->HasInstance(info[i])) {
        return v8utils::throwTypeError("RoaringBitmap32::orMany accepts only RoaringBitmap32 instances");
      }
      auto p = v8utils::ObjectWrap::Unwrap<RoaringBitmap32>(info[i]->ToObject());
      if (p == nullptr) {
        free(x);
        return v8utils::throwTypeError("RoaringBitmap32::orMany accepts only RoaringBitmap32 instances");
      }
      x[i] = &p->roaring;
    }

    auto resultMaybe = cons->NewInstance(isolate->GetCurrentContext(), 0, nullptr);
    if (resultMaybe.IsEmpty()) {
      free(x);
      return;
    }

    auto result = resultMaybe.ToLocalChecked();
    auto self = v8utils::ObjectWrap::Unwrap<RoaringBitmap32>(result);

    ra_clear(&self->roaring.high_low_container);

    roaring_bitmap_t * r = roaring_bitmap_or_many(length, x);
    if (r == nullptr) {
      free(x);
      return v8utils::throwTypeError("RoaringBitmap32::orMany failed roaring allocation");
    }

    self->roaring.high_low_container = std::move(r->high_low_container);

    info.GetReturnValue().Set(result);
  }
}