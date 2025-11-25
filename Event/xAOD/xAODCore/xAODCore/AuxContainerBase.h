// Dear emacs, this is -*- c++ -*-
/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODCORE_AUXCONTAINERBASE_H
#define XAODCORE_AUXCONTAINERBASE_H

// STL include(s):
#include <vector>
#include <string>
#include <memory>

// EDM include(s):
#include "AthContainersInterfaces/IAuxStore.h"
#include "AthContainersInterfaces/IAuxStoreIO.h"
#include "AthContainersInterfaces/IAuxStoreHolder.h"
#include "AthContainers/AuxTypeRegistry.h"
#include "AthContainers/tools/threading.h"
#include "AthContainers/PackedContainer.h"
#include "CxxUtils/CachedPointer.h"
#include "CxxUtils/checker_macros.h"
#include "SGCore/ILockable.h"

// Local include(s):
#include "xAODCore/AuxSelection.h"

// Forward declaration(s):
namespace SG {
   class IAuxTypeVector;
}
class xAODAuxContainerBaseCnv;
namespace std { namespace pmr {
class memory_resource;
}}

/// Namespace holding all the xAOD EDM classes
namespace xAOD {

   /// Common base class for the auxiliary containers
   ///
   /// To make the development of auxiliary containers simpler,
   /// they can all inherit from this one class. Then all they
   /// need to do is just to declare their data members, everything
   /// else is taken care of by this transient base class.
   ///
   /// @author Attila Krasznahorkay <Attila.Krasznahorkay@cern.ch>
   ///
   /// $Revision: 793737 $
   /// $Date: 2017-01-24 21:11:10 +0100 (Tue, 24 Jan 2017) $
   ///
   class AuxContainerBase : public SG::IAuxStore,
                            public SG::IAuxStoreIO,
                            public SG::IAuxStoreHolder,
                            public ILockable
   {

   public:
      /// The aux ID type definition
      typedef SG::auxid_t auxid_t;
      /// The aux ID set type definition
      typedef SG::auxid_set_t auxid_set_t;

      /// Default constructor
      AuxContainerBase( bool allowDynamicVars = true );
      /// Passing in a memory resource.
      AuxContainerBase( std::pmr::memory_resource* memResource,
                        bool allowDynamicVars = true );
      /// Copy constructor
      AuxContainerBase( const AuxContainerBase& parent );
      /// Constructor receiving a "dynamic auxiliary store"
      AuxContainerBase( SG::IAuxStore* store,
                        std::pmr::memory_resource* memResource = nullptr );
      /// Destructor
      ~AuxContainerBase();

      /// Assignment operator
      AuxContainerBase& operator=( const AuxContainerBase& rhs );

      /// @name Functions implementing the SG::IAuxStoreHolder interface
      /// @{

      /// Get the currently used internal store object
      virtual SG::IAuxStore* getStore() override;
      virtual const SG::IAuxStore* getStore() const override;
      /// Set a different internal store object
      virtual void setStore( SG::IAuxStore* store ) override;
      /// Return the type of the store object
      virtual AuxStoreType getStoreType() const override { return AST_ContainerStore; }

      /// Return the memory resource to use.
      std::pmr::memory_resource* memResource();

      /// @}

      /// @name Functions implementing the SG::IConstAuxStore interface
      /// @{

      /// Get a pointer to a given array
      virtual const void* getData( auxid_t auxid ) const override;

      /// Return vector interface for one aux data item.
      virtual const SG::IAuxTypeVector* getVector (SG::auxid_t auxid) const override final;

      /// Get the types(names) of variables handled by this container
      virtual const auxid_set_t& getAuxIDs() const override;

      /// Get the types(names) of decorations handled by this container
      virtual const auxid_set_t& getDecorIDs() const override;

      /// Test if a variable is a decoration.
      virtual bool isDecoration (auxid_t auxid) const override;

      /// Get a pointer to a given array, as a decoration.
      virtual void* getDecoration( auxid_t auxid, size_t size,
                                   size_t capacity ) override;

      /// Lock the container.
      virtual void lock() override;

      /// Clear all decorations.
      virtual bool clearDecorations() override;

      /// Get the size of the container.
      virtual size_t size() const override;

      /// Lock a decoration.
      virtual void lockDecoration (SG::auxid_t auxid) override;

      /// @brief Return interface for a linked variable.
      virtual const SG::IAuxTypeVector* linkedVector (SG::auxid_t auxid) const override;

      /// @}

      /// @name Functions implementing the SG::IAuxStore interface
      /// @{

      /// Get a pointer to a given array, creating the array if necessary
      virtual void* getData( auxid_t auxid, size_t size,
                             size_t capacity ) override;

      /// Return a set of writable data identifiers
      virtual const auxid_set_t& getWritableAuxIDs() const override;

      /// Resize the arrays to a given size
      virtual bool resize( size_t size ) override;
      /// Reserve a given size for the arrays
      virtual void reserve( size_t size ) override;
      /// Shift the contents of the stored arrays
      virtual void shift( size_t pos, ptrdiff_t offs ) override;
      /// Insert contents of another store via move.
      virtual bool insertMove (size_t pos,
                               IAuxStore& other,
                               const SG::auxid_set_t& ignore) override;
      /// Make an option setting on an aux variable.
      virtual bool setOption( auxid_t id, const SG::AuxDataOption& option ) override;

      /// @brief Return interface for a linked variable.
      virtual SG::IAuxTypeVector* linkedVector (SG::auxid_t auxid) override;

      /// @}

      /// @name Functions implementing the SG::IAuxStoreIO interface
      /// @{

      /// Get a pointer to the data being stored for one aux data item
      virtual const void* getIOData( auxid_t auxid ) const override;

      /// Return the type of the data to be stored for one aux data item
      virtual const std::type_info* getIOType( auxid_t auxid ) const override;

      /// Get the types(names) of variables created dynamically
      virtual const auxid_set_t& getDynamicAuxIDs() const override;

      /// Get the IDs of the selected dynamic Aux variables (for writing)
      virtual SG::auxid_set_t getSelectedAuxIDs() const override;

      /// @}

      /// @name Functions managing the instance name of the container
      /// @{

      /// Get the name of the container instance
      const char* name() const;
      /// Set the name of the container instance
      void setName( const char* name );

      /// @}

      /// Declare how to wrap variables for this sort of base.
      template <class T, class ALLOC = std::allocator<T> >
      using AuxVariable_t = std::vector<T, ALLOC>;
      template <class T, class ALLOC = std::allocator<T> >
      using LinkedVariable_t = AuxVariable_t<T, ALLOC>;

      /// Get the auxiliary ID for one of the persistent variables
      template< typename T, typename ALLOC >
      auxid_t getAuxID( const std::string& name,
                        std::vector< T, ALLOC >& /*vec*/,
                        SG::AuxVarFlags flags =
                        SG::AuxVarFlags::None,
                        const SG::auxid_t linkedVariable = SG::null_auxid );
      /// Get the auxiliary ID for one of the persistent variables
      template< typename T >
      auxid_t getAuxID( const std::string& name,
                        SG::PackedContainer< T >& /*vec*/,
                        SG::AuxVarFlags flags =
                        SG::AuxVarFlags::None,
                        const SG::auxid_t linkedVariable = SG::null_auxid );
      /// Register one of the persistent variables internally
      template< typename T, typename ALLOC >
      void regAuxVar( auxid_t auxid, const std::string& name,
                      std::vector< T, ALLOC >& vec );

      /// Register one of the persistent variables internally
      template< typename T >
      void regAuxVar( auxid_t auxid, const std::string& name,
                      SG::PackedContainer< T >& vec );

   private:
      friend class ::xAODAuxContainerBaseCnv;

      /// Common code between regAuxVar cases.
      template< typename ELT, typename CONT >
      void regAuxVar1( auxid_t auxid, const std::string& name,
                       CONT& vec );

      /// Internal list of all available variables
      auxid_set_t m_auxids;
      /// Internal list of all managed variables
      std::vector< SG::IAuxTypeVector* > m_vecs;

      /// Internal dynamic auxiliary store object
      SG::IAuxStore* m_store;
      /// The IO interface to the internal auxiliary store
      SG::IAuxStoreIO* m_storeIO;
      /// Flag deciding if the object owns the dynamic store or not
      bool m_ownsStore;
      /// Has the container been locked?
      bool m_locked;

      /// Mutex for multithread synchronization.
      typedef AthContainers_detail::mutex mutex_t;
      typedef AthContainers_detail::lock_guard< mutex_t > guard_t;
      mutable mutex_t m_mutex;

      /// Name of the container in memory. Set externally.
      std::string m_name;

      /// Memory resource to use for this container.
      mutable CxxUtils::CachedPointer<std::pmr::memory_resource> m_memResource ATLAS_THREAD_SAFE;

   }; // class AuxContainerBase

} // namespace xAOD

// Declare a class ID for the class:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF( xAOD::AuxContainerBase, 1225080690, 3 )

// Describe the inheritance of the class:
#include "xAODCore/BaseInfo.h"
SG_BASES3( xAOD::AuxContainerBase, SG::IAuxStore, SG::IAuxStoreIO,
           SG::IAuxStoreHolder );

// Include the template implementation:
#include "AuxContainerBase.icc"

#endif // XAODCORE_AUXCONTAINERBASE_H
