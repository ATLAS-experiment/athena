// Dear emacs, this is -*- c++ -*-
/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODCORE_AUXPERSVECTOR_H
#define XAODCORE_AUXPERSVECTOR_H

// EDM include(s):
#include "AthContainers/AuxTypeRegistry.h"
#include "AthContainers/tools/AuxTypeVector.h"
#include "AthContainersInterfaces/IAuxSetOption.h"

namespace xAOD {

   /// Class managing concrete vector variables
   ///
   /// This class is used internally by the "special" auxiliary store
   /// objects to manage the auxiliary variables handled by them. User
   /// code probably doesn't want to touch it directly...
   ///
   /// @author Attila Krasznahorkay <Attila.Krasznahorkay@cern.ch>
   ///
   /// $Revision: 793737 $
   /// $Date: 2017-01-24 21:11:10 +0100 (Tue, 24 Jan 2017) $
   ///
   template< class T, class VEC=std::vector< T > >
   class AuxPersVector : public SG::AuxTypeVectorHolder<T, VEC> {

   public:
      /// Convenience type definition
      typedef VEC& vector_type;

      /// Constructor
      AuxPersVector( SG::auxid_t auxid, vector_type vec, bool isLinked )
        : SG::AuxTypeVectorHolder<T, VEC> (auxid, &vec, false, isLinked) {}

      virtual std::unique_ptr<SG::IAuxTypeVector> clone() const override {
        return std::make_unique<AuxPersVector<T, VEC> >(*this);
      }

   }; // class AuxPersVector

} // namespace xAOD

#endif // XAODCORE_AUXPERSVECTOR_H
