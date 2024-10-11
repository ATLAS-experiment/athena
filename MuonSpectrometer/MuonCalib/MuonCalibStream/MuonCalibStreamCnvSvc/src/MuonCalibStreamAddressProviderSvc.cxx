/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonCalibStreamCnvSvc/MuonCalibStreamAddressProviderSvc.h"

#include "GaudiKernel/IClassIDSvc.h"
#include "GaudiKernel/TypeNameString.h"
#include "MuonCalibStreamCnvSvc/MuonCalibStreamAddress.h"
#include "SGTools/TransientAddress.h"

/// Standard constructor
MuonCalibStreamAddressProviderSvc::MuonCalibStreamAddressProviderSvc(const std::string &name, ISvcLocator *svc) :
    base_class(name, svc), m_dataSvc(nullptr) {
    m_typeNames.push_back("Muon::MdtPrepDataContainer/MDT_DriftCircles");
    m_typeNames.push_back("RpcPadContainer/RPCPAD");
    m_typeNames.push_back("TgcRdoContainer/TGCRDO");
    //m_typeNames.push_back("CscRawDataContainer/CSCRDO");
    m_typeNames.push_back("xAOD::EventInfo");
    declareProperty("TypeNames", m_typeNames);
}

/// Standard Destructor
MuonCalibStreamAddressProviderSvc::~MuonCalibStreamAddressProviderSvc() {}

/// Initialize the service.
StatusCode MuonCalibStreamAddressProviderSvc::initialize() {
    ATH_CHECK(service("MuonCalibStreamDataProviderSvc", m_dataSvc));
    ATH_MSG_INFO(" initialized ");
    return StatusCode::SUCCESS;
}

StatusCode MuonCalibStreamAddressProviderSvc::preLoadAddresses(StoreID::type idp, tadList &tlist) {
    ATH_MSG_DEBUG(" call MuonCalibStreamAddressProviderSvc::preLoadAddresses");

    if (idp != StoreID::EVENT_STORE) {
        ATH_MSG_DEBUG("idp = " << idp << " != StoreID::EVENT_STORE; not creating TADs");
        return StatusCode::SUCCESS;
    } else {
        ATH_MSG_DEBUG("idp = " << idp << " == StoreID::EVENT_STORE; Creating TADs for muon detectors!");
    }

    // only deal with event store.
    std::vector<std::string>::const_iterator it = m_typeNames.begin();
    std::vector<std::string>::const_iterator it_e = m_typeNames.end();

    IClassIDSvc *clidSvc;
    ATH_CHECK(service("ClassIDSvc", clidSvc));

    for (; it != it_e; ++it) {
        Gaudi::Utils::TypeNameString item(*it);
        std::string t = item.type();
        std::string nm = item.name();

        CLID id;
        ATH_CHECK(clidSvc->getIDOfTypeName(t, id));

        SG::TransientAddress *tad = new SG::TransientAddress(id, nm);
        tlist.push_back(tad);
        ATH_MSG_DEBUG(" created TAD for (type,clid,name,TAD)" << t << " " << id << " " << nm);

        // save the clid and key.
        m_clidKey[id].insert(nm);
    }
    return StatusCode::SUCCESS;
}  // MuonCalibStreamAddressProviderSvc::preLoadAddresses()

// update an existing transient Address
StatusCode MuonCalibStreamAddressProviderSvc::updateAddress(StoreID::type /*tp*/, SG::TransientAddress *tad, const EventContext &) {
    CLID clid = tad->clID();
    std::string nm = tad->name();
    std::map<CLID, std::set<std::string> >::const_iterator it = m_clidKey.find(clid);

    if (it == m_clidKey.end()) return StatusCode::FAILURE;

    if ((*it).second.count(nm) == 0) return StatusCode::FAILURE;

    ATH_MSG_DEBUG(" creating address for " << clid << " " << nm);

    MuonCalibStreamAddress *add = new MuonCalibStreamAddress(clid, nm, "");
    tad->setAddress(add);
    return StatusCode::SUCCESS;
}
