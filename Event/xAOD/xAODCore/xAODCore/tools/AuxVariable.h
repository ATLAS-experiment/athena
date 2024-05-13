// Dear emacs, this is -*- c++ -*-
//
// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
//
#ifndef XAODCORE_TOOLS_AUXVARIABLE_H
#define XAODCORE_TOOLS_AUXVARIABLE_H

// EDM include(s).
#include "AthContainers/AuxTypeRegistry.h"
#include "CxxUtils/pputils.h"

/// Convenience macro for declaring an auxiliary variable
///
/// Should be used in the constructor of the derived class, like:
///   <code>
///      AUX_VARIABLE( RunNumber );
///   </code>
///
/// Can take a optional additional argument giving SG::AuxVarFlags
/// to use for this variable.
///
#define AUX_VARIABLE( VAR, ... )                              \
   do {                                                       \
      static const auxid_t auxid =                            \
      getAuxID( #VAR, VAR CXXUTILS_PP_FIRST( __VA_ARGS__ ) ); \
      regAuxVar( auxid, #VAR, VAR );                          \
   } while( false )



/// Alternatively, an auxiliary variable may be declared using the AUXVAR_DECL
/// macro in the class definition.  In this case, no explicit code
/// is required in the constructor.  It takes two arguments: the type
/// of the variable and the variable name.  Example:
///   <code>
///      AUXVAR_DECL( int32_t, RunNumber );
///   </code>
///
/// If AUXVAR_DECL is used with type T, the actual type of the member will
/// be given by the alias AuxVariable_t<T>.  For AuxInfoBase, this will
/// be T, and for AuxContainerBase std::vector<T>.  It may, however,
/// be overridden by derived classes to customize the container type used.
///
/// AuxVariable_t should actually take a second, defaultable, template
/// argument giving the allocator type to use for the container.
/// This may be provided as an optional third argument to AUXVAR_DECL.
/// AUXVAR_DECL may also take an optional fourth argument giving SG::AuxVarFlags
/// to use for the variable.
///
/// If an auxiliary variable declared with AUXVAR_DECL can be initialized
/// from a std::pmr::memory_resource*, then it will be initialized
/// with the resource associated with the AuxContainerBase/AuxInfoBase instance.
/// 
namespace xAOD { namespace detail {


template <class VARTYPE, class AUXBASE>
VARTYPE initAuxVar2( AUXBASE* auxcont,
                     std::true_type)
{
  return VARTYPE (auxcont->memResource());
}


template <class VARTYPE, class AUXBASE>
VARTYPE initAuxVar2( AUXBASE* /*auxcont*/,
                     std::false_type)
{
  return VARTYPE();
}


template <class VARTYPE, class AUXBASE>
VARTYPE initAuxVar1( AUXBASE* auxcont)
{
  return initAuxVar2<VARTYPE> ( auxcont, std::is_constructible<VARTYPE, std::pmr::memory_resource*>() );
}


template <const char* NAME, class AUXBASE, class VARTYPE>
VARTYPE initAuxVar( AUXBASE* auxcont,
                    VARTYPE& var,
                    SG::AuxVarFlags flags = SG::AuxVarFlags::None )
{
  static const SG::auxid_t auxid = auxcont->getAuxID( NAME, var, flags );
  auxcont->regAuxVar (auxid, NAME, var);
  return initAuxVar1<VARTYPE>( auxcont );
}


}} // namespace xAOD::detail


#define AUXVAR_DECL( TYPE, NAME, ... )                                  \
  static constexpr const char NAME ## __name[] = #NAME;                 \
  AuxVariable_t< TYPE AUXVAR_DECL_ALLOC_( TYPE, __VA_ARGS__ ) > NAME     \
    { xAOD::detail::initAuxVar< NAME ## __name >( this, NAME CXXUTILS_PP_SECOND( __VA_ARGS__ ) ) }
#define AUXVAR_DECL_ALLOC_( TYPE, ... ) \
  __VA_OPT__( CXXUTILS_PP_FIRST( __VA_ARGS__ )< TYPE > )


#define AUXVAR_PACKEDCONTAINER_DECL( TYPE, NAME )           \
  static constexpr const char NAME ## __name[] = #NAME; \
  SG::PackedContainer< TYPE > NAME  { xAOD::detail::initAuxVar< NAME ## __name >( this, NAME ) }


#endif // XAODCORE_TOOLS_AUXVARIABLE_H
