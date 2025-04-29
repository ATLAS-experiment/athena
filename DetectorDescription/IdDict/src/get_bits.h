/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
 */

#ifndef IDDICT_get_bits_H
#define IDDICT_get_bits_H

#include <string>
#include <vector>

class IdDictRegion;

namespace IdDict {
  using RV = std::vector <IdDictRegion*>;

  /**
   * Compute the OR of all fields at <level>, for the
   * subset of overlapping regions
   **/
  void
  compute_bits(const RV& regions, size_t level, const std::string& group);


  // This function is recursively called for each level to get the
  // number of bits. By "definition" we require regions in the same
  // group to overlap to have a uniform bit-allocation for a group.
  //
  // For each call, the first region within the specified "group"
  // which has a (non-empty) field value at the current level is
  // used as reference. According to this reference, the regions are
  // divided into two sets: a set overlapping with the reference and
  // a non-overlapping set. For the overlapping set, get_bits is
  // called again, but for the next level. When control comes back,
  // then a reference is chosen from the non-overlapping set and the
  // procedure continues.
  //
  // The calculation of bits needed for a particular level is done
  // in compute_bits before the overlapping is check for this level.
  //
  void
  get_bits(const RV& regions, size_t level, const std::string& group);
}
#endif
