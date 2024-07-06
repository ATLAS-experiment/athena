/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonNRPC_CablingAlg.h"

#include <stdlib.h>

#include <fstream>
#include <map>
#include <string>

#include "AthenaKernel/IOVInfiniteRange.h"
#include "AthenaPoolUtilities/AthenaAttributeList.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeListSpecification.h"
#include "MuonIdHelpers/RpcIdHelper.h"
#include "PathResolver/PathResolver.h"
#include "SGTools/TransientAddress.h"

MuonNRPC_CablingAlg::MuonNRPC_CablingAlg(const std::string& name,
                                         ISvcLocator* pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator) {}

StatusCode MuonNRPC_CablingAlg::initialize() {
    ATH_MSG_DEBUG("initialize " << name());
    ATH_CHECK(m_writeKey.initialize());
    ATH_CHECK(m_idHelperSvc.retrieve());
    const bool ext_json = !m_extJSONFile.value().empty();
    /// Only load the readout geometry if an external JSON file is defined
    ATH_CHECK(m_readKeyMap.initialize(!ext_json));
    return StatusCode::SUCCESS;
}

StatusCode MuonNRPC_CablingAlg::execute(const EventContext& ctx) const {
    ATH_MSG_VERBOSE("MuonNRPC_CablingAlg::execute()");
   
    // Write Cond Handle
    SG::WriteCondHandle<MuonNRPC_CablingMap> writeCablingHandle{m_writeKey,
                                                                ctx};

    if (writeCablingHandle.isValid()) {
        ATH_MSG_DEBUG("CondHandle "
                      << writeCablingHandle.fullKey() << " is already valid."
                      << ". In theory this should not be called, but may happen"
                      << " if multiple concurrent events are being processed "
                         "out of order.");
        return StatusCode::SUCCESS;
    }
    writeCablingHandle.addDependency(EventIDRange(IOVInfiniteRange::infiniteRunLB()));

    ATH_MSG_INFO("Load the Nrpc cabling");
    std::unique_ptr<MuonNRPC_CablingMap> writeCdo{
        std::make_unique<MuonNRPC_CablingMap>()};

    /// If the JSON file is defined use the readout geometry as IOV definition
    if (!m_extJSONFile.value().empty()) {        
        std::ifstream in_json{m_extJSONFile};
        if (!in_json.good()) {
            ATH_MSG_FATAL("Failed to open external JSON file "
                          << m_extJSONFile);
            return StatusCode::FAILURE;
        }
        nlohmann::json payload;
        in_json>>payload;
        ATH_CHECK(payLoadJSON(*writeCdo, payload));
    } else {
        SG::ReadCondHandle<CondAttrListCollection> coolHandle{m_readKeyMap,
                                                              ctx};
        if (!coolHandle.isValid()) {
            ATH_MSG_FATAL("Failed to load cabling map from COOL "
                          << m_readKeyMap.fullKey());
            return StatusCode::FAILURE;
        }
        writeCablingHandle.addDependency(coolHandle);
        for (const auto& itr : **coolHandle) {
            const coral::AttributeList& atr = itr.second;
            nlohmann::json payload = nlohmann::json::parse(*(static_cast<const std::string*>((atr["data"]).addressOfData())));
            ATH_CHECK(payLoadJSON(*writeCdo, payload));
        }
    }
    if (!writeCdo->finalize(msgStream()))
        return StatusCode::FAILURE;

    ATH_CHECK(writeCablingHandle.record(std::move(writeCdo)));

    return StatusCode::SUCCESS;
}
StatusCode MuonNRPC_CablingAlg::payLoadJSON(MuonNRPC_CablingMap& cabling_map,
                                            const nlohmann::json& payload) const {

    constexpr int8_t phiBit = CablingData::measPhiBit;
    constexpr int8_t sideBit = CablingData::stripSideBit;
    for (const auto& cabl_chan : payload.items()) {
        nlohmann::json cabl_payload = cabl_chan.value();
        CablingData cabl_data{};
        const std::string stName{cabl_payload["station"]};
        cabl_data.stationIndex = m_idHelperSvc->rpcIdHelper().stationNameIndex(stName);
        cabl_data.eta = cabl_payload["eta"];
        cabl_data.phi = cabl_payload["phi"];
        cabl_data.doubletR = cabl_payload["doubletR"];
        cabl_data.doubletPhi = cabl_payload["doubletPhi"];
        cabl_data.doubletZ = cabl_payload["doubletZ"];
        const int8_t phiAndStrip = cabl_payload["measPhi"];
        cabl_data.setMeasPhiAndSide(phiAndStrip & phiBit, phiAndStrip & sideBit);
        cabl_data.gasGap = cabl_payload["gasGap"];
        /// Online part
        cabl_data.subDetector = cabl_payload["subDetector"];
        cabl_data.tdcSector = cabl_payload["tdcSector"];
        cabl_data.tdc = cabl_payload["tdc"];
        cabl_data.firstStrip = cabl_payload["firstStrip"];
        cabl_data.lastStrip = cabl_payload["lastStrip"];
        cabl_data.firstChannel = cabl_payload["firstTdcChan"];
        cabl_data.lastChannel = cabl_payload["lastTdcChan"];
        if (!cabling_map.insertChannels(cabl_data, msgStream())) {
            return StatusCode::FAILURE;
        }
    }
    return StatusCode::SUCCESS;
}
