// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

// ROOT include(s):
#include <TBranch.h>
#include <TTree.h>
#include <TError.h>

// EDM include(s):
#include "AthContainers/AuxTypeRegistry.h"

// Local include(s):
#include "xAODRootAccess/tools/RAuxFieldManager.h"
#include "xAODRootAccess/tools/THolder.h"
#include "xAODRootAccess/tools/Message.h"

namespace xAOD::Experimental {

RAuxFieldManager::RAuxFieldManager( std::unique_ptr<THolder> holder, bool isPrimitive )
  : 
  IObjectManager(std::move(holder)),
  m_isSet( kTRUE ), 
  m_isPrimitive( isPrimitive ) {}


RAuxFieldManager::~RAuxFieldManager()  = default;

::Bool_t RAuxFieldManager::isPrimitive() const { 

  return m_isPrimitive; 
}


::Int_t RAuxFieldManager::getEntry( ::Int_t /*getall*/ ) {

  return 0;
}

const void* RAuxFieldManager::object() const {

  return holder()->get();
}

void* RAuxFieldManager::object() {

  return holder()->get();
}

void RAuxFieldManager::setObject( void* obj ) {

  holder()->set(obj);
  m_isSet = kTRUE;
  return;
}


::Bool_t RAuxFieldManager::create() {

  return m_isSet;
}

::Bool_t RAuxFieldManager::isSet() const {

  return m_isSet;
}

void RAuxFieldManager::reset() {

  m_isSet = kFALSE;
  return;
}

} // namespace xAOD::Experimental 

