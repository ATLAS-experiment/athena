// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

// Framework include(s).
#include "AthContainersInterfaces/AuxTypes.h"

namespace xAOD::details {

/// Check if the auxiliary variable has a registered type
///
/// This function is used to test if a given auxiliary variable is known
/// in the registry with a proper type.
///
/// @returns @c true if the full type of the auxiliary property
///          is known, @c false otherwise
///
bool isRegisteredType(SG::auxid_t auxid);

}  // namespace xAOD::details
