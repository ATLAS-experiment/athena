// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#ifndef ATHDEVICEUTILS_DETAILS_GETDATA_H
#define ATHDEVICEUTILS_DETAILS_GETDATA_H

// Project include(s).
#include "CxxUtils/span.h"

// VecMem include(s).
#include <vecmem/containers/data/vector_view.hpp>

namespace AthDevice::Details {

/// Construct @c vecmem::data::vector_view from @c CxxUtils::span
template <typename T>
vecmem::data::vector_view<T> getData(CxxUtils::span<T> data);

}  // namespace AthDevice::Details

// Include the implementation.
#include "AthDeviceUtils/impl/getData.icc"

#endif  // ATHDEVICEUTILS_DETAILS_GETDATA_H
