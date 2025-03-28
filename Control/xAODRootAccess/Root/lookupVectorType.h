// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

// Forward declaration(s).
class TClass;

namespace xAOD::details {

/// @brief Internal function used by @c xAOD::TAuxStore and @c xAOD::RAuxStore
///
/// It looks for a typedef called @c vector_type on the type given to it.
///
/// @param cl The type that should be searched for the typedefed type
/// @return The dictionary for the vector type, if it exists
///
TClass* lookupVectorType(TClass& cl);

}  // namespace xAOD::details
