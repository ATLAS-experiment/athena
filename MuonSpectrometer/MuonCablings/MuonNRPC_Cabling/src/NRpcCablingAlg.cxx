/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "NRpcCablingAlg.h"


#include <fstream>
#include <map>

#include "AthenaKernel/IOVInfiniteRange.h"
#include "AthenaPoolUtilities/AthenaAttributeList.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeListSpecification.h"
#include "MuonIdHelpers/RpcIdHelper.h"
#include "PathResolver/PathResolver.h"
#include "MuonCablingData/RpcFlatCableTranslator.h"

namespace Muon{
StatusCode NRpcCablingAlg::initialize() {
    ATH_MSG_DEBUG("initialize " << name());
    ATH_CHECK(m_writeKey.initialize());
    ATH_CHECK(m_idHelperSvc.retrieve());
    const bool ext_json = !m_extJSONFile.value().empty();
    /// Only load the readout geometry if an external JSON file is defined
    ATH_CHECK(m_readKeyMap.initialize(!ext_json));
    return StatusCode::SUCCESS;
}

StatusCode NRpcCablingAlg::execute(const EventContext& ctx) const {
    ATH_MSG_VERBOSE("NRpcCablingAlg::execute()");
   
    // Write Cond Handle
    SG::WriteCondHandle writeCablingHandle{m_writeKey, ctx};

    if (writeCablingHandle.isValid()) {
        ATH_MSG_DEBUG("CondHandle "<< writeCablingHandle.fullKey() << " is already valid."
                      << ". In theory this should not be called, but may happen"
                      << " if multiple concurrent events are being processed out of order.");
        return StatusCode::SUCCESS;
    }
    writeCablingHandle.addDependency(EventIDRange(IOVInfiniteRange::infiniteRunLB()));

    ATH_MSG_INFO("Load the Nrpc cabling");
    auto writeCdo{std::make_unique<RpcCablingMap>(m_idHelperSvc.get())};

    /// If the JSON file is defined use the readout geometry as IOV definition
    if (!m_extJSONFile.value().empty()) {        
        std::ifstream in_json{m_extJSONFile};
        if (!in_json.good()) {
            ATH_MSG_FATAL("Failed to open external JSON file " << m_extJSONFile);
            return StatusCode::FAILURE;
        }
        nlohmann::json payload;
        in_json>>payload;
        ATH_CHECK(parsePayload(*writeCdo, payload));
    } else {
        SG::ReadCondHandle coolHandle{m_readKeyMap, ctx};
        if (!coolHandle.isValid()) {
            ATH_MSG_FATAL("Failed to load cabling map from COOL "<< m_readKeyMap.fullKey());
            return StatusCode::FAILURE;
        }
        writeCablingHandle.addDependency(coolHandle);
        for (const auto& itr : **coolHandle) {
            const coral::AttributeList& atr = itr.second;
            nlohmann::json payload = nlohmann::json::parse(*(static_cast<const std::string*>((atr["data"]).addressOfData())));
            ATH_CHECK(parsePayload(*writeCdo, payload));
        }
    }
    if (!writeCdo->finalize(msgStream()))
        return StatusCode::FAILURE;

    ATH_CHECK(writeCablingHandle.record(std::move(writeCdo)));
    return StatusCode::SUCCESS;
}
StatusCode NRpcCablingAlg::parsePayload(RpcCablingMap& cabling_map,
                                        const nlohmann::json& payload) const {

    
    using CablingData = RpcCablingMap::CoolDBEntry;
    using FlatCablePtr = RpcCablingMap::FlatCablePtr;
    
    std::map<unsigned, FlatCablePtr> readoutCards{};
    for (const auto& flatCable : payload["readoutCards"].items()) {
        nlohmann::json cabl_payload = flatCable.value(); 
        const unsigned id = cabl_payload["flatCableId"];
        FlatCablePtr& newCard = readoutCards[id];
        newCard = std::make_unique<RpcFlatCableTranslator>(id);
        for (const auto& pinAssign : cabl_payload["pinAssignment"]) {
            const std::array<unsigned, 2> pin = pinAssign;
            if (!newCard->mapChannels(pin[0], pin[1], msgStream())){
                return StatusCode::FAILURE;
            }
        }
        ATH_MSG_DEBUG("Add new card "<<(*newCard));
    }
    constexpr int8_t phiBit = CablingData::measPhiBit;
    constexpr int8_t sideBit = CablingData::stripSideBit;
    for (const auto& cabl_chan : payload["chamberMap"].items()) {
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
        unsigned int flatCable = cabl_payload["flatCableId"];
        if (!readoutCards[flatCable]) {
            ATH_MSG_FATAL("Failed to find a valid flat cable for "<<static_cast<const RpcCablingOfflineID&>(cabl_data)<<", flatCableId "<<flatCable);
            return StatusCode::FAILURE;
        }
        cabl_data.flatCable = readoutCards[flatCable];
        if (!cabling_map.insertChannels(std::move(cabl_data), msgStream())) {
            return StatusCode::FAILURE;
        }
    }
    return StatusCode::SUCCESS;
}
}