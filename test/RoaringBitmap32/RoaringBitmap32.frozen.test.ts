import RoaringBitmap32 from "../../RoaringBitmap32";
import { expect } from "chai";

describe("RoaringBitmap32 frozen", () => {
  describe("getFrozenSizeInBytes", () => {
    it("returns standard value for empty bitmap", () => {
      const bitmap = new RoaringBitmap32();
      expect(bitmap.getFrozenSizeInBytes()).eq(4);
    });

    it("returns the correct amount of bytes", () => {
      const bitmap = new RoaringBitmap32([1, 2, 3, 4, 5, 6, 100, 101, 105, 109, 0x7fffffff, 0xfffffffe, 0xffffffff]);
      expect(bitmap.getFrozenSizeInBytes()).eq(bitmap.frozenSerialize().byteLength);
    });
  });

  describe("frozenSerialize", () => {
    it("returns a Buffer", () => {
      expect(new RoaringBitmap32().frozenSerialize()).to.be.instanceOf(Buffer);
    });

    it("returns standard value for empty bitmap", () => {
      const bitmap = new RoaringBitmap32();
      expect(Array.from(bitmap.frozenSerialize())).deep.equal([198, 53, 0, 0]);
    });
  });

  describe("frozenView static", () => {
    it("frozenViews zero length buffer", () => {
      const bitmap = RoaringBitmap32.frozenView(Buffer.from([]));
      expect(bitmap.size).eq(0);
      expect(bitmap.isEmpty).eq(true);
    });

    it("frozenViews empty bitmap", () => {
      const bitmap = RoaringBitmap32.frozenView(Buffer.from([198, 53, 0, 0]));
      expect(bitmap.size).eq(0);
      expect(bitmap.isEmpty).eq(true);
    });
  });

  describe("frozenSerialize, frozenView", () => {
    it("is able to frozen serialize and frozenView data", () => {
      const values = [1, 2, 100, 101, 105, 109, 0x7fffffff, 0xfffffffe, 0xffffffff];
      const a = new RoaringBitmap32(values);
      const b = RoaringBitmap32.frozenView(a.frozenSerialize());
      expect(b.toArray()).deep.equal(values);
    });
  });
});
