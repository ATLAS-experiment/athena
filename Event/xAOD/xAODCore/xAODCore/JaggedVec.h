// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file xAODCore/JaggedVec.h
 * @author scott snyder <snyder@bnl.gov>
 * @date May, 2024
 * @brief Auxiliary variable type allowing storage as a jagged vector.
 *        That is, the payloads for all the DataVector elements are
 *        stored contiguously in a single vector.
 *
 * See AthContainers/JaggedVec.h for full documentation.
 */


#ifndef XAODCORE_JAGGEDVEC_H
#define XAODCORE_JAGGEDVEC_H

#include "AthContainers/JaggedVec.h"
#include "xAODCore/tools/JaggedVecPersVector.h"


// Macro for declaring a packed link variable.  This should be used
// in the definition for the container class.  For example, this:
//   <code>
//     AUXVAR_JAGGEDVEC_DECL( int, intVec );
//   </code>
// will declare a JaggedVecElt<int> variable named intVec,
// along with the the corresponding linked variable.
// An allocator type may be supplied as an optional third template argument.
#define AUXVAR_JAGGEDVEC_DECL( T, NAME, ... )                           \
  static constexpr const char NAME ## _internal_name[] = #NAME;         \
  AuxVariable_t<SG::JaggedVecElt<T> AUXVAR_DECL_ALLOC_( SG::JaggedVecElt<T> )> NAME \
    { xAOD::detail::initAuxVar1<AuxVariable_t<SG::JaggedVecElt<T> AUXVAR_DECL_ALLOC_( SG::JaggedVecElt<T>, __VA_ARGS__ ) > > (this) }; \
  LinkedVariable_t<T AUXVAR_DECL_ALLOC_( T, __VA_ARGS__ ) > NAME ## _linked { \
    xAOD::detail::initLinkedVar<SG::JaggedVecElt<T>, NAME ## _internal_name> (this, NAME, NAME ## _linked) }


#endif // not XAODCORE_JAGGEDVEC_H
