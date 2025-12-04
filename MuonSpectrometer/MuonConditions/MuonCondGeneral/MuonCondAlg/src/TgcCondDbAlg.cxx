/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonCondAlg/TgcCondDbAlg.h"
#include <AthenaKernel/IOVInfiniteRange.h>
#include <PathResolver/PathResolver.h>
#include <fstream>

// constructor
TgcCondDbAlg::TgcCondDbAlg(const std::string& name, ISvcLocator* pSvcLocator) :
    AthCondAlgorithm(name, pSvcLocator) {
    
}

// Initialize
StatusCode TgcCondDbAlg::initialize() {
    ATH_MSG_DEBUG("initializing " << name());
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_writeKey.initialize());
    ATH_CHECK(m_readKeyDb.initialize(!m_readKeyDb.empty()));
    if (m_readKeyDb.empty() && m_readFromJSON.value().empty()){
        ATH_MSG_FATAL("No data source is given to load the dead chamber data from. Please provide either a COOL folder or a json file");
        return StatusCode::FAILURE;
    } else if (m_readKeyDb.empty()) {
        ATH_MSG_INFO("Load the chamber status from a JSON file "<<m_readFromJSON);
    } else {
        ATH_MSG_INFO("Load the dead chamber list from COOL "<<m_readKeyDb.fullKey());
    }
    return StatusCode::SUCCESS;
}

// execute
StatusCode TgcCondDbAlg::execute(const EventContext& ctx) const {
    ATH_MSG_DEBUG("execute " << name());
    // launching Write Cond Handle
    SG::WriteCondHandle<TgcCondDbData> writeHandle{m_writeKey, ctx};
    if (writeHandle.isValid()) {
        ATH_MSG_DEBUG("CondHandle " << writeHandle.fullKey() << " is already valid."
                                    << " In theory this should not be called, but may happen"
                                    << " if multiple concurrent events are being processed out of order.");
        return StatusCode::SUCCESS;
    }
    writeHandle.addDependency(EventIDRange(IOVInfiniteRange::infiniteTime()));
    std::unique_ptr<TgcCondDbData> writeCdo{std::make_unique<TgcCondDbData>(m_idHelperSvc.get())};
    if (!m_readKeyDb.empty()) {
        SG::ReadCondHandle<CondAttrListCollection> readHandle{m_readKeyDb, ctx};
        if (!readHandle.isValid()) {
            ATH_MSG_FATAL("Failed to initialize the COOL folder "<<m_readKeyDb.fullKey());
            return StatusCode::FAILURE;
        }
        writeHandle.addDependency(readHandle);
        for (CondAttrListCollection::const_iterator itr = readHandle->begin(); itr != readHandle->end(); ++itr) {
            const coral::AttributeList& atr = itr->second;
            const std::string data{*(static_cast<const std::string*>((atr["data"]).addressOfData()))};
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

StatusCode TgcCondDbAlg::parseDataFromJSON(const nlohmann::json& lines,
                                           TgcCondDbData& deadChannels) const {    
    for (auto& corr : lines.items()) {
        nlohmann::json line = corr.value();    
         /// Station Component identification
        const std::string stationType = line["station"];
        const int stationPhi = line["phi"];
        const int stationEta = line["eta"];
        const int gasGap     = line["gasGap"];
        bool is_valid{false};
        const Identifier id = m_idHelperSvc->tgcIdHelper().channelID(stationType, stationEta, stationPhi, gasGap, false, 1, is_valid);
        if (!is_valid) {
            ATH_MSG_FATAL("The Identifier identifier "<<stationType<<", "<<stationEta<<", "<<stationPhi<<", "<<gasGap<<" is invalid");
            return StatusCode::FAILURE;
        }
        deadChannels.setDeadGasGap(id);
    }
    return StatusCode::SUCCESS;
}