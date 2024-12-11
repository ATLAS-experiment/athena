// Dear emacs, this is -*- c++ -*-
//
// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
//
#ifndef XAODROOTACCESS_RAUXSTORE_H
#define XAODROOTACCESS_RAUXSTORE_H

// EDM include(s):
#include "AthContainers/tools/threading.h"
#include "AthContainersInterfaces/IAuxStore.h"
#include "AthContainersInterfaces/IAuxStoreIO.h"
#include "xAODCore/AuxSelection.h"
#include "xAODRootAccess/tools/ReturnCheck.h"

// ROOT include(s):
#include <ROOT/RNTupleModel.hxx>
#include <ROOT/RNTupleReader.hxx>
#include <ROOT/RNTupleView.hxx>

#include <string>
#include <vector>
#include <memory>


namespace SG {
   class IAuxTypeVector;
   class AuxStoreInternal;
}  // namespace SG

namespace xAOD {
   
   using ROOT::Experimental::RNTupleModel;
   using ROOT::Experimental::RNTupleReader;
   using ROOT::Experimental::RNTupleView;

   class RAuxStore : public SG::IAuxStore, public SG::IAuxStoreIO {

      /// Convenience type declaration
      typedef SG::auxid_t auxid_t;
      /// Convenience type declaration
      typedef SG::auxid_set_t auxid_set_t;

   public:
      /// "Structural" modes of the object
      enum EStructMode {
         kUndefinedStore = 0,  ///< The structure mode is not defined
         kContainerStore = 1,  ///< The object describes an entire container
         kObjectStore = 2      ///< The object describes a single object
      };

      RAuxStore( const char* prefix = "", Bool_t topStore = kTRUE, EStructMode mode = kUndefinedStore );
      /// Destructor
      virtual ~RAuxStore();

      /// Return the structure mode of the object
      EStructMode structMode() const;

      /// Connect the object to an input RNTuple
      StatusCode readFrom( const std::string& fileName, const std::string& ntupleName );
      /// Connect the object to an output RNTuple
      StatusCode writeTo( const std::string& fileName, const std::string& ntupleName );
      /// Get entry from the input RNTuple
      ::Int_t getEntry( Long64_t entry, ::Int_t getall = 0 );
      /// Reset the store
      void reset();

      /// @name Functions implementing the SG::IConstAuxStore interface
      /// @{
      /// Get a pointer to a given array
      virtual const void* getData( auxid_t auxid ) const override;

      /// Return vector interface for one aux data item.
      virtual const SG::IAuxTypeVector* getVector(
          SG::auxid_t auxid ) const override;

      /// Get the types(names) of variables handled by this container
      virtual const auxid_set_t& getAuxIDs() const override;

      /// Get the types(names) of decorations handled by this container
      virtual const auxid_set_t& getDecorIDs() const override;

      /// Get a pointer to a given array, creating the array if necessary
      virtual void* getDecoration( auxid_t auxid, size_t size,
                                   size_t capacity ) override;

      /// Test if a variable is a decoration.
      virtual bool isDecoration( auxid_t auxid ) const override;

      /// Lock the object, and don't let decorations be added
      virtual void lock() override;
      /// Remove the decorations added so far. Only works for transient
      /// decorations.
      virtual bool clearDecorations() override;

      /// Lock a decoration.
      virtual void lockDecoration( SG::auxid_t auxid ) override;

      /// Return the number of elements in the store
      virtual size_t size() const override;

      /// Return interface for a linked variable.
      virtual SG::IAuxTypeVector* linkedVector( SG::auxid_t auxid ) override;
      /// Return const interface for a linked variable.
      virtual const SG::IAuxTypeVector* linkedVector(
          SG::auxid_t auxid ) const override;

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
      virtual bool insertMove( size_t pos, IAuxStore& other,
                               const SG::auxid_set_t& ignore ) override;

      /// @}

      /// @name Functions implementing the SG::IAuxStoreIO interface
      /// @{

      /// Get a pointer to the data being stored for one aux data item
      virtual const void* getIOData( auxid_t auxid ) const override;

      /// Return the type of the data to be stored for one aux data item
      virtual const std::type_info* getIOType( auxid_t auxid ) const override;

      /// Get the types(names) of variables created dynamically
      virtual const auxid_set_t& getDynamicAuxIDs() const override;

      /// Select dynamic auxiliary attributes for writing
      virtual void selectAux( const std::set< std::string >& attributes );

      /// Get the IDs of the selected aux variables
      virtual auxid_set_t getSelectedAuxIDs() const override;

      /// @}

   private:
      /// Connect a variable to the input tree
      StatusCode setupInputData( auxid_t auxid );
      /// Connect a variable to the output ntuple
      StatusCode setupOutputData( auxid_t auxid );
      /// Scan the input RNTuple for auxiliary fields
      StatusCode scanInputNtuple();
      /// Find the type_info to use as the aux type for a given field.
      const std::type_info* auxFieldType( const std::string& fieldName,
                                           const std::string& auxName, 
                                           ::Bool_t staticField );
      /// Register one input field as an available auxiliary variable
      StatusCode setupAuxField( const std::string & fieldName,
                                const std::string & auxName,
                                ::Bool_t staticField );
      /// Check if this auxiliary variable needs to go to the output
      ::Bool_t isAuxIDSelected( auxid_t auxid ) const;
      /// Check if the auxiliary variable has a registered type
      static ::Bool_t isRegisteredType( auxid_t auxid );
      /// Check if a field holds a primitive variable or not
      ::Bool_t isPrimitiveField( std::string fieldName );
      /// Check if a field describes a container or a single object
      ::Bool_t isContainerField( std::string fieldName, auxid_t auxid ) const;
      /// Add a vector field to a model (either the input or the output one)
      StatusCode addVectorField( std::string fieldName,
                                 std::string fieldTypeName,
                                 ::Bool_t isInputFile );
      ///
      ::Bool_t fieldInfosHasAuxid( auxid_t auxid ) const;

      /// Static prefix for the field names
      std::string m_prefix;
      /// Dynamic prefix for the field names
      std::string m_dynPrefix;
      /// Flag stating whether this is a "top store"
      ::Bool_t m_topStore;

      /// The "structural" mode of the object
      EStructMode m_structMode;

      /// The ntuple being read from
      std::unique_ptr< RNTupleReader > m_inNtuple;
      /// The the ntuple being written to
      std::unique_ptr< RNTupleReader > m_outNtuple;

      /// "Scan status" of the input RNTuple
      ::Bool_t m_inputScanned;

      /// Object helping to select which auxiliary variables to write
      AuxSelection m_selection;
      /// Store for the in-memory-only variables
      SG::AuxStoreInternal* m_transientStore{};

      /// Internal list of variable IDs handled currently by the object
      auxid_set_t m_auxIDs;
      auxid_set_t m_decorIDs;
      /// Variables handled currently by the object
      std::vector< SG::IAuxTypeVector* > m_vecs;
      /// The current size of the container being described
      std::size_t m_size{};

      /// Is this container locked?
      ::Bool_t m_locked;
      /// Flags items as decorations
      std::vector< ::Bool_t > m_isDecoration;

      /// Mutex type for multithread synchronization
      typedef AthContainers_detail::mutex mutex_t;
      /// Guard type for multithreaded synchronisation
      typedef AthContainers_detail::lock_guard< mutex_t > guard_t;
      /// Mutexes object used for multithreaded synchronisation
      mutable mutex_t m_mutex1, m_mutex2;

      /// The input file that we are reading from
      std::string m_inFileName;
      /// The output file that we are writing to
      std::string m_outFileName;
      /// The name of the ntuple that we are reading from
      std::string m_inputNtupleName;
      /// The name of the ntuple that we are writing to
      std::string m_outputNtupleName;
      /// The model of the output ntuple
      std::unique_ptr< RNTupleModel > m_outModel;
      /// The model of the input ntuple
      std::unique_ptr< RNTupleModel > m_inModel;
      /// The entry to load from the ntuple
      ::Long64_t m_entryToLoad;

      class RFieldInfo {
      public:
         /// Constructor
         RFieldInfo();
         /// Get entry from the field
         ::Int_t getEntry(::Long64_t entryToLoad);
         /// The different available statuses for the field
         enum Status { NotInitialized, Initialized, TypeError, NotFound };
         /// The current status of the field
         enum Status status = NotInitialized;
         /// The name of the field
         std::string fieldName;
         /// The name of the field's type
         std::string fieldTypeName;
         /// A pointer to the field in the ntuple
#if ROOT_VERSION_CODE >= ROOT_VERSION( 6, 33, 0 )
         std::unique_ptr< RNTupleView< void > >
#else
         std::unique_ptr< RNTupleView< void, true > >
#endif
         field;
         /// The name of the ntuple
         std::string ntupleName;
         /// The entry that was loaded from the ntuple
         ::Long64_t entryLoaded;
         /// The pointer to the object in memory
         void* object{};
         /// The typeinfo of the object
         std::type_info* typeInfo{};
      };

      /// A way to acces an representation of a field with an auxid.
      std::map< auxid_t, RFieldInfo > m_fields;

   };  // class RAuxStore
}  // namespace xAOD

#endif  // XAODROOTACCESS_RAUXSTORE_H
