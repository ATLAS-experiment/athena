// Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

// ROOT include(s):
#include <TClass.h>
#include <TError.h>
#include <TMethodCall.h>
#include <TString.h>
#include <TVirtualCollectionProxy.h>
#include <TInterpreter.h>

// Local include(s):
#include "xAODRootAccess/tools/TAuxVectorFactory.h"
#include "xAODRootAccess/tools/TAuxVector.h"
#include "xAODRootAccess/tools/Message.h"
#include "AthContainers/normalizedTypeinfoName.h"
#include "AthContainers/AuxVectorData.h"
#include "CxxUtils/ClassName.h"
#include "CxxUtils/as_const_ptr.h"

namespace xAOD {

   TAuxVectorFactory::TAuxVectorFactory( ::TClass* cl )
      : m_class( cl ), m_proxy( cl->GetCollectionProxy() ),
        m_defElt( 0 ) {

      // A little sanity check:
      if( ! m_proxy ) {
         ::Fatal( "xAOD::TAuxVectorFactory::TAuxVectorFactory",
                  XAOD_MESSAGE( "No collection proxy found for type %s" ),
                  cl->GetName() );
      }
      else {
         // Check if the elements of the vector are objects:
        ::TClass* eltClass = m_proxy->GetValueClass();
        if( eltClass ) {
           // Initialise the assignment operator's method call:
          std::string proto = "const ";
          proto += eltClass->GetName();
          proto += "&";
          m_assign.setProto( eltClass, "operator=", proto );
          if( m_assign.call() == nullptr ) {
             ::Warning( "xAOD::TAuxVectorFactory::TAuxVectorFactory",
                        XAOD_MESSAGE( "Can't get assignment operator for "
                                      "class %s" ),
                        eltClass->GetName() );
          }
          m_defElt = eltClass->New();
        }
      }
   }

   TAuxVectorFactory::~TAuxVectorFactory() {

      // Remove the default element from memory if it exists:
      if( m_defElt ) {
         m_proxy->GetValueClass()->Destructor( m_defElt );
      }
   }

   std::unique_ptr< SG::IAuxTypeVector >
   TAuxVectorFactory::create( SG::auxid_t auxid, size_t size, size_t capacity ) const {

     return std::make_unique< TAuxVector >( this, auxid,
                                            CxxUtils::as_const_ptr(m_class),
                                            size, capacity );
   }

   std::unique_ptr< SG::IAuxTypeVector >
   TAuxVectorFactory::createFromData( SG::auxid_t /*auxid*/,
                                      void* /*data*/, bool /*isPacked*/,
                                      bool /*ownFlag*/ ) const {

      std::abort();
   }

   void TAuxVectorFactory::copy( SG::auxid_t auxid,
                                 SG::AuxVectorData& dst,        size_t dst_index,
                                 const SG::AuxVectorData& src,  size_t src_index,
                                 size_t n ) const {

      // The size of one element in memory:
      const size_t eltsz = m_proxy->GetIncrement();

      // Get the location of the source and target element in memory:
      char* dstptr = reinterpret_cast<char*> (dst.getDataArray (auxid));
      const char* srcptr = &dst == &src ? dstptr : reinterpret_cast<const char*>(src.getDataArray (auxid));
      dstptr += eltsz * dst_index;
      srcptr += eltsz * src_index;

      // Do the copy either using the assignment operator of the type, or using
      // simple memory copying:
      TMethodCall* mc = m_assign.call();
      if( mc ) {
        auto copyone = [mc] (void* dst, const void* src)
          {
            mc->ResetParam();
            mc->SetParam( ( Long_t ) src );
            mc->Execute( dst );
          };

         // If the source range doesn't overlap with the destination:
         if( dstptr > srcptr && ( srcptr + n * eltsz ) > dstptr ) {
            for( size_t i = n - 1; i < n; --i ) {
               copyone( dstptr + i*eltsz, srcptr + i*eltsz );
            }
         }
         // If it does:
         else {
            for( size_t i = 0; i < n; ++i ) {
               copyone( dstptr + i*eltsz, srcptr + i*eltsz );
            }
         }

      } else {
         memmove( dstptr, srcptr, n * eltsz );
      }

      return;
   }

   void TAuxVectorFactory::copyForOutput( SG::auxid_t auxid,
                                          SG::AuxVectorData& dst, size_t dst_index,
                                          const SG::AuxVectorData& src,
                                          size_t src_index,
                                          size_t n) const {

      // Do a "regular" copy.
      copy( auxid, dst, dst_index, src, src_index, n );

      ::Warning( "xAOD::TAuxVectorFactory::TAuxVectorFactory",
                 XAOD_MESSAGE( "copyForOutput called; should only be used "
                               "with pool converters." ) );
   }

   void TAuxVectorFactory::swap( SG::auxid_t auxid,
                                 SG::AuxVectorData& a, size_t aindex,
                                 SG::AuxVectorData& b, size_t bindex,
                                 size_t n ) const {

      // The size of one element in memory:
      const size_t eltsz = m_proxy->GetIncrement();

      // Get the location of the two elements in memory:
      char* aptr = reinterpret_cast<char*>( a.getDataArray (auxid) );
      char* bptr = &a == &b ? aptr : reinterpret_cast<char*>( b.getDataArray (auxid) );
      aptr += eltsz * aindex;
      bptr += eltsz * bindex;

      TMethodCall* mc = m_assign.call();
      if( mc ) {

         // Create a temporary object in memory:
         TClass* eltClass = m_proxy->GetValueClass();
         void* tmp = eltClass->New();

         for (size_t i = 0; i < n; ++i) {
           // tmp = a
           mc->ResetParam();
           mc->SetParam( ( Long_t ) aptr );
           mc->Execute( static_cast<void*>( tmp ) );
           // a = b
           mc->ResetParam();
           mc->SetParam( ( Long_t ) bptr );
           mc->Execute( static_cast<void*>( aptr ) );
           // b = tmp
           mc->ResetParam();
           mc->SetParam( ( Long_t ) tmp );
           mc->Execute( static_cast<void*>( bptr ) );
           aptr += eltsz;
           bptr += eltsz;
         }

         // Delete the temporary object:
         eltClass->Destructor( tmp );

      } else {

         // Allocate some temporary memory for the swap:
         std::vector< char > tmp( eltsz*n );
         // tmp = a
         memcpy( tmp.data(), aptr, eltsz*n );
         // a = b
         memcpy( aptr, bptr, eltsz*n );
         // b = tmp
         memcpy( bptr, tmp.data(), eltsz*n );
      }

      return;
   }

   void TAuxVectorFactory::clear( void* dst,
                                  size_t dst_index,
                                  size_t n ) const {

      // The size of one element in memory:
      const size_t eltsz = m_proxy->GetIncrement();

      // Get the memory address of the element:
      char* dptr = reinterpret_cast< char* >( dst ) + eltsz * dst_index;

      TMethodCall* mc = m_assign.call();
      if( mc ) {
         // Assign the default element's contents to this object:
         for (size_t i = 0; i < n; ++i) {
            mc->ResetParam();
            mc->SetParam( ( Long_t ) m_defElt );
            mc->Execute( static_cast<void*>( dptr ) );
            dptr += eltsz;
         }
      } else {
         // Set the memory to zero:
         memset( dst, 0, eltsz*n );
      }

      return;
   }


   void TAuxVectorFactory::clear( SG::auxid_t auxid,
                                  SG::AuxVectorData& dst,
                                  size_t dst_index,
                                  size_t n ) const {

      clear( dst.getDataArray (auxid), dst_index, n );
      return;
   }

   size_t TAuxVectorFactory::getEltSize() const {

      return m_proxy->GetIncrement();
   }

   const std::type_info* TAuxVectorFactory::tiVec() const {

      return m_class->GetTypeInfo();
   }

   const std::type_info* TAuxVectorFactory::tiAlloc() const {

      return nullptr;
   }

   std::string TAuxVectorFactory::tiAllocName() const {

     std::string name = SG::normalizedTypeinfoName (*m_class->GetTypeInfo());
     CxxUtils::ClassName cn (name);
     std::string alloc_name;
     if (cn.ntargs() >= 2) {
       alloc_name = cn.targ(1).fullName();
     }
     else if (cn.ntargs() == 1) {
       alloc_name = "std::allocator<" + cn.targ(0).fullName();
       if (alloc_name[alloc_name.size()-1] == '>') alloc_name += " ";
       alloc_name += ">";
     }
     return alloc_name;
   }

} // namespace xAOD
