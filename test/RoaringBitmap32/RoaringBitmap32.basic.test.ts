import RoaringBitmap32 from '../../RoaringBitmap32'

describe('RoaringBitmap32 basic', () => {
  describe('add', () => {
    it('works with multiple values', () => {
      const values = [100, 120, 130, 140, 150, 99, 1, 13, 64]
      const bitmap = new RoaringBitmap32()
      for (const value of values) {
        expect(bitmap.has(value)).toBe(false)
        expect(bitmap.add(value)).toBe(bitmap)
      }
      for (const value of values) {
        expect(bitmap.has(value)).toBe(true)
      }
      expect(Array.from(bitmap)).toEqual(values.slice().sort((a, b) => a - b))
    })

    it('accepts 32 bit big numbers', () => {
      const bitmap = new RoaringBitmap32()
      expect(bitmap.has(0x7fffffff)).toBe(false)
      expect(bitmap.has(0xfffffffe)).toBe(false)
      expect(bitmap.has(0xffffffff)).toBe(false)
      bitmap.add(0x7fffffff)
      bitmap.add(0xfffffffe)
      bitmap.add(0xffffffff)
      expect(bitmap.has(0x7fffffff)).toBe(true)
      expect(bitmap.has(0xfffffffe)).toBe(true)
      expect(bitmap.has(0xffffffff)).toBe(true)
      expect(Array.from(bitmap)).toEqual([2147483647, 4294967294, 4294967295])
    })
  })

  describe('addMany', () => {
    it('works with an array', () => {
      const values = [100, 120, 130, 140, 150, 99, 1, 13, 64]
      const sortedValues = values.slice().sort((a, b) => a - b)
      const bitmap = new RoaringBitmap32()
      bitmap.addMany(values)
      for (const value of values) {
        expect(bitmap.has(value)).toBe(true)
      }

      let i = 0
      for (const value of bitmap) {
        expect(value).toBe(sortedValues[i++])
      }

      expect(i).toBe(sortedValues.length)
      expect(bitmap.size).toBe(values.length)
      expect(Array.from(bitmap)).toEqual(sortedValues)
    })

    it('works with an Uint32Array', () => {
      const values = [100, 120, 130, 140, 150, 99, 1, 13, 64]
      const bitmap = new RoaringBitmap32()
      bitmap.addMany(new Uint32Array(values))
      for (const value of values) {
        expect(bitmap.has(value)).toBe(true)
      }
      expect(Array.from(bitmap)).toEqual(values.slice().sort((a, b) => a - b))
    })
  })

  describe('constructor', () => {
    it('works with an array', () => {
      const values = [100, 120, 130, 140, 150, 99, 1, 13, 64]
      const bitmap = new RoaringBitmap32(values)
      for (const value of values) {
        expect(bitmap.has(value)).toBe(true)
      }
      expect(Array.from(bitmap)).toEqual(values.slice().sort((a, b) => a - b))
    })

    it('works with an Uint32Array', () => {
      const values = [100, 120, 130, 140, 150, 99, 1, 13, 64]
      const bitmap = new RoaringBitmap32(new Uint32Array(values))
      for (const value of values) {
        expect(bitmap.has(value)).toBe(true)
      }
      expect(Array.from(bitmap)).toEqual(values.slice().sort((a, b) => a - b))
    })
  })

  describe('toString', () => {
    it('returns "RoaringBitmap32:1" for a bitmap with 1 element', () => {
      const bitmap = new RoaringBitmap32([1])
      expect(bitmap.toString()).toEqual('RoaringBitmap32:1')
    })

    it('returns "RoaringBitmap32:2" for a bitmap with 2 elements', () => {
      const bitmap = new RoaringBitmap32([1, 2])
      expect(bitmap.toString()).toEqual('RoaringBitmap32:2')
    })
  })

  describe('contentToString', () => {
    it('generates a valid string for 1 value', () => {
      const bitmap = new RoaringBitmap32([1])
      expect(bitmap.contentToString()).toEqual('{1}')
    })
    it('generates a valid string for few values', () => {
      const values = [100, 200, 201, 202, 203, 204, 300, 0x7fffffff, 0xffffffff]
      const bitmap = new RoaringBitmap32(values)
      expect(bitmap.contentToString()).toEqual('{100,200,201,202,203,204,300,2147483647,4294967295}')
    })
  })
})
