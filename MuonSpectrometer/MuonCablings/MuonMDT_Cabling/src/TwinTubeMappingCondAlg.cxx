/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "TwinTubeMappingCondAlg.h"

#include <fstream>

#include "StoreGate/ReadCondHandle.h"
#include "StoreGate/WriteCondHandle.h"
#include "AthenaKernel/IOVInfiniteRange.h"
#include "nlohmann/json.hpp"
namespace Muon{
    StatusCode TwinTubeMappingCondAlg::initialize() {
        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_CHECK(m_readKey.initialize(m_extJSONFile.value().empty()));
        ATH_CHECK(m_writeKey.initialize());

        return StatusCode::SUCCESS;
    }
    StatusCode TwinTubeMappingCondAlg::execute(const EventContext& ctx) const {
        ATH_MSG_VERBOSE("execute()");
        // Write Cond Handle
        SG::WriteCondHandle writeHandle{m_writeKey, ctx};
        if (writeHandle.isValid()) {
            ATH_MSG_DEBUG("CondHandle " << writeHandle.fullKey() << " is already valid."
                                        << ". In theory this should not be called, but may happen"
                                        << " if multiple concurrent events are being processed out of order.");
            return StatusCode::SUCCESS;
        }
        nlohmann::json blob{};
        if (m_extJSONFile.value().size()){
            std::ifstream in_json{m_extJSONFile};
            if (!in_json.good()) {
                ATH_MSG_FATAL("Failed to open external JSON file "<<m_extJSONFile);
                return StatusCode::FAILURE;
            }
            in_json >> blob;
        } else {
            SG::ReadCondHandle readHandle{m_readKey, ctx};
                                                                
            if (!readHandle.isValid()) {
                ATH_MSG_FATAL("Failed to load cabling map from COOL "<< m_readKey.fullKey());
                return StatusCode::FAILURE;
            }
            writeHandle.addDependency(readHandle);
            for (const auto& itr : **readHandle) {
                const coral::AttributeList& atr = itr.second;
                blob = nlohmann::json::parse(*(static_cast<const std::string*>((atr["data"]).addressOfData())));
                /// Ideally there should be one big JSON blob
                break;
            }
        }
        writeHandle.addDependency(EventIDRange(IOVInfiniteRange::infiniteRunLB()));
        auto condObj = std::make_unique<TwinTubeMap>(m_idHelperSvc.get());
        condObj->setDefaultHVDelay(m_hvDelay);
        const MdtIdHelper& idHelper{m_idHelperSvc->mdtIdHelper()};
        for (const auto& twinMapping : blob.items()) {
             nlohmann::json payLoad = twinMapping.value();

             const int stIndex =  idHelper.stationNameIndex(payLoad["station"]);
             const int eta = payLoad["eta"];
             const int phi = payLoad["phi"];
             const int ml = payLoad["ml"];

             const int tubeLayer1 = payLoad["layer"];
             const int tube1 = payLoad["tube"];
             const int tubeLayer2 = payLoad["layerTwin"];
             const int tube2 = payLoad["tubeTwin"];

             bool isValid{false};
             const Identifier sibling1{idHelper.channelID(stIndex, eta, phi, ml, tubeLayer1, tube1, isValid)};
             if(!isValid) {
                ATH_MSG_FATAL("Failed to build valid identifier from "<<payLoad);
                return StatusCode::FAILURE;
             }
             const Identifier sibling2{idHelper.channelID(stIndex, eta, phi ,ml ,tubeLayer2, tube2, isValid)};
             if (!isValid) {
                ATH_MSG_FATAL("The twin of "<<m_idHelperSvc->toString(sibling1)<<" is very evlil evil evil: "<<tubeLayer2<<", "<<tube2);
                return StatusCode::FAILURE;
             }
             ATH_CHECK(condObj->addTwinPair(sibling1, sibling2));
        }



        ATH_CHECK(writeHandle.record(std::move(condObj)));
        return StatusCode::SUCCESS;
    }
}
