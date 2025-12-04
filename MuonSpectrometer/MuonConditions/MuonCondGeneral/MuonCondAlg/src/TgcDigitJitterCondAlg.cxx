/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonCondAlg/TgcDigitJitterCondAlg.h"
#include <StoreGate/WriteCondHandle.h>
#include <AthenaKernel/IOVInfiniteRange.h>
#include <PathResolver/PathResolver.h>
#include <fstream>

// constructor
TgcDigitJitterCondAlg::TgcDigitJitterCondAlg(const std::string& name, ISvcLocator* pSvcLocator) :
    AthCondAlgorithm(name, pSvcLocator) {}

// Initialize
StatusCode TgcDigitJitterCondAlg::initialize() {
    ATH_MSG_DEBUG("initializing " << name());
    ATH_CHECK(m_writeKey.initialize());
    ATH_CHECK(m_readKeyDb.initialize(!m_readKeyDb.empty()));
    if (m_readKeyDb.empty() && m_readFromJSON.value().empty()){
        ATH_MSG_FATAL("No data source is given to load the jitter constants from. Please provide either a COOL folder or a json file");
        return StatusCode::FAILURE;
    } else if (m_readKeyDb.empty()) {
        ATH_MSG_INFO("Load jitter constants from a JSON file "<<m_readFromJSON);
    } else {
        ATH_MSG_INFO("Load the jitter constants list from COOL "<<m_readKeyDb.fullKey());
    }
    return StatusCode::SUCCESS;
}

// execute
StatusCode TgcDigitJitterCondAlg::execute(const EventContext& ctx) const {
    ATH_MSG_DEBUG("execute " << name());
    // launching Write Cond Handle
    SG::WriteCondHandle<TgcDigitJitterData> writeHandle{m_writeKey, ctx};
    if (writeHandle.isValid()) {
        ATH_MSG_DEBUG("CondHandle " << writeHandle.fullKey() << " is already valid."
                                    << " In theory this should not be called, but may happen"
                                    << " if multiple concurrent events are being processed out of order.");
        return StatusCode::SUCCESS;
    }
    writeHandle.addDependency(EventIDRange(IOVInfiniteRange::infiniteTime()));
    std::unique_ptr<TgcDigitJitterData> writeCdo{std::make_unique<TgcDigitJitterData>()};
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
    ATH_CHECK(writeCdo->initialize());
    ATH_CHECK(writeHandle.record(std::move(writeCdo)));
    ATH_MSG_DEBUG("Recorded new " << writeHandle.key() << " with range " << writeHandle.getRange() << " into Conditions Store");
    return StatusCode::SUCCESS;
}

StatusCode TgcDigitJitterCondAlg::parseDataFromJSON(const nlohmann::json& lines,
                                                    TgcDigitJitterData& jitterData) const {    
    for (auto& corr : lines.items()) {
        nlohmann::json line = corr.value();    
        const double angle = line["angle"];
        std::vector<double> values = line["values"];
        jitterData.cacheAngleInterval(angle, std::move(values));
    }
    return StatusCode::SUCCESS;
}