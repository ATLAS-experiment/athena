/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

//====================================================================
//  Package    : StorageSvc (The POOL project)
//
//  @author      M.Frank
//====================================================================
#include "PersistentDataModel/Guid.h"
#include "StorageSvc/IStorageSvc.h"

// Declaration of the interface ID needs to be unique within ONE process
const Guid& pool::IStorageSvc::interfaceID()  {
  static const Guid id(true);
  return id;
}

