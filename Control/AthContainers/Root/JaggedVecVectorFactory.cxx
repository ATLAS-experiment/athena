#include "AthContainers/tools/JaggedVecVectorFactory.h"
#include <cstring>


namespace SG::JaggedVecVectorFactoryFuncs {


/**
 * @brief Fill in trailing zeros in a range.
 * @param elts Pointer to the vector on which to operate.
 * @param index Index of the last element to fill.
 * @param sz Size of the vector.
 * @param lv Corresponding linked vector.
 *
 * Fill fill in trailing zeros working backwards from @c index.
 */
void fillTrailingZeros (JaggedVecEltBase* elts,
                        size_t index,
                        size_t sz,
                        IAuxTypeVector* lv)
{
  if (index > 0 && elts[index-1].end() == 0 && elts[sz-1].end() == 0) {
    if (size_t n_payload = lv->getDataSpan().size) {
      Shift shift (n_payload);
      for (--index; elts[index].end() == 0; --index) {
        shift (elts[index]);
        if (index == 0) break;
      }
    }
  }
}


/**
 * @brief Copy elements between vectors: out-of-line portion.
 * @param auxid The aux data item being operated on.
 * @param dst Container for the destination vector.
 * @param dst_index Index of the first destination element in the vector.
 * @param src Container for the source vector.
 * @param src_index Index of the first source element in the vector.
 * @param n Number of elements to copy.
 * @param for_output If true, then need to use copyForOutput on the
 *        payload, to update links due to thinning.
 *
 * @c dst and @ src can be either the same or different.
 */
void copyImpl (SG::auxid_t auxid,
               AuxVectorData& dst,
               size_t dst_index,
               const AuxVectorData& src,
               size_t src_index,
               size_t n,
               bool for_output)
{
  using vector_value_type = JaggedVecEltBase;

  if (n == 0) return;

  // Check for overlaps.
  if (&src == &dst) {
    size_t src_end = src_index + n;
    size_t dst_end = dst_index + n;
    if ((src_end > dst_index && src_end <= dst_end) ||
        (dst_end > src_index && dst_end <= src_end))
    {
      // Overlapping copies not implemented for jagged vectors.
      // Talk to core software if this is an issue.
      throwJaggedVecOverlappingCopy();
    }
  }

  vector_value_type* v_dst = reinterpret_cast<vector_value_type*> (dst.getDataArray (auxid));
  const vector_value_type* v_src = &dst==&src ? v_dst : reinterpret_cast<const vector_value_type*> (src.getDataArray (auxid));
  IAuxTypeVector* dst_lv = dst.getStore()->linkedVector (auxid);
  const SG::auxid_t payload_auxid = dst_lv->auxid();
  const AuxTypeRegistry& r = AuxTypeRegistry::instance();
  const IAuxTypeVectorFactory* payload_fac = r.getFactory (payload_auxid);
  const size_t dst_sz = dst.size_v();

  // Make sure there are no trailing zeros in the destination within the
  fillTrailingZeros (v_dst, dst_index+n, dst_sz, dst_lv);

  const size_t dst_first = v_dst[dst_index].begin(dst_index);
  size_t src_first = v_src[src_index].begin(src_index);
  size_t src_first_orig = src_first;
  const size_t n_dst = v_dst[dst_index+n-1].end() - dst_first;
  size_t n_src = v_src[src_index+n-1].end() - src_first;

  // First copy the Elt elements.  We'll fix up the indices below.
  std::copy_n (v_src+src_index, n, v_dst+dst_index);

  // If the source had trailing zeros, fix them up here.
  if (v_dst[dst_index+n-1].end() == 0) {
    const size_t src_sz = src.size_v();
    if (v_src[src_sz-1].end() == 0) {
      const IAuxTypeVector* src_lv = src.getConstStore()->linkedVector (auxid);
      if (size_t src_n_payload = src_lv->getDataSpan().size) {

        bool adjust = true;
        for (size_t i = src_index+n; i < src_sz; ++i) {
          if (v_src[i].end() != 0) {
            adjust = false;
            break;
          }
        }
        if (adjust) {
          Shift shift (src_n_payload);
          for (size_t i = dst_index+n-1; v_dst[i].end() == 0; --i) {
            shift (v_dst[i]);
            if (i == dst_index) break;
          }
          if (src_first == 0 && src_index > 0) {
            src_first = v_src[src_index-1].end();
            src_first_orig = src_first;
          }
          n_src = src_n_payload - src_first;
        }
      }
    }
  }

  // Adjust the size of the destination payload container.
  if (n_src != n_dst) {
    if (!dst_lv->shift (dst_first+n_dst, n_src - n_dst)) {
      dst.clearCache (payload_auxid);
    }
    if (&dst == &src && src_first > dst_first) {
      src_first += (n_src - n_dst);
    }
  }

  // Copy the payload elements.
  if (for_output) {
    payload_fac->copyForOutput (payload_auxid, dst, dst_first, src, src_first, n_src);
  }
  else {
    payload_fac->copy (payload_auxid, dst, dst_first, src, src_first, n_src);
  }

  // Fix up the Elt entries --- first the ones we copied, then the following
  // ones in the destination container.
  std::for_each_n (v_dst+dst_index, n, Shift (dst_first - src_first_orig));
  // Avoid N^2.
  if (dst_index+n < dst_sz && !(v_dst[dst_index+n].end() == 0 && v_dst[dst_sz-1].end() == 0))
  {
    std::for_each (v_dst+dst_index+n, v_dst+dst_sz, Shift (n_src - n_dst));
  }
}


/**
 * @brief Swap elements between vectors.
 * @param auxid The aux data item being operated on.
 * @param a Container for the first vector.
 * @param aindex Index of the first element in the first vector.
 * @param b Container for the second vector.
 * @param bindex Index of the first element in the second vector.
 * @param n Number of elements to swap.
 *
 * @c a and @ b can be either the same or different.
 * However, the ranges should not overlap.
 */
void swap (SG::auxid_t auxid,
           AuxVectorData& a, size_t aindex,
           AuxVectorData& b, size_t bindex,
           size_t n)
{
  using vector_value_type = JaggedVecEltBase;

  if (n == 0) return;

  vector_value_type* v_a = reinterpret_cast<vector_value_type*> (a.getDataArray (auxid));
  vector_value_type* v_b = &a==&b ? v_a : reinterpret_cast<vector_value_type*> (b.getDataArray (auxid));
  IAuxTypeVector* alv = a.getStore()->linkedVector (auxid);
  IAuxTypeVector* blv = &a==&b ? alv : b.getStore()->linkedVector (auxid);
  const SG::auxid_t payload_auxid = alv->auxid();
  const AuxTypeRegistry& r = AuxTypeRegistry::instance();
  const IAuxTypeVectorFactory* payload_fac = r.getFactory (payload_auxid);
  const size_t a_sz = a.size_v();
  const size_t b_sz = b.size_v();

  // Make sure there are no trailing zeros in the ranges we're looking at.
  fillTrailingZeros (v_a, a_sz, a_sz, alv);
  fillTrailingZeros (v_b, b_sz, b_sz, blv);

  size_t a_first = v_a[aindex].begin(aindex);
  size_t b_first = v_b[bindex].begin(bindex);
  const size_t a_first_orig = a_first;
  const size_t b_first_orig = b_first;
  const size_t n_a = v_a[aindex+n-1].end() - a_first;
  const size_t n_b = v_b[bindex+n-1].end() - b_first;

  // First swap the Elt entries.  We'll fix up the indices below.
  std::swap_ranges (v_a+aindex, v_a+aindex+n, v_b+bindex);

  // We have two payload ranges to swap, a and b.
  // One is probably longer than the other, so we'll need to adjust the
  // payload vector lengths.
  // But first, swap the common part of the payloads.
  const size_t n_common = std::min (n_a, n_b);
  payload_fac->swap (payload_auxid, a, a_first, b, b_first, n_common);

  // Now we move the tail; that is, the piece of one payload range
  // that is not in the other.  Define a function to reduce duplicate
  // code.  Here, 1 is the range that is longer and 2 the range
  // that is shorter.
  auto shiftTail = [payload_fac, n_common, payload_auxid]
    (AuxVectorData& vd1,
     IAuxTypeVector* lv1,
     size_t& first1,
     const size_t n1,
     AuxVectorData& vd2,
     IAuxTypeVector* lv2,
     size_t& first2)
  {
    // Lengthen the shorter payload to receive the extra elements from
    // the longer one.
    if (!lv2->shift (first2 + n_common, n1 - n_common)) {
      vd2.clearCache (payload_auxid);
    }
    if (&vd1 == &vd2 && first1 > first2) {
      // Special case for self-swapping: keep indices consistent.
      first1 += (n1 - n_common);
    }
    // Copy the extra elements.
    payload_fac->copy (payload_auxid,
                       vd2, first2 + n_common, vd1, first1 + n_common,
                       n1 - n_common);
    // Now remove those elements from the longer one.
    lv1->shift (first1 + n1, - (n1 - n_common));
    if (&vd1 == &vd2 && first2 > first1) {
      // Special case for self-swapping: keep indices consistent.
      first2 -= (n1 - n_common);
    }
  };

  // Now move the tail, depending on which is larger.
  if (n_a > n_b) {
    shiftTail (a, alv, a_first, n_a, b, blv, b_first);
  }
  else if (n_b > n_a) {
    shiftTail (b, blv, b_first, n_b, a, alv, a_first);
  }

  // Now adjust the indices in the Elt ranges that were swapped.
  // In the case of self-swapping, the _first indices may have changed,
  // so need to remember to use the original values.
  std::for_each_n (v_a+aindex, n, Shift (a_first - b_first_orig));
  std::for_each_n (v_b+bindex, n, Shift (b_first - a_first_orig));

  if (n_a != n_b) {
    if (&a == &b) {
      // Self-swapping case.
      // Adjust the indices between the two ranges that were swapped
      // (but we needn't do anything if the number of payload items
      // was the same).
      if (aindex < bindex) {
        std::for_each (v_a+aindex+n, v_a+bindex, Shift (n_b - n_a));
      }
      else {
        std::for_each (v_a+bindex+n, v_a+aindex, Shift (n_a - n_b));
      }
    }
    else {
      // Distinct container case.  Adjust indices after the swapped ranges.
      std::for_each (v_a+aindex+n, v_a+a_sz, Shift (n_b - n_a));
      std::for_each (v_b+bindex+n, v_b+b_sz, Shift (n_a - n_b));
    }
  }
}


/**
 * @brief Clear a range of elements within a vector.
 * @param v Pointer to the vector being operated on.
 * @param auxid The aux data item being operated on.
 * @param dst Container holding the element
 * @param dst_index Index of the first element in the vector.
 * @param n Number of elements to clear.
 */
void clear (JaggedVecEltBase* v,
            auxid_t auxid,
            AuxVectorData& dst,
            size_t dst_index,
            size_t n)
{
  const size_t dst_sz = dst.size_v();
  IAuxTypeVector* lv = dst.getStore()->linkedVector (auxid);

  // Make sure there are no trailing zeros in the destination.
  fillTrailingZeros (v, dst_sz, dst_sz, lv);

  size_t begin = v[dst_index].begin(dst_index);
  size_t end = v[dst_index+n-1].end();

  size_t n_payload = end - begin;

  // Erase the payload elements.
  lv->shift (end, - n_payload);

  // Adjust indices for following elements.
  std::for_each (v+dst_index+n, v+dst_sz, Shift (-n_payload));
  // Clear out the given range.
  std::fill_n (v+dst_index, n, JaggedVecEltBase(begin));
}


} // namespace SG::JaggedVecVectorFactoryFuncs
