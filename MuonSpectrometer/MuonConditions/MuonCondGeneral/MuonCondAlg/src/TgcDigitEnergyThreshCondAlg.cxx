/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonCondAlg/TgcDigitEnergyThreshCondAlg.h"
#include <StoreGate/WriteCondHandle.h>
#include <AthenaKernel/IOVInfiniteRange.h>
#include <PathResolver/PathResolver.h>
#include <fstream>


TgcDigitEnergyThreshCondAlg::TgcDigitEnergyThreshCondAlg(const std::string& name, ISvcLocator* pSvcLocator) :
    AthCondAlgorithm(name, pSvcLocator) {}

StatusCode TgcDigitEnergyThreshCondAlg::initialize() {
    ATH_MSG_DEBUG("initializing " << name());
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_writeKey.initialize());
    ATH_CHECK(m_readKeyDb.initialize(!m_readKeyDb.empty()));
    if (m_readKeyDb.empty() && m_readFromJSON.value().empty()){
        ATH_MSG_FATAL("No data source is given to load the energy thresholds from. Please provide either a COOL folder or a json file");
        return StatusCode::FAILURE;
    } else if (m_readKeyDb.empty()) {
        ATH_MSG_INFO("Load the energy thresholds from a JSON file "<<m_readFromJSON);
    } else {
        ATH_MSG_INFO("Load the energy thresholds list from COOL "<<m_readKeyDb.fullKey());
    }
    return StatusCode::SUCCESS;
}

// execute
StatusCode TgcDigitEnergyThreshCondAlg::execute(const EventContext& ctx) const {
    ATH_MSG_DEBUG("execute " << name());
    // launching Write Cond Handle
    SG::WriteCondHandle<TgcDigitThresholdData> writeHandle{m_writeKey, ctx};
    if (writeHandle.isValid()) {
        ATH_MSG_DEBUG("CondHandle " << writeHandle.fullKey() << " is already valid."
                                    << " In theory this should not be called, but may happen"
                                    << " if multiple concurrent events are being processed out of order.");
        return StatusCode::SUCCESS;
    }
    writeHandle.addDependency(EventIDRange(IOVInfiniteRange::infiniteTime()));
    std::unique_ptr<TgcDigitThresholdData> writeCdo{std::make_unique<TgcDigitThresholdData>(m_idHelperSvc.get())};
    if (!m_readKeyDb.empty()) {
        SG::ReadCondHandle<CondAttrListCollection> readHandle{m_readKeyDb, ctx};
        if (!readHandle.isValid()) {
            ATH_MSG_FATAL("Failed to initialize the COOL folder "<<m_readKeyDb.fullKey());
            return StatusCode::FAILURE;
        }
        writeHandle.addDependency(readHandle);
        for (CondAttrListCollection::const_iterator itr = readHandle->begin(); itr != readHandle->end(); ++itr) {
            const coral::AttributeList& atr = itr->second;
            const std::string& data{*(static_cast<const std::string*>((atr["data"]).addressOfData()))};
            nlohmann::json lines = nlohmann::json::parse(data);           
            ATH_CHECK(parseDataFromJSON(lines, *writeCdo)); 
        }
    } else {
       std::ifstream inStream{PathResolverFindCalibFile(m_readFromJSON)};
        if (!inStream.good()) {
            ATH_MSG_FATAL("No such file or directory");
            return StatusCode::FAILURE;
        }
        nlohmann::json lines;
        inStream >> lines;
        ATH_CHECK(parseDataFromJSON(lines, *writeCdo));          
    }
    ATH_CHECK(writeHandle.record(std::move(writeCdo)));
    ATH_MSG_DEBUG("Recorded new " << writeHandle.key() << " with range " << writeHandle.getRange() << " into Conditions Store");
    return StatusCode::SUCCESS;
}

StatusCode TgcDigitEnergyThreshCondAlg::parseDataFromJSON(const nlohmann::json& lines,
                                                          TgcDigitThresholdData& channelThresholds) const {    
    for (auto& corr : lines.items()) {
        nlohmann::json line = corr.value();    
         /// Station Component identification
        const std::string stationType = line["station"];
        const int stationPhi = line["phi"];
        const int stationEta = line["eta"];
        const int gasGap = line["gasGap"];
        const int isStrip = line["isStrip"];
        const double threshold = line["threshold"];
        bool is_valid{false};
        const Identifier id = m_idHelperSvc->tgcIdHelper().channelID(stationType, stationEta, stationPhi, gasGap, isStrip, 1, is_valid);
        if (!is_valid) {
            ATH_MSG_FATAL("The Identifier identifier "<<stationType<<", "<<stationEta<<", "<<stationPhi
                        <<", "<<gasGap<<", "<<isStrip<<" is invalid");
            return StatusCode::FAILURE;
        }
        if(!channelThresholds.setThreshold(id, threshold)) return StatusCode::FAILURE;
    }
    return StatusCode::SUCCESS;
}