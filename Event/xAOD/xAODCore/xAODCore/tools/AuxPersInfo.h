// Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// $Id: AuxPersInfo.h 793737 2017-01-24 20:11:10Z ssnyder $
#ifndef XAODCORE_TOOLS_AUXPERSINFO_H
#define XAODCORE_TOOLS_AUXPERSINFO_H

// System include(s):
#include <stdexcept>

// EDM include(s):
#include "AthContainersInterfaces/IAuxTypeVector.h"

namespace xAOD {

   /// Class managing concrete persistent variables
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
   template< class T >
   class AuxPersInfo : public SG::IAuxTypeVector {

   public:
      /// Convenience type definition
      typedef T& info_type;

      /// Constructor
      AuxPersInfo( SG::auxid_t auxid, info_type info ) :
         m_info( info ), m_auxid( auxid ) {}

      virtual SG::auxid_t auxid() const override {
         return m_auxid;
      }

      virtual std::unique_ptr<SG::IAuxTypeVector> clone() const override {
        return std::make_unique<AuxPersInfo<T> >(*this);
      }

      virtual void* toPtr() override {
         return &m_info;
      }
      virtual const void* toPtr() const override {
         return &m_info;
      }
      virtual void* toVector() override {
         return &m_info;
      }
      virtual size_t size() const override {
         return 1;
      }
      virtual bool resize( size_t sz ) override {
         if( sz != 1 ) {
            throw std::runtime_error( "Calling resize with != 1 on a "
                                      "non-vector" );
         }
         return true;
      }
      virtual void reserve( size_t sz ) override {
         if( sz != 1 ) {
            throw std::runtime_error( "Calling reserve with != 1 on a "
                                      "non-vector" );
         }
      }
      virtual void shift( size_t /*pos*/, ptrdiff_t /*offs*/ ) override {
         throw std::runtime_error( "Calling shift on a non-vector" );
      }

      virtual bool insertMove (size_t /*pos*/, void* /*beg*/, void* /*end*/,
                               SG::IAuxStore& /*srcStore*/) override
      {
        throw std::runtime_error( "Calling insertMove on a non-vector" );
      }
     
      virtual const std::type_info* objType() const override {
         return &typeid(T);
      }

   private:
      /// Reference to the info being handled
      info_type m_info;

      SG::auxid_t m_auxid;

   }; // class AuxPersInfo

} // namespace xAOD

#endif // XAODCORE_TOOLS_AUXPERSINFO_H
