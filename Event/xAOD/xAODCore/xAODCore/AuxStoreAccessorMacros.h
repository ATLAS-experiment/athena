// Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// $Id: AuxStoreAccessorMacros.h 633587 2014-12-04 09:09:41Z ssnyder $
#ifndef XAODCORE_AUXSTOREACCESSORMACROS_H
#define XAODCORE_AUXSTOREACCESSORMACROS_H

/// Macro creating the accessors of primitive auxiliary properties
///
/// The idea with this macro is to save ourselves a bit of time
/// and typing. The macro assumes that for a given auxiliary
/// variable, like a floating point variable of name "sthg", we
/// define the functions:
///
///  <code>
///    float sthg() const;<br/>
///    void setSthg( float value );
///  </code>
///
/// in the xAOD class's header. This macro provides the implementation
/// code for these two functions.
///
/// So, in the previous case, we would use:
///  <code>
///    AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( Dummy, float, sthg, setSthg )
///  </code>
///
/// @param CL The name of the xAOD class
/// @param TYPE The (primitive) type name
/// @param NAME The name of the auxiliary variable
/// @param SETTER The name of the "setter function"
///
#define AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(CL, TYPE, NAME, \
                                             SETTER)         \
   TYPE CL::NAME() const {                                   \
      static const SG::Accessor< TYPE > acc( #NAME );         \
      return acc( *this );                                   \
   }                                                         \
   void CL::SETTER( TYPE value ) {                           \
      static const SG::Accessor< TYPE > acc( #NAME );         \
      acc( *this ) = value;                                  \
      return;                                                \
   }

/// Macro creating the reader function for a primitive auxiliary property
///
/// This macro is a simplified version of the previous one. It just provides
/// the accessor function for reading an auxiliary variable. It can be
/// useful when setting an auxiliary variable is done in a more involved
/// function, but reading it is done through a simple accessor function.
///
/// @param CL The name of the xAOD class
/// @param TYPE The (primitive) type name
/// @param NAME The name of the auxiliary variable
///
#define AUXSTORE_PRIMITIVE_GETTER(CL, TYPE, NAME ) \
   TYPE CL::NAME() const {                         \
      static const SG::Accessor< TYPE > acc( #NAME );    \
      return acc( *this );                         \
   }

/// Macro creating the accessors of complex auxiliary properties
///
/// This macro should be used in the same way as the
/// AUXSTORE_PRIMITIVE_SETTER_AND_GETTER one. However in contrary to that
/// one, it assumes that the type given to it is heavy enough that it should
/// be passed around using constant references instead of by value.
///
/// @param CL The name of the xAOD class
/// @param TYPE The (complex) type name
/// @param NAME The name of the auxiliary variable
/// @param SETTER The name of the "setter function"
///
/// If TYPE is an instance of SG::JaggedVecElt<T>, then accessors will
/// be generated handling jagged vector variables.  In that case, the
/// read accessor will return type SG::ConstAccessor<T>::const_reference_type
/// (which is the same as SG::ConstAccessor<T>::const_reference_type), and
/// the write accessor will take type std::span<const T>.
///
/// If TYPE is an instance of SG::PackedLink<T>, then accessors will
/// be generated handling packed link variables.  In that case, the
/// read accessor will return type ElementLink<T>, and
/// the write accessor will take type const ElementLink<T>&.
///
#define AUXSTORE_OBJECT_SETTER_AND_GETTER(CL, TYPE, NAME, SETTER)           \
   AUXSTORE_OBJECT_SETTER_AND_GETTER_VARNAME(CL, TYPE, NAME, SETTER, NAME)


/// Macro creating the accessors of complex auxiliary properties,
/// allowing specifying the variable name.
///
/// This macro should be used in the same way as the
/// AUXSTORE_PRIMITIVE_SETTER_AND_GETTER one. However in contrary to that
/// one, it assumes that the type given to it is heavy enough that it should
/// be passed around using constant references instead of by value.
///
/// @param CL The name of the xAOD class
/// @param TYPE The (complex) type name
/// @param NAME The name of the auxiliary variable
/// @param SETTER The name of the "setter function"
/// @param VARNAME The auxiliary variable name.
///
/// If TYPE is an instance of SG::JaggedVecElt<T>, then accessors will
/// be generated handling jagged vector variables.  In that case, the
/// read accessor will return type SG::ConstAccessor<T>::const_reference_type
/// (which is the same as SG::ConstAccessor<T>::const_reference_type), and
/// the write accessor will take type std::span<const T>.
///
/// If TYPE is an instance of SG::PackedLink<T>, then accessors will
/// be generated handling packed link variables.  In that case, the
/// read accessor will return type ElementLink<T>, and
/// the write accessor will take type const ElementLink<T>&.
///
#define AUXSTORE_OBJECT_SETTER_AND_GETTER_VARNAME(CL, TYPE, NAME, SETTER, VARNAME) \
   AUXSTORE_OBJECT_GETTER_VARNAME(CL, TYPE, NAME, VARNAME)                  \
   void CL::SETTER( typename SG::Accessor< TYPE >::rhs_const_reference_type value ) { \
      static const SG::Accessor< TYPE > acc( #VARNAME );                    \
      acc( *this ) = value;                                                 \
      return;                                                               \
   }


/// Macro creating a move accessor for complex auxiliary properties.
///
/// This macro should be used in the same way as the
/// AUXSTORE_OBJECT_SETTER_AND_GETTER one.  In C++11, this will create
/// a setter that takes its argument using move semantics.
/// It is a no-op in C++98.
///
/// @param CL The name of the xAOD class
/// @param TYPE The (complex) type name
/// @param NAME The name of the auxiliary variable
/// @param SETTER The name of the "setter function"
///
# define AUXSTORE_OBJECT_MOVE(CL, TYPE, NAME, SETTER)             \
   void CL::SETTER( typename SG::AuxDataTraits<TYPE>::element_type&& value ) { \
      static const SG::Accessor< TYPE > acc( #NAME );             \
      acc( *this ) = std::move(value);                            \
      return;                                                     \
   }


/// Macro creating the reader function for a complex auxiliary property
///
/// Just like AUXSTORE_OBJECT_SETTER_AND_GETTER, this macro also handles
/// an auxiliary property that should be passed around using constant
/// references.
///
/// @param CL The name of the xAOD class
/// @param TYPE The (complex) type name
/// @param NAME The name of the auxiliary variable
///
/// If TYPE is an instance of SG::JaggedVecElt<T>, then an accessor will
/// be generated handling jagged vector variables, returning type
/// SG::ConstAccessor<T>::const_reference_type (which is the same as
/// SG::ConstAccessor<T>::const_reference_type).
///
/// If TYPE is an instance of SG::PackedLink<T>, then an accessor will
/// be generated handling packed link variables, returning type
/// ElementLink<T>.
///
#define AUXSTORE_OBJECT_GETTER(CL, TYPE, NAME )    \
   AUXSTORE_OBJECT_GETTER_VARNAME(CL, TYPE, NAME, NAME)

/// Macro creating the reader function for a complex auxiliary property
/// allowing specifying the variable name.
///
/// Just like AUXSTORE_OBJECT_SETTER_AND_GETTER, this macro also handles
/// an auxiliary property that should be passed around using constant
/// references.
///
/// @param CL The name of the xAOD class
/// @param TYPE The (complex) type name
/// @param NAME The name of the auxiliary variable
/// @param VARNAME The auxiliary variable name.
///
/// If TYPE is an instance of SG::JaggedVecElt<T>, then an accessor will
/// be generated handling jagged vector variables, returning type
/// SG::ConstAccessor<T>::const_reference_type (which is the same as
/// SG::ConstAccessor<T>::const_reference_type).
///
/// If TYPE is an instance of SG::PackedLink<T>, then an accessor will
/// be generated handling packed link variables, returning type
/// ElementLink<T>.
///
#define AUXSTORE_OBJECT_GETTER_VARNAME(CL, TYPE, NAME, VARNAME)             \
   typename SG::ConstAccessor< TYPE >::const_reference_type CL::NAME() const { \
      static const SG::ConstAccessor< TYPE > acc( #VARNAME );               \
      return acc( *this );                                                  \
   }                                                                        \

/// Macro creating a getter function with a type conversion
///
/// In a number of cases we use a different primitive type in the interface
/// classes than what we end up putting into the auxiliary store. In most
/// cases we do double<->float, and enum<->int conversions in these cases.
///
/// This macro can be used to slightly lower the number of lines needed
/// for implementing such a function.
///
/// @param CL The name of the xAOD class
/// @param PERSTYPE The persistent (primitive) type name
/// @param TRANSTYPE The transient (primitive) type name
/// @param NAME The name of the auxiliary variable
///
#define AUXSTORE_PRIMITIVE_GETTER_WITH_CAST(CL, PERSTYPE, TRANSTYPE,    \
                                            NAME)                       \
   TRANSTYPE CL::NAME() const {                                         \
      static const SG::Accessor< PERSTYPE > acc( #NAME );               \
      return static_cast< TRANSTYPE >( acc( *this ) );                  \
   }

/// Macro creating a setter function with a type conversion
///
/// In a number of cases we use a different primitive type in the interface
/// classes than what we end up putting into the auxiliary store. In most
/// cases we do double<->float, and enum<->int conversions in these cases.
///
/// This macro can be used to slightly lower the number of lines needed
/// for implementing such a function.
///
/// @param CL The name of the xAOD class
/// @param PERSTYPE The persistent (primitive) type name
/// @param TRANSTYPE The transient (primitive) type name
/// @param NAME The name of the auxiliary variable
///
#define AUXSTORE_PRIMITIVE_SETTER_WITH_CAST(CL, PERSTYPE, TRANSTYPE,  \
                                            NAME, SETTER)             \
   void CL::SETTER( TRANSTYPE value ) {                               \
      static const SG::Accessor< PERSTYPE > acc( #NAME );             \
      acc( *this ) = static_cast< PERSTYPE >( value );                \
      return;                                                         \
   }

#endif // XAODCORE_AUXSTOREACCESSORMACROS_H
