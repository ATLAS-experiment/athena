/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
 */

#include "AthenaPoolExampleData/versions/ExampleElectron_v1.h"

#include "xAODCore/AuxStoreAccessorMacros.h"

namespace xAOD {
AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(ExampleElectron_v1, float, pt, setPt)
AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(ExampleElectron_v1, float, charge,
                                     setCharge)
}  // namespace xAOD
