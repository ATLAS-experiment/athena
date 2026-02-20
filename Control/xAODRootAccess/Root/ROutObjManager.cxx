/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/


// ROOT include(s):
#include <TBranch.h>
#include <TTree.h>
#include <TError.h>

// Local include(s):
#include "xAODRootAccess/tools/ROutObjManager.h"
#include "xAODRootAccess/tools/THolder.h"
#include "xAODRootAccess/tools/Message.h"

namespace xAOD::Experimental {

ROutObjManager::ROutObjManager( std::string_view key, std::unique_ptr<THolder> holder )
  :
  IObjectManager(std::move(holder)),
  m_key( key ), 
  m_isSet( kTRUE ) {}

ROutObjManager::~ROutObjManager()  = default;

/// @return field name
const std::string& ROutObjManager::key() {
  return m_key;
}

/// RDS: not exactly sure of the description here. We always get the entry for the field view.

/// This function is used to load the contents of a branch only when it
/// needs to be done. It keeps track of which entry was already loaded for
/// a branch/object, and only asks the branch to load an entry when it
/// really has to be done.
///
/// @return 0 if no new entry was read, the number of read bytes otherwise
///
::Int_t ROutObjManager::getEntry(  ::Int_t /*getall*/ ) {

  return 0;
}


/// This function gives an easy access to the object managed by this
/// object.
///
/// @return A typeless pointer to the object being managed
///
const void* ROutObjManager::object() const {

  return holder()->get();
}

void* ROutObjManager::object() {

  return holder()->get();
}

/// This is just a convenient way of calling THolder::Set from TEvent.
///
/// @param obj The object to replace the previously managed one
///
void ROutObjManager::setObject( void* obj ) {

  holder()->set( obj );
  m_isSet = kTRUE;
  return;
}

/// Dummy implementation as full objects can't be missing
///
::Bool_t ROutObjManager::create() {

  return m_isSet;
}

/// @returns <code>kTRUE</code> if the object for this event was set,
///          <code>kFALSE</code> otherwise
///
::Bool_t ROutObjManager::isSet() const {

  return m_isSet;
}

/// This function needs to be called after an event was filled into
/// the output TTree. It tells the manager object that it needs to wait
/// for another object to be set up for the upcoming event.
///
void ROutObjManager::reset() {

  m_isSet = kFALSE;
  return;
}

} // namespace xAOD::Experimental 
