/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/** @file PoolCollectionConverter.cxx
 *  @brief This file contains the implementation for the PoolCollectionConverter class.
 *  @author Peter van Gemmeren <gemmeren@anl.gov>
 **/

#include "PoolCollectionConverter.h"
#include "PoolSvc/IPoolSvc.h"
#include "PersistentDataModel/Token.h"

// Pool
#include "CoralBase/AttributeList.h"
#include "CoralBase/Attribute.h"

#include "CollectionBase/ICollection.h"
#include "CollectionBase/ICollectionQuery.h"
#include "CollectionBase/ICollectionCursor.h"
#include "CollectionBase/ICollectionDescription.h"

// Gaudi
#include "GaudiKernel/StatusCode.h"

#include <assert.h>
#include <exception>

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
	m_collectionQuery(nullptr),
	m_inputContainer() {
}
//______________________________________________________________________________
PoolCollectionConverter::~PoolCollectionConverter() {
   if (m_poolCollection) {
      m_poolCollection->close();
      delete m_collectionQuery; m_collectionQuery = nullptr;
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
         // Aready prefixed
         m_connection = m_inputCollection;
      } else {
         // Prefix with PFN:
         m_connection = "PFN:" + m_inputCollection;
      }
      try {
         m_poolCollection = m_poolSvc->createCollection("RootCollection", m_connection, m_inputCollection, m_contextId);
      } catch (std::exception &e) {
         m_poolCollection = nullptr;
      }
      if (m_poolCollection == nullptr) {
         // Now set where to look in the implicit file
         m_inputCollection = m_inputContainer + "(DataHeader)";
      }
   }
   try {
      if (m_poolCollection == nullptr) {
         m_poolCollection = m_poolSvc->createCollection(m_collectionType, m_connection, m_inputCollection, m_contextId);
      }
      if (m_poolCollection == nullptr && m_collectionType == "ImplicitCollection") {
         m_inputCollection = m_inputContainer + "_DataHeader";
         m_poolCollection = m_poolSvc->createCollection(m_collectionType, m_connection, m_inputCollection, m_contextId);
      }
   } catch (std::exception &e) {
      return(StatusCode::RECOVERABLE);
   }
   return(StatusCode::SUCCESS);
}
//______________________________________________________________________________
StatusCode PoolCollectionConverter::disconnectDb() {
   if (m_poolCollection == nullptr) {
      return(StatusCode::SUCCESS);
   }
   if (m_poolCollection->description().type() == "ImplicitCollection") {
      return(m_poolSvc->disconnectDb(m_connection));
   }
   return(StatusCode::SUCCESS);
}
//______________________________________________________________________________
StatusCode PoolCollectionConverter::isValid() const {
   return(m_poolCollection != nullptr ? StatusCode::SUCCESS : StatusCode::FAILURE);
}
//______________________________________________________________________________
pool::ICollectionCursor& PoolCollectionConverter::selectAll() {
   delete m_collectionQuery; m_collectionQuery = nullptr;
   m_collectionQuery = m_poolCollection->newQuery();
   m_collectionQuery->selectAll();
   return(m_collectionQuery->execute());
}
