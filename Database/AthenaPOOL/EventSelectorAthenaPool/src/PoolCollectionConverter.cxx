/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/** @file PoolCollectionConverter.cxx
 *  @brief This file contains the implementation for the PoolCollectionConverter class.
 *  @author Peter van Gemmeren <gemmeren@anl.gov>
 **/

#include "PoolCollectionConverter.h"
#include "PoolSvc/IPoolSvc.h"
#include "PoolSvc/ISession.h"
#include "PersistentDataModel/Token.h"

// Pool
#include "CollectionSvc/ICollection.h"
#include "CollectionSvc/ICollectionCursor.h"
#include "CollectionSvc/CollectionService.h"
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
      delete m_poolCollection; m_poolCollection = nullptr;
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
      sc = m_poolSvc->connectCollection(m_inputCollection, "Input", m_contextId);
      m_poolCollection = pool::CollectionService::open("Input", m_inputCollection, m_poolSvc->getInputContextSession(m_contextId));
   } catch (std::exception &e) {
      return StatusCode::RECOVERABLE;
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
