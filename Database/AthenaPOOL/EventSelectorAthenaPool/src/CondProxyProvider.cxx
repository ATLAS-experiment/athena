/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/** @file CondProxyProvider.cxx
 *  @brief This file contains the implementation for the CondProxyProvider class.
 *  @author Peter van Gemmeren <gemmeren@anl.gov>
 **/

#include "CondProxyProvider.h"
#include "registerKeys.h"

#include "PersistentDataModel/DataHeader.h"
#include "PersistentDataModel/TokenAddress.h"
#include "PoolSvc/IPoolSvc.h"

// Framework
#include "GaudiKernel/ClassID.h"
#include "GaudiKernel/StatusCode.h"

#include "StoreGate/StoreGateSvc.h"

// Pool
#include "CollectionSvc/ICollection.h"
#include "CollectionSvc/ICollectionCursor.h"
#include "CollectionSvc/CollectionService.h"

#include "StorageSvc/DbType.h"

#include <vector>
#include <list>

//________________________________________________________________________________
CondProxyProvider::CondProxyProvider(const std::string& name, ISvcLocator* pSvcLocator) :
    base_class(name, pSvcLocator){
}
//________________________________________________________________________________
CondProxyProvider::~CondProxyProvider() {
}
//________________________________________________________________________________
StatusCode CondProxyProvider::initialize() {
   ATH_MSG_INFO("Initializing " << name());
   // Check for input collection
   if (m_inputCollectionsProp.value().size() == 0) {
      return StatusCode::FAILURE;
   }
   // Retrieve PoolSvc
   ATH_CHECK( m_poolSvc.retrieve() );

   // Get PoolSvc and connect as "Conditions"
   IPoolSvc *poolSvc = m_poolSvc.get();
   m_contextId = poolSvc->getInputContext("Conditions");
   ATH_CHECK( poolSvc->connect(Io::READ, m_contextId) );

   for( const auto &inp : m_inputCollectionsProp.value() ) {
      ATH_MSG_INFO("Inputs: " << inp);
   }
   return StatusCode::SUCCESS;
}
//________________________________________________________________________________
StatusCode CondProxyProvider::preLoadAddresses(StoreID::type storeID,
		IAddressProvider::tadList& tads) {
   if (storeID != StoreID::DETECTOR_STORE) {
      return StatusCode::SUCCESS;
   }
   ServiceHandle<StoreGateSvc> detectorStoreSvc("DetectorStore", name());
   // Retrieve DetectorStoreSvc
   ATH_CHECK( detectorStoreSvc.retrieve() );

   for (const auto &inputCollectionsIterator : m_inputCollectionsProp.value()) {
      // Create an poolCollectionConverter to read the objects in
      ATH_MSG_DEBUG("Try item: \"" << inputCollectionsIterator << "\" from the collection list.");
      std::string inputCollection = inputCollectionsIterator;
      // Check if already prefixed
      if (!inputCollection.starts_with( "PFN:")
              && !inputCollection.starts_with( "LFN:")
              && !inputCollection.starts_with( "FID:")) {
         // Prefix with PFN:
         inputCollection = std::format("PFN:{}", inputCollection);
      }
      StatusCode sc = m_poolSvc->connectCollection(inputCollection, "Input", m_contextId);
      m_poolCollection = pool::CollectionService::open("Input", inputCollection, m_poolSvc->getInputContextSession(m_contextId));
      if( sc.isRecoverable() || m_poolCollection == nullptr ) {
         sc = m_poolSvc->checkCollection(inputCollection, m_contextId, m_poolCollection == nullptr);
      }
      if( !sc.isSuccess() || m_poolCollection == nullptr ) {
         ATH_MSG_ERROR("Could not open item: \"" << inputCollection << "\" from the collection list.");
         return StatusCode::FAILURE;
      }
      std::unique_ptr<pool::ICollectionCursor> headerIterator = m_poolCollection->cursor();
      if (!headerIterator->next()) {
         ATH_MSG_WARNING("Cannot retrieve Collection.");
         continue;
      }
      auto token = std::make_unique<Token>();
      token->fromString(headerIterator->eventRef().toString());
      const std::string key = token->dbID().toString();
      CxxUtils::RefCountedPtr<TokenAddress> tokenAddr
           (new TokenAddress(pool::POOL_StorageType.type(), ClassID_traits<DataHeader>::ID(), "", key, m_contextId, std::move(token)));
      if (!detectorStoreSvc->recordAddress(std::move(tokenAddr)).isSuccess()) {
         ATH_MSG_ERROR("Cannot record DataHeader.");
         return StatusCode::FAILURE;
      }
      const DataHeader* dataHeader = nullptr;
      if (!detectorStoreSvc->retrieve(dataHeader, key).isSuccess()) {
         ATH_MSG_DEBUG("Cannot retrieve DataHeader from DetectorStore.");
         continue;
      }
      ATH_MSG_DEBUG("The current File contains: " << dataHeader->size() << " objects");
      for (const auto& element : *dataHeader) {
         SG::TransientAddress* tadd = element.getAddress(pool::POOL_StorageType.type());
         if (tadd->clID() == ClassID_traits<DataHeader>::ID()) {
            delete tadd; tadd = 0;
         } else {
            ATH_MSG_DEBUG("preLoadAddresses: DataObject address, clid = " << tadd->clID() << ", name = " << tadd->name());
            tads.push_back(tadd);
         }
         EventSelectorAthenaPoolUtil::registerKeys(element, &*detectorStoreSvc);
      }
      m_poolSvc->disconnectDb(inputCollection).ignore();
   }
   return StatusCode::SUCCESS;
}
//________________________________________________________________________________
StatusCode CondProxyProvider::loadAddresses(StoreID::type /*storeID*/,
	IAddressProvider::tadList& /*tads*/) {
   return StatusCode::SUCCESS;
}
//________________________________________________________________________________
StatusCode CondProxyProvider::updateAddress(StoreID::type /*storeID*/,
                                            SG::TransientAddress* /*tad*/,
                                            const EventContext& /*ctx*/) {
   return StatusCode::FAILURE;
}
