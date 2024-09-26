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
    using HedgehogBoardPtr = HedgehogBoard::HedgehogBoardPtr;
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
        writeHandle.addDependency(EventIDRange(IOVInfiniteRange::infiniteRunLB()));

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
        auto condObj = std::make_unique<TwinTubeMap>(m_idHelperSvc.get());
        condObj->setDefaultHVDelay(m_hvDelay);
        const MdtIdHelper& idHelper{m_idHelperSvc->mdtIdHelper()};
        
        std::unordered_map<unsigned int, HedgehogBoardPtr> hedgeHogBoards{};
        using Mapping = HedgehogBoard::Mapping;
        for (const auto& boardMapping : blob["HedgehogBoards"].items()) {
            nlohmann::json payLoad = boardMapping.value();
            ATH_MSG_VERBOSE("Parse payload "<<payLoad);
            const unsigned boardId = payLoad["boardId"];
            const Mapping mapping = payLoad["hedgeHogPins"];
            const unsigned tubeLayers = payLoad["nTubeLayers"];
            auto newBoard = std::make_unique<HedgehogBoard>(mapping, tubeLayers, boardId);
            if (payLoad.find("hvDelayTime") != payLoad.end()) {
                newBoard->setHVDelayTime(payLoad["hvDelayTime"]);
            }
            HedgehogBoardPtr& storeMe = hedgeHogBoards[newBoard->boardId()];
            if (storeMe) {
                ATH_MSG_FATAL("There's already a board registered under "<<storeMe->boardId()<<".");
                return StatusCode::FAILURE;
            }
            storeMe = std::move(newBoard);
        }        
        for (const auto& twinMapping : blob["TwinTubeMapping"].items()) {
            nlohmann::json payLoad = twinMapping.value();
            ATH_MSG_VERBOSE("Parse "<<payLoad);
            const int stIndex =  idHelper.stationNameIndex(payLoad["station"]);
            const int eta = payLoad["eta"];
            const int phi = payLoad["phi"];
            const int ml = payLoad["ml"];
            bool isValid{false};
            const Identifier detElId{idHelper.channelID(stIndex, eta, phi, ml, 1, 1, isValid)};
            if(!isValid) {
                ATH_MSG_FATAL("Failed to build valid identifier from "<<payLoad);
                return StatusCode::FAILURE;
            };
            const std::vector<unsigned int> boardMounting = payLoad["mountedBoards"];
            for (unsigned int place = 0; place < boardMounting.size(); ++place) {
                ATH_CHECK(condObj->addHedgeHogBoard(detElId, hedgeHogBoards[boardMounting[place]], place));
            }
        }
        ATH_CHECK(writeHandle.record(std::move(condObj)));
        return StatusCode::SUCCESS;
    }
}
