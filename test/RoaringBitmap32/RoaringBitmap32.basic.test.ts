import RoaringBitmap32 from "../../RoaringBitmap32";
import { expect, use as chaiUse } from "chai";

chaiUse(require("chai-as-promised"));

describe("RoaringBitmap32 basic", () => {
  describe("minimum", () => {
    it("returns 4294967295 for an empty bitmap", () => {
      expect(new RoaringBitmap32().minimum()).eq(4294967295);
    });
    it("returns the minimum value", () => {
      const bitmap = new RoaringBitmap32([5, 3, 1, 2, 9]);
      expect(bitmap.minimum()).eq(1);
    });
  });

  describe("maximum", () => {
    it("returns 0 for an empty bitmap", () => {
      expect(new RoaringBitmap32().maximum()).eq(0);
    });
    it("returns the maximum value", () => {
      const bitmap = new RoaringBitmap32([5, 3, 1, 2, 9]);
      expect(bitmap.maximum()).eq(9);
    });
  });

  describe("toString", () => {
    it('returns "RoaringBitmap32', () => {
      expect(new RoaringBitmap32().toString()).eq("RoaringBitmap32");
    });
  });

  describe("contentToString", () => {
    it('returns "[]" for an empty bitmap', () => {
      expect(new RoaringBitmap32().contentToString()).eq("[]");
    });
    it("generates a valid string for 1 value", () => {
      const bitmap = new RoaringBitmap32([1]);
      expect(bitmap.contentToString()).eq("[1]");
    });
    it("generates a valid string for few values", () => {
      const values = [100, 200, 201, 202, 203, 204, 300, 0x7fffffff, 0xffffffff];
      const bitmap = new RoaringBitmap32(values);
      expect(bitmap.contentToString()).eq("[100,200,201,202,203,204,300,2147483647,4294967295]");
    });
    it("supports maxLength correctly", () => {
      const values = [100, 200, 201, 202, 203, 204, 300, 0x7fffffff, 0xffffffff];
      const bitmap = new RoaringBitmap32(values);
      expect(bitmap.contentToString(13)).eq("[100,200,201,202...]");
    });
  });

  describe("clone", () => {
    it("returns a cloned empty bitmap", () => {
      const bitmap1 = new RoaringBitmap32();
      const bitmap2 = bitmap1.clone();
      expect(bitmap1 !== bitmap2).eq(true);
      expect(bitmap2).to.be.instanceOf(RoaringBitmap32);
      expect(bitmap2.size).eq(0);
      expect(bitmap2.isEmpty).eq(true);
    });

    it("returns a cloned bitmap", () => {
      const values = [1, 2, 100, 101, 200, 400, 0x7fffffff, 0xffffffff];
      const bitmap1 = new RoaringBitmap32(values);
      const bitmap2 = bitmap1.clone();
      expect(bitmap1 !== bitmap2).eq(true);
      expect(bitmap2).to.be.instanceOf(RoaringBitmap32);
      expect(bitmap2.size).eq(values.length);
      expect(bitmap2.isEmpty).eq(false);
      expect(Array.from(bitmap2.toUint32Array())).deep.equal(values);
    });
  });

  describe("select", () => {
    it("returns undefined for invalid values on an empty bitmap", () => {
      const bitmap = new RoaringBitmap32();
      expect(bitmap.select(null as any)).to.be.undefined;
      expect(bitmap.select(undefined as any)).to.be.undefined;
      expect(bitmap.select(-123 as any)).to.be.undefined;
      expect(bitmap.select([123] as any)).to.be.undefined;
      expect(bitmap.select(0)).to.be.undefined;
      expect(bitmap.select(100)).to.be.undefined;
      expect(bitmap.select(0x7fffffff)).to.be.undefined;
      expect(bitmap.select(3)).to.be.undefined;
    });

    it("returns the correct value", () => {
      const bitmap = new RoaringBitmap32([1, 2, 3, 4, 5, 10, 100, 1000, 2000, 3000]);
      expect(bitmap.select(0)).eq(1);
      expect(bitmap.select(1)).eq(2);
      expect(bitmap.select(2)).eq(3);
      expect(bitmap.select(3)).eq(4);
      expect(bitmap.select(4)).eq(5);
      expect(bitmap.select(5)).eq(10);
      expect(bitmap.select(6)).eq(100);
      expect(bitmap.select(7)).eq(1000);
      expect(bitmap.select(8)).eq(2000);
      expect(bitmap.select(9)).eq(3000);
      expect(bitmap.select(10)).to.be.undefined;
    });
  });

  describe("rank", () => {
    it("returns 0 for invalid values", () => {
      const bitmap = new RoaringBitmap32();
      expect(bitmap.rank(null as any)).eq(0);
      expect(bitmap.rank(undefined as any)).eq(0);
      expect(bitmap.rank(-123 as any)).eq(0);
      expect(bitmap.rank([123] as any)).eq(0);
    });

    it("returns 0 with any value on an empty bitmap", () => {
      const bitmap = new RoaringBitmap32();
      expect(bitmap.rank(0)).eq(0);
      expect(bitmap.rank(100)).eq(0);
      expect(bitmap.rank(0x7fffffff)).eq(0);
      expect(bitmap.rank(0xffffffff)).eq(0);
    });

    it("returns the correct value", () => {
      const bitmap = new RoaringBitmap32([1, 2, 3, 4, 5, 10, 100, 1000, 2000, 3000]);
      expect(bitmap.rank(0)).eq(0);
      expect(bitmap.rank(1)).eq(1);
      expect(bitmap.rank(2)).eq(2);
      expect(bitmap.rank(3)).eq(3);
      expect(bitmap.rank(4)).eq(4);
      expect(bitmap.rank(5)).eq(5);
      expect(bitmap.rank(10)).eq(6);
      expect(bitmap.rank(100)).eq(7);
      expect(bitmap.rank(1000)).eq(8);
      expect(bitmap.rank(2000)).eq(9);
      expect(bitmap.rank(3000)).eq(10);
    });
  });

  describe("toUint32Array", () => {
    it("returns an empty Uint32Array for an empty bitmap", () => {
      const a = new RoaringBitmap32().toUint32Array();
      expect(a).to.be.instanceOf(Uint32Array);
      expect(a).to.have.lengthOf(0);
    });

    it("returns an array with 1 element for 1 element", () => {
      const bitmap = new RoaringBitmap32([1]);
      const x = bitmap.toUint32Array();
      expect(x).to.be.instanceOf(Uint32Array);
      expect(x).to.have.lengthOf(1);
      expect(Array.from(x)).deep.equal([1]);
    });

    it("returns an array with multiple elements", () => {
      const bitmap = new RoaringBitmap32([1, 2, 10, 30, 0x7fffffff, 0xffffffff]);
      const x = bitmap.toUint32Array();
      expect(x).to.be.instanceOf(Uint32Array);
      expect(x).to.have.lengthOf(6);
      expect(Array.from(x)).deep.equal([1, 2, 10, 30, 0x7fffffff, 0xffffffff]);
    });
  });

  describe("rangeUint32Array", () => {
    it("returns an empty Uint32Array for an empty bitmap", () => {
      const a = new RoaringBitmap32().rangeUint32Array(0, 10);
      expect(a).to.be.instanceOf(Uint32Array);
      expect(a).to.have.lengthOf(0);
    });

    it("returns a paginated array with multiple elements", () => {
      const bitmap = new RoaringBitmap32([1, 2, 10, 30, 50, 70, 100]);
      const x = bitmap.rangeUint32Array(0, 3);
      expect(x).to.be.instanceOf(Uint32Array);
      expect(x).to.have.lengthOf(3);
      expect(Array.from(x)).deep.equal([1, 2, 10]);
    });

    it("returns a paginated array with offset", () => {
      const bitmap = new RoaringBitmap32([1, 2, 10, 30, 50, 70, 100]);
      const x = bitmap.rangeUint32Array(2, 3);
      expect(x).to.be.instanceOf(Uint32Array);
      expect(x).to.have.lengthOf(3);
      expect(Array.from(x)).deep.equal([10, 30, 50]);
    });

    it("returns a paginated array with offset almost out of scope", () => {
      const bitmap = new RoaringBitmap32([1, 2, 10, 30, 50, 70, 100]);
      const x = bitmap.rangeUint32Array(6, 3);
      expect(x).to.be.instanceOf(Uint32Array);
      expect(x).to.have.lengthOf(1);
      expect(Array.from(x)).deep.equal([100]);
    });

    it("returns a paginated array with offset out of scope", () => {
      const bitmap = new RoaringBitmap32([1, 2, 10, 30, 50, 70, 100]);
      const x = bitmap.rangeUint32Array(10, 3);
      expect(x).to.be.instanceOf(Uint32Array);
      expect(x).to.have.lengthOf(0);
      expect(Array.from(x)).deep.equal([]);
    });

    it("returns a paginated array with limit out of scope", () => {
      const bitmap = new RoaringBitmap32([1, 2, 10, 30, 50, 70, 100]);
      const x = bitmap.rangeUint32Array(0, 100);
      expect(x).to.be.instanceOf(Uint32Array);
      expect(x).to.have.lengthOf(7);
    });

    it("returns a paginated array with limit and offset out of scope", () => {
      const bitmap = new RoaringBitmap32([1, 2, 10, 30, 50, 70, 100]);
      const x = bitmap.rangeUint32Array(5, 100);
      expect(x).to.be.instanceOf(Uint32Array);
      expect(x).to.have.lengthOf(2);
    });
  });

  describe("toArray", () => {
    it("returns an empty array for an empty bitmap", () => {
      expect(new RoaringBitmap32().toArray()).deep.equal([]);
    });

    it("returns an array with 1 element for 1 element", () => {
      const bitmap = new RoaringBitmap32([1]);
      expect(bitmap.toArray()).deep.equal([1]);
    });

    it("returns an array with multiple elements", () => {
      const bitmap = new RoaringBitmap32([1, 2, 10, 30, 0x7fffffff, 0xffffffff]);
      expect(bitmap.toArray()).deep.equal([1, 2, 10, 30, 0x7fffffff, 0xffffffff]);
    });
  });

  describe("toSet", () => {
    it("returns an empty set for an empty bitmap", () => {
      const set = new RoaringBitmap32().toSet();
      expect(set).to.be.instanceOf(Set);
      expect(Array.from(set)).deep.equal([]);
    });

    it("returns an array with 1 element for 1 element", () => {
      const set = new RoaringBitmap32([1]).toSet();
      expect(set).to.be.instanceOf(Set);
      expect(Array.from(set)).deep.equal([1]);
    });

    it("returns an array with multiple elements", () => {
      const values = [1, 2, 10, 30, 0x7fffffff, 0xffffffff];
      const set = new RoaringBitmap32(values).toSet();
      expect(set).to.be.instanceOf(Set);
      expect(Array.from(set)).deep.equal(values);
    });
  });

  describe("toJSON", () => {
    it("returns an empty array with an empty bitmap", () => {
      expect(new RoaringBitmap32().toJSON()).deep.equal([]);
    });

    it("works with JSON.stringify with an empty bitmap", () => {
      const bitmap = new RoaringBitmap32();
      expect(JSON.stringify(bitmap)).eq("[]");
    });

    it("returns an array with all the values", () => {
      const bitmap = new RoaringBitmap32([1, 2, 3]);
      expect(bitmap.toJSON()).deep.equal([1, 2, 3]);
    });

    it("works with JSON.stringify", () => {
      const bitmap = new RoaringBitmap32([1, 2, 3]);
      expect(JSON.stringify(bitmap)).eq("[1,2,3]");
    });
  });

  describe("statistics", () => {
    it("returns a statistics object for an empty bitmap", () => {
      const bitmap = new RoaringBitmap32();
      const statistics = bitmap.statistics();
      expect(statistics).deep.equal({
        containers: 0,
        arrayContainers: 0,
        runContainers: 0,
        bitsetContainers: 0,
        valuesInArrayContainers: 0,
        valuesInRunContainers: 0,
        valuesInBitsetContainers: 0,
        bytesInArrayContainers: 0,
        bytesInRunContainers: 0,
        bytesInBitsetContainers: 0,
        maxValue: 0,
        minValue: 4294967295,
        sumOfAllValues: 0,
        size: 0,
      });
    });

    it("returns valid statistics", () => {
      const rb = new RoaringBitmap32();
      expect(rb.addMany([1, 2, 3, 4, 6, 7])).eq(rb);
      expect(rb.addMany([999991, 999992, 999993, 999994, 999996, 999997])).eq(rb);
      expect(rb.statistics()).deep.equal({
        containers: 2,
        arrayContainers: 2,
        runContainers: 0,
        bitsetContainers: 0,
        valuesInArrayContainers: 12,
        valuesInRunContainers: 0,
        valuesInBitsetContainers: 0,
        bytesInArrayContainers: 24,
        bytesInRunContainers: 0,
        bytesInBitsetContainers: 0,
        maxValue: 999997,
        minValue: 1,
        sumOfAllValues: 5999986,
        size: 12,
      });
      rb.runOptimize();
      rb.shrinkToFit();
      expect(rb.statistics()).deep.equal({
        containers: 2,
        arrayContainers: 0,
        runContainers: 2,
        bitsetContainers: 0,
        valuesInArrayContainers: 0,
        valuesInRunContainers: 12,
        valuesInBitsetContainers: 0,
        bytesInArrayContainers: 0,
        bytesInRunContainers: 20,
        bytesInBitsetContainers: 0,
        maxValue: 999997,
        minValue: 1,
        sumOfAllValues: 5999986,
        size: 12,
      });
    });
  });

  describe("removeRunCompression", () => {
    it("does nothing with an empty bitmap", () => {
      const bitmap = new RoaringBitmap32();
      expect(bitmap.removeRunCompression()).eq(false);
    });
  });

  describe("runOptimize", () => {
    it("does nothing with an empty bitmap", () => {
      const bitmap = new RoaringBitmap32();
      expect(bitmap.runOptimize()).eq(false);
    });
    it("run optimizes", () => {
      const bitmap = new RoaringBitmap32();
      for (let i = 200; i > 0; --i) {
        bitmap.add(i);
      }
      expect(bitmap.runOptimize()).eq(true);
      expect(bitmap.statistics().runContainers).eq(1);
      bitmap.removeRunCompression();
      bitmap.shrinkToFit();
      expect(bitmap.statistics().runContainers).eq(0);
      expect(bitmap.size).eq(200);
    });
  });

  describe("shrinkToFit", () => {
    it("shrinks some memory", () => {
      const bitmap = new RoaringBitmap32([1, 2, 3]);
      bitmap.removeMany([1, 2, 3]);
      expect(bitmap.shrinkToFit()).to.be.gt(0);
    });
  });

  describe("general tests", () => {
    it("allows adding 900 values", () => {
      const bitmap = new RoaringBitmap32();
      for (let i = 100; i < 1000; ++i) {
        bitmap.add(i);
      }
      expect(bitmap.size).eq(900);
      expect(bitmap.runOptimize()).eq(true);
      expect(bitmap.size).eq(900);
      expect(bitmap.minimum()).eq(100);
      expect(bitmap.maximum()).eq(999);
    });

    it('works with some "fancy" operations', () => {
      const rb1 = new RoaringBitmap32();
      rb1.add(1);
      rb1.add(2);
      rb1.add(3);
      rb1.add(4);
      rb1.add(5);
      rb1.add(100);
      rb1.add(1000);
      rb1.runOptimize();

      const rb2 = new RoaringBitmap32([3, 4, 1000]);
      rb2.runOptimize();
      const rb3 = new RoaringBitmap32();

      expect(rb1.size).eq(7);
      expect(rb1.has(1)).eq(true);
      expect(rb1.has(3)).eq(true);

      expect(rb1.contentToString()).eq("[1,2,3,4,5,100,1000]");

      rb1.andInPlace(rb2);

      expect(rb1.has(1)).eq(false);
      expect(rb1.has(3)).eq(true);

      expect(rb1.size).eq(3);
      expect(rb1.contentToString()).eq("[3,4,1000]");

      rb3.add(5);
      rb3.orInPlace(rb1);

      expect(Array.from(rb3)).deep.equal([3, 4, 5, 1000]);
      expect(rb3.toArray()).deep.equal([3, 4, 5, 1000]);
      expect(Array.from(rb3.toUint32Array())).deep.equal([3, 4, 5, 1000]);

      const rb4 = RoaringBitmap32.orMany(rb1, rb2, rb3);
      expect(rb4.toArray()).deep.equal([3, 4, 5, 1000]);
    });
  });

  describe("functions are protected against illegal invocations", () => {
    it("allows subclassing", () => {
      class Subclass extends RoaringBitmap32 {
        public func(): void {
          this.add(1);
          this.addMany([3, 4]);
        }

        public add(value: number): this {
          super.add(value);
          super.add(value + 1);
          return this;
        }
      }

      const instance = new Subclass();
      expect(instance).to.be.instanceOf(RoaringBitmap32);
      instance.func();
      expect(instance.toArray()).deep.equal([1, 2, 3, 4]);
      instance.add(4);
      expect(instance.toArray()).deep.equal([1, 2, 3, 4, 5]);
    });

    it("throws with prototype.add direct call", () => {
      expect(() => {
        RoaringBitmap32.prototype.add(123);
      }).to.throw(Error);
    });

    it("throws with prototype.add.call on null", () => {
      expect(() => {
        RoaringBitmap32.prototype.add.call(null, 123);
      }).to.throw(Error);
    });

    it("throws with prototype.add.call on a wrong object", () => {
      expect(() => {
        RoaringBitmap32.prototype.add.call({}, 123);
      }).to.throw(Error);
    });

    it("throws when calling on the wrong native object", () => {
      const bitmap = new RoaringBitmap32();
      expect(() => {
        bitmap.add.call(bitmap[Symbol.iterator](), 123);
      }).to.throw(Error);
    });

    it("allows prototype.add.call on the right object", () => {
      const bitmap = new RoaringBitmap32();
      RoaringBitmap32.prototype.add.call(bitmap, 123);
      expect(bitmap.toArray()).deep.equal([123]);
    });
  });

  it("implements Set<> interface properly", () => {
    const x: Set<number> = new RoaringBitmap32([1, 3]);
    x.add(2);
    expect(x.has(2));
    expect(Array.from(x.entries())).to.deep.equal([
      [1, 1],
      [2, 2],
      [3, 3],
    ]);
  });

  describe("forEach", () => {
    it("does nothing for an empty roaring bitmap", () => {
      let invoked = false;
      const bitmap = new RoaringBitmap32();
      bitmap.forEach(() => (invoked = true));
      expect(invoked).to.equal(false);
    });

    it("invokes the function with a single item", () => {
      const invoked: any[] = [];
      const bitmap = new RoaringBitmap32([1]);
      bitmap.forEach((...args) => {
        invoked.push(args);
      });
      expect(invoked).to.deep.equal([[1, 1, bitmap]]);
      expect(typeof invoked[0][0]).eq("number");
    });

    it("invokes the function with multiple items", () => {
      const invoked: any[] = [];
      const bitmap = new RoaringBitmap32([2, 5, 1, 7, 6]);
      bitmap.forEach((...args) => invoked.push(args));
      expect(invoked).to.deep.equal([
        [1, 1, bitmap],
        [2, 2, bitmap],
        [5, 5, bitmap],
        [6, 6, bitmap],
        [7, 7, bitmap],
      ]);
    });

    it("handles exceptions well", () => {
      const invoked: any[] = [];
      const bitmap = new RoaringBitmap32([2, 5, 1, 7, 6]);
      expect(() =>
        bitmap.forEach((...args) => {
          invoked.push(args);
          if (invoked.length === 3) {
            throw new Error("expected");
          }
        }),
      ).to.throw("expected");
      expect(invoked).to.deep.equal([
        [1, 1, bitmap],
        [2, 2, bitmap],
        [5, 5, bitmap],
      ]);
    });
  });
});
