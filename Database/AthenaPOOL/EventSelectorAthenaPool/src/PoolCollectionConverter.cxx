/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
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
#include "CollectionSvc/ICollectionDescription.h"
#include "StorageSvc/DbType.h"
#include "RootUtils/APRDefaults.h"

// Gaudi
#include "GaudiKernel/StatusCode.h"

#include <exception>
#include <format>

//______________________________________________________________________________
PoolCollectionConverter::PoolCollectionConverter(const std::string& collectionType,
	const std::string& inputCollection,
	unsigned int contextId,
	const IPoolSvc* svc) :
	m_collectionType(collectionType),
	m_connection(),
	m_inputCollection(inputCollection),
	m_contextId(contextId),
	m_poolSvc(svc),
	m_poolCollection(nullptr),
	m_collectionCursor(nullptr) {
}
//______________________________________________________________________________
PoolCollectionConverter::~PoolCollectionConverter() {
   if (m_poolCollection) {
      m_poolCollection->close();
      delete m_collectionCursor; m_collectionCursor = nullptr;
      delete m_poolCollection; m_poolCollection = nullptr;
   }
}
//______________________________________________________________________________
StatusCode PoolCollectionConverter::initialize() {
   // Check if already prefixed
   if (m_inputCollection.starts_with( "PFN:")
           || m_inputCollection.starts_with( "LFN:")
           || m_inputCollection.starts_with( "FID:")) {
      // Already prefixed
      m_connection = m_inputCollection;
   } else {
      // Prefix with PFN:
      m_connection = std::format("PFN:{}", m_inputCollection);
   }
   try {
      if (m_collectionType == "RootCollection") {
         m_poolCollection = m_poolSvc->createCollection(m_connection, "Input", pool::ROOT_StorageType.type(), m_contextId);
      }
      if (m_poolCollection == nullptr) { // Open as ImplicitCollection if technologies fail, or none was specified
         m_poolCollection = m_poolSvc->createCollection(m_connection, "Input", pool::POOL_StorageType.type(), m_contextId);
      }
   } catch (std::exception &e) {
      if (m_poolCollection == nullptr) return StatusCode::RECOVERABLE;
   }
   return StatusCode::SUCCESS;
}
//______________________________________________________________________________
StatusCode PoolCollectionConverter::disconnectDb() {
   if (m_poolCollection == nullptr) {
      return StatusCode::SUCCESS;
   }
   if (m_poolCollection->description().type() == pool::POOL_StorageType.type()) {
      return m_poolSvc->disconnectDb(m_connection);
   }
   return StatusCode::SUCCESS;
}
//______________________________________________________________________________
StatusCode PoolCollectionConverter::isValid() const {
   return m_poolCollection != nullptr ? StatusCode::SUCCESS : StatusCode::FAILURE;
}
//______________________________________________________________________________
pool::ICollectionCursor& PoolCollectionConverter::selectAll() {
   delete m_collectionCursor; m_collectionCursor = nullptr;
   m_collectionCursor = &m_poolCollection->cursor();
   return *m_collectionCursor;
}
