/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// Include files
#include "ByteStreamCnvSvcBase/ByteStreamAddressProviderSvc.h"
#include "ByteStreamCnvSvcBase/ByteStreamAddress.h"
#include "ByteStreamCnvSvcBase/IROBDataProviderSvc.h"

#include "SGTools/TransientAddress.h"
#include "GaudiKernel/TypeNameString.h"
#include "GaudiKernel/IClassIDSvc.h"

#include "eformat/SourceIdentifier.h"

/// Standard constructor
ByteStreamAddressProviderSvc::ByteStreamAddressProviderSvc(const std::string& name, ISvcLocator* pSvcLocator)
  : base_class(name, pSvcLocator), m_clidSvc("ClassIDSvc", name) {}
//________________________________________________________________________________
StatusCode ByteStreamAddressProviderSvc::initialize() {
   ATH_MSG_INFO("Initializing");

   // Retrieve ClassIDSvc
   ATH_CHECK( m_clidSvc.retrieve() );

   if (m_storeID < 0 || m_storeID > StoreID::UNKNOWN) {
      ATH_MSG_FATAL("Invalid StoreID " << m_storeID);
      return StatusCode::FAILURE;
   }
   ATH_MSG_INFO("-- Will fill Store with id =  " << m_storeID.value());
   return StatusCode::SUCCESS;
}
//________________________________________________________________________________
StatusCode ByteStreamAddressProviderSvc::preLoadAddresses(StoreID::type id, tadList& tlist) {
   ATH_MSG_DEBUG("in preLoadAddress");
   if (id != m_storeID) {
      ATH_MSG_DEBUG("StoreID = " << id << " does not match required id (" << m_storeID << ") skip");
      return StatusCode::SUCCESS;
   }

   for (const std::string& typeName : m_typeNames) {
      Gaudi::Utils::TypeNameString item(typeName);
      const std::string& t = item.type();
      const std::string& nm = item.name();
      CLID classid;
      if (!m_clidSvc->getIDOfTypeName(t, classid).isSuccess()) {
         ATH_MSG_WARNING("Cannot create TAD for (type, name)" << " no CLID for " << t << " " << nm);
      } else {
         SG::TransientAddress* tad = new SG::TransientAddress(classid, nm);
         tlist.push_back(tad);
         ATH_MSG_DEBUG("Created TAD for (type, clid, name)" << t << " " << classid << " " << nm);
         // save the clid and key.
         m_clidKey[classid].insert(nm);
      }
   }
   return StatusCode::SUCCESS;
}
//________________________________________________________________________________
StatusCode ByteStreamAddressProviderSvc::updateAddress(StoreID::type id,
                                                       SG::TransientAddress* tad,
                                                       const EventContext& ctx) {
   if (id != m_storeID) {
      return StatusCode::FAILURE;
   }
   CLID clid = tad->clID();
   std::string nm = tad->name();
   auto it = m_clidKey.find(clid);
   if (it == m_clidKey.end() || it->second.count(nm) == 0) {
      return StatusCode::FAILURE;
   }
   ATH_MSG_DEBUG("Creating address for " << clid << " " << nm);
   ByteStreamAddress* add = new ByteStreamAddress(clid, nm, "");
   add->setEventContext(ctx);
   tad->setAddress(add);
   return StatusCode::SUCCESS;
}
