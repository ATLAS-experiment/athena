/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//====================================================================
//        Root Database implementation
//--------------------------------------------------------------------
//
//        Package    : RootStorageSvc (The POOL project)
//
//        Author     : M.Frank
//====================================================================
#ifndef POOL_ROOTSTORAGESVC_RootTechFactory_H
#define POOL_ROOTSTORAGESVC_RootTechFactory_H 1

// Framework include files
#include "StorageSvc/ITechFactory.h"
#include "StorageSvc/DbType.h"

#include "Gaudi/PluginService.h"

/** @class RootTechFactory RootTechFactory.h src/RootTechFactory.h
    *
    * Factory for creating domains, databases and containers implemented 
    * using ROOT technologies (TFile, TTree, TKey and RNTuple)
    * 
    */
class RootTechFactory  : public ITechFactory  {
  public:
    typedef Gaudi::PluginService::Factory<ITechFactory*()> Factory;

    /// Standard Constructor
    explicit RootTechFactory(pool::DbType typ=pool::ROOT_StorageType);

    /// Name of the Database implementation
    virtual const std::string&  name () const;

    /// Label of the specific class
    static const char* catalogLabel()  {   return "ROOT_All";       }

/// Create Root Domain object
    ITechDomain* createDomain();
    /// Create Root Database object (TFile)
    ITechDatabase* createDatabase();
    /// Create Root Container object
    ITechContainer* createContainer(const std::string& name, const pool::DbType& typ);
  private:
    /// Non-owning cache for the domain pointer
    ITechDomain* m_domainCache{};
  };

  class RootKeyStorageFactory : public RootTechFactory {
  public:
    /// Standard Constructor
    RootKeyStorageFactory() : RootTechFactory(pool::ROOTKEY_StorageType)   {   }
    /// Label of the specific class
    static const char* catalogLabel()  {   return "ROOT_Key";         }
  };

  class RootTreeStorageFactory : public RootTechFactory {
  public:
    /// Standard Constructor
    RootTreeStorageFactory() : RootTechFactory(pool::ROOTTREE_StorageType)   {  }
    /// Label of the specific class
    static const char* catalogLabel()  {   return "ROOT_Tree";        }
  };

  class RootTreeIdxStorageFactory : public RootTechFactory {
  public:
    /// Standard Constructor
    RootTreeIdxStorageFactory() : RootTechFactory(pool::ROOTTREEINDEX_StorageType)   {  }
    /// Label of the specific class
    static const char* catalogLabel()  {   return "ROOT_Tree";        }
  };

  class RootRNTupleStorageFactory : public RootTechFactory {
  public:
    /// Standard Constructor
    RootRNTupleStorageFactory() : RootTechFactory(pool::ROOTRNTUPLE_StorageType)   {  }
    /// Label of the specific class
    static const char* catalogLabel()  {   return "ROOT_RNTuple";      }
  };

  
#endif  /* POOL_ROOTSTORAGESVC_RootTechFactory_H  */
