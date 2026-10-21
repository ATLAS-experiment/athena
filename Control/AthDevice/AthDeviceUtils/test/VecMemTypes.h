// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#ifndef ATHDEVICEUTILS_VECMEMTYPES_H
#define ATHDEVICEUTILS_VECMEMTYPES_H

// VecMem include(s).
#include <vecmem/edm/container.hpp>

namespace AthDeviceUtilsTest {

/// @name VecMem type(s) for a container without any jagged vectors
/// @{

template <typename BASE>
struct SimpleVecMemObject : public BASE {
  using BASE::BASE;
  using BASE::operator=;
};

using SimpleVecMemObjectContainer =
    vecmem::edm::container<SimpleVecMemObject, vecmem::edm::type::vector<int>,
                           vecmem::edm::type::vector<float>>;

/// @}

}  // namespace AthDeviceUtilsTest

#endif  // ATHDEVICEUTILS_VECMEMTYPES_H
