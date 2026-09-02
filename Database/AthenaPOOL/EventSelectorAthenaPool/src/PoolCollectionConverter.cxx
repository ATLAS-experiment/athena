/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/** @file PoolCollectionConverter.cxx
 *  @brief This file contains the implementation for the PoolCollectionConverter class.
 *  @author Peter van Gemmeren <gemmeren@anl.gov>
 **/

#include "PoolCollectionConverter.h"
#include "PoolSvc/IPoolSvc.h"
#include "PersistentDataModel/Token.h"

// Pool
#include "CollectionSvc/ICollection.h"
#include "CollectionSvc/ICollectionCursor.h"
#include "CollectionSvc/CollectionService.h"

#include "PersistencySvc/ISession.h"

#include "StorageSvc/DbType.h"

// Gaudi
#include "GaudiKernel/StatusCode.h"

#include <exception>
#include <format>
#include <mutex>
#include <stdexcept>

//______________________________________________________________________________
PoolCollectionConverter::PoolCollectionConverter(const std::string& collectionType,
	const std::string& inputCollection,
	unsigned int contextId,
	const IPoolSvc* svc) :
	m_collectionType(collectionType),
	m_inputCollection(inputCollection),
	m_contextId(contextId),
	m_poolSvc(svc),
	m_poolCollection(nullptr) {
}
//______________________________________________________________________________
PoolCollectionConverter::~PoolCollectionConverter() {
   if (m_poolCollection) {
      m_poolCollection->close();
   }
}
//______________________________________________________________________________
StatusCode PoolCollectionConverter::initialize() {
   // Check if already prefixed
   if (!m_inputCollection.starts_with( "PFN:")
           && !m_inputCollection.starts_with( "LFN:")
           && !m_inputCollection.starts_with( "FID:")) {
      // Prefix with PFN:
      m_inputCollection = std::format("PFN:{}", m_inputCollection);
   }
   StatusCode sc = StatusCode::SUCCESS;
   try {
      if (m_collectionType == "RootCollection") {
         sc = m_poolSvc->connectCollection(m_inputCollection, "Input", pool::ROOT_StorageType.type(), m_contextId);
         m_poolCollection = createCollection(m_inputCollection, "Input", pool::ROOT_StorageType.type(), m_contextId);
      }
      if (m_poolCollection == nullptr) { // Open as ImplicitCollection if technologies fail, or none was specified
         sc = m_poolSvc->connectCollection(m_inputCollection, "Input", pool::POOL_StorageType.type(), m_contextId);
         m_poolCollection = createCollection(m_inputCollection, "Input", pool::POOL_StorageType.type(), m_contextId);
      }
   } catch (std::exception &e) {
      if (m_poolCollection == nullptr) return StatusCode::RECOVERABLE;
   }
   bool insertFile = false;
   if (sc.isRecoverable()) {
      insertFile = true;
   } else if (sc.isFailure()) {
      return StatusCode::FAILURE;
   }
   if (m_poolCollection == nullptr || insertFile) {
      return m_poolSvc->checkCollection(m_inputCollection, m_contextId, m_poolCollection == nullptr);
   }
   return StatusCode::SUCCESS;
}
//______________________________________________________________________________
std::unique_ptr<pool::ICollection> PoolCollectionConverter::createCollection(const std::string& connection,
                const std::string& collectionName,
                const pool::DbType& collectionType,
                unsigned int contextId) const {
   // access to these variables is serial, since this is called by event selector only
   pool::CollectionService collSvc ATLAS_THREAD_SAFE = pool::CollectionService();
   std::unique_ptr<pool::ICollection> collPtr ATLAS_THREAD_SAFE ;

   // Try to open EventTags Collection in the input file
   try {
      collPtr = collSvc.open(collectionName, collectionType, connection, m_poolSvc->getInputContextSession(contextId)) ;
   } catch (std::exception &e) {
      collPtr = nullptr;
   }
   return collPtr;
}
//______________________________________________________________________________
StatusCode PoolCollectionConverter::disconnectDb() {
   if (m_poolCollection == nullptr) {
      return StatusCode::SUCCESS;
   }
   return m_poolSvc->disconnectDb(m_inputCollection);
}
//______________________________________________________________________________
StatusCode PoolCollectionConverter::isValid() const {
   return m_poolCollection != nullptr ? StatusCode::SUCCESS : StatusCode::FAILURE;
}
//______________________________________________________________________________
std::unique_ptr<pool::ICollectionCursor> PoolCollectionConverter::selectAll() {
   if (m_poolCollection == nullptr)[[unlikely]] {
     throw std::runtime_error("PoolCollectionConverter::selectAll: m_poolCollection is nullptr.");
   }
   return m_poolCollection->cursor();
}
