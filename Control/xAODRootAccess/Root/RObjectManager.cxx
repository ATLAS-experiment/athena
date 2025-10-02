/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


// ROOT include(s):
#include <TBranch.h>
#include <TTree.h>
#include <TError.h>

// Local include(s):
#include "xAODRootAccess/tools/RObjectManager.h"
#include "xAODRootAccess/tools/THolder.h"
#include "xAODRootAccess/tools/Message.h"

namespace xAOD {

   namespace Experimental {

   RObjectManager::RObjectManager( ROOT::RNTupleView<void> field,::Long64_t& entry, THolder* holder, ::Bool_t renewOnRead )
      :
        m_field( std::move( field ) ),
        m_holder( holder ), 
        m_entryToLoad( entry ), m_entry ( -1 ), m_isSet( kTRUE ), m_renewOnRead( renewOnRead ) {
   }

   RObjectManager::~RObjectManager() {}

   /// @return field name
   const std::string& RObjectManager::fieldName() const {
      return m_field.GetField().GetFieldName();
   }

   /// @return A pointer to the internal data holding object
   ///
   const THolder* RObjectManager::holder() const {

      return m_holder.get();
   }

   /// @return A pointer to the internal data holding object
   ///
   THolder* RObjectManager::holder() {

      return m_holder.get();
   }
   
   /// This function is used to load the contents of a field only when it
   /// needs to be done. It keeps track of which entry was already loaded for
   /// a field/object, and only asks the RNTupleView for the field to load an 
   /// entry when it really has to be done. The next entry to load (m_entryToLoad)
   /// is managed by the owning Event object, set in this object's constructor.
   ///
   /// @return 0 if no new entry was read, the number of read bytes otherwise
   ///
   ::Int_t RObjectManager::getEntry( [[maybe_unused]] ::Int_t getall ) {

      // Must be valid entry value
      if ( m_entryToLoad  < 0 ){
         // Raise error as a negative entry is incorrect
         Error("xAOD::RObjectManager::getEntry", 
            XAOD_MESSAGE( "Entry to read must be larger than or equal to 0. entry=%i"), static_cast< int >( m_entryToLoad ) );
         return -1;
      }

      // Check if anything needs to be done:
      if( m_entryToLoad == m_entry ) return 0;

      // Renew the object in memory if we are in such a mode:
      if( m_renewOnRead ) {
         m_holder->renew();
      }

      // Load the entry.
      m_field(m_entryToLoad);

      // If successful, save entry number
      m_entry = m_entryToLoad;

      // For the moment, we don't know how to get 
      // the number of bytes read, so we just return 1
      return 1; 

   }


   /// This function gives an easy access to the object managed by this
   /// object.
   ///
   /// @return A typeless pointer to the object being managed
   ///
   const void* RObjectManager::object() const {

      return std::as_const(*m_holder).get();
   }

   void* RObjectManager::object() {

      return m_holder->get();
   }

   /// This is just a convenient way of calling THolder::Set from TEvent.
   ///
   /// @param obj The object to replace the previously managed one
   ///
   void RObjectManager::setObject( void* obj ) {

      m_holder->set( obj );
      m_isSet = kTRUE;
      return;
   }

   /// Dummy implementation as full objects can't be missing
   ///
   ::Bool_t RObjectManager::create() {

      return m_isSet;
   }

   /// @returns <code>kTRUE</code> if the object for this event was set,
   ///          <code>kFALSE</code> otherwise
   ///
   ::Bool_t RObjectManager::isSet() const {

      return m_isSet;
   }

   /// This function needs to be called after an event was filled into
   /// the output TTree. It tells the manager object that it needs to wait
   /// for another object to be set up for the upcoming event.
   ///
   void RObjectManager::reset() {

      m_isSet = kFALSE;
      return;
   }

   } // namespace Experimental 

} // namespace xAOD
