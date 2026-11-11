/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "RootTechFactory.h"
#include "RootDatabase.h"
#include "RootDomain.h"
#include "RootKeyContainer.h"
#include "RootTreeContainer.h"
#include "RootTreeIndexContainer.h"
#include "RNTupleContainer.h"

#include "StorageSvc/DbOption.h"

#include "GaudiKernel/StatusCode.h"

// declare the types provided by this Storage plugin
DECLARE_COMPONENT_WITH_ID(RootTechFactory, "ROOT_All")
DECLARE_COMPONENT_WITH_ID(RootKeyStorageFactory, "ROOT_Key")
DECLARE_COMPONENT_WITH_ID(RootTreeStorageFactory, "ROOT_Tree")
DECLARE_COMPONENT_WITH_ID(RootTreeIdxStorageFactory, "ROOT_TreeIndex")
DECLARE_COMPONENT_WITH_ID(RootRNTupleStorageFactory, "ROOT_RNTuple")


/// Standard Constructor
RootTechFactory::RootTechFactory(DbType) : m_domainCache(nullptr)
{
}

const std::string&  RootTechFactory::name () const {
   static const std::string name = "APR.RootStorageSvc";
   return name;
}

/// Create Root Domain object
ITechDomain* RootTechFactory::createDomain()  {
  // We don't own this domain, so we don't have to delete it
  // cppcheck-suppress publicAllocationError
  m_domainCache = new RootDomain();
  return m_domainCache;
}

/// Create Root Database object (TFile)
ITechDatabase* RootTechFactory::createDatabase()  {
  return new RootDatabase();
}

/// Create Root Container object
ITechContainer* RootTechFactory::createContainer(const std::string& name, const DbType& inType) {
  // Read the default container type from the current domain
  int optValue;
  DbOption opt("DEFAULT_CONTAINER_TYPE","");
  if( !m_domainCache->getOption(opt).isSuccess() || !opt.getValue(optValue).isSuccess() ) {
    return nullptr;
  }
  const DbType defaultContainerType = DbType(optValue);
  // If no minor type is specified, use the default one from the domain
  const DbType type = inType.match(pool::ROOT_StorageType) ?  defaultContainerType : inType;
  if ( type.match(pool::ROOTKEY_StorageType) )  {
    return new RootKeyContainer(name);
  }
  else if ( type.match(pool::ROOTTREE_StorageType) )    {
    return new RootTreeContainer(name);
  }
  else if ( type.match(pool::ROOTTREEINDEX_StorageType) )    {
    return new RootTreeIndexContainer(name);
  }
  else if ( type.match(pool::ROOTRNTUPLE_StorageType) )    {
    return new RNTupleContainer(name);
  }
  return 0;
}
