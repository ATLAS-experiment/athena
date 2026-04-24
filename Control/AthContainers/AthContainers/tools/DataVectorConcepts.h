/*
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 */

#ifndef ATHCONTAINERS_DATAVECTORCONCEPTS_H
#define ATHCONTAINERS_DATAVECTORCONCEPTS_H

// Local include(s).
#include "AthContainers/AuxElement.h"
#include "AthContainers/DataVector.h"

// System include(s).
#include <concepts>

namespace SG {

/// Concept for "xAOD style" DataVector interface containers.
template <class T>
concept IsAuxDataVector =
    std::derived_from<T, DataVector<typename T::base_value_type>> &&
    std::derived_from<typename T::base_value_type, AuxElement>;

}  // namespace SG

#endif  // not ATHCONTAINERS_DATAVECTORCONCEPTS_H
