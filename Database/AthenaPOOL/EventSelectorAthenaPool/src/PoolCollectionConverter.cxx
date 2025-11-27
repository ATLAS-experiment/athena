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
#include "CollectionBase/ICollection.h"
#include "CollectionBase/ICollectionCursor.h"
#include "CollectionBase/ICollectionDescription.h"

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
	m_collectionCursor(nullptr),
	m_inputContainer() {
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
   // Find out if the user specified a container
   const std::string collectionType = m_collectionType;
   std::string::size_type p_colon = collectionType.rfind(':');
   if (p_colon != std::string::npos) {
      m_inputContainer = collectionType.substr(p_colon + 1);
      m_collectionType = collectionType.substr(0, p_colon);
   }
   if (m_collectionType == "ImplicitCollection") {
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
         m_poolCollection = m_poolSvc->createCollection("RootCollection", m_connection, m_inputCollection, m_contextId);
      } catch (std::exception &e) {
         m_poolCollection = nullptr;
      }
      if (m_poolCollection == nullptr) {
         // Now set where to look in the implicit file
         m_inputCollection = std::format("{}(DataHeader)", m_inputContainer);
      }
   }
   try {
      if (m_poolCollection == nullptr) {
         m_poolCollection = m_poolSvc->createCollection(m_collectionType, m_connection, m_inputCollection, m_contextId);
      }
      if (m_poolCollection == nullptr && m_collectionType == "ImplicitCollection") {
         m_inputCollection = std::format("{}_DataHeader", m_inputContainer);
         m_poolCollection = m_poolSvc->createCollection(m_collectionType, m_connection, m_inputCollection, m_contextId);
      }
   } catch (std::exception &e) {
      return StatusCode::RECOVERABLE;
   }
   return StatusCode::SUCCESS;
}
//______________________________________________________________________________
StatusCode PoolCollectionConverter::disconnectDb() {
   if (m_poolCollection == nullptr) {
      return StatusCode::SUCCESS;
   }
   if (m_poolCollection->description().type() == "ImplicitCollection") {
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
