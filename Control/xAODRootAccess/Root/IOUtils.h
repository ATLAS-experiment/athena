// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#ifndef XAODROOTACCESS_IOUTILS_H
#define XAODROOTACCESS_IOUTILS_H

// Framework include(s).
#include "AthContainers/AuxVectorBase.h"
#include "CxxUtils/no_sanitize_undefined.h"

// ROOT include(s).
#include <TClass.h>

namespace xAOD::Details {

/// Helper function for calling @c SG::AuxVectorBase::initAuxVectorBase
void forceTrackIndices NO_SANITIZE_UNDEFINED(SG::AuxVectorBase& vec);

/// Helper function deciding if a given type "has an auxiliary store"
///
/// @param cl The dictionary for the type being interrogated
/// @returns @c true if the type has an auxiliary store, @c false otherwise
///
bool hasAuxStore(const TClass& cl);

/// Helper function deciding if a given type "is an auxiliary store"
///
/// @param cl The dictionary for the type being interrogated
/// @returns @c true if the type is an auxiliary store, @c false otherwise
///
bool isAuxStore(const TClass& cl);

/// Helper function deciding if a given type "is a standalone object"
///
/// @param cl The dictionary for the type being interrogated
/// @returns @c true if the type is a standalone object, @c false otherwise
///
bool isStandalone(const TClass& cl);

}  // namespace xAOD::Details

#endif  // XAODROOTACCESS_IOUTILS_H
