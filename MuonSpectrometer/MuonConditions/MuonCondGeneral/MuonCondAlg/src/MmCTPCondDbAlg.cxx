/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MmCTPCondDbAlg.h"
#include <StoreGate/WriteCondHandle.h>
#include <StoreGate/ReadCondHandle.h>
#include <AthenaKernel/IOVInfiniteRange.h>
#include <PathResolver/PathResolver.h>
#include <CoralBase/Blob.h>
#include <CoralUtilities/blobaccess.h>
#include <fstream>


namespace Muon{
// Initialize
StatusCode MmCTPCondDbAlg::initialize() {

    ATH_CHECK(m_idHelperSvc.retrieve());
    if( !m_idHelperSvc->hasMM() ){
         ATH_MSG_ERROR("MuonIdHelperSvc does not have MM, the CTP calibration should not be run!");
         return StatusCode::FAILURE;
    }

    ATH_CHECK(m_readKeyDb.initialize(m_readFromJSON.value().empty()));
    if (m_readFromJSON.value().size()) {
        ATH_MSG_INFO("Read the uncertainty data from a JSON file "<<m_readFromJSON);
    } else if (!m_readKeyDb.empty()) {
        ATH_MSG_INFO("Read the uncertainty data from a COOL databse: " << m_readKeyDb.fullKey() );
    } else {
        ATH_MSG_FATAL("Neither an extrenal JSON nor a COOL folder were defined. Please check");
        return StatusCode::FAILURE;
    }
    ATH_CHECK(m_writeKey.initialize());

    

    return StatusCode::SUCCESS;
}

StatusCode MmCTPCondDbAlg::execute(const EventContext& ctx) const {
    ATH_MSG_DEBUG("execute " << name());
    // launching Write Cond Handle
    SG::WriteCondHandle writeHandle{m_writeKey, ctx};
    if (writeHandle.isValid()) {
        ATH_MSG_DEBUG("CondHandle " << writeHandle.fullKey() << " is already valid."
                                    << " In theory this should not be called, but may happen"
                                    << " if multiple concurrent events are being processed out of order.");
        return StatusCode::SUCCESS;
    }

    std::unique_ptr<Muon::mmCTPClusterCalibData> writeCdo{std::make_unique<Muon::mmCTPClusterCalibData>(m_idHelperSvc.get())};
    writeHandle.addDependency(EventIDRange(IOVInfiniteRange::infiniteTime()));

    if (!m_readFromJSON.value().empty()) {
       std::ifstream inStream{PathResolverFindCalibFile(m_readFromJSON)};
       if (!inStream.good()) {
          ATH_MSG_FATAL("No such file or directory");
          return StatusCode::FAILURE;
       }
       nlohmann::json lines;
       inStream >> lines;
       ATH_CHECK(parseDataFromJSON(lines, *writeCdo));          
    } else{

       SG::ReadCondHandle readHandle{m_readKeyDb, ctx};
       if (!readHandle.isValid()) {
          ATH_MSG_FATAL("Failed to load NSW error calibration folder from "<<m_readKeyDb.fullKey());
          return StatusCode::FAILURE;
       }

       for (CondAttrListCollection::const_iterator itr = readHandle->begin(); 
             itr != readHandle->end(); ++itr) {
          const coral::AttributeList& atr = itr->second;
          std::string data = *(static_cast<const std::string*>((atr["data"]).addressOfData()));
          nlohmann::json lines = nlohmann::json::parse(data);
          ATH_CHECK(parseDataFromJSON(lines, *writeCdo));
       }
    }
    
    ATH_CHECK(writeHandle.record(std::move(writeCdo)));    
    return StatusCode::SUCCESS;
}

StatusCode MmCTPCondDbAlg::parseDataFromJSON(const nlohmann::json& lines,
                                                 Muon::mmCTPClusterCalibData& ctpClusterCondData) const {
    for (auto& corr : lines.items()) {
        nlohmann::json line = corr.value();    

        //Check entry keys, shall we make it to just check the first entry and then assume the rest is okay? maybe it doesn't take that much time to do it on the whole thing...
        if (!line.contains("station") || !line.contains("phi") || !line.contains("eta") || !line.contains("multilayer") || !line.contains("gasGap")) {
            ATH_MSG_ERROR("Missing expected keys in the JSON file");
            return StatusCode::FAILURE; // Handle error appropriately
        }

        // Ensure the JSON array exists and has exactly 2 elements
        if (!line.contains("P1Parameters") || !line["P1Parameters"].is_array() || line["P1Parameters"].size() != 2) {
            ATH_MSG_ERROR("Get unexpected size of the P1Parameters array");
            return StatusCode::FAILURE; // Handle error appropriately
        }

         /// Station Component identification
        const std::string stationType = line["station"];
        const int stationPhi          = line["phi"];
        const int stationEta          = line["eta"];
        const int multiLayer          = line["multilayer"];
        const int gasGap              = line["gasGap"];

        // Convert JSON array to std::array<double, 2>
        std::array<double, 2> modelPars;
        std::copy_n(line["P1Parameters"].begin(), 2, modelPars.begin());

        Identifier errorCalibId{};

        //Using PCB 1 as default
        errorCalibId = m_idHelperSvc->mmIdHelper().channelID(stationType, stationEta, stationPhi, multiLayer, gasGap, 1 );

        Muon::mmCTPClusterCalibData::CTPParameters constants{std::move(modelPars)};

        ATH_CHECK(ctpClusterCondData.storeConstants(errorCalibId, std::move(constants)));


    }
    return StatusCode::SUCCESS;
}
}