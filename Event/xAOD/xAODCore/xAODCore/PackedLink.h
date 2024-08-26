// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file xAODCore/PackedLink.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Jan, 2024
 * @brief Auxiliary variable type allowing to store @c ElementLinks
 *        as packed 32-bit values.
 *
 * See AthContainers/PackedLink.h for full documentation.
 */


#ifndef XAODCORE_PACKEDLINK_H
#define XAODCORE_PACKEDLINK_H


#include "AthContainers/PackedLink.h"
#include "xAODCore/tools/AuxVariable.h"
#include "xAODCore/tools/PackedLinkPersVector.h"


// Macro for declaring a packed link variable.  This should be used
// in the definition for the container class.  For example, this:
//   <code>
//     AUXVAR_PACKEDLINK_DECL( DataVector<Foo>, fooLink );
//   </code>
// will declare a PackedLink<DataVector<Foo> > variable named fooLink,
// along with the the corresponding linked variable.
// An allocator type may be supplied as an optional third template argument.
#define AUXVAR_PACKEDLINK_DECL( CONT, NAME, ... )                       \
  static constexpr const char NAME ## _internal_name[] = #NAME;          \
  AuxVariable_t<SG::PackedLink<CONT> AUXVAR_DECL_ALLOC_( SG::PackedLink<CONT>, __VA_ARGS__ )> NAME \
    { xAOD::detail::initAuxVar1<AuxVariable_t<SG::PackedLink<CONT> AUXVAR_DECL_ALLOC_( SG::PackedLink<CONT>, __VA_ARGS__ ) > > (this) }; \
  LinkedVariable_t<DataLink<CONT> AUXVAR_DECL_ALLOC_( DataLink<CONT>, __VA_ARGS__ )> NAME ## _linked { \
    xAOD::detail::initLinkedVar<SG::PackedLink<CONT>, NAME ## _internal_name> (this, NAME, NAME ## _linked) }


// Macro for declaring a vector of packed link variables.  This should be used
// in the definition for the container class.  For example, this:
//   <code>
//     AUXVAR_PACKEDLINKVEC_DECL( std::vector, DataVector<Foo>, fooLinks );
//   </code>
// will declare a std::vector<PackedLink<DataVector<Foo> > >
// variable named fooLinks, along with the the corresponding linked variable.
// An allocator type may be supplied as an optional fourth template argument.
#define AUXVAR_PACKEDLINKVEC_DECL( VEC, CONT, NAME, ...)                  \
  static constexpr const char NAME ## _internal_name[] = #NAME;                 \
  AuxVariable_t<VEC<SG::PackedLink<CONT> > AUXVAR_DECL_ALLOC_( VEC<SG::PackedLink<CONT> >, __VA_ARGS__ ) > NAME \
    { xAOD::detail::initAuxVar1<AuxVariable_t<VEC<SG::PackedLink<CONT> > AUXVAR_DECL_ALLOC_( VEC<SG::PackedLink<CONT> >, __VA_ARGS__ ) > > (this) }; \
  LinkedVariable_t<DataLink<CONT> AUXVAR_DECL_ALLOC_( DataLink<CONT>, __VA_ARGS__ ) > NAME ## _linked { \
    xAOD::detail::initLinkedVar<VEC<SG::PackedLink<CONT> >, NAME ## _internal_name> (this, NAME, NAME ## _linked) }


#endif // not XAODCORE_PACKEDLINK_H
