// Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// $Id: ShallowAuxInfo.h 671678 2015-06-02 12:28:46Z krasznaa $
#ifndef XAODCORE_SHALLOWAUXINFO_H
#define XAODCORE_SHALLOWAUXINFO_H

// Local include(s):
#include "xAODCore/ShallowAuxContainer.h"
#include "AthContainersInterfaces/ToTransient.h"
#ifndef XAOD_STANDALONE
#include "GaudiKernel/ThreadLocalContext.h"
#endif

namespace xAOD {

   /// Shallow copy for the auxiliary store of standalone objects
   ///
   /// The design of this is exactly the same as for the
   /// <code>SG::AuxStoreInternal</code> - <code>SG::AuxStoreStandalone</code>
   /// pair. All the code is in <code>xAOD::ShallowAuxContainer</code>, this
   /// class is just a convenience shorthand for calling
   /// <code>xAOD::ShallowAuxContainer(true)</code> in the code.
   ///
   /// @author Attila Krasznahorkay <Attila.Krasznahorkay@cern.ch>
   ///
   /// $Revision: 671678 $
   /// $Date: 2015-06-02 14:28:46 +0200 (Tue, 02 Jun 2015) $
   ///
   class ShallowAuxInfo : public ShallowAuxContainer {

   public:
      /// Flag that we should _not_ use the xAOD aux store pool converter
      /// for this type.
      // cppcheck-suppress duplInheritedMember
      static constexpr bool supportsThinning = false;

      /// Default constructor
      ShallowAuxInfo();
      /// Constructor with a parent object
      ShallowAuxInfo( const DataLink< SG::IConstAuxStore >& parent );

      /// Return the type of the store object
      virtual AuxStoreType getStoreType() const { return AST_ObjectStore; }

   }; // class ShallowAuxInfo

} // namespace xAOD



// This class declares supportsThinning=false, meaning that it will be
// handled by the generic POOL converter rather than the one specialized
// for xAOD auxiliary stores.  Hence, we need to specialize ToTransient
// in order to get toTransient called for this type.
namespace SG {


template<>
class ToTransient<xAOD::ShallowAuxInfo>
{
public:
  static bool toTransient (xAOD::ShallowAuxInfo& s, const EventContext& ctx)
  {
    s.toTransient( ctx );
    return true;
  }
#ifndef XAOD_STANDALONE
  static bool toTransient (xAOD::ShallowAuxInfo& s)
  {
    return toTransient( s, Gaudi::Hive::currentContext() );
  }
#endif
};


}

// Declare a class ID for the class:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF( xAOD::ShallowAuxInfo, 196927374, 1 )

// Describe the inheritance of the class:
#include "xAODCore/BaseInfo.h"
SG_BASE( xAOD::ShallowAuxInfo, xAOD::ShallowAuxContainer );

#endif // XAODCORE_SHALLOWAUXINFO_H
