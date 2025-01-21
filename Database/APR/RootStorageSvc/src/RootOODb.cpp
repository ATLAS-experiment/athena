/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//====================================================================
//        Root Database factories implementation
//--------------------------------------------------------------------
//
//        Package    : RootDb (The POOL project)
//
//        Author     : M.Frank
//====================================================================
#include "RootOODb.h"
#include "RootDatabase.h"
#include "RootDomain.h"
#include "RootKeyContainer.h"
#include "RootTreeContainer.h"
#include "RootTreeIndexContainer.h"
#include "RNTupleContainer.h"

#include "StorageSvc/DbOption.h"

// declare the types provided by this Storage plugin
DECLARE_COMPONENT_WITH_ID(pool::RootOODb, "ROOT_All")
DECLARE_COMPONENT_WITH_ID(pool::RootOOKey, "ROOT_Key")
DECLARE_COMPONENT_WITH_ID(pool::RootOOTree, "ROOT_Tree")
DECLARE_COMPONENT_WITH_ID(pool::RootOOTreeIndex, "ROOT_TreeIndex")
DECLARE_COMPONENT_WITH_ID(pool::RootOORNTuple, "ROOT_RNTuple")

using namespace pool;

/// Standard Constructor
RootOODb::RootOODb(DbType) : IOODatabase(), m_domainCache(nullptr)
{
}

const std::string&  RootOODb::name () const {
   static const std::string name = "APR.RootStorageSvc";
   return name;
}

/// Standard Destructor
RootOODb::~RootOODb()  {
}

/// Create Root Domain object
IDbDomain* RootOODb::createDomain()  {
  // We don't own this domain, so we don't have to delete it
  // cppcheck-suppress publicAllocationError
  m_domainCache = new RootDomain();
  return m_domainCache;
}

/// Create Root Database object (TFile)
IDbDatabase* RootOODb::createDatabase()  {
  return new RootDatabase();
}

/// Create Root Container object
IDbContainer* RootOODb::createContainer(const DbType& inType) {
  // Read the default container type from the current domain
  int optValue;
  DbOption opt("DEFAULT_CONTAINER_TYPE","");
  m_domainCache->getOption(opt);
  opt._getValue(optValue);
  const DbType defaultContainerType = DbType(optValue);
  // If no minor type is specified, use the default one from the domain
  const DbType type = inType.match(ROOT_StorageType) ?  defaultContainerType : inType;
  if ( type.match(ROOTKEY_StorageType) )  {
    return new RootKeyContainer();
  }
  else if ( type.match(ROOTTREE_StorageType) )    {
    return new RootTreeContainer();
  }
  else if ( type.match(ROOTTREEINDEX_StorageType) )    {
    return new RootTreeIndexContainer();
  }
  else if ( type.match(ROOTRNTUPLE_StorageType) )    {
    return new RNTupleContainer();
  }
  return 0;
}
