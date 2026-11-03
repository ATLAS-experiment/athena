// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#ifndef ATHDEVICEUTILS_DETAILS_GETSIZES_H
#define ATHDEVICEUTILS_DETAILS_GETSIZES_H

// Local include(s).
#include "AthDeviceUtils/SoAColumn.h"

// Project include(s).
#include "AthContainers/AuxVectorBase.h"

// VecMem include(s).
#include <vecmem/edm/schema.hpp>

// System include(s).
#include <tuple>
#include <vector>

namespace AthDevice::Details {

/// Get "the sizes" that we can pass to a vecmem buffer object
///
/// For an xAOD container that has one or more jagged vector(s).
///
template <typename SCHEMA, Concepts::SoAColumn... COLUMNS>
std::vector<std::size_t> getSizes(const SG::AuxVectorBase& input,
                                  const std::tuple<COLUMNS...>& columns);

}  // namespace AthDevice::Details

// Include the implementation.
#include "AthDeviceUtils/impl/getSizes.icc"

#endif  // ATHDEVICEUTILS_DETAILS_GETSIZES_H
