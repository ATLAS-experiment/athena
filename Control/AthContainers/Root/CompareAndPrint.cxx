/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file  AthContainers/src/CompareAndPrint.cxx
 * @author scott snyder, Paolo Calafiura, etc
 * @date May 2005; rewritten from earlier version.
 * @brief helper class for remove_duplicates
 *        Out-of-line implementations.
 */

#include "AthContainers/tools/CompareAndPrint.h"
#include "AthContainers/tools/error.h"
#include <print>


namespace DataModel_detail {

/**
 * @brief Duplicate elements were found; print a warning.
 * @param ti @c type_info for the element.
 * @param f The element pointer.
 */
void CompareAndPrint::warn (const std::type_info& ti, const void* f) const
{
  std::println ("DataVector ERROR: duplicated pointer found in a DataVector of {} owning its elements! {}",
                AthContainers_detail::typeinfoName(ti), f);
}

} // namespace DataModel_detail
