/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/** @file CondProxyProvider.cxx
 *  @brief This file contains the implementation for the CondProxyProvider class.
 *  @author Peter van Gemmeren <gemmeren@anl.gov>
 **/

#include "CondProxyProvider.h"
#include "PoolCollectionConverter.h"
#include "registerKeys.h"

#include "PersistentDataModel/DataHeader.h"
#include "PersistentDataModel/TokenAddress.h"
#include "PoolSvc/IPoolSvc.h"
#include "RootUtils/APRDefaults.h"

// Framework
#include "GaudiKernel/ClassID.h"
#include "GaudiKernel/StatusCode.h"

#include "StoreGate/StoreGateSvc.h"

// Pool
#include "CollectionBase/ICollectionCursor.h"
#include "StorageSvc/DbType.h"

#include <vector>
#include <list>

//________________________________________________________________________________
CondProxyProvider::CondProxyProvider(const std::string& name, ISvcLocator* pSvcLocator) :
    base_class(name, pSvcLocator),
	m_contextId(IPoolSvc::kInputStream)
	{
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
   // Retrieve AthenaPoolCnvSvc
   ATH_CHECK( m_athenaPoolCnvSvc.retrieve() );

   // Get PoolSvc and connect as "Conditions"
   IPoolSvc *poolSvc = m_athenaPoolCnvSvc->getPoolSvc();
   m_contextId = poolSvc->getInputContext("Conditions");
   ATH_CHECK( poolSvc->connect(pool::ITransaction::READ, m_contextId) );

   for( const auto &inp : m_inputCollectionsProp.value() ) {
      ATH_MSG_INFO("Inputs: " << inp);
   }
   // Initialize
   m_inputCollectionsIterator = m_inputCollectionsProp.value().begin();
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

   // Create an poolCollectionConverter to read the objects in
   std::unique_ptr<PoolCollectionConverter> poolCollectionConverter = getCollectionCnv();
   if (!poolCollectionConverter) {
     return StatusCode::FAILURE;
   }
   // Create DataHeader iterators
   pool::ICollectionCursor* headerIterator = &poolCollectionConverter->selectAll();

   for (int verNumber = 0; verNumber < 100; verNumber++) {
      if (!headerIterator->next()) {
         poolCollectionConverter->disconnectDb().ignore();
         poolCollectionConverter.reset();
         ++m_inputCollectionsIterator;
         if (m_inputCollectionsIterator != m_inputCollectionsProp.value().end()) {
            // Create PoolCollectionConverter for input file
            poolCollectionConverter = getCollectionCnv();
            if (!poolCollectionConverter) {
               return StatusCode::FAILURE;
            }
            // Get DataHeader iterator
            headerIterator = &poolCollectionConverter->selectAll();
            if (!headerIterator->next()) {
               return StatusCode::FAILURE;
            }
         } else {
            break;
         }
      }
      SG::VersionedKey myVersKey(name(), verNumber);
      auto token = std::make_unique<Token>();
      token->fromString(headerIterator->eventRef().toString());
      CxxUtils::RefCountedPtr<TokenAddress> tokenAddr
        (new TokenAddress(pool::POOL_StorageType.type(), ClassID_traits<DataHeader>::ID(), "", myVersKey, m_contextId, std::move(token)));
      if (!detectorStoreSvc->recordAddress(std::move(tokenAddr)).isSuccess()) {
         ATH_MSG_ERROR("Cannot record DataHeader.");
         return StatusCode::FAILURE;
      }
   }
   std::list<SG::ObjectWithVersion<DataHeader> > allVersions;
   if (!detectorStoreSvc->retrieveAllVersions(allVersions, name()).isSuccess()) {
      ATH_MSG_DEBUG("Cannot retrieve DataHeader from DetectorStore.");
      return StatusCode::SUCCESS;
   }
   for (const auto& version : allVersions) {
      SG::ReadHandle<DataHeader> dataHeader = version.dataObject;
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
//__________________________________________________________________________
std::unique_ptr<PoolCollectionConverter> CondProxyProvider::getCollectionCnv() {
   ATH_MSG_DEBUG("Try item: \"" << *m_inputCollectionsIterator << "\" from the collection list.");
   auto pCollCnv = std::make_unique<PoolCollectionConverter>(std::string("ImplicitCollection:") + APRDefaults::TTreeNames::DataHeader,
	   *m_inputCollectionsIterator,
	   m_contextId,
	   m_athenaPoolCnvSvc->getPoolSvc());
   if (!pCollCnv->initialize().isSuccess()) {
      // Close previous collection.
      pCollCnv.reset();
      ATH_MSG_ERROR("Unable to open: " << *m_inputCollectionsIterator);
   }
   return(pCollCnv);
}
