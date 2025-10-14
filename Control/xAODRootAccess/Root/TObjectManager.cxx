/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/


// ROOT include(s):
#include <TBranch.h>
#include <TTree.h>
#include <TError.h>

// Local include(s):
#include "xAODRootAccess/tools/TObjectManager.h"
#include "xAODRootAccess/tools/THolder.h"
#include "xAODRootAccess/tools/Message.h"

namespace xAOD {

   TObjectManager::TObjectManager( ::TBranch* br,
                                   std::unique_ptr<THolder> holder,
                                   ::Bool_t renewOnRead )
      : IObjectManager(std::move(holder)),
        m_branch( br ), m_entry( -1 ), m_isSet( kTRUE ),
        m_renewOnRead( renewOnRead ) {
   }

   TObjectManager::TObjectManager( const TObjectManager& parent )
      : IObjectManager(parent),
        m_branch( parent.m_branch ), m_entry( parent.m_entry ),
        m_isSet( parent.m_isSet ), m_renewOnRead( parent.m_renewOnRead ) {

   }

   TObjectManager::~TObjectManager() = default;

   TObjectManager& TObjectManager::operator=( const TObjectManager& parent ) {

      // Check if we need to do anything:
      if( this == &parent ) {
         return *this;
      }

      m_branch = parent.m_branch;
      IObjectManager::operator=(parent);
      m_entry       = parent.m_entry;
      m_isSet       = parent.m_isSet;
      m_renewOnRead = parent.m_renewOnRead;

      return *this;
   }

   ::TBranch* TObjectManager::branch() {

      return m_branch;
   }

   /// This sort of access to the internal TBranch object is needed when
   /// connecting to the input tree. One has to make sure that the correct
   /// pointer is being pointed to by the input tree.
   ///
   /// @return A pointer to the internal branch pointer
   ///
   ::TBranch** TObjectManager::branchPtr() {

      return &m_branch;
   }

   /// This function is used to load the contents of a branch only when it
   /// needs to be done. It keeps track of which entry was already loaded for
   /// a branch/object, and only asks the branch to load an entry when it
   /// really has to be done.
   ///
   /// @return 0 if no new entry was read, the number of read bytes otherwise
   ///
   ::Int_t TObjectManager::getEntry( ::Int_t getall ) {

      // Make sure that the branch is associated to a tree
      // as the entry to be read is retrieved from the tree
      if (!m_branch->GetTree()){
         Error("xAOD::TObjectManager::getEntry",
            XAOD_MESSAGE("Branch=%s is not associated to any tree while reading of branches within this class relies on that"),
            m_branch->GetName());
         return -1;
      }

      // Get the entry that should be read
      // The entry to be read is set with TTree::LoadTree()
      // NB: for a branch from a friend tree and if the friend tree has an index built,
      // then the entry to read is found when calling the TTree::LoadTree() function
      // that matches the major and minor values between the main tree and the friend tree
      ::Long64_t entry = m_branch->GetTree()->GetReadEntry();

      if ( entry  < 0 ){
         // Raise error as it implies
         // either that the TTree::LoadTree() function has not been called
         // or
         // the entry requested to be read by the user
         // is not corresponding to any entry for the friend tree
         Error("xAOD::TObjectManager::getEntry",
            XAOD_MESSAGE( "Entry to read is not set for branch=%s from tree=%s. "
            "It is either because TTree::LoadTree(entry) was not called "
            "beforehand in the TEvent class OR "
            "the entry requested to be read for the main tree is not corresponding to an event for the friend tree" ),
            m_branch->GetName(),
            m_branch->GetTree()->GetName());
         return -1;
      }

      // Check if anything needs to be done:
      if( entry == m_entry ) return 0;

      // Make sure that the branch is not in "MakeClass mode":
      if( m_branch->GetMakeClass() ) {
         m_branch->SetMakeClass( 0 );
      }

      // Renew the object in memory if we are in such a mode:
      if( m_renewOnRead ) {
         holder()->renew();
      }

      // Load the entry.
      const ::Int_t nbytes = m_branch->GetEntry( entry, getall );

      // If the load was successful, remember that we loaded this entry.
      if( nbytes >= 0 ) {
         m_entry = entry;
      }

      // This is a tricky thing... Parts of the code try to minimise some
      // operations to only happen when an object is getting updated. But
      // it can happen that an interface or auxiliary store object doesn't
      // actually store any data itself. So TBranch::GetEntry will return 0.
      // But we still need to tell the code looking at this return value
      // that we did go to a new event. So we should never return 0 at
      // this point...
      if( nbytes ) {
         return nbytes;
      } else {
         return 1;
      }
   }

   /// This function gives an easy access to the object managed by this
   /// object.
   ///
   /// @return A typeless pointer to the object being managed
   ///
   const void* TObjectManager::object() const {

      return holder()->get();
   }

   void* TObjectManager::object() {

      return holder()->get();
   }

   /// This is just a convenient way of calling THolder::Set from TEvent.
   ///
   /// @param obj The object to replace the previously managed one
   ///
   void TObjectManager::setObject( void* obj ) {

      holder()->set( obj );
      m_isSet = kTRUE;
      return;
   }

   /// Dummy implementation as full objects can't be missing
   ///
   ::Bool_t TObjectManager::create() {

      return m_isSet;
   }

   /// @returns <code>kTRUE</code> if the object for this event was set,
   ///          <code>kFALSE</code> otherwise
   ///
   ::Bool_t TObjectManager::isSet() const {

      return m_isSet;
   }

   /// This function needs to be called after an event was filled into
   /// the output TTree. It tells the manager object that it needs to wait
   /// for another object to be set up for the upcoming event.
   ///
   void TObjectManager::reset() {

      m_isSet = kFALSE;
      return;
   }

} // namespace xAOD
