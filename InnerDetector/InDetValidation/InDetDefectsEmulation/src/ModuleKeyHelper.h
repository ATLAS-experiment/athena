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
    * @tparam T_MASK_SEL_BITS number of bits to store the mask index of a group defect.
    * @tparam T_N_MASKS total number of masks supported by this helper; must be representable by the
    *         number of @T_MASK_SEL_BITS
    *
    * The key assumes a hierarchical ordering of the indices where the chip index ranks highest and the
    * row index lowest. The mask is applied to the final key and can be used to represent a defect of
    * adjacent pixels by a single key, provided it is possible to compute the first pixel of such a
    * group from a key which addresses a single pixel by a simple mask i.e. single-pixel-key
    * bit-wise-and mask = group-key where the group-key is the single pixel key of the first pixel in
    * that group.
    */
   template <typename T, unsigned  int T_ROW_BITS, unsigned int T_COL_BITS, unsigned int T_CHIP_BITS, unsigned int T_MASK_SEL_BITS, unsigned int T_N_MASKS=3>
   struct ModuleKeyHelper {
      static constexpr unsigned int ROW_BITS = T_ROW_BITS;
      static constexpr unsigned int COL_BITS = T_COL_BITS;
      static constexpr unsigned int CHIP_BITS = T_CHIP_BITS;
      static constexpr unsigned int MASK_SEL_BITS = T_MASK_SEL_BITS;
      static constexpr unsigned int N_MASKS = T_N_MASKS;
      static constexpr T ROW_SHIFT   = 0u;
      static constexpr T COL_SHIFT   = ROW_BITS;
      static constexpr T CHIP_SHIFT  = ROW_BITS + COL_BITS;
      static constexpr T MASK_SEL_SHIFT = ROW_BITS + COL_BITS + CHIP_BITS;
      static constexpr T ROW_MASK    = MaskUtils::createMask<0,                          ROW_BITS>();
      static constexpr T COL_MASK    = MaskUtils::createMask<ROW_BITS,                   ROW_BITS+COL_BITS>();
      static constexpr T CHIP_MASK   = MaskUtils::createMask<ROW_BITS+COL_BITS,          ROW_BITS+COL_BITS+CHIP_BITS>();
      static constexpr T MASK_SEL_MASK  = MaskUtils::createMask<ROW_BITS+COL_BITS+CHIP_BITS,ROW_BITS+COL_BITS+CHIP_BITS+MASK_SEL_BITS>();
      static constexpr unsigned int MASKS_SIZE=N_MASKS;
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

      /** Create a key from mask, chip, column and row indices.
       * @param mask_sel the index of a mask starting from zero
       * @param chip the index of a chip starting from zero
       * @param col the index of a column starting from zero
       * @param row the index of a row starting from zero
       *
       * The indices must be representable by the number of reserved bits.
       */
      static constexpr T makeKey(unsigned int mask_sel, unsigned int chip, unsigned int col, unsigned int row=0u) {
         return   makeKeyPart<MASK_SEL_SHIFT,MASK_SEL_MASK>(mask_sel)
            | makeKeyPart<CHIP_SHIFT,CHIP_MASK>(chip)
            | makeKeyPart<COL_SHIFT,COL_MASK>(col)
            | makeKeyPart<ROW_SHIFT,ROW_MASK>(row);
      }
   public:

      /** Get the column index from a full key.
       */
      static constexpr T getColumn(T key) { return (key & COL_MASK)   >> COL_SHIFT; }

      /** Get the row index from a full key.
       */
      static constexpr T getRow(T key)    { return (key & ROW_MASK)   >> ROW_SHIFT; }

      /** Get the column index from a full key.
       */
      static constexpr T getChip(T key)   { return (key & CHIP_MASK)  >> CHIP_SHIFT; }

      /** Get the mask index from a full key.
       */
      static constexpr T getMaskIdx(T key)  { return (key & MASK_SEL_MASK) >> MASK_SEL_SHIFT; }

      /** Get the number of possible masks.
       */
      static constexpr unsigned int nMasks() { return std::max(1u,N_MASKS);}

      /** Construct this key helper.
       * @param masks the possible masks available for this key helper.
       *
       * The first mask should have all bits set which specify the row, column and chip index i.e.
       * a mask for keys addressing individual cells e.g. pixel or strip.
       * The other masks may have the lowest n-bits set to zero. Masks are expected to be in
       * ascending order i.e. the number of trailing zero value bits is increasing.
       */
      ModuleKeyHelper(std::array<T, N_MASKS> &&masks) requires (N_MASKS>0) : m_masks( masks) {}
      ModuleKeyHelper() = default;

      /** Get the mask specified by the full key.
       * @param key a full defect key.
       *
       * The mask index must be smaller than the total number of masks available in this helper. The
       * result will be undefined otherwise.
       */
      T getMask(T key) const {
         if constexpr(N_MASKS>0) {
            unsigned int idx;
            if constexpr(N_MASKS==1) {
               idx=0u;
            }
            else {
                  idx = getMaskIdx(key);
            }
            assert( idx < m_masks.size());
            return m_masks[idx];
         }
         else {
            return static_cast<T>(1u);
         }
      }

      /** Create a key for a group defect.
       * @param mask_idx the index of mask associated to this group defect
       * @param chip the chip index of one cell (e.g. pixel or strip) of this group defect
       * @param column the column index of one cell (e.g. pixel or strip) of this group defect
       * @param row the row index of one cell (e.g. pixel or strip) of this group defect
       *
       * The resulting key will be the key of the first cell (e.g. pixel or strip) of this group
       * defect, and the mask index will be set accordingly.
       */
      unsigned int maskedKey([[maybe_unused]] unsigned int mask_idx, unsigned int chip, unsigned int col, unsigned int row=0u) const {
         if constexpr(N_MASKS>0) {
            assert( mask_idx < m_masks.size());
            return (m_masks[mask_idx] & makeKey(0u, chip, col, row)) | makeKey(mask_idx, 0u,0u,0u);
         }
         else {
            return makeKey(0u, chip, col, row);
         }
      }

      /** Turn a single cell (e.g. pixel or strip) defect key into a group defect key.
       * @param mask_idx the index of mask associated to this group defect
       * @param key single pixel key
       *
       * The resulting key will be the key of the first pixel of this group
       * defect, and the mask index will be set accordingly.
       */
      unsigned int maskedKey(unsigned int mask_idx, unsigned int key) const {
         if constexpr(N_MASKS>0) {
            assert( mask_idx < m_masks.size());
            return (m_masks[mask_idx] & key) | makeKey(mask_idx, 0u,0u,0u);
         }
         else {
            return key;
         }
      }


      /** Test whether a single cell (e.g. pixel or strip) key is compatible with a defect key
       * @param key_ref a defect key
       * @param key_test a single cell (e.g. pixel or strip) key.
       * The defect key can be either the key of a single cell (e.g. pixel or strip) or the first cell
       * (e.g. pixel or strip) of a group, which is fully identified by this key and the mask also specified
       * by the key and which can be obtained by @ref getMask.
       */
      bool isOverlapping(T key_ref, T key_test) const {
         if constexpr(N_MASKS>0) {
            unsigned int mask = getMask(key_ref);
            return (key_ref & mask) == (key_test & mask);
         }
         else {
            return key_ref == key_test;
         }
      }

   protected:
      struct Empty {};
      std::conditional< (N_MASKS>0), std::array<T, MASKS_SIZE>, Empty>::type m_masks; ///< the masks for this helper.
   };

}
#endif
