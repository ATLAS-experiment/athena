// Dear emacs, this is -*- c++ -*-
//
// Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
//
#ifndef XAODROOTACCESS_TOOLS_TAUXVECTORFACTORY_H
#define XAODROOTACCESS_TOOLS_TAUXVECTORFACTORY_H

// Athena include(s):
#include "AthContainersInterfaces/IAuxTypeVectorFactory.h"
#include "CxxUtils/checker_macros.h"
#include "RootUtils/TSMethodCall.h"

// Forward declaration(s):
class TClass;
class TVirtualCollectionProxy;

namespace xAOD {

   /// Auxiliary vector factory based on a ROOT dictionary
   ///
   /// Factory implementation that creates vectors that can be operated on
   /// using ROOT's dictionary for the underlying vector type.
   ///
   /// The code is pretty much a copy of what Scott wrote for RootStorageSvc for
   /// the offline code.
   ///
   /// @author Scott Snyder <Scott.Snyder@cern.ch>
   /// @author Attila Krasznahorkay <Attila.Krasznahorkay@cern.ch>
   ///
   class TAuxVectorFactory : public SG::IAuxTypeVectorFactory {

   public:
      /// Constructor, setting up the object based on a dictionary
      TAuxVectorFactory( ::TClass* cl );
      /// Destructor
      virtual ~TAuxVectorFactory();

      TAuxVectorFactory (const TAuxVectorFactory&) = delete;
      TAuxVectorFactory& operator= (const TAuxVectorFactory&) = delete;

      /// @name Functions implementing the SG::IAuxTypeVectorFactory interface
      /// @{

      /// Create a new vector in memory with the requested size and capacity
      virtual std::unique_ptr< SG::IAuxTypeVector >
      create( SG::auxid_t auxid, size_t size, size_t capacity ) const override;

      /// Create a vector object of this type from a data blob
      virtual std::unique_ptr< SG::IAuxTypeVector >
      createFromData( SG::auxid_t auxid, void* data, bool isPacked,
                      bool ownFlag ) const override;

      /// Copy one element from one location to another
      virtual void copy( void* dst,       size_t dst_index,
                         const void* src, size_t src_index ) const override;

      /// Copy one element from one location to another
      virtual void
      copyForOutput( void* dst,       size_t dst_index,
                     const void* src, size_t src_index ) const override;

      /// Swap the payload of two ranges of elements in memory
      virtual void swap( SG::auxid_t auxid,
                         SG::AuxVectorData& a, size_t aindex,
                         SG::AuxVectorData& b, size_t bindex,
                         size_t n ) const override;

      /// Clear the payload of a given range inside a vector
      virtual void clear( void* dst, size_t dst_index ) const override;

      /// Size of the elements inside the vector type
      virtual size_t getEltSize() const override;

      /// Type info of the vector type handled by the factory object
      virtual const std::type_info* tiVec() const override;

      /// Type of the factory
      virtual bool isDynamic() const override { return true; }

      /**
       * @brief Return the @c type_info of the vector allocator.
       *
       * May be nullptr for a dynamic vector.
       */
      virtual const std::type_info* tiAlloc() const override;


      /**
       * @brief Return the (demangled) name of the vector allocator.
       */
      virtual std::string tiAllocName() const override;


      /// @}

   private:
      /// The type that this factory operates on
      ::TClass* m_class;
      /// ROOT's description of the vector type
      ::TVirtualCollectionProxy* m_proxy;
      /// Assignment operator
      mutable RootUtils::TSMethodCall m_assign ATLAS_THREAD_SAFE;
      /// Pointer to a default element object in memory
      void* m_defElt;

   }; // class TAuxVectorFactory

} // namespace xAOD

#endif // XAODROOTACCESS_TOOLS_TAUXVECTORFACTORY_H
