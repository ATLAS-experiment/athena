// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

// System include(s):
#include <filesystem>

// Local include(s):
#include "xAODRootAccess/RAuxStore.h"
#include "xAODRootAccess/tools/TAuxVectorFactory.h"
#include "xAODRootAccess/tools/THolder.h"
#include "xAODRootAccess/tools/Utils.h"

// ROOT include(s):
#include <TClass.h>
#include <TROOT.h>
#include <ROOT/RNTupleInspector.hxx>
#include <ROOT/RNTupleWriter.hxx>

// Athena include(s):
#include "AthContainers/AuxStoreInternal.h"
#include "AthContainers/AuxTypeRegistry.h"
#include "AthContainers/tools/AuxVectorInterface.h"
#include "AthContainers/exceptions.h"
#include "CxxUtils/as_const_ptr.h"


using ROOT::Experimental::RFieldBase;
using ROOT::Experimental::RNTupleInspector;
using ROOT::Experimental::RNTupleWriter;

namespace {

   TClass* lookupVectorType( TClass* cl ) {
      std::string tname = cl->GetName();
      tname += "::vector_type";
      TDataType* typ = gROOT->GetType( tname.c_str() );
      if( typ ) return TClass::GetClass( typ->GetFullTypeName() );
      return nullptr;
   }

}  // namespace

namespace xAOD {

   RAuxStore::RAuxStore( const char* prefix, Bool_t topStore, EStructMode mode )
       : SG::IAuxStore(),
         m_prefix( prefix ),
         m_dynPrefix( Utils::dynFieldPrefix( prefix ) ),
         m_topStore( topStore ),
         m_structMode( mode ),
         m_inNtuple( nullptr ),
         m_outNtuple( nullptr ),
         m_inputScanned( kFALSE ),
         m_selection(),
         m_transientStore( 0 ),
         m_auxIDs(),
         m_decorIDs(),
         m_vecs(),
         m_size( 0 ),
         m_locked( kFALSE ),
         m_isDecoration(),
         m_mutex1(),
         m_mutex2(),
         m_inFileName(),
         m_outFileName(),
         m_inputNtupleName(),
         m_outputNtupleName(),
         m_outModel( nullptr ),
         m_inModel( nullptr ),
         m_fields() {}

  RAuxStore::~RAuxStore() {
    delete m_transientStore;
    m_transientStore = nullptr;
    for( auto& fieldPair : m_fields ) {
      fieldPair.second.field.release();//not a leak (coverity 17624)
    }
    m_fields.clear();  // Ensure m_fields is properly cleared
  }

   /// This function gets the structure mode of the object.
   ///
   /// @returns The structure mode of the object
   ///
   RAuxStore::EStructMode RAuxStore::structMode() const {

      return m_structMode;
   }

   /// This function is called by the infrastructure to connect the object
   /// to an input RNTuple whenever a new input file is opened.
   ///
   /// @param fileName Pointer to the file that is being read from
   /// @param ntupleName Name of the ntuple that is being read from
   /// @returns <code>kTRUE</code> if successful, <code>kFALSE</code> otherwise
   ///
   StatusCode RAuxStore::readFrom( const std::string& fileName,
                                   const std::string& ntupleName ) {

      // Remember the file name:
      m_inFileName = fileName;
      // Remember the ntuple name:
      m_inputNtupleName = ntupleName;

      // load the ntuple:
      try {
         // Try to access RNTuple.
         m_inNtuple = RNTupleReader::Open( m_inputNtupleName, fileName );
      } catch( ... ) {
         // Handle the case where the RNTuple is not found.
         ::Warning( "xAOD::RAuxStore::readFrom",
                    "RNTuple %s not found in file %s", m_inputNtupleName.c_str(),
                    m_inFileName.c_str() );
         m_inNtuple = nullptr;
      }

      // Catalogue all the fields:
      RETURN_CHECK( "xAOD::RAuxStore::readFrom", scanInputNtuple() );

      return StatusCode::SUCCESS;
   }

   /// This function is called by the infrastructure to connect the object
   /// to an output RNTuple.
   ///
   /// @param fileName Pointer to the file that is being written to
   /// @param ntupleName Name of the ntuple that is being written to
   /// @returns <code>kTRUE</code> if successful, <code>kFALSE</code> otherwise
   ///
   StatusCode RAuxStore::writeTo( const std::string & fileName,
                                  const std::string & ntupleName ) {
      // Look for any auxiliary fields that have not been connected to yet:
      RETURN_CHECK( "xAOD::RAuxStore::writeTo", scanInputNtuple() );
      m_outFileName = fileName;
      m_outputNtupleName = ntupleName;
      m_outNtuple = RNTupleReader::Open( m_outputNtupleName, m_outFileName );

      // Create all the variables that we already know about. Notice that the
      // code makes a copy of the auxid set on purpose. Because the underlying
      // AuxSelection object gets modified while doing the for loop.
      const auxid_set_t selAuxIDs = getSelectedAuxIDs();
      for( SG::auxid_t id : selAuxIDs ) {
         RETURN_CHECK( "xAOD::RAuxStore::writeTo", setupOutputData( id ) );
      }

      return StatusCode::SUCCESS;
   }

   ::Int_t RAuxStore::getEntry( ::Int_t getall ) {

      // Guard against multi-threaded execution:
      guard_t guard( m_mutex1 );

      // Reset the transient store. TEvent::fill() calls this function with
      // getall==99. When that is happening, we need to keep the transient
      // store still around. Since the user may want to interact with the
      // object after it was written out. (And since TEvent::fill() asks for
      // the transient decorations after calling getEntry(...).)
      if( m_transientStore && ( getall != 99 ) ) {
         // Remove the transient auxiliary IDs from the internal list:
         m_auxIDs -= m_transientStore->getAuxIDs();
         m_decorIDs -= m_transientStore->getDecorIDs();
         // Delete the object:
         delete m_transientStore;
         m_transientStore = 0;
      }

      // Now remove the IDs of the decorations that are getting persistified:
      if( getall != 99 ) {
         for( auxid_t auxid = 0; auxid < m_isDecoration.size(); ++auxid ) {
            if( !m_isDecoration[auxid] ) {
               continue;
            }
            m_auxIDs.erase( auxid );
            m_decorIDs.erase( auxid );
         }
      }

      // If we don't need everything loaded, return now:
      if( !getall ) return 0;

      // Get all the variables at once:
      ::Int_t minEntryStatus = 1;
      for( auto& fieldPair : m_fields ) {
         if( fieldPair.second.field ) {
            minEntryStatus =
                std::min( minEntryStatus, ( fieldPair.second ).getEntry() );
         }
      }
      return minEntryStatus;
   }

   /// This function resets the store to its initial state.
   ///
   void RAuxStore::reset() {
     for( auto& fieldPair : m_fields ) {
       fieldPair.second.field.release();//not a leak (coverity 17638)
     }
     m_fields.clear();  // Ensure m_fields is properly cleared
     // Remember that the input RNTuple needs to be re-scanned:
     m_inputScanned = kFALSE;
   }

   // SG::IConsRAuxStore functions

   /// Get a pointer to a given array
   ///
   /// @param auxid The auxiliary ID for which to retrieve the data pointer
   /// @return Pointer to the data if it exists; otherwise, nullptr
   ///
   const void* RAuxStore::getData( auxid_t auxid ) const {
      const SG::IAuxTypeVector* v = getVector( auxid );
      if( v ) {
         return v->toPtr();
      }
      return nullptr;
   }

   /// Return vector interface for one aux data item.
   ///
   /// @param auxid The auxiliary ID for which to retrieve the data vector
   /// @return Pointer to the SG::IAuxTypeVector if it exists; otherwise,
   /// nullptr
   ///
   const SG::IAuxTypeVector* RAuxStore::getVector( SG::auxid_t auxid ) const {
      // Guard against multi-threaded execution:
      guard_t guard( m_mutex1 );

      // Check if the transient store already handles this variable:
      if( m_transientStore &&
          ( m_transientStore->getAuxIDs().test( auxid ) ) ) {
         return m_transientStore->getVector( auxid );
      }

      // Connect this auxiliary variable both to the input and output
      // if needed:
      if( ( auxid >= m_vecs.size() ) || ( !m_vecs[auxid] ) ||
          ( !fieldInfosHasAuxid( auxid ) ) ) {
         auto this_nc ATLAS_THREAD_SAFE =
             const_cast< RAuxStore* >( this );  // locked above
         if( ( !this_nc->setupInputData( auxid ).isSuccess() ) ||
             ( !this_nc->setupOutputData( auxid ).isSuccess() ) ) {
            return nullptr;
         }
      }

      if( fieldInfosHasAuxid( auxid ) ) {
         auto it = m_fields.find( auxid );
         if( it != m_fields.end() ) {
            const ::Int_t result =
                const_cast< RFieldInfo& >( it->second ).getEntry();
            if( result < 0 ) {
               ::Error(
                   "xAOD::RAuxStore::getData",
                   XAOD_MESSAGE( "Couldn't read in variable %s" ),
                   SG::AuxTypeRegistry::instance().getName( auxid ).c_str() );
               return nullptr;
            }
         }
      }

      // Return the pointer to the data vector:
      return m_vecs[auxid];
   }

   /// Get the types(names) of variables handled by this container
   /// @return A constant reference to the set of auxiliary IDs
   ///
   const RAuxStore::auxid_set_t& RAuxStore::getAuxIDs() const {
      return m_auxIDs;
   }

   /// Retrieves the set of decoration IDs currently associated with this store.
   /// @return A constant reference to the set of decoration IDs
   const RAuxStore::auxid_set_t& RAuxStore::getDecorIDs() const {
      return m_decorIDs;
   }

   /// Returns a pointer to the decoration data corresponding to the specified
   /// `auxid`. If the decoration does not exist, it creates it. Handles locked
   /// stores and transient stores appropriately.
   ///
   /// @param auxid The auxiliary ID for which to retrieve or create the
   /// decoration.
   /// @param size The size of the decoration vector.
   /// @param capacity The capacity of the decoration vector.
   /// @return Pointer to the decoration data if successful; otherwise, nullptr.
   ///
   void* RAuxStore::getDecoration( auxid_t auxid, size_t size,
                                   size_t capacity ) {

      // Guard against multi-threaded execution:
      guard_t guard( m_mutex1 );

      // Remember the requested size:
      m_size = size;

      // If this is a locked object, deal with it correctly:
      if( m_locked ) {
         // If the variable exists already and it's a decoration, then let's
         // give it back.
         if( ( auxid < m_vecs.size() ) && m_vecs[auxid] &&
             ( auxid < m_isDecoration.size() && m_isDecoration[auxid] ) ) {
            // Things look okay...
            m_vecs[auxid]->reserve( capacity );
            m_vecs[auxid]->resize( size );
            return m_vecs[auxid]->toPtr();
         }
         // If it's in the transient store already, return it from there.
         // Since in a locked store *everything* is a decoration in the
         // transient store.
         if( m_transientStore && m_transientStore->getAuxIDs().test( auxid ) ) {
            return m_transientStore->getDecoration( auxid, size, capacity );
         }
         // If we know this auxiliary ID, but it was not found as a decoration
         // by the previous checks, then we're in trouble.
         if( m_auxIDs.test( auxid ) ) {
            // It may still be a decoration in a transient store. If such
            // a store exists, leave it up to that store to
            throw SG::ExcStoreLocked( auxid );
         }
      }

      // Check if we want to write this variable to the output:
      if( ( !isAuxIDSelected( auxid ) ) || ( !m_outNtuple ) ) {

         // Create the store only when necessary:
         if( !m_transientStore ) {
            m_transientStore =
                new SG::AuxStoreInternal( m_structMode == kObjectStore );
            if( m_locked ) {
               m_transientStore->lock();
            }
         }
         // Let the transient store create the decoration:
         const size_t nids = m_transientStore->getAuxIDs().size();
         void* result =
             m_transientStore->getDecoration( auxid, size, capacity );
         if( result && ( nids != m_transientStore->getAuxIDs().size() ) ) {
            m_auxIDs.insert( auxid );
            if( m_transientStore->isDecoration( auxid ) ) {
               m_decorIDs.insert( auxid );
            }
         }
         // Return the memory address from the transient store:
         return result;
      }

      // Doesn't exist yet. So let's make it:
      void* result = getData( auxid, size, capacity );
      if( m_locked ) {
         // If the container is locked, remember that this is a decoration:
         if( m_isDecoration.size() <= auxid ) {
            m_isDecoration.resize( auxid + 1 );
         }
         m_isDecoration[auxid] = ::kTRUE;
         m_decorIDs.insert( auxid );
      }
      // Return the pointer made by getData(...):
      return result;
   }

   /// Test if a variable is a decoration.
   ///
   /// @param auxid The auxiliary ID for which to check if it is a decoration
   /// @return True if the variable is a decoration; otherwise, false
   ///
   bool RAuxStore::isDecoration( auxid_t auxid ) const {
      if( m_locked ) {
         if( auxid < m_isDecoration.size() && m_isDecoration[auxid] ) {
            return true;
         }
         if( m_transientStore ) {
            return m_transientStore->isDecoration( auxid );
         }
      }
      return false;
   }

   /// Lock the object, and don't let decorations be added
   ///
   void RAuxStore::lock() {
      // Guard against multi-threaded execution:
      guard_t guard( m_mutex1 );

      m_locked = true;
      if( m_transientStore ) {
         m_transientStore->lock();
      }

      return;
   }

   /// Remove the decorations added so far. Only works for transient
   /// decorations.
   ///
   /// @return True if any decorations were removed; otherwise, false
   ///
   bool RAuxStore::clearDecorations() {
      // Guard against multi-threaded execution:
      guard_t guard( m_mutex1 );

      // Clear the transient decorations:
      bool anycleared = false;
      if( m_transientStore ) {
         SG::auxid_set_t oldIdSet = m_transientStore->getAuxIDs();

         // Clear the decorations from the transient store:
         anycleared = m_transientStore->clearDecorations();

         // Remove ids that were cleared.
         if( anycleared ) {
            oldIdSet -= m_transientStore->getAuxIDs();
            // oldIdSet is now the set of ids that were cleared.
            m_auxIDs -= oldIdSet;
            m_decorIDs.clear();
         }
      }

      return anycleared;
   }

   /// Lock a decoration.
   ///
   /// @param auxid The auxiliary ID for which to lock the decoration
   ///
   void RAuxStore::lockDecoration( SG::auxid_t auxid ) {
      if( m_transientStore ) {
         m_transientStore->lockDecoration( auxid );
      }
      m_decorIDs.erase( auxid );
   }

   /// Return the number of elements in the store
   ///
   /// @return The number of elements in the store
   ///
   size_t RAuxStore::size() const {
      // First, try to find a managed vector in the store:
      for( SG::auxid_t id : m_auxIDs ) {
         // Make sure that we are still within the bounds of our vector:
         if( id >= m_vecs.size() ) break;
         // Skip non-existent or linked objects:
         if( !m_vecs[id] || m_vecs[id]->isLinked() ) continue;
         // Ask the vector for its size:
         const size_t size = m_vecs[id]->size();
         // Only accept a non-zero size. Not sure why...
         if( size > 0 ) {
            return size;
         }
      }

      // Check if we have a transient store, and get the size from that:
      if( m_transientStore ) {
         return m_transientStore->size();
      }

      // Apparently the store is empty:
      return 0;
   }

   /// Looks up a linked variable associated with the provided `auxid`.
   /// If a linked variable exists within the current vector storage (`m_vecs`),
   /// it returns the corresponding data vector. If not found, it checks the
   /// transient store for the linked vector.
   ///
   /// @param auxid The auxiliary ID for which to retrieve the linked variable
   /// @return Pointer to the linked variable if found; otherwise, nullptr
   ///
   const SG::IAuxTypeVector* RAuxStore::linkedVector(
       SG::auxid_t auxid ) const {
      const SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();
      auxid_t linked_id = r.linkedVariable( auxid );
      guard_t guard( m_mutex1 );
      if( linked_id < m_vecs.size() ) {
         return m_vecs[linked_id];
      }
      if( m_transientStore ) {
         return CxxUtils::as_const_ptr( m_transientStore )
             ->linkedVector( auxid );
      }
      return nullptr;
   }

   /// (const version) Looks up a linked variable associated with the provided
   /// `auxid`. If a linked variable exists within the current vector storage
   /// (`m_vecs`), it returns the corresponding data vector. If not found, it
   /// checks the transient store for the linked vector.
   ///
   /// @param auxid The auxiliary ID for which to retrieve the linked variable
   /// @return Pointer to the linked variable if found; otherwise, nullptr
   ///
   SG::IAuxTypeVector* RAuxStore::linkedVector( SG::auxid_t auxid ) {
      const SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();
      auxid_t linked_id = r.linkedVariable( auxid );
      guard_t guard( m_mutex1 );
      if( linked_id < m_vecs.size() ) {
         return m_vecs[linked_id];
      }
      if( m_transientStore ) {
         return m_transientStore->linkedVector( auxid );
      }
      return nullptr;
   }

   // SG::IAuxStore functions

   /// Retrieves or creates data for a given auxiliary ID. It handles
   /// both transient and output data, ensuring that the data is properly
   /// set up and sized.
   ///
   /// @param auxid The auxiliary ID for which data is requested
   /// @param size The size of the data to be retrieved or created
   /// @param capacity The capacity of the data to be retrieved or created
   /// @return A pointer to the data for the given auxiliary ID
   ///
   void* RAuxStore::getData( auxid_t auxid, size_t size, size_t capacity ) {
      // Guard against multi-threaded execution:
      guard_t guard( m_mutex2 );

      // Remember the size:
      m_size = size;

      // Check if we want to write this variable to the output:
      if( ( !m_outNtuple ) || ( !isAuxIDSelected( auxid ) ) ) {
         // Create the store only when necessary:
         if( !m_transientStore ) {
            m_transientStore =
                new SG::AuxStoreInternal( m_structMode == kObjectStore );
            if( m_locked ) {
               m_transientStore->lock();
            }
         }
         // Let the transient store create the variable:
         size_t nids = m_transientStore->getAuxIDs().size();
         void* result = m_transientStore->getData( auxid, size, capacity );
         if( result && ( nids != m_transientStore->getAuxIDs().size() ) ) {
            m_auxIDs.insert( auxid );
         }
         // Return the address in the transient memory:
         return result;
      }

      // If the variable exists already, and this is a locked store, then
      // we are in trouble.
      if( m_locked && ( auxid < m_vecs.size() ) && m_vecs[auxid] ) {
         if( !( ( auxid < m_isDecoration.size() ) && m_isDecoration[auxid] ) ) {
            throw SG::ExcStoreLocked( auxid );
         }
      }

      // Connect this auxiliary variable just to the output:
      if( setupOutputData( auxid ).isFailure() ) {
         ::Error( "xAOD::RAuxStore::getData",
                  XAOD_MESSAGE( "Failed to set up variable %s" ),
                  SG::AuxTypeRegistry::instance().getName( auxid ).c_str() );
         return 0;
      }

      // Check whether things make sense:
      if( ( m_structMode == kObjectStore ) && ( size != 1 ) ) {
         ::Error( "xAOD::RAuxStore::getData",
                  XAOD_MESSAGE( "Field creation requested with:" ) );
         ::Error( "xAOD::RAuxStore::getData", XAOD_MESSAGE( "  name = %s" ),
                  SG::AuxTypeRegistry::instance().getName( auxid ).c_str() );
         ::Error( "xAOD::RAuxStore::getData", XAOD_MESSAGE( "  size = %i" ),
                  static_cast< int >( size ) );
         ::Error( "xAOD::RAuxStore::getData",
                  XAOD_MESSAGE( "  m_structMode = kObjectStore" ) );
         return 0;
      }

      // Make sure the variable is of the right size:
      m_vecs[auxid]->reserve( capacity );
      m_vecs[auxid]->resize( size );

      // Return the object:
      return m_vecs[auxid]->toPtr();
   }

   /// Retrieves a set of writable auxiliary IDs.
   ///
   /// @return A const reference to the set of writable auxiliary IDs
   ///
   const SG::auxid_set_t& RAuxStore::getWritableAuxIDs() const {
      return getAuxIDs();
   }

   /// Resize the arrays to a given size.
   ///
   /// @param size The size to which the arrays should be resized
   /// @return True if the resize was successful; otherwise, false
   ///
   bool RAuxStore::resize( size_t size ) {
      // Guard against multi-threaded execution:
      guard_t guard( m_mutex1 );

      // A sanity check:
      if( ( m_structMode == kObjectStore ) && ( size != 1 ) ) {
         ::Error( "xAOD::RAuxStore::resize",
                  XAOD_MESSAGE( "size = %i for single-object store" ),
                  static_cast< int >( size ) );
         return false;
      }

      // Remember the new size:
      m_size = size;

      bool nomoves = true;
      for( SG::IAuxTypeVector* v : m_vecs ) {
         if( v && !v->isLinked() ) {
            if( !v->resize( size ) ) nomoves = false;
         }
      }
      if( m_transientStore ) {
         if( !m_transientStore->resize( size ) ) nomoves = false;
      }

      return nomoves;
   }

   /// Reserves space for the auxiliary data vectors. It ensures
   /// that the vectors have enough capacity to hold the specified number of
   /// elements.
   ///
   /// @param size The number of elements to reserve space for.
   ///
   void RAuxStore::reserve( size_t size ) {
      // Guard against multi-threaded execution:
      guard_t guard( m_mutex1 );

      // A sanity check:
      if( ( m_structMode == kObjectStore ) && ( size != 1 ) ) {
         ::Error( "xAOD::RAuxStore::reserve",
                  XAOD_MESSAGE( "size = %i for single-object store" ),
                  static_cast< int >( size ) );
         return;
      }

      for( SG::IAuxTypeVector* v : m_vecs ) {
         if( v && !v->isLinked() ) {
            v->reserve( size );
         }
      }

      if( m_transientStore ) {
         m_transientStore->reserve( size );
      }

      return;
   }

   /// Shifts the elements in the auxiliary data vectors by a specified
   /// offset. It adjusts the size of the container accordingly.
   ///
   /// @param pos The position at which to start shifting.
   /// @param offs The offset by which to shift the elements.
   ///
   void RAuxStore::shift( size_t pos, ptrdiff_t offs ) {
      // Guard against multi-threaded execution:
      guard_t guard( m_mutex1 );

      // A sanity check:
      if( m_structMode == kObjectStore ) {
         ::Error( "xAOD::RAuxStore::shift",
                  XAOD_MESSAGE( "Should not have been called for single-object "
                                "store" ) );
         return;
      }

      // Adjust the size of the container:
      if( ( static_cast< std::size_t >( std::abs( offs ) ) > m_size ) &&
          ( offs < 0 ) ) {
         m_size = 0;
      } else {
         m_size += offs;
      }

      for( SG::IAuxTypeVector* v : m_vecs ) {
         if( v && !v->isLinked() ) {
            v->shift( pos, offs );
         }
      }

      if( m_transientStore ) {
         m_transientStore->shift( pos, offs );
      }

      return;
   }

   /// inserts and moves elements from another auxiliary store into this one,
   ///  starting at the specified position. It adjusts the size of the container
   ///  accordingly.
   ///
   ///  @param pos The position at which to start inserting elements.
   ///  @param other The auxiliary store from which to move elements.
   ///  @param ignore_in A set of auxiliary IDs to ignore during the move.
   ///  @return True if the move was successful, false otherwise.
   ///
   bool RAuxStore::insertMove( size_t pos, SG::IAuxStore& other,
                               const SG::auxid_set_t& ignore_in ) {
      // Guard against multi-threaded execution:
      guard_t guard( m_mutex1 );

      // A sanity check:
      if( m_structMode == kObjectStore ) {
         ::Error( "xAOD::RAuxStore::insertMove",
                  XAOD_MESSAGE( "Should not have been called for single-object "
                                "store" ) );
         return false;
      }

      bool nomove = true;
      size_t other_size = other.size();

      SG::auxid_set_t ignore = ignore_in;

      for( SG::auxid_t id : m_auxIDs ) {
         SG::IAuxTypeVector* v_dst = nullptr;
         if( id < m_vecs.size() ) v_dst = m_vecs[id];
         if( v_dst && !v_dst->isLinked() ) {
            ignore.insert( id );
            if( other.getData( id ) ) {
               void* src_ptr = other.getData( id, other_size, other_size );
               if( src_ptr ) {
                  if( !v_dst->insertMove( pos, src_ptr, 0, other_size, other ) )
                     nomove = false;
               }
            } else {
               const void* orig = v_dst->toPtr();
               v_dst->shift( pos, other_size );
               if( orig != v_dst->toPtr() ) nomove = false;
            }
         }
      }

      if( m_transientStore ) {
         if( !m_transientStore->insertMove( pos, other, ignore ) )
            nomove = false;
      }

      return nomove;
   }

   // SG::AuxStoreIO functions

   /// Retrieves the input/output data for a given auxiliary ID. It ensures
   /// that the data is properly set up and available in memory.
   ///
   /// @param auxid The auxiliary ID for which data is requested.
   /// @return A pointer to the data for the given auxiliary ID, or nullptr if
   /// the data is not available.
   ///
   const void* RAuxStore::getIOData( SG::auxid_t auxid ) const {

      // Guard against multi-threaded execution:
      guard_t guard( m_mutex1 );

      // If the variable is connected to already:
      if( fieldInfosHasAuxid( auxid ) ) {
         // Return the pointer:
         return m_fields.at( auxid ).object;
      }

      // Check if it's in the transient store:
      if( m_transientStore && m_transientStore->getAuxIDs().test( auxid ) ) {
         return m_transientStore->getIOData( auxid );
      }

      // If not, try connecting to it now:
      auto this_nc ATLAS_THREAD_SAFE =
          const_cast< RAuxStore* >( this );  // locked above
      if( !this_nc->setupInputData( auxid ).isSuccess() ) {
         // This is not actually an error condition anymore. We can end up here
         // when we decorate constant objects coming from the input file, but
         // on one event we can't set any decorations. For instance when the
         // input container is empty. In that case the object will still list
         // the auxiliary ID belonging to that decoration as being available,
         // but it really isn't.
         //
         // Later on it might be needed to tweak the logic of all of this, but
         // for now just silently returning 0 seems to do the right thing.
         return 0;
      }

      // Now we should know this variable:
      if( !fieldInfosHasAuxid( auxid ) ) {
         ::Fatal( "xAOD::RAuxStore::getIOData",
                  XAOD_MESSAGE( "Internal logic error detected" ) );
         return 0;
      }

      // Return the pointer:
      return m_fields.at( auxid ).object;
   }

   const std::type_info* RAuxStore::getIOType( SG::auxid_t auxid ) const {

      // Guard against multi-threaded execution:
      guard_t guard( m_mutex1 );

      // If the variable is connected to already:
      if( ( auxid < m_fields.size() ) &&
          ( m_fields.find( auxid ) == m_fields.end() ) ) {
         return m_fields.at( auxid ).typeInfo;
      }

      // Check if it's in the transient store:
      if( m_transientStore && m_transientStore->getAuxIDs().test( auxid ) ) {
         return m_transientStore->getIOType( auxid );
      }

      // If not, try connecting to it now:
      auto this_nc ATLAS_THREAD_SAFE =
          const_cast< RAuxStore* >( this );  // locked above
      if( !this_nc->setupInputData( auxid ).isSuccess() ) {
         ::Error( "xAOD::RAuxStore::getIOType",
                  XAOD_MESSAGE( "Couldn't connect to auxiliary variable "
                                "%i %s" ),
                  static_cast< int >( auxid ),
                  SG::AuxTypeRegistry::instance().getName( auxid ).c_str() );
         return 0;
      }

      // Now we should know this variable:
      if( ( auxid >= m_fields.size() ) ||
          ( m_fields.find( auxid ) == m_fields.end() ) ) {
         ::Fatal( "xAOD::RAuxStore::getIOType",
                  XAOD_MESSAGE( "Internal logic error detected" ) );
         return 0;
      }

      // Return the type info:
      return m_fields.at( auxid ).typeInfo;
   }

   /// Get the types(names) of variables created dynamically
   /// @return A const reference to the set of auxiliary IDs
   ///
   const SG::auxid_set_t& RAuxStore::getDynamicAuxIDs() const {

      // All the auxiliary decorations handled by this object are considered
      // dynamic:
      return getAuxIDs();
   }

   /// Selects the auxiliary attributes specified in the input set.
   ///
   /// @param attributes A set of attribute names to be selected
   ///
   void RAuxStore::selectAux( const std::set< std::string >& attributes ) {

      // Guard against multi-threaded execution:
      guard_t guard( m_mutex1 );

      m_selection.selectAux( attributes );
   }

   /// Retrieves the selected auxiliary IDs.
   ///
   /// @return A set of selected auxiliary IDs
   ///
   SG::auxid_set_t RAuxStore::getSelectedAuxIDs() const {
      // Guard against multi-threaded execution:
      guard_t guard( m_mutex1 );

      // Leave the calculation up to the internal object:
      return m_selection.getSelectedAuxIDs( m_auxIDs );
   }

   // Private functions

   /// Scans the input ntuple for auxiliary data fields and sets up
   /// the necessary structures to access them. It ensures that the input ntuple
   /// is properly scanned and the auxiliary data fields are set up.
   ///
   /// @returns <code>kTRUE</code> if successful, <code>kFALSE</code> otherwise
   ///
   StatusCode RAuxStore::scanInputNtuple() {
      // Check if an input ntuple is even available:
      if( !m_inNtuple ) {
         // It's not an error if it isn't.
         return StatusCode::SUCCESS;
      }

      // Check if the input was already scanned:
      if( m_inputScanned ) {
         return StatusCode::SUCCESS;
      }

      m_inNtuple->LoadEntry( 0 );
      for( const auto& field :
#if ROOT_VERSION_CODE >= ROOT_VERSION( 6, 33, 0 )
         m_inNtuple->GetModel().GetConstFieldZero().GetSubFields()
#else
         m_inNtuple->GetModel().GetFieldZero().GetSubFields()
#endif
         ) {
         auto fieldName = field->GetQualifiedFieldName();

         if( m_topStore && ( fieldName == m_prefix ) ) {
            // loop over the subfields of the top store
            for( const auto& subField : 
#if ROOT_VERSION_CODE >= ROOT_VERSION( 6, 33, 0 )
                                          m_inNtuple->GetModel()
                                          .GetConstField( fieldName )
                                          .GetSubFields() 
#else
                                          m_inNtuple->GetModel()
                                          .GetField( fieldName )
                                          .GetSubFields() 
#endif                   
                                            ) {
               auto fieldName = subField->GetQualifiedFieldName();
               auto auxName = fieldName.substr( fieldName.find( "." ) + 1 );

               // Skip this entry if it refers to a base class:
               if( ( auxName.rfind( "xAOD::", 0 ) == 0 ) ||
                   ( auxName.rfind( "SG::", 0 ) == 0 ) ||
                   ( auxName.rfind( "ILockable", 0 ) == 0 ) ) {
                  continue;
               }

               if( !Utils::fieldExists( fieldName, *m_inNtuple ) ) {
                  ::Fatal( "xAOD::RAuxStore::scanInputNtuple",
                           XAOD_MESSAGE( "Logic error detected" ) );
                  return StatusCode::FAILURE;
               }

               RETURN_CHECK( "xAOD::RAuxStore::scanInputNtuple",
                             setupAuxField( fieldName, auxName, kTRUE ) );
            }
            // Don't check the rest of the loop's body:
            continue;
         }

         // if fieldname doesnt start with the value of m_dynPrefix, skip
         if( fieldName.rfind( m_dynPrefix, 0 ) != 0 ) {
            continue;
         }
         if( fieldName == m_dynPrefix ) {
            ::Warning( "xAOD::RAuxStore::scanInputNtuple",
                       "Dynamic field with empty name found on container: %s",
                       m_prefix.c_str() );
            continue;
         }
         // The auxiliary property name:
         const auto auxName = fieldName.substr( fieldName.find( ":" ) + 1 );
         // Leave the rest up to the function that is shared with the
         // dynamic fields:
         RETURN_CHECK( "xAOD::RAuxStore::scanInputNtuple",
                       setupAuxField( fieldName, auxName, kFALSE ) );
      }
      // the input was successfully scanned:
      m_inputScanned = kTRUE;

      return StatusCode::SUCCESS;
   }

   /// This internal function takes care of connecting to an individual
   /// (sub-)field in the input file for a given auxiliary variable.
   ///
   /// @param auxid The ID of the variable to connect to
   /// @returns <code>kTRUE</code> if successful, <code>kFALSE</code> otherwise
   ///
   StatusCode RAuxStore::setupInputData( auxid_t auxid ) {
      // Return right away if we already know that the field is not found:
      if( fieldInfosHasAuxid( auxid ) &&
          m_fields[auxid].status == RAuxStore::RFieldInfo::NotFound ) {
         return StatusCode::RECOVERABLE;
      }

      // Make sure the internal storage is large enough:
      if( m_vecs.size() <= auxid ) {
         m_vecs.resize( auxid + 1 );
      }

      // Check if we need to do anything:
      if( m_vecs[auxid] && fieldInfosHasAuxid( auxid ) ) {
         return StatusCode::SUCCESS;
      }

      // A little sanity check:
      if( !m_inNtuple ) {
         ::Error( "xAOD::RAuxStore::setupInputData",
                  XAOD_MESSAGE( "No input RNTuple set up!" ) );
         return StatusCode::FAILURE;
      }

      // Another sanity check:
      if( m_vecs[auxid] || fieldInfosHasAuxid( auxid ) ) {
         ::Error( "xAOD::RAuxStore::setupInputData",
                  XAOD_MESSAGE( "The internal variables of the object got "
                                "messed up?!?" ) );
         return StatusCode::FAILURE;
      }

      const SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();

      // Get the property name:
      const TString statFieldName = m_prefix + r.getName( auxid ).c_str();
      const TString dynFieldName = m_dynPrefix + r.getName( auxid ).c_str();

      // Check if the field exists:
      TString fieldName = statFieldName;
#if ROOT_VERSION_CODE >= ROOT_VERSION( 6, 33, 0 )
      std::unique_ptr< RNTupleView< void > >
#else
      std::unique_ptr< RNTupleView< void, true > >
#endif
      field;
      if( !Utils::fieldExists( statFieldName.Data(), *m_inNtuple ) ) {
         if( !Utils::fieldExists( dynFieldName.Data(), *m_inNtuple ) ) {
            m_fields[auxid].status = RAuxStore::RFieldInfo::NotFound;
            return StatusCode::RECOVERABLE;
         }
         // We have a dynamic field:
         fieldName = dynFieldName;
      }

      // Check if it's a "primitive field":
      const Bool_t primitiveField = isPrimitiveField( fieldName.Data() );
      // Check if it's a "container field":
      const Bool_t containerField =
          ( primitiveField ? kFALSE
                           : isContainerField( fieldName.Data(), auxid ) );

      // Set the structure mode if it has not been defined externally:
      if( m_structMode == kUndefinedStore ) {
         m_structMode = ( containerField ? kContainerStore : kObjectStore );
      }

      // Check that the field type makes sense:
      if( ( containerField && ( m_structMode != kContainerStore ) &&
            !r.isLinked( auxid ) ) ||
          ( ( !containerField ) && ( m_structMode != kObjectStore ) ) ) {
         ::Error( "xAOD::RAuxStore::setupInputData",
                  XAOD_MESSAGE( "Field type and requested structure mode "
                                "differ for field: %s" ),
                  fieldName.Data() );
         return StatusCode::FAILURE;
      }
      // Get the field's type:
      auto inspector =
          RNTupleInspector::Create( m_inputNtupleName, m_inFileName );
      const auto & fieldInspector = inspector->GetFieldTreeInspector(
          Utils::getFirstFieldMatch( *m_inNtuple, fieldName.Data() ) );

      std::string typeName =
          fieldInspector.GetDescriptor().GetTypeName().c_str();
      const std::type_info* ti = nullptr;
      const auto tClass = ::TClass::GetClass(
          fieldInspector.GetDescriptor().GetTypeName().c_str() );
      ti = tClass->GetTypeInfo();
      TClass* clDummy = TClass::GetClass( *ti );

      ::EDataType dType = kOther_t;  // Default to a generic type
      dType = TDataType::GetType( *ti );

      // Get the property type:
      const std::type_info* fieldType = 0;
      if( isRegisteredType( auxid ) ) {
         // Get the type from the auxiliary type registry:
         fieldType =
             ( containerField ? r.getVecType( auxid ) : r.getType( auxid ) );
      } else {
         // Get the type from the input field itself:
         fieldType = ( clDummy ? clDummy->GetTypeInfo()
                               : &( Utils::getTypeInfo( dType ) ) );
      }
      if( !fieldType ) {
         ::Error( "xAOD::RAuxStore::setupInputData",
                  XAOD_MESSAGE( "Can't read/copy variable %s (%s)" ),
                  fieldName.Data(), clDummy->GetName() );
         return StatusCode::RECOVERABLE;
      }
      const TString fieldTypeName = Utils::getTypeName( *fieldType ).c_str();

      // Check if we have the needed dictionary for an object field:
      ::TClass* fieldClass = 0;
      if( !primitiveField ) {
         // Get the property's class:
         fieldClass = ::TClass::GetClass( *fieldType, kTRUE, kTRUE );
         if( !fieldClass ) {
            fieldClass = ::TClass::GetClass( fieldTypeName );
         }
         if( !fieldClass ) {
            ::Error( "xAOD::RAuxStore::setupInputData",
                     XAOD_MESSAGE( "No dictionary available for class \"%s\"" ),
                     fieldTypeName.Data() );
            return StatusCode::FAILURE;
         }
      }

      // Create the smart object holding this vector:
      if( isRegisteredType( auxid ) ) {
         m_vecs[auxid] = r.makeVector( auxid, (size_t)0, (size_t)0 ).release();
         if( !containerField ) {
            m_vecs[auxid]->resize( 1 );
         }
         if( clDummy &&
             strncmp( clDummy->GetName(), "SG::PackedContainer<", 20 ) == 0 ) {
            SG::IAuxTypeVector* packed = m_vecs[auxid]->toPacked().release();
            std::swap( m_vecs[auxid], packed );
            delete packed;
         }
      } else {
         ::Error( "xAOD::RAuxStore::setupInputData",
                  XAOD_MESSAGE( "Couldn't create in-memory vector for "
                                "variable %s (%i)" ),
                  fieldName.Data(), static_cast< int >( auxid ) );
         return StatusCode::FAILURE;
      }

      if( m_inModel == nullptr ) {
         m_inModel = RNTupleModel::Create();
      }

      if( primitiveField ) {
         auto fieldResult = RFieldBase::Create( fieldName.Data(),
                                                fieldTypeName.Data() );
         if( fieldResult ) {
            auto field = fieldResult.Unwrap();
            m_inModel->AddField( std::move( field ) );
         }
      } else {
         RETURN_CHECK(
             "xAOD::RAuxStore::setupInputData",
             addVectorField( fieldName.Data(), fieldTypeName.Data(), kTRUE ) );
      }

      auto file = ::TFile::Open( m_inFileName.c_str(), "UPDATE" );

      // Inside it's own scope to ensure it is not open when we try to read from
      // it
      {
         auto ntuple = RNTupleWriter::Append(
             std::move( m_inModel ), m_inputNtupleName, *file );
         ntuple->Fill();
      }

      // Since we just moved the model, we need to reset it
      m_inModel = RNTupleModel::Create();
      RETURN_CHECK(
          "xAOD::RAuxStore::setupInputData",
          addVectorField( fieldName.Data(), fieldTypeName.Data(), kTRUE ) );

      m_fields[auxid].field = 
#if ROOT_VERSION_CODE >= ROOT_VERSION( 6, 33, 0 )
      std::make_unique< RNTupleView< void > > (
#else
      std::make_unique< RNTupleView< void, true > >(
#endif
         m_inNtuple->GetView< void >( fieldName.Data(), nullptr ) 
      );
      m_fields[auxid].status = RAuxStore::RFieldInfo::Initialized;
      m_fields[auxid].fieldTypeName = fieldTypeName.Data();
      m_fields[auxid].ntupleName = m_inputNtupleName;
      m_fields[auxid].object = ( containerField ? m_vecs[auxid]->toVector()
                                                : m_vecs[auxid]->toPtr() );
      m_fields[auxid].typeInfo = const_cast< std::type_info* >( fieldType );

      // Remember which variable got created:
      m_auxIDs.insert( auxid );

      // Check if we just replaced a generic object:
      if( isRegisteredType( auxid ) ) {
         // The name of the variable we just created:
         const std::string auxname = r.getName( auxid );
         // Check if there's another variable with this name already:
         for( auxid_t i = 0; i < m_vecs.size(); ++i ) {
            // Check if we have this aux ID:
            if( !m_vecs[i] ) continue;
            // Ingore the object that we *just* created:
            if( i == auxid ) continue;
            // The name of the variable:
            const std::string testname = r.getName( i );
            // Check if it has the same name:
            if( testname != auxname ) continue;
            // Check that the other one is a non-registered type:
            if( isRegisteredType( i ) ) {
               ::Error( "xAOD::RAuxStore::setupInputData",
                        XAOD_MESSAGE( "Internal logic error!" ) );
               continue;
            }
            // Okay, we do need to remove this object:
            delete m_vecs[i];
            m_vecs[i] = 0;
            m_auxIDs.erase( i );
         }
      }

      SG::auxid_t linked_auxid = r.linkedVariable( auxid );
      if( linked_auxid != SG::null_auxid ) {
         return setupInputData( linked_auxid );
      }

      // Return gracefully:
      return StatusCode::SUCCESS;
   }

   /// This function is used internally to create an output field
   /// with the contents of a single auxiliary variable.
   ///
   /// @param auxid The ID of the variable to create an output field for
   /// @returns <code>kTRUE</code> if the operation was successful,
   ///          <code>kFALSE</code> if not
   ///
   StatusCode RAuxStore::setupOutputData( auxid_t auxid ) {
      // Check whether we need to do anything:
      if( !m_outNtuple ) return StatusCode::SUCCESS;
      // Check if the variable needs to be written out:
      if( !isAuxIDSelected( auxid ) ) return StatusCode::SUCCESS;

      // Make sure that containers are large enough:
      if( m_vecs.size() <= auxid ) {
         m_vecs.resize( auxid + 1 );
      }

      // Check if the variable was put into the transient store as a
      // decoration, and now needs to be put into the output file:
      if( ( !m_vecs[auxid] ) && m_transientStore &&
          ( m_transientStore->getAuxIDs().test( auxid ) ) ) {

         // Get the variable from the transient store:
         const void* pptr = m_transientStore->getData( auxid );
         if( !pptr ) {
            ::Fatal( "xAOD::RAuxStore::setupOutputData",
                     XAOD_MESSAGE( "Internal logic error detected" ) );
            return StatusCode::FAILURE;
         }

         // The registry:
         SG::AuxTypeRegistry& reg = SG::AuxTypeRegistry::instance();

         // Inside it's own scope to ensure that the lock is released
         {
            std::lock_guard<std::mutex> lock(m_mutex1);
            // Create the new object:
            m_vecs[auxid] = reg.makeVector(auxid, m_size, m_size).release();
         }
         void* ptr = m_vecs[auxid]->toPtr();
         if( !ptr ) {
            ::Error( "xAOD::RAuxStore::setupOutputData",
                     XAOD_MESSAGE( "Couldn't create decoration in memory "
                                   "for writing" ) );
            return StatusCode::FAILURE;
         }

         // Get the type of this variable:
         const std::type_info* type = reg.getType( auxid );

         if( !type ) {
            ::Error( "xAOD::RAuxStore::setupOutputData",
                     XAOD_MESSAGE( "Couldn't get the type of transient "
                                   "variable %i" ),
                     static_cast< int >( auxid ) );
            return StatusCode::FAILURE;
         }
         // Now get the factory for this variable:
         const SG::IAuxTypeVectorFactory* factory = reg.getFactory( auxid );
         if( !factory ) {
            ::Error( "xAOD::RAuxStore::setupOutputData",
                     XAOD_MESSAGE( "No factory found for transient variable "
                                   "%i" ),
                     static_cast< int >( auxid ) );
            return StatusCode::FAILURE;
         }

         // Finally, do the copy:
         factory->copy( auxid, SG::AuxVectorInterface( *this ), 0,
                        SG::AuxVectorInterface( *m_transientStore ), 0,
                        m_size );

         // And now remember that this is a decoration:
         if( m_isDecoration.size() <= auxid ) {
            m_isDecoration.resize( auxid + 1 );
         }
         m_isDecoration[auxid] = ::kTRUE;
      }

      // Check if we know about this variable to be on the input,
      // but haven't connected to it yet:
      if( ( m_auxIDs.test( auxid ) ) && ( !m_vecs[auxid] ) &&
          ( !fieldInfosHasAuxid( auxid ) ) ) {
         RETURN_CHECK( "xAOD::RAuxStore::setupOutputData",
                       setupInputData( auxid ) );
      }

      // Check that we know the store's type:
      if( ( m_structMode != kContainerStore ) &&
          ( m_structMode != kObjectStore ) ) {
         ::Error( "xAOD::RAuxStore::setupOutputData",
                  XAOD_MESSAGE( "Structure mode unknown for variable %s" ),
                  SG::AuxTypeRegistry::instance().getName( auxid ).c_str() );
         return StatusCode::FAILURE;
      }

      // Check if the variable exists already in memory:
      if( !m_vecs[auxid] ) {
         m_vecs[auxid] = SG::AuxTypeRegistry::instance()
                             .makeVector( auxid, (size_t)0, (size_t)0 )
                             .release();
         if( m_structMode == kObjectStore ) {
            m_vecs[auxid]->resize( 1 );
         }
      }
      if( !m_outNtuple ) {
      }

      // if we have not called this method before, the model will be null
      if( m_outModel == nullptr ) {
         m_outModel = RNTupleModel::Create();
      }

      const TString fieldName =
          m_dynPrefix + SG::AuxTypeRegistry::instance().getName( auxid );
      const std::type_info* fieldType =
          m_structMode == kContainerStore
              ? SG::AuxTypeRegistry::instance().getVecType( auxid )
              : SG::AuxTypeRegistry::instance().getType( auxid );
      const TString fieldTypeName = Utils::getTypeName( *fieldType ).c_str();
      // If it is a primitive field, it only has 1 character (e.g. 'f' for
      // float)
      const Bool_t isPrimitiveField = ( strlen( fieldTypeName.Data() ) == 1 );

      if( isPrimitiveField ) {
         auto fieldResult = RFieldBase::Create( fieldName.Data(),
                                                fieldTypeName.Data() );
         if( fieldResult ) {
            auto field = fieldResult.Unwrap();
            m_outModel->AddField( std::move( field ) );
         }
      } else {
         RETURN_CHECK( "xAOD::RAuxStore::setupOutputData",
                       addVectorField( fieldName.Data(),
                                       fieldTypeName.Data(), kFALSE ) );
      }

      auto file = ::TFile::Open( m_outFileName.c_str(), "UPDATE" );

      // Inside it's own scope to ensure it is not open when we try to read from
      // it
      {
         auto ntuple =
             RNTupleWriter::Append( std::move( m_outModel ),
                                    m_outputNtupleName, *file );
         ntuple->Fill();
      }

      // Since we just moved the model, we need to reset it
      m_outModel = RNTupleModel::Create();
      RETURN_CHECK( "xAOD::RAuxStore::setupOutputData",
                    addVectorField( fieldName.Data(),
                                    fieldTypeName.Data(), kFALSE ) );
      auto ntuple = RNTupleReader::Open( m_outputNtupleName, m_outFileName );
      m_fields[auxid].field = 
#if ROOT_VERSION_CODE >= ROOT_VERSION( 6, 33, 0 )
      std::make_unique< RNTupleView< void > >
#else
      std::make_unique< RNTupleView< void, true > >
#endif
      ( ntuple->GetView< void >( fieldName, nullptr ) );
      m_fields[auxid].status = RAuxStore::RFieldInfo::Initialized;
      m_fields[auxid].fieldTypeName = fieldTypeName.Data();
      m_fields[auxid].ntupleName = m_inputNtupleName;
      m_fields[auxid].object =
          ( m_structMode == kObjectStore ? m_vecs[auxid]->toVector()
                                         : m_vecs[auxid]->toPtr() );
      m_fields[auxid].typeInfo = const_cast< std::type_info* >( fieldType );

      // Also, remember that we now handle this variable:
      m_auxIDs.insert( auxid );
      // We were successful:
      return StatusCode::SUCCESS;
   }

   /// This function retrieves the type information for a given auxiliary field.
   /// It uses the field's inspector to determine the type and handles cases
   /// where the expected class or collection proxy is not available.
   ///
   /// @param auxName The name of the auxiliary field.
   /// @param isStaticField <code>kTRUE</code> if this is a static field, and
   ///                     <code>kFALSE</code> if it's a dynamic one
   ///
   const std::type_info* RAuxStore::auxFieldType( const std::string& auxName,
                                                  ::Bool_t isStaticField ) {

      auto inspector =
          RNTupleInspector::Create( m_inputNtupleName, m_inFileName );
      auto fieldName = Utils::getFirstFieldMatch( *m_inNtuple, auxName );
      const auto& fieldInspector = inspector->GetFieldTreeInspector( fieldName );

      std::string typeName =
          fieldInspector.GetDescriptor().GetTypeName().c_str();
      const auto expectedClass = ::TClass::GetClass(
          fieldInspector.GetDescriptor().GetTypeName().c_str() );

      if( ( !expectedClass ) && ( m_structMode == kUndefinedStore ) ) {
         m_structMode = kObjectStore;
      }

      const std::type_info* ti = 0;
      if( m_structMode == kObjectStore ) {
         if( expectedClass ) {
            ti = expectedClass->GetTypeInfo();
         } else {
            ti = &( Utils::getTypeInfo( kOther_t ) );
         }
      } else {
         if( !expectedClass ) {
            if( ( !isStaticField ) ||
                ( strncmp( auxName.c_str(), "m_", 2 ) != 0 ) ) {
               ::Warning( "xAOD::RAuxStore::auxFieldType",
                          "Couldn't get the type of field \"%s\"",
                          fieldName.c_str() );
            }
         } else {
            ::TVirtualCollectionProxy* prox =
                expectedClass->GetCollectionProxy();

            if( !prox ) {
               TClass* cl2 = lookupVectorType( expectedClass );
               if( cl2 ) prox = cl2->GetCollectionProxy();
            }

            if( !prox ) {
               if( ( !isStaticField ) ||
                   ( strncmp( auxName.c_str(), "m_", 2 ) != 0 ) ) {
                  ::Warning( "xAOD::RAuxStore::auxFieldType",
                             "Couldn't get the type of field \"%s\"",
                             fieldName.c_str() );
               }
            } else {
               if( prox->GetValueClass() ) {
                  ti = prox->GetValueClass()->GetTypeInfo();
               } else {
                  ti = &( Utils::getTypeInfo( prox->GetType() ) );
               }
            }
         }
      }
      return ti;
   }

   /// This function sets up an auxiliary field by determining its type and
   /// attempting to register it with the auxiliary type registry.
   /// If the field type is not known, it tries to create a factory for the
   /// field's type. The function handles both static and dynamic fields and
   /// updates the set of known auxiliary IDs upon success.
   /// @param fieldName The name of the field in the ntuple
   /// @param auxName The name of the auxiliary field
   /// @param isStaticField <code>kTRUE</code> if this is a static field, and
   ///                     <code>kFALSE</code> if it's a dynamic one
   /// @returns <code>kTRUE</code> if successful, <code>kFALSE</code> if not
   ///
   StatusCode RAuxStore::setupAuxField( const std::string & fieldName,
                                        const std::string & auxName,
                                        ::Bool_t isStaticField ) {

      std::string expectedClassName;
      const std::type_info* ti = auxFieldType( auxName, isStaticField );

      // Get the registry:
      SG::AuxTypeRegistry& registry = SG::AuxTypeRegistry::instance();

      // Check if the registry already knows this variable name. If yes, let's
      // use the type known by the registry. To be able to deal with simple
      // schema evolution in dynamic fields.
      const auxid_t regAuxid = registry.findAuxID( auxName );
      if( regAuxid != SG::null_auxid ) {
         m_auxIDs.insert( regAuxid );
         return StatusCode::SUCCESS;
      }

      // If we didn't find a type_info for the field
      if( !ti ) {
         return StatusCode::SUCCESS;
      }

      SG::AuxVarFlags flags = SG::AuxVarFlags::SkipNameCheck;
      SG::auxid_t linkedAuxId = SG::null_auxid;

      if( SG::AuxTypeRegistry::isLinkedName( auxName ) ) {
         flags |= SG::AuxVarFlags::Linked;
      } else if( SG::AuxTypeRegistry::classNameHasLink( expectedClassName ) ) {
         std::string linkedAttr = SG::AuxTypeRegistry::linkedName( auxName );
         std::string linkedFieldName =
             SG::AuxTypeRegistry::linkedName( fieldName );
         const std::type_info* linkedTi = nullptr;
         if( Utils::fieldExists(  std::move(linkedFieldName), *m_inNtuple ) ) {
            linkedTi = auxFieldType( linkedAttr.c_str(), isStaticField );
         }
         if( linkedTi ) {
            linkedAuxId = registry.getAuxID(
                *linkedTi, linkedAttr, "",
                SG::AuxVarFlags::SkipNameCheck | SG::AuxVarFlags::Linked );
         }
         if( linkedAuxId == SG::null_auxid ) {
            ::Error( "xAOD::RAuxStore::setupAuxField",
                     "Could not find linked variable for %s type %s",
                     auxName.c_str(), expectedClassName.c_str() );
         }
      }

      // Check for an auxiliary ID for this field:
      auxid_t auxid = registry.getAuxID( *ti, auxName, "", flags, linkedAuxId );

      // First try to find a compiled factory for the vector type:
      if( auxid == SG::null_auxid ) {
         // Construct the name of the factory's class:
         // But be careful --- if we don't exactly match the name
         // in TClassTable, then we may trigger autoparsing.  Besides the
         // resource usage that implies, that can lead to crashes in dbg
         // builds due to cling bugs.
         std::string typeName = Utils::getTypeName( *ti );
         if( typeName.starts_with( "std::vector<" ) ) typeName.erase( 0, 5 );
         std::string factoryClassName =
             "SG::AuxTypeVectorFactory<" + typeName + ",allocator<" + typeName;
         if( factoryClassName[factoryClassName.size() - 1] == '>' ) {
            factoryClassName += ' ';
         }
         factoryClassName += "> >";

         // Look for the dictionary of this type:
         ::TClass* factoryClass = TClass::GetClass( factoryClassName.c_str() );
         if( factoryClass && factoryClass->IsLoaded() ) {
            ::TClass* baseClass =
                ::TClass::GetClass( "SG::IAuxTypeVectorFactory" );
            if( baseClass && baseClass->IsLoaded() ) {
               const Int_t offset =
                   factoryClass->GetBaseClassOffset( baseClass );
               if( offset >= 0 ) {
                  void* factoryVoidPointer = factoryClass->New();
                  if( factoryVoidPointer ) {
                     unsigned long tmp = reinterpret_cast< unsigned long >(
                                             factoryVoidPointer ) +
                                         offset;
                     SG::IAuxTypeVectorFactory* factory =
                         reinterpret_cast< SG::IAuxTypeVectorFactory* >( tmp );
                     registry.addFactory(
                         *ti, *factory->tiAlloc(),
                         std::unique_ptr< SG::IAuxTypeVectorFactory >(
                             factory ) );
                     auxid = registry.getAuxID( *ti, auxName, "", flags,
                                                linkedAuxId );
                  }
               }
            }
         }
      }

      // If that didn't succeed, let's assign a generic factory to this type:
      if( auxid == SG::null_auxid && linkedAuxId == SG::null_auxid ) {
         // Construct the name of the vector type:
         std::string vectorClassName =
             "std::vector<" + Utils::getTypeName( *ti );
         if( vectorClassName[vectorClassName.size() - 1] == '>' ) {
            vectorClassName += ' ';
         }
         vectorClassName += '>';

         // Get the dictionary for the type:
         ::TClass* vectorClass = ::TClass::GetClass( vectorClassName.c_str() );
         if( vectorClass && vectorClass->IsLoaded() ) {
            auto factory = std::make_unique< TAuxVectorFactory >( vectorClass );
            if( factory->tiAlloc() ) {
               const std::type_info* tiAlloc = factory->tiAlloc();
               registry.addFactory( *ti, *tiAlloc, std::move( factory ) );
            } else {
               std::string tiAllocName = factory->tiAllocName();
               registry.addFactory( *ti, tiAllocName, std::move( factory ) );
            }
            auxid = registry.getAuxID( *ti, auxName, "",
                                       SG::AuxVarFlags::SkipNameCheck );
         } else {
            ::Warning( "xAOD::RAuxStore::setupAuxField",
                       "Couldn't find dictionary for type: %s",
                       vectorClassName.c_str() );
         }
      }
      // Check if we succeeded:
      if( auxid == SG::null_auxid ) {
         if( linkedAuxId != SG::null_auxid ) {
            ::Error(
                "xAOD::RAuxStore::setupAuxField",
                XAOD_MESSAGE( "Dynamic ROOT vector factory not implemented for "
                              "linked types; field "
                              "\"%s\"" ),
                fieldName.c_str() );
         } else {
            ::Error( "xAOD::RAuxStore::setupAuxField",
                     XAOD_MESSAGE( "Couldn't assign auxiliary ID to field "
                                   "\"%s\"" ),
                     fieldName.c_str() );
         }
         return StatusCode::FAILURE;
      }

      // Remember the auxiliary ID:
      m_auxIDs.insert( auxid );

      return StatusCode::SUCCESS;
   }

   /// This function checks whether a given auxiliary ID is selected
   ///
   /// @param auxid The auxiliary ID that should be checked
   /// @returns <code>kTRUE</code> if the variable needs to be written out,
   ///          <code>kFALSE</code> if not
   ///
   ::Bool_t RAuxStore::isAuxIDSelected( auxid_t auxid ) const {

      // A temporary object:
      auxid_set_t auxids;
      auxids.insert( auxid );

      // Check if the auxid is returned as a selected ID:
      return m_selection.getSelectedAuxIDs( auxids ).size();
   }

   /// This function is used to test if a given auxiliary variable is known
   /// in the registry with a proper type.
   ///
   /// @param auxid The auxiliary ID to check
   /// @returns <code>kTRUE</code> if the full type of the auxiliary property
   ///          is known, <code>kFALSE</code> otherwise
   ///
   ::Bool_t RAuxStore::isRegisteredType( auxid_t auxid ) {

      // Cache some data:
      static SG::AuxTypeRegistry& registry ATLAS_THREAD_SAFE =
          SG::AuxTypeRegistry::instance();
      static const auxid_t sauxid =
          registry.getAuxID< SG::AuxTypePlaceholder >( "AuxTypePlaceholder" );
      static const std::type_info* const sti = registry.getVecType( sauxid );

      // Check if the types match:
      return ( *sti != *( registry.getVecType( auxid ) ) );
   }

   /// This function checks whether a given field (determined by the fieldName)
   /// corresponds to a container or a single element based on its type
   /// information
   ///
   /// @param fieldName The name of the field to check.
   /// @param auxid The auxiliary ID of the variable to check.
   /// @return <code>kTRUE</code> if the field represents a container,
   /// code>kFALSE</code> otherwise.
   ///
   ::Bool_t RAuxStore::isContainerField( std::string fieldName,
                                         auxid_t auxid ) const {
      // For unknown types it doesn't matter if the field describes a
      // container or a single element.
      if( !isRegisteredType( auxid ) ) return kTRUE;

      auto inspector =
          RNTupleInspector::Create( m_inputNtupleName, m_inFileName );
      const auto & fieldInspector = inspector->GetFieldTreeInspector( fieldName );

      const std::string & typeName =
          fieldInspector.GetDescriptor().GetTypeName();

      const auto tClass = ::TClass::GetClass(
          typeName.c_str() );
      const std::type_info* ti = tClass->GetTypeInfo();

      TClass* cl = TClass::GetClass( *ti );

      // If there is no class associated with the field then it should be
      // a field describing a standalone object. (As it should be a
      // "primitive" field in this case.)
      if( !cl ) {
         return kFALSE;
      }

      // If there is a class, ask for the type_info of its type:
      const std::type_info* root_ti = cl->GetTypeInfo();
      if( !root_ti ) {
         // Type name may be saved as "basic_string<char>" rather than "string"
         // by Athena I/O.
         ::TString typeName( cl->GetName() );
         typeName.ReplaceAll( "basic_string<char>", "string" );
         ::TClass* newCl = ::TClass::GetClass( typeName );
         if( newCl ) {
            root_ti = newCl->GetTypeInfo();
         }
      }
      if( !root_ti ) {
         ::Error( "xAOD::RAuxStore::isContainerField",
                  XAOD_MESSAGE( "Couldn't get an std::type_info object out of "
                                "field \"%s\" of type \"%s\"" ),
                  m_prefix.c_str(), cl->GetName() );
         return kFALSE;
      }

      // Ask for the auxiliary type infos:
      const std::type_info* aux_obj_ti =
          SG::AuxTypeRegistry::instance().getType( auxid );
      if( !aux_obj_ti ) {
         ::Error( "xAOD::RAuxStore::isContainerField",
                  XAOD_MESSAGE( "Couldn't get std::type_info object for "
                                "auxiliary id: %i" ),
                  static_cast< int >( auxid ) );
         return kFALSE;
      }
      const std::type_info* aux_vec_ti =
          SG::AuxTypeRegistry::instance().getVecType( auxid );
      if( !aux_vec_ti ) {
         ::Error( "xAOD::RAuxStore::isContainerField",
                  XAOD_MESSAGE( "Couldn't get std::type_info object for "
                                "auxiliary id: %i" ),
                  static_cast< int >( auxid ) );
         return kFALSE;
      }

      // Check which one the ROOT type info agrees with:
      if( *root_ti == *aux_obj_ti ) {
         // This field describes a single object:
         return kFALSE;
      } else if( *root_ti == *aux_vec_ti ) {
         // This field describes a container of objects:
         return kTRUE;
      }

      // For enum and vector<enum> types (PFO...) the type given by
      // the aux type registry is vector<int>. We have to take it into account
      // here...
      if( cl->GetCollectionProxy() &&
          ( *aux_vec_ti == typeid( std::vector< int > ) ) ) {
         return kTRUE;
      }

      TClass* cl2 = lookupVectorType( cl );
      if( cl2 ) {
         if( *cl2->GetTypeInfo() == *aux_vec_ti ) return kTRUE;
      }

      // If we got this far, the field may have undergone schema evolution. If
      // it's one that ROOT can deal with itself, then we should still be able
      // to read the field with this code.
      TClass* aux_vec_cl =
          TClass::GetClass( Utils::getTypeName( *aux_vec_ti ).c_str() );
      if( aux_vec_cl &&
          aux_vec_cl->GetConversionStreamerInfo( cl, cl->GetClassVersion() ) ) {
         return kTRUE;
      }
      TClass* aux_obj_cl =
          TClass::GetClass( Utils::getTypeName( *aux_obj_ti ).c_str() );
      if( aux_obj_cl &&
          aux_obj_cl->GetConversionStreamerInfo( cl, cl->GetClassVersion() ) ) {
         return kFALSE;
      }

      // If neither, then something went wrong...
      ::Error( "xAOD::RAuxStore::isContainerField",
               XAOD_MESSAGE( "Couldn't determine if field describes a single "
                             "object or a container" ) );
      ::Error( "xAOD::RAuxStore::isContainerField",
               XAOD_MESSAGE( "ROOT type  : %s" ),
               Utils::getTypeName( *root_ti ).c_str() );
      ::Error( "xAOD::RAuxStore::isContainerField",
               XAOD_MESSAGE( "Object type: %s" ),
               Utils::getTypeName( *aux_obj_ti ).c_str() );
      ::Error( "xAOD::RAuxStore::isContainerField",
               XAOD_MESSAGE( "Vector type: %s" ),
               Utils::getTypeName( *aux_vec_ti ).c_str() );
      return kFALSE;
   }

   /// This function checks whether the given field corresponds to a primitive
   /// data type based on its type information and the auxiliary ID provided.
   ///
   /// @param fieldName The name of the field to check.
   /// @returns <code>kTRUE</code> if the field describes a primitive variable,
   ///          <code>kFALSE</code> otherwise
   ///
   ::Bool_t RAuxStore::isPrimitiveField( std::string fieldName ) {
      auto inspector =
          RNTupleInspector::Create( m_inputNtupleName, m_inFileName );
      const auto& fieldInspector = inspector->GetFieldTreeInspector( fieldName );

      std::string typeName =
          fieldInspector.GetDescriptor().GetTypeName().c_str();
      const std::type_info* ti = nullptr;
      const auto tClass = ::TClass::GetClass(
          fieldInspector.GetDescriptor().GetTypeName().c_str() );
      ti = tClass->GetTypeInfo();
      ::EDataType dType = kOther_t;  // Default to a generic type
      dType = TDataType::GetType( *ti );

      // The check is made using the data type variable:
      return ( ( dType != kOther_t ) && ( dType != kNoType_t ) &&
               ( dType != kVoid_t ) );
   }

   /// This function creates a base field using the provided field name and type
   /// name. Depending on whether the field is for the input file or the output
   /// file, it adds the field to the appropriate ntuple model.
   ///
   /// @param fieldName The name of the field to be added.
   /// @param fieldTypeName The type name of the field to be added.
   /// @param isInputFile A boolean flag indicating whether the field is for the
   /// input file.
   /// @return StatusCode::SUCCESS if the field is added successfully,
   /// StatusCode::FAILURE otherwise.
   ///
   StatusCode RAuxStore::addVectorField( std::string fieldName,
                                         std::string fieldTypeName,
                                         ::Bool_t isInputFile ) {
      auto baseFieldResult = RFieldBase::Create( fieldName, fieldTypeName );
      if( baseFieldResult ) {
         auto baseField = baseFieldResult.Unwrap();
         if( isInputFile ) {
            if( m_inModel != nullptr ) {
               m_inModel->AddField( std::move( baseField ) );
            } else {
               return StatusCode::FAILURE;
            }
         } else {
            if( m_outModel != nullptr ) {
               m_outModel->AddField( std::move( baseField ) );
            } else {
               return StatusCode::FAILURE;
            }
         }
      }
      return StatusCode::SUCCESS;
   }

   /// Checks if the fieldInfos map has an entry for the given auxid.
   ///
   /// @param auxid The auxid to check for in the fieldInfos map.
   ///
   /// @return <code>kTRUE</code> if the fieldInfos map has an entry for the
   /// given auxid, <code>kFALSE</code> otherwise.
   ::Bool_t RAuxStore::fieldInfosHasAuxid( auxid_t auxid ) const {
      return !( m_fields.find( auxid ) == m_fields.end() );
   }

   RAuxStore::RFieldInfo::RFieldInfo()
       : status( NotInitialized ),
         fieldName(),
         fieldTypeName(),
         field( nullptr ),
         ntupleName(),
         entryToLoad( -1 ),
         entryLoaded( -1 ) {}

   ::Int_t RAuxStore::RFieldInfo::getEntry() {

      // A little sanity check:
      if( !field ) {
         // This is no longer an error. We can have such objects for
         // decorations, which don't exist on the input.
         return 0;
      }

      if( ntupleName == "" ) {
         Error( "xAOD::RAuxStore::RFieldInfo::getEntry",
                XAOD_MESSAGE( "field=%s has not defined a name of the ntuple "
                              "it comes from" ),
                ntupleName.c_str() );
         return -1;
      }
      if( entryToLoad < 0 ) {
         Error( "xAOD::RAuxStore::RFieldInfo::getEntry",
                XAOD_MESSAGE(
                    "Entry to read is not set for field=%s from ntuple=%s. " ),
                fieldName.c_str(), ntupleName.c_str() );
         return -1;
      }

      // Check if anything needs to be done:
      if( entryLoaded == entryToLoad ) {
         return 0;
      }

      try {
         auto obj = field->GetField().CreateObject< void >().release();
         field->BindRawPtr( obj );
         // Load the entry
         ( *field )( entryToLoad );

      } catch( ROOT::Experimental::RException& e ) {
         ::Error( "xAOD::RAuxStore::RFieldInfo::getEntry",
                  "Failed to load entry %d for field %s: %s", entryToLoad,
                  fieldName.c_str(), e.what() );
         return -1;
      }

      return 1;
   }

}  // namespace xAOD
