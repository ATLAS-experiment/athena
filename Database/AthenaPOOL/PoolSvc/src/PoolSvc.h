/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef POOLSVC_H
#define POOLSVC_H

/** @file PoolSvc.h
 *  @brief This file contains the class definition for the PoolSvc class.
 *  @author Peter van Gemmeren <gemmeren@anl.gov>
 **/

#include "PoolSvc/IPoolSvc.h"
#include "PoolSvc/FileCatalogUtils.h"
#include "GaudiKernel/IIoComponent.h"
#include "GaudiKernel/SmartIF.h"
#include "AthenaBaseComps/AthService.h"
#include "CxxUtils/checker_macros.h"
#include "PersistentDataModel/Guid.h"

#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <memory>

// Forward declarations
namespace pool {
   class IContainer;
   class IDatabase;
   class ISession;
}


/** @class PoolSvc
 *  @brief This class provides the interface to the LCG POOL persistency software.
 **/
class PoolSvc : public extends<AthService, IPoolSvc, IIoComponent> {

public: // Non-static members
   /// Required of all Gaudi services:
   virtual StatusCode initialize() override;
   virtual StatusCode io_reinit() override;
   /// Required of all Gaudi services:
   virtual StatusCode start() override;
   virtual StatusCode stop() override;
   /// Required of all Gaudi services:
   virtual StatusCode finalize() override;
   virtual StatusCode io_finalize() override;

   /// @return a token to a Data Object written to Pool.
   /// @param placement [IN] pointer to the placement hint.
   /// @param obj [IN] pointer to the Data Object to be written to Pool.
   /// @param classDesc [IN] pointer to the Seal class description for the Data Object.
   virtual
   Token* registerForWrite(const Placement* placement,
                           const void* obj,
                           const RootType& classDesc) override;

   /// @return void
   /// @param obj [OUT] pointer to the Data Object.
   /// @param token [IN] token of the Data Object for which a Pool Ref is filled.
   virtual
   void setObjPtr(void*& obj, const Token* token) override;

   /// @return an Id for an output context (POOL persistency service) and create it if needed.
   /// @param label [IN] string label to name new context and allow sharing (returns existing contextId)
   virtual
   unsigned int getOutputContext(const std::string& label) override;

   /// @return an Id for an input context (POOL persistency service) and create it if needed.
   /// @param label [IN] string label to name new context and allow sharing (returns existing contextId)
   /// @param maxFile [IN] maximum number of open input files.
   virtual
   unsigned int getInputContext(const std::string& label,
                                unsigned int maxFile = 0) override;

   /// @return copy of the map of all labelled input contexts.
   virtual
   std::map<std::string, unsigned int> getInputContextMap() const override;

   /// @return size of the map of all labelled input contexts.
   virtual
   unsigned int getInputContextMapSize() const override;

   /// @return size of the map of all labelled input contexts.
   virtual
   pool::ISession* getInputContextSession(unsigned int contextId) const override;

   /// @return void
   /// @param shareCat [IN] bool to share the file catalog.
   virtual
   void setShareMode(bool shareCat) override;

   /// @return void
   virtual
   void startCatalog() override;

   /// @return void
   virtual
   void commitCatalog() override;

   /// @return void
   /// @param dbID [IN] database ID to be translated
   /// @param pfn [OUT] string PFN of database
   /// @param type [OUT] string filetype of database
   virtual
   void lookupBestPfn(const std::string& dbID, std::string& pfn, std::string& type) const override;

   /// @return status of connect
   /// @param connection [IN] string containing the connection.
   /// @param collectionName [IN] string containing the persistent name of the collection.
   /// @param contextId [IN] id for PoolSvc persistency service to use for input.
   virtual
   StatusCode connectCollection(const std::string& connection,
           const std::string& collectionName,
           unsigned int contextId = IPoolSvc::kInputStream) const override;

   /// @return status of check
   /// @param connection [IN] string containing the connection.
   /// @param contextId [IN] id for PoolSvc persistency service to use for input.
   /// @param noContainer [IN] if no collection was found check whether file exists or had no events
   virtual
   StatusCode checkCollection(const std::string& connection,
           unsigned int contextId,
           bool noContainer) const override;

   /// @return a token for a container entry.
   /// @param connection [IN] string containing the connection/file name.
   /// @param collection [IN] string containing the persistent name of the collection.
   /// @param ientry [IN] entry number for the token to be returned
   virtual
   Token* getToken(const std::string& connection,
	   const std::string& collection,
	   const unsigned long ientry) const override;

   /// Connect to a logical database unit; PersistencySvc is chosen according to transaction type (accessmode).
   virtual
   StatusCode connect(Io::IoFlag type,
	   unsigned int contextId = IPoolSvc::kInputStream) override;

   /// Commit data for a given contextId and flush buffer.
   /// @param contextId [IN] poolStream to be commited.
   virtual
   StatusCode commit(unsigned int contextId = IPoolSvc::kInputStream) const override;

   /// Commit data for a given contextId and hold buffer.
   /// @param contextId [IN] poolStream to be commited.
   virtual
   StatusCode commitAndHold(unsigned int contextId = IPoolSvc::kInputStream) const override;

   /// Disconnect PersistencySvc associated with a contextId.
   /// @param contextId [IN] poolStream to be disconnected.
   virtual
   StatusCode disconnect(unsigned int contextId = IPoolSvc::kInputStream) const override;

   /// Disconnect single Database.
   /// @param connection [IN] connection string for Database to be disconnected.
   /// @param contextId [IN] context id of database to be disconnected.
   virtual
   StatusCode disconnectDb(const std::string& connection,
	   unsigned int contextId = IPoolSvc::kInputStream) const override;

   /// Get POOL attributes - domain
   virtual
   StatusCode getAttribute(const std::string& optName,
	   std::string& data,
	   long tech,
	   unsigned int contextId = IPoolSvc::kInputStream) const override;

   /// Get POOL attributes - db/file, container/collection
   virtual
   StatusCode getAttribute(const std::string& optName,
	   std::string& data,
	   long tech,
	   const std::string& dbName,
	   const std::string& contName = "",
	   unsigned int contextId = IPoolSvc::kInputStream) const override;

   /// Set POOL attributes - domain
   virtual
   StatusCode setAttribute(const std::string& optName,
	   const std::string& data,
	   long tech,
	   unsigned int contextId = IPoolSvc::kOutputStream) const override;

   /// Set POOL attributes - db/file, container/collection
   virtual
   StatusCode setAttribute(const std::string& optName,
	   const std::string& data,
	   long tech,
	   const std::string& dbName,
	   const std::string& contName = "",
	   unsigned int contextId = IPoolSvc::kOutputStream) const override;

   /// Standard Service Constructor
   using base_class::base_class;

   /// Destructor
   virtual ~PoolSvc();

private: // data
   using CallMutex = std::recursive_mutex;
   // Lock Guard class to safely lock a mutex for a given contextId
   class ContextLock {
      std::unique_lock< CallMutex > m_lock;
   public:
      ContextLock(int contextId, CallMutex &glob_mtx, const std::vector<CallMutex*> &ctx_mutexes) {
         // lock the global mutex to gain exclusive access to the context mutexes vector
         std::lock_guard<CallMutex>    temp_lock( glob_mtx );
         // lock the mutex for the given context ID for the lifetime of this object
         m_lock = std::unique_lock< CallMutex >{ *ctx_mutexes[contextId] };
      }
      ~ContextLock() { m_lock.unlock(); }
   };

   mutable CallMutex                                 m_pool_mut;
 
   bool                                              m_shareCat{false};
   SmartIF<Gaudi::IFileCatalogMgr>                    m_catalogMgr;
   SmartIF<Gaudi::IFileCatalog>                       m_catalog;
   std::vector<pool::ISession*>      m_dbSessionVec;
   std::vector<CallMutex*>                           m_pers_mut;
   std::map<std::string, unsigned int>               m_inputContextLabel;
   std::map<std::string, unsigned int>               m_outputContextLabel;
   std::string                                       m_mainOutputLabel{};
   std::map<unsigned int, unsigned int>              m_contextMaxFile;
   // Cache for open file guids for each m_dbsessionVec member, protected by m_pers_mut
   mutable std::map<unsigned int, std::list<Guid> >  m_guidLists ATLAS_THREAD_SAFE;

private: // properties
   /// MaxFilesOpen, option to have PoolSvc limit the number of open Input Files: default = 0
   ///  (No files are closed automatically)
   Gaudi::Property<int> m_dbAgeLimit{this,"MaxFilesOpen",0};
   /// WriteCatalog, the file catalog to be used to register output files (also default input catalog):
   ///	default = "" (use POOL default).
   Gaudi::Property<std::string> m_writeCatalog{this,"WriteCatalog","xmlcatalog_file:PoolFileCatalog.xml"};
   /// ReadCatalog, the list of additional POOL input file catalogs to consult: default = empty vector.
   Gaudi::Property<std::vector<std::string>> m_readCatalog{this,"ReadCatalog",{},"List of catalog files to read from","OrderedSet<std::string>"};
   /// Use ROOT Implicit MultiThreading, default = true.
   Gaudi::Property<bool> m_useROOTIMT{this,"UseROOTImplicitMT",true};
   /// Increase virtual TTree size to avoid backreads in multithreading, default = false.
   Gaudi::Property<bool> m_useROOTMaxTree{this,"UseROOTIncreaseVMaxTree",false};

   /// AttemptCatalogPatch, option to create catalog: default = false.
   Gaudi::Property<bool> m_attemptCatalogPatch{this,"AttemptCatalogPatch",true};
 

private: // internal helper functions
   // delete all Persistency Services, Catalog, Mutexes and Indexes
   void clearState();

   // Set up m_catalogMgr/m_catalog. Returns false on failure.
   bool createCatalog();
   void patchCatalog(const std::string& pfn, pool::IDatabase& dbH) const;

   // setup persistency
   StatusCode setupPersistencySvc();

   /// Get Database handle
   std::unique_ptr<pool::IDatabase> getDbHandle(unsigned int contextId, const std::string& dbName) const;
   /// Get Container handle
   std::unique_ptr<pool::IContainer> getContainerHandle(pool::IDatabase* dbH, const std::string& contName) const;

   /// Resolve a file using ATLAS_POOLCOND_PATH
   std::string poolCondPath(const std::string& leaf);
};
#endif
