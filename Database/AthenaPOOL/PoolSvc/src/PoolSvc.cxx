/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/** @file PoolSvc.cxx
 *  @brief This file contains the implementation for the PoolSvc class.
 *  @author Peter van Gemmeren <gemmeren@anl.gov>
 **/

#include "PoolSvc.h"
#include "ITechnologySpecificAttributes.h"

#include "GaudiKernel/IIoComponentMgr.h"
#include "GaudiKernel/ConcurrencyFlags.h"

#include "PathResolver/PathResolver.h"

#include "PersistentDataModel/Placement.h"
#include "PersistentDataModel/Token.h"

#include "PoolSvc/ISession.h"
#include "PoolSvc/IDatabase.h"
#include "PoolSvc/IContainer.h"
#include "PoolSvc/ITokenIterator.h"
#include "PoolSvc/IFileCatalog.h"

#include "StorageSvc/DbType.h"
#include "StorageSvc/DbPrint.h"

#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <cstdio>
#include <cctype>
#include <exception>  // for runtime_error

#include <iostream>
using namespace std;
bool isNumber(const std::string& s) {
   return !s.empty() && (std::isdigit(s[0]) || s[0] == '+' || s[0] == '-');
}

//__________________________________________________________________________
StatusCode PoolSvc::initialize() {
   ATH_CHECK(::AthService::initialize());

   // Register this service for 'I/O' events
   ServiceHandle<IIoComponentMgr> iomgr("IoComponentMgr", name());
   ATH_CHECK(iomgr.retrieve());
   ATH_CHECK(iomgr->io_register(this));
   // Register input file's names with the I/O manager, unless in SharedWrite mode, set by AthenaPoolCnvSvc
   bool allGood = true;
   for (const auto& catalog : m_readCatalog.value()) {
      if (catalog.starts_with("xmlcatalog_file:")) {
         const std::string fileName = catalog.substr(16);
         if (!iomgr->io_register(this, IIoComponentMgr::IoMode::READ, fileName, fileName).isSuccess()) {
            ATH_MSG_FATAL("could not register [" << catalog << "] for input !");
            allGood = false;
         } else {
            ATH_MSG_INFO("io_register[" << this->name() << "](" << catalog << ") [ok]");
         }
      }
   }
   if (m_writeCatalog.value().starts_with("xmlcatalog_file:")) {
      const std::string fileName = m_writeCatalog.value().substr(16);
      if (!iomgr->io_register(this, IIoComponentMgr::IoMode::WRITE, fileName, fileName).isSuccess()) {
         ATH_MSG_FATAL("could not register [" << m_writeCatalog.value() << "] for input !");
         allGood = false;
      } else {
         ATH_MSG_INFO("io_register[" << this->name() << "](" << m_writeCatalog.value() << ") [ok]");
      }
   }
   if (!allGood) {
      return(StatusCode::FAILURE);
   }
 
   MSG::Level athLvl = msg().level();
   ATH_MSG_DEBUG("OutputLevel is " << (int)athLvl);
   pool::DbPrintLvl::setLevel(athLvl);
   return(setupPersistencySvc());
}

//__________________________________________________________________________
StatusCode PoolSvc::io_reinit() {
   ATH_MSG_INFO("I/O reinitialization...");
   ServiceHandle<IIoComponentMgr> iomgr("IoComponentMgr", name());
   if (!iomgr.retrieve().isSuccess()) {
      ATH_MSG_FATAL("Could not retrieve IoComponentMgr !");
      return(StatusCode::FAILURE);
   }
   if (!iomgr->io_hasitem(this)) {
      ATH_MSG_FATAL("IoComponentMgr does not know about myself !");
      return(StatusCode::FAILURE);
   }
   std::vector<std::string> readcat = m_readCatalog.value();
   for (std::size_t icat = 0, imax = readcat.size(); icat < imax; icat++) {
      if (readcat[icat].starts_with("xmlcatalog_file:")) {
         std::string fileName = readcat[icat].substr(16);
         if (iomgr->io_contains(this, fileName)) {
            if (!iomgr->io_retrieve(this, fileName).isSuccess()) {
               ATH_MSG_FATAL("Could not retrieve new value for [" << fileName << "] !");
               return(StatusCode::FAILURE);
            }
            readcat[icat] = "xmlcatalog_file:" + fileName;
         }
      }
   }
   // all good... copy over.
   m_readCatalog = readcat;
   if (m_writeCatalog.value().starts_with("xmlcatalog_file:")) {
      std::string fileName = m_writeCatalog.value().substr(16);
      if (iomgr->io_contains(this, fileName)) {
         if (!iomgr->io_retrieve(this, fileName).isSuccess()) {
            ATH_MSG_FATAL("Could not retrieve new value for [" << fileName << "] !");
            return(StatusCode::FAILURE);
         }
         if (!m_shareCat) {
            m_writeCatalog.setValue("xmlcatalog_file:" + fileName);
         }
      }
   }
   return(setupPersistencySvc());
}
//__________________________________________________________________________
StatusCode PoolSvc::setupPersistencySvc() {
   clearState();
   ATH_MSG_INFO("Setting up FileCatalog and Streams");
   m_catalog = createCatalog();
   if (m_catalog != nullptr) {
      m_catalog->start();
   } else {
      ATH_MSG_FATAL("Failed to setup POOL File Catalog.");
      return(StatusCode::FAILURE);
   }
   // Setup a persistency services
   m_dbSessionVec.push_back(pool::createSession(*m_catalog).release()); // Read Service
   m_pers_mut.push_back(new CallMutex);
   if (!m_dbSessionVec[IPoolSvc::kInputStream]->technologySpecificAttributes(pool::ROOT_StorageType.type()).setAttribute<bool>("ENABLE_THREADSAFETY", true)) {
      ATH_MSG_FATAL("Failed to enable thread safety in ROOT via PersistencySvc.");
      return(StatusCode::FAILURE);
   }
   m_contextMaxFile.try_emplace(IPoolSvc::kInputStream, m_dbAgeLimit.value());
   if (!connect(Io::READ, IPoolSvc::kInputStream).isSuccess()) {
      ATH_MSG_FATAL("Failed to connect Input PersistencySvc.");
      return(StatusCode::FAILURE);
   }
   m_dbSessionVec.push_back(pool::createSession(*m_catalog).release()); // Write Service
   m_pers_mut.push_back(new CallMutex);

   return(StatusCode::SUCCESS);
}
//__________________________________________________________________________
StatusCode PoolSvc::start() {
   // Switiching on ROOT implicit multi threading for AthenaMT
   if (m_useROOTIMT && Gaudi::Concurrency::ConcurrencyFlags::numThreads() > 1) {
      if (!m_dbSessionVec[IPoolSvc::kInputStream]->technologySpecificAttributes(pool::ROOT_StorageType.type()).setAttribute<int>("ENABLE_IMPLICITMT", Gaudi::Concurrency::ConcurrencyFlags::numThreads() - 1)) {
         ATH_MSG_FATAL("Failed to enable implicit multithreading in ROOT via PersistencySvc.");
         return(StatusCode::FAILURE);
      }
      ATH_MSG_INFO("Enabled implicit multithreading in ROOT via PersistencySvc to: " << Gaudi::Concurrency::ConcurrencyFlags::numThreads() - 1);
   }
   return(StatusCode::SUCCESS);
}
//__________________________________________________________________________
StatusCode PoolSvc::stop() {
   ATH_MSG_VERBOSE("stop()");
   bool retError = false;
   for (unsigned int contextId = 0, imax = m_dbSessionVec.size(); contextId < imax; contextId++) {
      if (!disconnect(contextId).isSuccess()) {
         ATH_MSG_FATAL("Cannot disconnect Stream: " << contextId);
         retError = true;
      }
   }
   return(retError ? StatusCode::FAILURE : StatusCode::SUCCESS);
}

//__________________________________________________________________________
void PoolSvc::clearState() {
   std::lock_guard<CallMutex> lock(m_pool_mut);
   // Cleanup persistency service
   for (const auto& dbSession : m_dbSessionVec) {
      delete dbSession;
   }
   m_dbSessionVec.clear();
   for (const auto& persistencyMutex : m_pers_mut) {
      delete persistencyMutex;
   }
   m_mainOutputLabel.clear();
   m_inputContextLabel.clear();
   m_outputContextLabel.clear();
   m_pers_mut.clear();
   if (m_catalog != nullptr) {
      m_catalog->commit();
      delete m_catalog; m_catalog = nullptr;
   }
}
//__________________________________________________________________________
StatusCode PoolSvc::finalize() {
   clearState();
   return(::AthService::finalize());
}
//__________________________________________________________________________
StatusCode PoolSvc::io_finalize() {
   ATH_MSG_INFO("I/O finalization...");
   for (size_t i = 0; i < m_dbSessionVec.size(); i++) {
      if ((m_dbSessionVec[i]->transaction().type() == Io::WRITE || m_dbSessionVec[i]->transaction().type() == Io::APPEND) &&
	      !disconnect(i).isSuccess()) {
         ATH_MSG_WARNING("Cannot disconnect output Stream " << i);
      }
   }
   clearState();
   return(StatusCode::SUCCESS);
}
//__________________________________________________________________________
Token* PoolSvc::registerForWrite(const Placement* placement,
                                 const void* obj,
                                 const RootType& classDesc) {
   unsigned int contextId = IPoolSvc::kOutputStream;
   const std::string& auxString = placement->auxString();
   if (!auxString.empty()) {
      if (auxString.starts_with("[CTXT=")) {
         ::sscanf(auxString.c_str(), "[CTXT=%08X]", &contextId);
      } else if (auxString.starts_with("[CLABEL=")) {
         contextId = this->getOutputContext(auxString);
      }
      if (contextId >= m_dbSessionVec.size()) {
         ATH_MSG_WARNING("registerForWrite: Using default output Stream instead of id = " << contextId);
         contextId = IPoolSvc::kOutputStream;
      }
   }
   std::lock_guard<CallMutex> lock(*m_pers_mut[contextId]);
   Token* token = m_dbSessionVec[contextId]->registerForWrite(*placement, obj, classDesc);
   if (token == nullptr) {
      ATH_MSG_WARNING("Cannot write object: " << placement->containerName());
   }
   return(token);
}
//__________________________________________________________________________
void PoolSvc::setObjPtr(void*& obj, const Token* token) {
   unsigned int contextId = IPoolSvc::kInputStream;
   const std::string& auxString = token->auxString();
   if (!auxString.empty()) {
      if (auxString.starts_with("[CTXT=")) {
         ::sscanf(auxString.c_str(), "[CTXT=%08X]", &contextId);
      } else if (auxString.starts_with("[CLABEL=")) {
         contextId = this->getInputContext(auxString);
      }
      if (contextId >= m_dbSessionVec.size()) {
         ATH_MSG_WARNING("setObjPtr: Using default input Stream instead of id = " << contextId);
         contextId = IPoolSvc::kInputStream;
      }
   }
   ATH_MSG_VERBOSE("setObjPtr: token=" << token->toString() << ", auxString=" << auxString << ", contextID=" << contextId);
   // Get Context ID/label from Token
   std::lock_guard<CallMutex> lock(*m_pers_mut[contextId]);
   obj = m_dbSessionVec[contextId]->readObject(*token, obj);
   std::map<unsigned int, unsigned int>::const_iterator maxFileIter = m_contextMaxFile.find(contextId);
   if (maxFileIter != m_contextMaxFile.end() && maxFileIter->second > 0) {
      m_guidLists[contextId].remove(token->dbID());
      m_guidLists[contextId].push_back(token->dbID());
      while (m_guidLists[contextId].size() > maxFileIter->second) {
         this->disconnectDb("FID:" + m_guidLists[contextId].begin()->toString(), contextId).ignore();
      }
   }
}
//__________________________________________________________________________
unsigned int PoolSvc::getOutputContext(const std::string& label) {
   std::lock_guard<CallMutex> lock(m_pool_mut);
   if (m_mainOutputLabel.empty()) {
      m_mainOutputLabel = label;
      m_outputContextLabel.try_emplace(label, IPoolSvc::kOutputStream);
   }
   if (label == m_mainOutputLabel || label.empty()) {
      return(IPoolSvc::kOutputStream);
   }
   std::map<std::string, unsigned int>::const_iterator contextIter = m_outputContextLabel.find(label);
   if (contextIter != m_outputContextLabel.end()) {
      return(contextIter->second);
   }
   const unsigned int id = m_dbSessionVec.size();
   m_dbSessionVec.push_back(pool::createSession(*m_catalog).release());
   m_pers_mut.push_back(new CallMutex);
   m_outputContextLabel.try_emplace(label, id);
   return(id);
}
//__________________________________________________________________________
unsigned int PoolSvc::getInputContext(const std::string& label, unsigned int maxFile) {
   std::lock_guard<CallMutex> lock(m_pool_mut);
   if (!label.empty()) {
      std::map<std::string, unsigned int>::const_iterator contextIter = m_inputContextLabel.find(label);
      if (contextIter != m_inputContextLabel.end()) {
         if (maxFile > 0) {
            m_contextMaxFile[contextIter->second] = maxFile;
         }
         return(contextIter->second);
      }
   }
   const unsigned int id = m_dbSessionVec.size();
   m_dbSessionVec.push_back( pool::createSession(*m_catalog, maxFile).release() );
   m_pers_mut.push_back(new CallMutex);
   if (!connect(Io::READ, id).isSuccess()) {
      ATH_MSG_WARNING("Failed to connect Input PersistencySvc: " << id);
      return(IPoolSvc::kInputStream);
   }
   if (!label.empty()) {
      m_inputContextLabel.try_emplace(label, id);
   }
   m_contextMaxFile.try_emplace(id, maxFile);
   return(id);
}
//__________________________________________________________________________
std::map<std::string, unsigned int> PoolSvc::getInputContextMap() const {
   std::lock_guard<CallMutex> lock(m_pool_mut);
   return(m_inputContextLabel);
}
//__________________________________________________________________________
unsigned int PoolSvc::getInputContextMapSize() const {
   std::lock_guard<CallMutex> lock(m_pool_mut);
   return(m_inputContextLabel.size());
}
//__________________________________________________________________________
pool::ISession* PoolSvc::getInputContextSession(unsigned int contextId) const {
   if (contextId >= m_dbSessionVec.size()) {
      ATH_MSG_WARNING("getInputContextSession: Using default input Stream instead of id = " << contextId);
      contextId = IPoolSvc::kInputStream;
   }
   return(m_dbSessionVec[contextId]);
}
//__________________________________________________________________________
void PoolSvc::setShareMode(bool shareCat) {
   m_shareCat = shareCat;
}
//__________________________________________________________________________
void PoolSvc::startCatalog() {
   if (m_catalog != nullptr) {
      m_catalog->start();
   }
}
//__________________________________________________________________________
void PoolSvc::commitCatalog() {
   if (m_catalog != nullptr) {
      m_catalog->commit();
   }
}
//__________________________________________________________________________
void PoolSvc::lookupBestPfn(const std::string& token, std::string& pfn, std::string& type) const {
   std::string dbID;
   if (token.starts_with("PFN:")) {
      m_catalog->lookupFileByPFN(token.substr(4), dbID, type); // PFN -> FID
   } else if (token.starts_with("LFN:")) {
      dbID = m_catalog->lookupLFN(token.substr(4)); // LFN -> FID
   } else if (token.starts_with("FID:")) {
      dbID = token.substr(4);
   } else if (token.size() > Guid::stringSize()) { // full token
      Token tok;
      tok.fromString(token);
      dbID = tok.dbID().toString();
   } else { // guid only
      dbID = token;
   }
   m_catalog->getFirstPFN(dbID, pfn, type); // FID -> best PFN
}
//__________________________________________________________________________
void PoolSvc::renamePfn(const std::string& pf, const std::string& newpf) {
   std::string dbID, type;
    m_catalog->lookupFileByPFN(pf, dbID, type);
   if (dbID.empty()) {
      ATH_MSG_WARNING("Failed to lookup: " << pf << " in FileCatalog");
      return;
   }
   m_catalog->lookupFileByPFN(newpf, dbID, type);
   if (!dbID.empty()) {
      ATH_MSG_INFO("Found: " << newpf << " in FileCatalog");
      return;
   }
   m_catalog->renamePFN(pf, newpf);
}
//__________________________________________________________________________
StatusCode PoolSvc::connectCollection(const std::string& connection,
		const std::string& collectionName,
		unsigned int contextId) const
{
   ATH_MSG_DEBUG("connectCollection() connection=" << connection
                 << ", name=" << collectionName << ", contextID=" << contextId);
   if (contextId >= m_dbSessionVec.size()) {
      ATH_MSG_WARNING("connectCollection: Using default input Stream instead of id = " << contextId);
      contextId = IPoolSvc::kInputStream;
   }

   ContextLock lock(contextId, m_pool_mut, m_pers_mut);
   // Check POOL FileCatalog entry.
   bool insertFile = false;
   if (connection.starts_with("PFN:")) {
      std::string fid, fileType;
      m_catalog->lookupFileByPFN(connection.substr(4), fid, fileType);
      if (fid.empty()) { // No entry in file catalog
         insertFile = true;
         ATH_MSG_INFO("File is not in Catalog! Attempt to open it anyway.");
      }
   }

   // Check whether Collection Container exists.
   std::unique_ptr<pool::IDatabase> dbH = getDbHandle(contextId, connection);
   if( dbH ) {
      try {
         if (dbH->openMode() == Io::INVALID) {
            dbH->connectForRead();
         }
         std::map<unsigned int, unsigned int>::const_iterator maxFileIter = m_contextMaxFile.find(contextId);
         if (maxFileIter != m_contextMaxFile.end() && maxFileIter->second > 0 && !dbH->fid().empty()) {
            const Guid guid(dbH->fid());
            m_guidLists[contextId].remove(guid);
            m_guidLists[contextId].push_back(guid);
            while (m_guidLists[contextId].size() > maxFileIter->second + 1) {
               this->disconnectDb("FID:" + m_guidLists[contextId].begin()->toString(), contextId).ignore();
            }
         }
      } catch (std::exception& e) {
         ATH_MSG_INFO("Failed to open container to check POOL collection - trying.");
      }
   }
   // For multithreaded processing (with multiple events in flight),
   // increase virtual tree size to accomodate back reads
   if (m_useROOTMaxTree && contextId == IPoolSvc::kInputStream && Gaudi::Concurrency::ConcurrencyFlags::numConcurrentEvents() > 1) {
      if (!this->setAttribute("TREE_MAX_VIRTUAL_SIZE", "-1", pool::ROOT_StorageType.type(), connection.substr(4), "CollectionTree", contextId).isSuccess()) {
         ATH_MSG_DEBUG("Failed to increase maximum virtual TTree size.");
      }
   }
   if (insertFile) return(StatusCode::RECOVERABLE);

   return(StatusCode::SUCCESS);
}
//__________________________________________________________________________
StatusCode PoolSvc::checkCollection(const std::string& connection,
		unsigned int contextId,
                bool noContainer) const {
   ContextLock lock(contextId, m_pool_mut, m_pers_mut);
   std::unique_ptr<pool::IDatabase> dbH = getDbHandle(contextId, connection);
   if (dbH != nullptr && !dbH->fid().empty()) {
      if (noContainer) {
         return(StatusCode::SUCCESS); // no events
      }
      if (m_attemptCatalogPatch.value()) {
         patchCatalog(connection.substr(4), *dbH);
      }
      return(StatusCode::SUCCESS);
   }
   return(StatusCode::FAILURE);
}
//__________________________________________________________________________
void PoolSvc::patchCatalog(const std::string& pfn, pool::IDatabase& dbH) const {
   std::scoped_lock lock(m_pool_mut);
   dbH.setTechnology(pool::ROOT_StorageType.type());
   std::string fid = dbH.fid();
   pool::IFileCatalog* catalog_locked ATLAS_THREAD_SAFE = m_catalog;
   catalog_locked->registerPFN(pfn, "ROOT_All", fid);
}
//__________________________________________________________________________
Token* PoolSvc::getToken(const std::string& connection,
	                      const std::string& collection,
	                      const unsigned long ientry) const {
   std::lock_guard<CallMutex> lock(*m_pers_mut[IPoolSvc::kInputStream]);
   std::unique_ptr<pool::IDatabase> dbH = getDbHandle(IPoolSvc::kInputStream, connection);
   if (dbH == nullptr) {
      return(nullptr);
   }
   if (dbH->openMode() == Io::INVALID) {
      dbH->connectForRead();
   }
   std::unique_ptr<pool::IContainer> contH = getContainerHandle(dbH.get(), collection);
   if (contH == nullptr) {
      return(nullptr);
   }
   auto tokenIter = std::unique_ptr<pool::ITokenIterator>(contH->tokens());
   // the Token returned by the iterator has the refCount already increased
   return tokenIter->seek(ientry)? tokenIter->next() : nullptr;
}
//__________________________________________________________________________
StatusCode PoolSvc::connect(Io::IoFlag type, unsigned int contextId) {
   if (type != Io::READ) {
      if (contextId >= m_dbSessionVec.size()) {
         ATH_MSG_WARNING("connect: Using default output Stream instead of id = " << contextId);
         contextId = IPoolSvc::kOutputStream;
      }
   } else {
      if (contextId > m_dbSessionVec.size()) {
         ATH_MSG_WARNING("connect: Using default input Stream instead of id = " << contextId);
         contextId = IPoolSvc::kInputStream;
      } else if (contextId == m_dbSessionVec.size()) {
         ATH_MSG_INFO("Connecting to InputStream for: " << contextId);
         contextId = this->getInputContext("");
      }
   }
   if (contextId >= m_dbSessionVec.size()) {
      return(StatusCode::FAILURE);
   }
   ContextLock lock(contextId, m_pool_mut, m_pers_mut);
   auto session = m_dbSessionVec[contextId];
   // Connect to a logical database using the pre-defined technology and dbID
   if (session->transaction().isActive()) {
      return(StatusCode::SUCCESS);
   }
   if (!session->start(type)) {
      ATH_MSG_ERROR("connect failed session = " << session << " type = " << type);
      return(StatusCode::FAILURE);
   }

   return(StatusCode::SUCCESS);
}
//__________________________________________________________________________
StatusCode PoolSvc::commit(unsigned int contextId) const {
   if (contextId >= m_dbSessionVec.size()) {
      return(StatusCode::FAILURE);
   }
   ContextLock lock(contextId, m_pool_mut, m_pers_mut);
   auto session = m_dbSessionVec[contextId];
   if (session != nullptr && session->transaction().isActive()) {
      if (!session->commit()) {
         ATH_MSG_ERROR("POOL commit failed " << session);
         return(StatusCode::FAILURE);
      }
      if (session->transaction().type() == Io::READ) {
         session->disconnectAll();
      }
   }
   return(StatusCode::SUCCESS);
}
//__________________________________________________________________________
StatusCode PoolSvc::commitAndHold(unsigned int contextId) const {
   if (contextId >= m_dbSessionVec.size()) {
      return(StatusCode::FAILURE);
   }
   ContextLock lock(contextId, m_pool_mut, m_pers_mut);
   pool::ISession* session = m_dbSessionVec[contextId];
   if (session != nullptr && session->transaction().isActive()) {
      if (!session->commitAndHold()) {
         ATH_MSG_ERROR("POOL commitAndHold failed " << session);
         return(StatusCode::FAILURE);
      }
   }
   return(StatusCode::SUCCESS);
}
//__________________________________________________________________________
StatusCode PoolSvc::disconnect(unsigned int contextId) const {
   ATH_MSG_DEBUG("Disconnect request for contextId=" << contextId);
   if (contextId >= m_dbSessionVec.size()) {
      return(StatusCode::SUCCESS);
   }
   ContextLock lock(contextId, m_pool_mut, m_pers_mut);
   pool::ISession* session = m_dbSessionVec[contextId];
   if (session != nullptr && session->transaction().isActive()) {
      if (!commit(contextId).isSuccess()) {
         ATH_MSG_ERROR("disconnect failed to commit " << session);
         return(StatusCode::FAILURE);
      }
      if (session->disconnectAll()) {
         ATH_MSG_DEBUG("Disconnected PersistencySvc session");
      } else {
         ATH_MSG_ERROR("disconnect failed to diconnect PersistencySvc");
         return(StatusCode::FAILURE);
      }
   }
   return(StatusCode::SUCCESS);
}
//__________________________________________________________________________
StatusCode PoolSvc::disconnectDb(const std::string& connection, unsigned int contextId) const {
   if (contextId >= m_dbSessionVec.size()) {
      return(StatusCode::SUCCESS);
   }
   ContextLock lock(contextId, m_pool_mut, m_pers_mut);
   std::unique_ptr<pool::IDatabase> dbH = getDbHandle(contextId, connection);
   if (dbH == nullptr) {
      ATH_MSG_ERROR("Failed to get Session/DatabaseHandle.");
      return(StatusCode::FAILURE);
   }
   std::map<unsigned int, unsigned int>::const_iterator maxFileIter = m_contextMaxFile.find(contextId);
   if (maxFileIter != m_contextMaxFile.end() && maxFileIter->second > 0) {
      m_guidLists[contextId].remove(Guid(dbH->fid()));
   }
   dbH->disconnect();
   return(StatusCode::SUCCESS);
}
//_______________________________________________________________________
StatusCode PoolSvc::getAttribute(const std::string& optName,
		std::string& data,
		long tech,
		unsigned int contextId) const {
   if (contextId >= m_dbSessionVec.size()) {
      ATH_MSG_WARNING("getAttribute: Using default input Stream instead of id = " << contextId);
      contextId = IPoolSvc::kInputStream;
   }
   ContextLock lock(contextId, m_pool_mut, m_pers_mut);
   pool::ISession* sesH = m_dbSessionVec[contextId];
   std::ostringstream oss;
   if (data == "DbLonglong") {
      oss << std::dec << sesH->technologySpecificAttributes(tech).attribute<long long int>(optName);
   } else if (data == "double") {
      oss << std::dec << sesH->technologySpecificAttributes(tech).attribute<double>(optName);
   } else {
      oss << std::dec << sesH->technologySpecificAttributes(tech).attribute<int>(optName);
   }
   data = oss.str();
   ATH_MSG_INFO("Domain attribute [" << optName << "]" << ": " << data);
   return(StatusCode::SUCCESS);
}
//_______________________________________________________________________
StatusCode PoolSvc::getAttribute(const std::string& optName,
		std::string& data,
		long tech,
		const std::string& dbName,
		const std::string& contName,
		unsigned int contextId) const {
   ContextLock lock(contextId, m_pool_mut, m_pers_mut);
   std::unique_ptr<pool::IDatabase> dbH = getDbHandle(contextId, dbName);
   if (dbH == nullptr) {
      ATH_MSG_DEBUG("getAttribute: Failed to get Session/DatabaseHandle to get POOL property.");
      return(StatusCode::FAILURE);
   }
   if (dbH->openMode() == Io::INVALID) {
      if (m_dbSessionVec[contextId]->transaction().type() == Io::WRITE || m_dbSessionVec[contextId]->transaction().type() == Io::APPEND) {
         dbH->setTechnology(tech);
         dbH->connectForWrite();
      } else {
         dbH->connectForRead();
      }
   }
   std::ostringstream oss;
   if (contName.empty()) {
      if (data == "DbLonglong") {
         oss << std::dec << dbH->technologySpecificAttributes().attribute<long long int>(optName);
      } else if (data == "double") {
         oss << std::dec << dbH->technologySpecificAttributes().attribute<double>(optName);
      } else if (data == "string") {
         oss << dbH->technologySpecificAttributes().attribute<char*>(optName);
      } else {
         oss << std::dec << dbH->technologySpecificAttributes().attribute<int>(optName);
      }
      ATH_MSG_INFO("Database (" << dbH->pfn() << ") attribute [" << optName << "]" << ": " << oss.str());
   } else {
      std::unique_ptr<pool::IContainer> contH = getContainerHandle(dbH.get(), contName);
      if (contH == nullptr) {
         ATH_MSG_DEBUG("Failed to get ContainerHandle to get POOL property.");
         return(StatusCode::FAILURE);
      }
      if (data == "DbLonglong") {
         oss << std::dec << contH->technologySpecificAttributes().attribute<long long int>(optName);
      } else if (data == "double") {
         oss << std::dec << contH->technologySpecificAttributes().attribute<double>(optName);
      } else {
         oss << std::dec << contH->technologySpecificAttributes().attribute<int>(optName);
      }
      ATH_MSG_INFO("Container attribute [" << contName << "." << optName << "]: " << oss.str());
   }
   data = oss.str();
   return(StatusCode::SUCCESS);
}
//_______________________________________________________________________
StatusCode PoolSvc::setAttribute(const std::string& optName,
		const std::string& data,
		long tech,
		unsigned int contextId) const {
   if (contextId >= m_dbSessionVec.size()) {
      ATH_MSG_WARNING("setAttribute: Using default output Stream instead of id = " << contextId);
      contextId = IPoolSvc::kOutputStream;
   }
   ContextLock lock(contextId, m_pool_mut, m_pers_mut);
   pool::ISession* sesH = m_dbSessionVec[contextId];
   if (data[data.size() - 1] == 'L') {
      if (!sesH->technologySpecificAttributes(tech).setAttribute<long long int>(optName, atoll(data.c_str()))) {
         ATH_MSG_DEBUG("Failed to set POOL property, " << optName << " to " << data);
         return(StatusCode::FAILURE);
      }
   } else {
      if (!sesH->technologySpecificAttributes(tech).setAttribute<int>(optName, atoi(data.c_str()))) {
         ATH_MSG_DEBUG("Failed to set POOL property, " << optName << " to " << data);
         return(StatusCode::FAILURE);
      }
   }
   return(StatusCode::SUCCESS);
}
//_______________________________________________________________________
StatusCode PoolSvc::setAttribute(const std::string& optName,
		const std::string& data,
		long tech,
		const std::string& dbName,
		const std::string& contName,
		unsigned int contextId) const {
   if (contextId >= m_dbSessionVec.size()) {
      ATH_MSG_WARNING("setAttribute: Using default output Stream instead of id = " << contextId);
      contextId = IPoolSvc::kOutputStream;
   }
   ContextLock lock(contextId, m_pool_mut, m_pers_mut);
   std::unique_ptr<pool::IDatabase> dbH = getDbHandle(contextId, dbName);
   if (dbH == nullptr) {
      ATH_MSG_DEBUG("Failed to get Session/DatabaseHandle to set POOL property.");
      return(StatusCode::FAILURE);
   }
   if (dbH->openMode() == Io::INVALID) {
      if (m_dbSessionVec[contextId]->transaction().type() == Io::WRITE || m_dbSessionVec[contextId]->transaction().type() == Io::APPEND) {
         dbH->setTechnology(tech);
         dbH->connectForWrite();
      } else {
         dbH->connectForRead();
      }
   }
   bool retError = false;
   std::string objName;
   bool hasTTreeName = contName.starts_with("TTree=");
   if (contName.empty() || hasTTreeName || m_dbSessionVec[contextId]->transaction().type() == Io::READ) {
      objName = hasTTreeName ? contName.substr(6) : contName;
      if( !isNumber(data) ) {
         retError = dbH->technologySpecificAttributes().setAttribute(optName, data.c_str(), objName);
      } else if( data[data.size() - 1] == 'L' ) {
         retError = dbH->technologySpecificAttributes().setAttribute<long long int>(optName, atoll(data.c_str()), objName);
      } else {
         retError = dbH->technologySpecificAttributes().setAttribute<int>(optName, atoi(data.c_str()), objName);
      }
      if (!retError) {
         ATH_MSG_DEBUG("Failed to set POOL property, " << optName << " to " << data);
         return(StatusCode::FAILURE);
      }
   } else {
      std::unique_ptr<pool::IContainer> contH = getContainerHandle(dbH.get(), contName);
      if (contH == nullptr) {
         ATH_MSG_DEBUG("Failed to get ContainerHandle to set POOL property.");
         return(StatusCode::FAILURE);
      }
      if (auto p = contName.find('('); p != std::string::npos) {
         objName = contName.substr(p + 1); // Get BranchName between parenthesis
         objName.erase(objName.find(')'));
      } else if (auto p = contName.find("::"); p != std::string::npos) {
         objName = contName.substr(p + 2); // Split off Tree name
      } else if (auto p = contName.find('_'); p != std::string::npos) {
         objName = contName.substr(p + 1); // Split off "POOLContainer"
         objName.erase(objName.find('/')); // Split off key
      }
      std::string::size_type off = 0;
      while ((off = objName.find_first_of("<>/")) != std::string::npos) {
         objName[off] = '_'; // Replace special chars (e.g. templates)
      }
      if (data[data.size() - 1] == 'L') {
         retError = contH->technologySpecificAttributes().setAttribute<long long int>(optName, atoll(data.c_str()), objName);
      } else {
         retError = contH->technologySpecificAttributes().setAttribute<int>(optName, atoi(data.c_str()), objName);
      }
      if (!retError) {
         ATH_MSG_DEBUG("Failed to set POOL container property, " << optName << " for " << contName << " : " << objName << " to " << data);
         return(StatusCode::FAILURE);
      }
   }
   return(StatusCode::SUCCESS);
}

//__________________________________________________________________________
pool::IFileCatalog* PoolSvc::createCatalog() {
   pool::IFileCatalog* ctlg = new pool::IFileCatalog;
   ctlg->removeCatalog("*");
   for (auto& catalog : m_readCatalog.value()) {
      ATH_MSG_DEBUG("POOL ReadCatalog is " << catalog);
      if (catalog.starts_with("apcfile:") || catalog.starts_with("prfile:")) {
         std::string::size_type cpos = catalog.find(':');
         // check for file accessed via ATLAS_POOLCOND_PATH
         std::string file = poolCondPath(catalog.substr(cpos + 1));
         if (!file.empty()) {
            ATH_MSG_INFO("Resolved path (via ATLAS_POOLCOND_PATH) is " << file);
            ctlg->addReadCatalog("file:" + file);
         } else {
            // As backup, check for file accessed via PathResolver
            file = PathResolver::find_file(catalog.substr(cpos + 1), "DATAPATH");
            if (!file.empty()) {
               ATH_MSG_INFO("Resolved path (via DATAPATH) is " << file);
               ctlg->addReadCatalog("file:" + file);
            } else {
               ATH_MSG_INFO("Unable find catalog "
	               << catalog
	               << " in $ATLAS_POOLCOND_PATH and $DATAPATH");
            }
         }
      } else {
         ctlg->addReadCatalog(catalog);
      }
   }
   try {
      ATH_MSG_INFO("POOL WriteCatalog is " << m_writeCatalog.value());
      ctlg->setWriteCatalog(m_writeCatalog.value());
   } catch(std::exception& e) {
      ATH_MSG_ERROR("setWriteCatalog - caught exception: " << e.what());
      return(nullptr); // This catalog is not setup properly!
   }
   return(ctlg);
}

//__________________________________________________________________________
PoolSvc::~PoolSvc() {
}
//__________________________________________________________________________
std::unique_ptr<pool::IDatabase> PoolSvc::getDbHandle(unsigned int contextId, const std::string& dbName) const {
   if (contextId >= m_dbSessionVec.size()) {
      ATH_MSG_WARNING("getDbHandle: Using default input Stream instead of id = " << contextId);
      contextId = IPoolSvc::kInputStream;
   }
   pool::ISession* sesH = m_dbSessionVec[contextId];
   if (!sesH->transaction().isActive()) {
      Io::IoFlag transMode = Io::READ;
      ATH_MSG_DEBUG("Start transaction, type = " << transMode);
      if (!sesH->transaction().start(transMode)) {
         ATH_MSG_WARNING("Failed to start transaction, type = " << transMode);
         return(nullptr);
      }
   }
   if (dbName.starts_with("PFN:")) {
      return sesH->databaseHandle(dbName.substr(4), pool::DatabaseSpecification::PFN);
   } else if (dbName.starts_with("LFN:")) {
      return sesH->databaseHandle(dbName.substr(4), pool::DatabaseSpecification::LFN);
   } else if (dbName.starts_with("FID:")) {
      return sesH->databaseHandle(dbName.substr(4), pool::DatabaseSpecification::FID);
   }
   return sesH->databaseHandle(dbName, pool::DatabaseSpecification::PFN);
}
//__________________________________________________________________________
std::unique_ptr<pool::IContainer> PoolSvc::getContainerHandle(pool::IDatabase* dbH, const std::string& contName) const {
   pool::IContainer* contH = nullptr;
   if (dbH == nullptr) {
      ATH_MSG_DEBUG("No DatabaseHandle to get Container.");
      return(nullptr);
   }
   if (contName.find("DataHeader") != std::string::npos) {
      contH = dbH->containerHandle(contName.substr(0, contName.find("_p")));
   } else {
      contH = dbH->containerHandle(contName);
   }
   return(std::unique_ptr<pool::IContainer>(contH));
}
//__________________________________________________________________________
std::string PoolSvc::poolCondPath(const std::string& leaf) {
   // look for files at $ATLAS_POOLCOND_PATH/<leaf>
   // return full filename if exists, or empty string if not
   const char* cpath = std::getenv("ATLAS_POOLCOND_PATH");
   if (cpath && strcmp(cpath, "") != 0) {
      const std::string testpath = std::string(cpath) + "/" + leaf;

      // Try to open file for reading. Note that a simple stat call may return
      // a wrong result if the file is residing on an auto-mounted FS (ATR-28801).
      if (FILE* fp = std::fopen(testpath.c_str(), "r")) {
         std::fclose(fp);
         return testpath;
      }
   }
   return {};
}
