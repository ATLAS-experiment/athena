// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#ifndef ATHDEVICEUTILS_DETAILS_GETDATA_H
#define ATHDEVICEUTILS_DETAILS_GETDATA_H

// Project include(s).
#include "CxxUtils/span.h"

// VecMem include(s).
#include <vecmem/containers/data/jagged_vector_data.hpp>
#include <vecmem/containers/data/vector_view.hpp>

namespace AthDevice::Details {

/// Construct @c vecmem::data::vector_view from @c CxxUtils::span
template <typename T>
vecmem::data::vector_view<T> getData(CxxUtils::span<T> data);

/// Construct @c vecmem::data::jagged_vector_data from @c CxxUtils::span
template <typename T, typename ALLOCATOR>
vecmem::data::jagged_vector_data<const T> getData(
    CxxUtils::span<const std::vector<T, ALLOCATOR>> data);

/// Construct @c vecmem::data::jagged_vector_data from @c CxxUtils::span
template <typename T, typename ALLOCATOR>
vecmem::data::jagged_vector_data<T> getData(
    CxxUtils::span<std::vector<T, ALLOCATOR>> data);

}  // namespace AthDevice::Details

// Include the implementation.
#include "AthDeviceUtils/impl/getData.icc"

#endif  // ATHDEVICEUTILS_DETAILS_GETDATA_H
