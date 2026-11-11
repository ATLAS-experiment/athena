/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef STORAGESVC_DBTYPE
#define STORAGESVC_DBTYPE

#include "StorageSvc/DbType.h"

#include <stdexcept>
#include <cstdio>


DbType DbType::getType(const std::string& name)  {
  if ( "ROOT_Key" == name )
    return pool::ROOTKEY_StorageType;
  else if ( "ROOT_Tree" == name or name == "ROOTTREE" )
    return pool::ROOTTREE_StorageType;
  else if ( "ROOT_TreeIndex" == name  or name == "ROOTTREEINDEX")
    return pool::ROOTTREEINDEX_StorageType;
  else if ( "ROOT_RNTuple" == name or name == "ROOTRNTUPLE")
    return pool::ROOTRNTUPLE_StorageType;
  else if ( "ROOT_All" == name )
    return pool::ROOT_StorageType;
  else if ( "Test" == name )
    return pool::TEST_StorageType;
  throw std::runtime_error("DbType::getType failed: Unknown storage type requested:"+name);
}

const std::string DbType::storageName()  const {
  if ( *this == pool::TEST_StorageType          )
    return "Test";
  else if ( exactMatch(pool::ROOTRNTUPLE_StorageType) )
    return "ROOT_RNTuple";
  else if ( exactMatch(pool::ROOTKEY_StorageType)    )
    return "ROOT_Key";
  else if ( exactMatch(pool::ROOTTREE_StorageType)   )
    return "ROOT_Tree";
  else if ( exactMatch(pool::ROOTTREEINDEX_StorageType)   )
    return "ROOT_TreeIndex";
  else if ( *this == pool::ROOT_StorageType     )
    return "ROOT_All";
  else if ( *this == pool::POOL_StorageType     )
    return "POOL";

  char nam[64];
  std::sprintf(nam,"%08X", type());
  return nam;
}

#endif