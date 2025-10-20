/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef POOLSVC_H
#define POOLSVC_H

/** @file PoolSvc.h
 *  @brief This file contains the class definition for the PoolSvc class.
 *  @author Peter van Gemmeren <gemmeren@anl.gov>
 **/

#include "PoolSvc/IPoolSvc.h"
#include "GaudiKernel/IIoComponent.h"
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
   class IPersistencySvc;
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

   /// @return map of all labelled input contexts.
   virtual
   const std::map<std::string, unsigned int>& getInputContextMap() const override;

   /// @return the context.
   virtual
   const coral::Context* context() const override;

   /// @return void
   /// @param compName [IN] string name of the component to be loaded.
   virtual
   void loadComponent(const std::string& compName) override;

   /// @return void
   /// @param shareCat [IN] bool to share the file catalog.
   virtual
   void setShareMode(bool shareCat) override;

   /// @return the file catalog.
   virtual
   const pool::IFileCatalog* catalog() const override;

   /// @return void
   /// @param token [IN] filename/token string to be translated
   /// @param pfn [OUT] string PFN of database
   /// @param type [OUT] string filetype of database
   virtual
   void lookupBestPfn(const std::string& token, std::string& pfn, std::string& type) const override;

   /// @return void
   /// @param pf [IN] filename to be renamed
   /// @param newpf [IN] new filename
   virtual
   void renamePfn(const std::string& pf, const std::string& newpf) override;

   /// @return a pointer to a Pool Collection.
   /// @param collectionType [IN] string containing the collection type.
   /// @param connection [IN] string containing the connection.
   /// @param collectionName [IN] string containing the persistent name of the collection.
   /// @param contextId [IN] id for PoolSvc persistency service to use for input.
   virtual
   pool::ICollection* createCollection(const std::string& collectionType,
	   const std::string& connection,
	   const std::string& collectionName,
	   unsigned int contextId = IPoolSvc::kInputStream) const override;

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
   StatusCode connect(pool::ITransaction::Type type,
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

   /// Get POOL FileSize attribute for database without logging a message
   virtual
   long long int getFileSize(const std::string& dbName, long tech, unsigned int contextId) const override;

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

   /// Setup Frontier cache for given logical or physical connection name
   virtual
   StatusCode setFrontierCache(const std::string& conn) override;

   /// Standard Service Constructor
   using base_class::base_class;

   /// Destructor
   virtual ~PoolSvc();

private: // data
   typedef std::recursive_mutex CallMutex;
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
   coral::Context*                                   m_context{nullptr};
   bool                                              m_shareCat{false};
   pool::IFileCatalog*                               m_catalog{nullptr};
   std::vector<pool::IPersistencySvc*>               m_persistencySvcVec;
   std::vector<CallMutex*>                           m_pers_mut;
   std::map<std::string, unsigned int>               m_inputContextLabel;
   std::map<std::string, unsigned int>               m_outputContextLabel;
   std::string                                       m_mainOutputLabel{};
   std::map<unsigned int, unsigned int>              m_contextMaxFile;
   // Cache for open file guids for each m_persistencySvcVec member, protected by m_pers_mut
   mutable std::map<unsigned int, std::list<Guid> >  m_guidLists ATLAS_THREAD_SAFE;

private: // properties
   /// FileOpen, the open mode for the file ("append" or "overwrite").
   Gaudi::Property<std::string> m_fileOpen{this,"FileOpen","overwrite"};
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
   /// ConnectionRetrialPeriod, retry period for CORAL Connection Service: default = 30 seconds
   Gaudi::Property<int> m_retrialPeriod{this,"ConnectionRetrialPeriod",300};
   /// ConnectionRetrialTimeOut, the retrial time out for CORAL Connection Service: default = 300 seconds
   Gaudi::Property<int> m_retrialTimeOut{this,"ConnectionRetrialTimeOut",3600};
   /// ConnectionTimeOut, the time out for CORAL Connection Service: default = 5 seconds
   Gaudi::Property<int> m_timeOut{this,"ConnectionTimeOut",5};
   /// ConnectionCleanUp - whether to use CORAL connection management thread: default = false.
   Gaudi::Property<bool> m_connClean{this,"ConnectionCleanUp",false};
   /// Frontier proprties, compression level and list of schemas to be refreshed: default = 5
   Gaudi::Property<int> m_frontierComp{this,"FrontierCompression",5};
   Gaudi::Property<std::vector<std::string>> m_frontierRefresh{this,"FrontierRefreshSchema",{}};
   /// Use DBReplicaSvc to sort database connections, default = true.
   Gaudi::Property<bool> m_sortReplicas{this,"SortReplicas",true};
   /// Default ROOT container type
   Gaudi::Property<std::string> m_defaultROOTContainerType{this, "DefaultContainerType", "ROOTTREEINDEX"};

private: // internal helper functions
   // delete all APR::Persistency Services, Catalog, Mutexes and Indexes
   void clearState();

   pool::IFileCatalog* createCatalog();
   void patchCatalog(const std::string& pfn, pool::IDatabase& dbH) const;

   // setup APR persistency
   StatusCode setupPersistencySvc();

   /// Get Database handle
   std::unique_ptr<pool::IDatabase> getDbHandle(unsigned int contextId, const std::string& dbName) const;
   /// Get Container handle
   std::unique_ptr<pool::IContainer> getContainerHandle(pool::IDatabase* dbH, const std::string& contName) const;

   /// Resolve a file using ATLAS_POOLCOND_PATH
   std::string poolCondPath(const std::string& leaf);
};
#endif
