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

#include <vector>

//________________________________________________________________________________
CondProxyProvider::CondProxyProvider(const std::string& name, ISvcLocator* pSvcLocator) :
    base_class(name, pSvcLocator),
	m_poolCollectionConverter(0),
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
      return(StatusCode::FAILURE);
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
   // Create an m_poolCollectionConverter to read the objects in
   m_poolCollectionConverter = getCollectionCnv();
   if (m_poolCollectionConverter == 0) {
      return(StatusCode::FAILURE);
   }
   return(StatusCode::SUCCESS);
}
//________________________________________________________________________________
StatusCode CondProxyProvider::finalize() {
   if (m_poolCollectionConverter != 0) {
      m_poolCollectionConverter->disconnectDb().ignore();
      delete m_poolCollectionConverter; m_poolCollectionConverter = 0;
   }
   // Release AthenaPoolCnvSvc
   if (!m_athenaPoolCnvSvc.release().isSuccess()) {
      ATH_MSG_WARNING("Cannot release AthenaPoolCnvSvc.");
   }
   return(StatusCode::SUCCESS);
}
//________________________________________________________________________________
StatusCode CondProxyProvider::preLoadAddresses(StoreID::type storeID,
		IAddressProvider::tadList& tads) {
   if (storeID != StoreID::DETECTOR_STORE) {
      return(StatusCode::SUCCESS);
   }
   ServiceHandle<StoreGateSvc> detectorStoreSvc("DetectorStore", name());
   // Retrieve DetectorStoreSvc
   ATH_CHECK( detectorStoreSvc.retrieve() );

   if (m_poolCollectionConverter == nullptr) {
     return StatusCode::FAILURE;
   }
   
   // Create DataHeader iterators
   pool::ICollectionCursor* headerIterator = &m_poolCollectionConverter->selectAll();
   for (int verNumber = 0; verNumber < 100; verNumber++) {
      if (!headerIterator->next()) {
         m_poolCollectionConverter->disconnectDb().ignore();
         delete m_poolCollectionConverter; m_poolCollectionConverter = 0;
         ++m_inputCollectionsIterator;
         if (m_inputCollectionsIterator != m_inputCollectionsProp.value().end()) {
            // Create PoolCollectionConverter for input file
            m_poolCollectionConverter = getCollectionCnv();
            if (m_poolCollectionConverter == 0) {
               return(StatusCode::FAILURE);
            }
            // Get DataHeader iterator
            headerIterator = &m_poolCollectionConverter->selectAll();
            if (!headerIterator->next()) {
               return(StatusCode::FAILURE);
            }
         } else {
            break;
         }
      }
      SG::VersionedKey myVersKey(name(), verNumber);
      auto token = std::make_unique<Token>();
      token->fromString(headerIterator->eventRef().toString());
      TokenAddress* tokenAddr = new TokenAddress(POOL_StorageType, ClassID_traits<DataHeader>::ID(), "", myVersKey, m_contextId, std::move(token));
      if (!detectorStoreSvc->recordAddress(tokenAddr).isSuccess()) {
         ATH_MSG_ERROR("Cannot record DataHeader.");
         return(StatusCode::FAILURE);
      }
   }
   std::list<SG::ObjectWithVersion<DataHeader> > allVersions;
   if (!detectorStoreSvc->retrieveAllVersions(allVersions, name()).isSuccess()) {
      ATH_MSG_DEBUG("Cannot retrieve DataHeader from DetectorStore.");
      return(StatusCode::SUCCESS);
   }
   for (std::list<SG::ObjectWithVersion<DataHeader> >::iterator iter = allVersions.begin();
                   iter != allVersions.end(); ++iter) {
      SG::ReadHandle<DataHeader> dataHeader = iter->dataObject;
      ATH_MSG_DEBUG("The current File contains: " << dataHeader->size() << " objects");
      for (const auto& element : *dataHeader) {
         SG::TransientAddress* tadd = element.getAddress();
         if (tadd->clID() == ClassID_traits<DataHeader>::ID()) {
            delete tadd; tadd = 0;
         } else {
            ATH_MSG_DEBUG("preLoadAddresses: DataObject address, clid = " << tadd->clID() << ", name = " << tadd->name());
            tads.push_back(tadd);
         }
         EventSelectorAthenaPoolUtil::registerKeys(element, &*detectorStoreSvc);
      }
   }

   return(StatusCode::SUCCESS);
}
//________________________________________________________________________________
StatusCode CondProxyProvider::loadAddresses(StoreID::type /*storeID*/,
	IAddressProvider::tadList& /*tads*/) {
   return(StatusCode::SUCCESS);
}
//________________________________________________________________________________
StatusCode CondProxyProvider::updateAddress(StoreID::type /*storeID*/,
                                            SG::TransientAddress* /*tad*/,
                                            const EventContext& /*ctx*/) {
   return(StatusCode::FAILURE);
}
//__________________________________________________________________________
PoolCollectionConverter* CondProxyProvider::getCollectionCnv() {
   ATH_MSG_DEBUG("Try item: \"" << *m_inputCollectionsIterator << "\" from the collection list.");
   PoolCollectionConverter* pCollCnv = new PoolCollectionConverter(std::string("ImplicitROOT:") + APRDefaults::TTreeNames::DataHeader,
	   *m_inputCollectionsIterator,
	   m_contextId,
	   m_athenaPoolCnvSvc->getPoolSvc());
   if (!pCollCnv->initialize().isSuccess()) {
      // Close previous collection.
      delete pCollCnv; pCollCnv = 0;
      ATH_MSG_ERROR("Unable to open: " << *m_inputCollectionsIterator);
   }
   return(pCollCnv);
}
