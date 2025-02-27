import RoaringBitmap32 from "../../RoaringBitmap32";
import { expect } from "chai";

describe("RoaringBitmap32 serialization", () => {
  describe("getSerializationSizeInBytes", () => {
    it("returns standard value for empty bitmap (non portable)", () => {
      const bitmap = new RoaringBitmap32();
      expect(bitmap.getSerializationSizeInBytes(false)).eq(5);
    });

    it("returns standard value for empty bitmap (portable)", () => {
      const bitmap = new RoaringBitmap32();
      expect(bitmap.getSerializationSizeInBytes(true)).eq(8);
    });

    it("returns the correct amount of bytes (non portable)", () => {
      const bitmap = new RoaringBitmap32([1, 2, 3, 4, 5, 6, 100, 101, 105, 109, 0x7fffffff, 0xfffffffe, 0xffffffff]);
      expect(bitmap.getSerializationSizeInBytes(false)).eq(bitmap.serialize(false).byteLength);
      bitmap.runOptimize();
      bitmap.shrinkToFit();
      expect(bitmap.getSerializationSizeInBytes(false)).eq(bitmap.serialize(false).byteLength);
    });

    it("returns the correct amount of bytes (portable)", () => {
      const bitmap = new RoaringBitmap32([1, 2, 3, 4, 5, 6, 100, 101, 105, 109, 0x7fffffff, 0xfffffffe, 0xffffffff]);
      expect(bitmap.getSerializationSizeInBytes(true)).eq(bitmap.serialize(true).byteLength);
      bitmap.runOptimize();
      bitmap.shrinkToFit();
      expect(bitmap.getSerializationSizeInBytes(true)).eq(bitmap.serialize(true).byteLength);
    });
  });

  describe("serialize", () => {
    it("returns a Buffer", () => {
      expect(new RoaringBitmap32().serialize(false)).to.be.instanceOf(Buffer);
    });

    it("returns standard value for empty bitmap (non portable)", () => {
      const bitmap = new RoaringBitmap32();
      expect(Array.from(bitmap.serialize(false))).deep.equal([1, 0, 0, 0, 0]);
    });

    it("returns standard value for empty bitmap (portable)", () => {
      expect(Array.from(new RoaringBitmap32().serialize(true))).deep.equal([58, 48, 0, 0, 0, 0, 0, 0]);
    });
  });

  describe("deserialize", () => {
    it("deserializes zero length buffer (non portable)", () => {
      const bitmap = new RoaringBitmap32([1, 2, 3]);
      bitmap.deserialize(Buffer.from([]), false);
      expect(bitmap.size).eq(0);
      expect(bitmap.isEmpty).eq(true);
    });

    it("deserializes empty bitmap (non portable)", () => {
      const bitmap = new RoaringBitmap32([1, 2, 3]);
      bitmap.deserialize(Buffer.from([1, 0, 0, 0, 0]), false);
      expect(bitmap.size).eq(0);
      expect(bitmap.isEmpty).eq(true);
    });

    it("deserializes empty bitmap (non portable)", () => {
      const bitmap = new RoaringBitmap32([1, 2, 3]);
      bitmap.deserialize(Buffer.from([1, 0, 0, 0, 0]), false);
      expect(bitmap.size).eq(0);
      expect(bitmap.isEmpty).eq(true);
    });

    it("deserializes empty bitmap (portable)", () => {
      const bitmap = new RoaringBitmap32([1, 2]);
      bitmap.deserialize(Buffer.from([58, 48, 0, 0, 0, 0, 0, 0]), true);
      expect(bitmap.size).eq(0);
      expect(bitmap.isEmpty).eq(true);
    });

    it("deserializes zero length buffer (portable)", () => {
      const bitmap = new RoaringBitmap32([1, 2]);
      bitmap.deserialize(Buffer.from([]), true);
      expect(bitmap.size).eq(0);
      expect(bitmap.isEmpty).eq(true);
    });

    it("is able to deserialize test data", () => {
      const testDataSerialized = require("./data/serialized.json");

      let total = 0;
      for (const s of testDataSerialized) {
        const bitmap = RoaringBitmap32.deserialize(Buffer.from(s, "base64"), false);
        const size = bitmap.size;
        if (size !== 0) {
          expect(bitmap.has(bitmap.minimum())).eq(true);
          expect(bitmap.has(bitmap.maximum())).eq(true);
        }
        total += size;
      }
      expect(total).eq(68031);
    });
  });

  describe("deserialize static", () => {
    it("deserializes zero length buffer (non portable)", () => {
      const bitmap = RoaringBitmap32.deserialize(Buffer.from([]), false);
      expect(bitmap.size).eq(0);
      expect(bitmap.isEmpty).eq(true);
    });

    it("deserializes zero length buffer (portable)", () => {
      const bitmap = RoaringBitmap32.deserialize(Buffer.from([]), true);
      expect(bitmap.size).eq(0);
      expect(bitmap.isEmpty).eq(true);
    });

    it("deserializes empty bitmap (non portable)", () => {
      const bitmap = RoaringBitmap32.deserialize(Buffer.from([1, 0, 0, 0, 0]), false);
      expect(bitmap.size).eq(0);
      expect(bitmap.isEmpty).eq(true);
    });

    it("deserializes empty bitmap (non portable)", () => {
      const bitmap = RoaringBitmap32.deserialize(Buffer.from([1, 0, 0, 0, 0]), false);
      expect(bitmap.size).eq(0);
      expect(bitmap.isEmpty).eq(true);
    });

    it("deserializes empty bitmap (non portable)", () => {
      const bitmap = RoaringBitmap32.deserialize(Buffer.from([1, 0, 0, 0, 0]), false);
      expect(bitmap.size).eq(0);
      expect(bitmap.isEmpty).eq(true);
    });

    it("deserializes empty bitmap (portable)", () => {
      const bitmap = RoaringBitmap32.deserialize(Buffer.from([58, 48, 0, 0, 0, 0, 0, 0]), true);
      expect(bitmap.size).eq(0);
      expect(bitmap.isEmpty).eq(true);
    });
  });

  describe("serialize, deserialize", () => {
    it("is able to serialize and deserialize data (non portable)", () => {
      const values = [1, 2, 100, 101, 105, 109, 0x7fffffff, 0xfffffffe, 0xffffffff];
      const a = new RoaringBitmap32(values);
      const b = RoaringBitmap32.deserialize(a.serialize(false), false);
      expect(b.toArray()).deep.equal(values);
    });

    it("is able to serialize and deserialize data (portable)", () => {
      const values = [1, 2, 100, 101, 105, 109, 0x7fffffff, 0xfffffffe, 0xffffffff];
      const a = new RoaringBitmap32(values);
      const b = RoaringBitmap32.deserialize(a.serialize(true), true);
      expect(b.toArray()).deep.equal(values);
    });
  });
});
