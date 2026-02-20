/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//====================================================================
//  Package    : Athena APR StorageSvc (former POOL project)
//
//  @author      M.Frank
//====================================================================

#include "StorageSvc/DbType.h"

#include <stdexcept>
#include <cstdio>

using namespace pool;

DbType DbType::getType(const std::string& name)  {
  if ( "ROOT_Key" == name )
    return ROOTKEY_StorageType;
  else if ( "ROOT_Tree" == name or name == "ROOTTREE" )
    return ROOTTREE_StorageType;
  else if ( "ROOT_TreeIndex" == name  or name == "ROOTTREEINDEX")
    return ROOTTREEINDEX_StorageType;
  else if ( "ROOT_RNTuple" == name or name == "ROOTRNTUPLE")
    return ROOTRNTUPLE_StorageType;
  else if ( "ROOT_All" == name )
    return ROOT_StorageType;
  else if ( "Test" == name )
    return TEST_StorageType;
  throw std::runtime_error("POOL::DbType::getType failed: Unknown storage type requested:"+name);
}

const std::string DbType::storageName()  const {
  if ( *this == TEST_StorageType          )
    return "Test";
  else if ( exactMatch(ROOTRNTUPLE_StorageType) )
    return "ROOT_RNTuple";
  else if ( exactMatch(ROOTKEY_StorageType)    )
    return "ROOT_Key";
  else if ( exactMatch(ROOTTREE_StorageType)   )
    return "ROOT_Tree";
  else if ( exactMatch(ROOTTREEINDEX_StorageType)   )
    return "ROOT_TreeIndex";
  else if ( *this == ROOT_StorageType     )
    return "ROOT_All";
  else if ( *this == POOL_StorageType     )
    return "POOL";

  char nam[64];
  std::sprintf(nam,"%08X", type());
  return nam;
}
