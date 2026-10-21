// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

// Local include(s).
#include "xAODTypes.h"

// Project include(s).
#include "xAODCore/AuxStoreAccessorMacros.h"

namespace AthDeviceUtilsTest {

AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(SimplexAODObject, int, intVar, setIntVar)
AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(SimplexAODObject, float, floatVar,
                                     setFloatVar)

}  // namespace AthDeviceUtilsTest
