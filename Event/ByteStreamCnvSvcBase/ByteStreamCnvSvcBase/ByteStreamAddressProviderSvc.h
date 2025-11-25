/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef BYTESTREAMCNVSVCBASE_BYTESTREAMADDRESSPROVIDERSVC_H
#define BYTESTREAMCNVSVCBASE_BYTESTREAMADDRESSPROVIDERSVC_H

#include "AthenaKernel/IAddressProvider.h"
#include "AthenaBaseComps/AthService.h"
#include "GaudiKernel/ServiceHandle.h"

#include <vector>
#include <string>
#include <map>
#include <set>

class IClassIDSvc;

/** @class ByteStreamAddressProviderSvc
    @brief A concrete IAddressProvider
*/
class ByteStreamAddressProviderSvc : public extends<AthService, IAddressProvider> {

public:
   ByteStreamAddressProviderSvc(const std::string& name, ISvcLocator* pSvcLocator);

   // Service initialize
   virtual StatusCode initialize() override;

   // IAddressProvider interface.
   // preload the address
   virtual StatusCode preLoadAddresses(StoreID::type id, tadList& tlist) override;

   /// update an existing transient Address
   virtual StatusCode updateAddress(StoreID::type id,
                                    SG::TransientAddress* tad,
                                    const EventContext& ctx) override;

private:
   // type and name of the objects to create the address for.
   Gaudi::Property<std::vector<std::string>> m_typeNames{this, "TypeNames", {},
       "Type and Name of objects to create the address for", "OrderedSet<std::string>"};

   ServiceHandle<IClassIDSvc> m_clidSvc;

   std::map<unsigned int, std::set<std::string>> m_clidKey;

   Gaudi::Property<int> m_storeID{this, "StoreID", StoreID::EVENT_STORE, "Store ID"};
};

#endif
