/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
  */
#ifndef INDET_MODULEKEYHELPER_H
#define INDET_MODULEKEYHELPER_H

#include <array>
#include <type_traits>

namespace InDet {
   namespace MaskUtils {
      /** Convenience method to create a mask for which exactly one  contiguous sequence of bits is set to 1.
       * @tparam bit lowest bit of the sequence
       * @tparam end the first bit not part of this sequence must be larger or equal (for an empty mask) )to bit.
       */
      template<unsigned int bit, unsigned int end, typename T=unsigned int>
      static consteval T createMask() {
         if constexpr(bit>31u) {
            return static_cast<T>(0u);
         }
         else if constexpr(bit>=end) {
            return static_cast<T>(0u);
         }
         else {
            return static_cast<T>(1u<<bit) | createMask<bit+1u,end, T>();
         }
      }
   }

   /** Helper class to create keys for defects described by chip, column and row indices, and a mask.
    *
    * @tparam T_ROW_BITS number of bits to store the row index of a defect.
    * @tparam T_COL_BITS number of bits to store the column index of a defect.
    * @tparam T_CHIP_BITS number of bits to store the chip index of a defect.
    *
    * The key assumes a hierarchical ordering of the indices where the chip index ranks highest and the
    * row index lowest. The range bits indicate that a key marks the beginning of an inclusive range
    * till the previous key (previous because of the reverse order).
    */
   template <typename T, unsigned  int T_ROW_BITS, unsigned int T_COL_BITS, unsigned int T_CHIP_BITS, unsigned int T_TYPE_BITS=0u>
   struct ModuleKeyHelper {
      static constexpr unsigned int ROW_BITS = T_ROW_BITS;
      static constexpr unsigned int COL_BITS = T_COL_BITS;
      static constexpr unsigned int CHIP_BITS = T_CHIP_BITS;
      static constexpr unsigned int RANGE_FLAG_BITS = 1u;
      static constexpr unsigned int TYPE_BITS = T_TYPE_BITS;
      static constexpr T ROW_SHIFT   = 0u;
      static constexpr T COL_SHIFT   = ROW_BITS;
      static constexpr T CHIP_SHIFT  = ROW_BITS + COL_BITS;
      static constexpr T RANGE_FLAG_SHIFT = ROW_BITS + COL_BITS + CHIP_BITS;
      static constexpr T TYPE_SHIFT  = RANGE_FLAG_SHIFT + RANGE_FLAG_BITS;
      static constexpr T ROW_MASK    = MaskUtils::createMask<0,                          ROW_BITS>();
      static constexpr T COL_MASK    = MaskUtils::createMask<ROW_BITS,                   ROW_BITS+COL_BITS>();
      static constexpr T CHIP_MASK   = MaskUtils::createMask<ROW_BITS+COL_BITS,          ROW_BITS+COL_BITS+CHIP_BITS>();
      static constexpr T RANGE_FLAG_MASK  = MaskUtils::createMask<ROW_BITS+COL_BITS+CHIP_BITS,ROW_BITS+COL_BITS+CHIP_BITS+RANGE_FLAG_BITS>();
      static constexpr T TYPE_MASK   = MaskUtils::createMask<TYPE_SHIFT,TYPE_SHIFT+TYPE_BITS>();
      using KEY_TYPE = T;

   protected:
      /** Convenience method to create part of a key.
       * @tparam SHIFT the given value will be shifted by this ammount
       * @tparam MASK the shifted value must not overflow this mask.
       * @param val the value to be stored in the key part
       */
      template <unsigned int SHIFT, T MASK>
      static constexpr T makeKeyPart([[maybe_unused]] T val) {
         if constexpr(MASK==0) {
            return T{};
         }
         else {
            assert (((val << SHIFT) & MASK) == (val << SHIFT));
            return (val << SHIFT);
         }
      }

   public:
      /** Create a key from mask, chip, column and row indices.
       * @param is_range if true the key marks the beginning of an inclusive range
       * @param chip the index of a chip starting from zero
       * @param col the index of a column starting from zero
       * @param row the index of a row starting from zero
       *
       * The indices must be representable by the number of reserved bits.
       */
      static constexpr T makeKey(bool is_range, unsigned int chip, unsigned int col, unsigned int row=0u) {
         return   static_cast<T>(is_range) << RANGE_FLAG_SHIFT
            | makeKeyPart<CHIP_SHIFT,CHIP_MASK>(chip)
            | makeKeyPart<COL_SHIFT,COL_MASK>(col)
            | makeKeyPart<ROW_SHIFT,ROW_MASK>(row);
      }

      /** Get the column index from a full key.
       */
      static constexpr T getColumn(T key) { return (key & COL_MASK)   >> COL_SHIFT; }

      /** Get the row index from a full key.
       */
      static constexpr T getRow(T key)    { return (key & ROW_MASK)   >> ROW_SHIFT; }

      /** Get the maximum row value
       */
      static constexpr T getLimitRowMax()     { return ROW_MASK; }

      /** Get the maximum row value
       */
      static constexpr T getLimitColumnMax()  { return COL_MASK; }

      /** Get the column index from a full key.
       */
      static constexpr T getChip(T key)   { return (key & CHIP_MASK)  >> CHIP_SHIFT; }

      /** Get an associated defect type.
       */
      static constexpr T getDefectType(T key) {
         if constexpr(TYPE_BITS>0) {
            return (key & TYPE_MASK)  >> TYPE_SHIFT;
         }
         else {
            return T{};
         }
      }

      /** Get key component of an associated defect type.
       */
      static constexpr T getDefectTypeComponent(T key) {
         if constexpr(TYPE_BITS>0) {
            return key & TYPE_MASK;
         }
         else {
            return T{};
         }
      }

      /** Make the key component representing the an associated defect type
       */
      static constexpr T makeDefectTypeKey(unsigned int defect_type)
      {
         if constexpr(TYPE_BITS>0) {
            assert( (((defect_type << TYPE_SHIFT ) & TYPE_MASK) >> TYPE_SHIFT) == defect_type);
            return (defect_type << TYPE_SHIFT ) & TYPE_MASK;
         }
         else {
            return T{};
         }
      }

      /** Test whether a key is a range key.
       * Range keys mark the beginning of inclusive range.
       */
      static constexpr bool isRangeKey(T key) {
         if constexpr(TYPE_MASK) {
            return ((key & RANGE_FLAG_MASK)>>RANGE_FLAG_SHIFT);
         }
         else {
            return ((key>>RANGE_FLAG_SHIFT) );
         }
      }

      /** Turn a key into a range key.
       * Such keys mark the beginning of an inclusive range.
       */
      static constexpr T makeRangeKey(T key) { return key | RANGE_FLAG_MASK; }

      /** Return the key with  the range flag removed.
       * If the key is a range key return the key without the range flag otherwise return the
       * the same key.
       */
      static constexpr T makeBaseKey(T key) { return key & (~(RANGE_FLAG_MASK|TYPE_MASK)); }

      /** Return a key pair marking the beginning and the end of the range for the given mask and key
       * @param key a key which marks a point in the range
       * @param mask a mask which defines the range
       * @preturn a pair containing the start key and end key of the range
       */
      static constexpr std::pair<T, T> makeRangeForMask( T key, T mask) {
         return std::make_pair( key & mask, (key | ((~mask) & (CHIP_MASK|COL_MASK|ROW_MASK))) );
      }

      /** Convenience method to check whether the key matches the defect.
       * @param defect_key the key of the defect returned by lower_bound of the emulated defects.
       * @param key the key to test
       * @return true if key overlaps with the defect range or defect.
       */
      static constexpr bool isMatchingDefect(T defect_key, T key) {
         return  (key == makeBaseKey(defect_key) || isRangeKey(defect_key));
      }

   };

}
#endif
