/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file CxxUtils/sgkey_utilities.h
 * @author Frank Winklmeier
 * @date Sep, 2025
 * @brief Additional utilities and types related to sgkey_t.
 *
 * To avoid dependency bloat of sgkey_t.h.
 */

#ifndef CXXUTILS_SGKEY_UTILITIES_H
#define CXXUTILS_SGKEY_UTILITIES_H

#include "CxxUtils/sgkey_t.h"
#include "CxxUtils/ConcurrentMap.h"
#include "CxxUtils/SimpleUpdater.h"


namespace SG {

  // Separating this avoids a cppcheck syntax error.
  static constexpr auto s_sgkey_nullval =
    static_cast<CxxUtils::detail::ConcurrentHashmapVal_t> (-1);

  /// A concurrent map using sgkey_t as key.
  template <class T>
  using ConcurrentSGKeyMap = CxxUtils::ConcurrentMap<
    sgkey_t, T,
    CxxUtils::SimpleUpdater,
    SGKeyHash, SGKeyEqual,
    0, s_sgkey_nullval>;

} // namespace SG


#endif
