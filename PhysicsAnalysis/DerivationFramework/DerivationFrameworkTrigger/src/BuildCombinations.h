/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORKTRIGGER_BUILDCOMBINATIONS_H
#define DERIVATIONFRAMEWORKTRIGGER_BUILDCOMBINATIONS_H

#include "RangedItr.h"

namespace DerivationFramework { namespace TriggerMatchingUtils {

  /**
   * @brief Helper function for inserting an element into a sorted vector
   * @tparam T The type stored in the vector
   * @param[out] vec The vector to insert into
   * @param ele The element to insert
   * @param proj Projection operation.
   * @return Whether or not the element was inserted.
   * The element will not be inserted if it already exists.
   *
   * If the optional projection operation is given, then the sort keys
   * are found by applying proj() to the vector elements.
   */
  template <typename T, typename PROJ = std::identity >
    bool insertIntoSortedVector(std::vector<T>& vec, const T& ele,
                                PROJ proj = {});

  /**
   * @brief Helper function to create a sorted vector from an unsorted range.
   * @tparam R The range type.
   * @param proj Projection operation.
   *
   * If the optional projection operation is given, then the sort keys
   * are found by applying proj() to the vector elements.
   */
  template <typename R, typename PROJ = std::identity>
    std::vector<typename R::value_type> sorted(const R& r, PROJ proj = {});

  /**
   * @brief Get all valid, unique combinations of distinct elements from the
   * input ranges.
   * @param inputs The ranges over vectors of possible elements.
   *
   * The optional projection operations can be used to customize the
   * sort keys used.  innerproj is applied to elements of the inner vectors
   * (that is, type T).  outerproj is applied to elements of the outer
   * vector (that is, inner vectors, of type std::vector<T>).
   */
  // First, a note on the type of the argument. This is essentially a way of
  // passing a vector of vectors but only with iterators over the vectors.
  template <typename T,
            typename INNERPROJ = std::identity,
            typename OUTERPROJ = std::identity>
    std::vector<std::vector<T>> getAllDistinctCombinations(
        std::vector<RangedItr<typename std::vector<T>::const_iterator>>& inputs,
        INNERPROJ innerproj = {},
        OUTERPROJ outerproj = {});

}} //> end namespace DerivationFramework::TriggerMatchingUtils
#include "BuildCombinations.icc"
#endif //> !DERIVATIONFRAMEWORKTRIGGER_BUILDCOMBINATIONS_H
