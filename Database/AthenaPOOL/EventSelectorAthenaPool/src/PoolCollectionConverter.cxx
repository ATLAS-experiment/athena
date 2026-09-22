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

// Gaudi
#include "GaudiKernel/StatusCode.h"

#include <exception>
#include <format>
#include <mutex>
#include <stdexcept>

//______________________________________________________________________________
PoolCollectionConverter::PoolCollectionConverter(
	const std::string& inputCollection,
	unsigned int contextId,
	const IPoolSvc* svc) :
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
   StatusCode sc = m_poolSvc->connectCollection(m_inputCollection, "Input", m_contextId);
   try {
      m_poolCollection = pool::CollectionService::open("Input", m_inputCollection, m_poolSvc->getInputContextSession(m_contextId));
      m_lastError.clear();
   } catch (std::exception &e) {
      m_lastError = std::format("Failed to open collection '{}': {}", m_inputCollection, e.what());
   }
   if( sc.isRecoverable() || m_poolCollection == nullptr ) {
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
