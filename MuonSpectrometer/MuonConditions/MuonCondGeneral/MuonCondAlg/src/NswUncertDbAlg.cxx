/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonCondAlg/NswUncertDbAlg.h"

#include <StoreGate/WriteCondHandle.h>
#include <StoreGate/ReadCondHandle.h>
#include <AthenaKernel/IOVInfiniteRange.h>
#include <PathResolver/PathResolver.h>
#include <CoralBase/Blob.h>
#include <CoralUtilities/blobaccess.h>
#include <fstream>


NswUncertDbAlg::NswUncertDbAlg(const std::string& name, ISvcLocator* svc):
    AthReentrantAlgorithm{name, svc}{}

StatusCode NswUncertDbAlg::initialize() {
    ATH_CHECK(m_readKeysDb.initialize(m_readFromJSON.value().empty()));
    if (m_readFromJSON.value().size()) {
        ATH_MSG_INFO("Read the uncertainty data from a JSON file "<<m_readFromJSON);
    } else if (m_readKeysDb.size()) {
        std::stringstream folderStr{};
        for (const SG::ReadCondHandleKey<CondAttrListCollection>& key : m_readKeysDb) {
            folderStr<<"   **** "<<key.fullKey()<<std::endl;
        }
        ATH_MSG_INFO("Read the parametrized NSW uncertainties from COOL: "<<std::endl<<folderStr.str());
    } else {
        ATH_MSG_FATAL("Neither an extrenal JSON nor a COOL folder were defined. Please check");
        return StatusCode::FAILURE;
    }
    ATH_CHECK(m_writeKey.initialize());
    ATH_CHECK(m_idHelperSvc.retrieve());
    return StatusCode::SUCCESS;
}

StatusCode NswUncertDbAlg::execute(const EventContext& ctx) const {
    ATH_MSG_DEBUG("execute " << name());
    SG::WriteCondHandle<NswErrorCalibData> writeHandle{m_writeKey, ctx};
    if (writeHandle.isValid()) {
        ATH_MSG_DEBUG("CondHandle " << writeHandle.fullKey() << " is already valid."
                                    << ". In theory this should not be called, but may happen"
                                    << " if multiple concurrent events are being processed out of order.");
        return StatusCode::SUCCESS;   
    }
    
    std::unique_ptr<NswErrorCalibData> writeCdo = std::make_unique<NswErrorCalibData>(m_idHelperSvc.get()); 
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
    } else {
        for (const SG::ReadCondHandleKey<CondAttrListCollection>& key : m_readKeysDb) {
            SG::ReadCondHandle<CondAttrListCollection> readHandle{key, ctx};
            if (!readHandle.isValid()) {
                ATH_MSG_FATAL("Failed to load NSW error calibration folder from "<<key.fullKey());
                return StatusCode::FAILURE;
            }
            for (CondAttrListCollection::const_iterator itr = readHandle->begin(); 
                                                        itr != readHandle->end(); ++itr) {
                const coral::AttributeList& atr = itr->second;
                std::string data{};
                if (atr["data"].specification().type() == typeid(coral::Blob)) {
                    ATH_MSG_VERBOSE("Loading data as a BLOB, uncompressing...");
                    if (!CoralUtilities::readBlobAsString(atr["data"].data<coral::Blob>(), data)) {
                        ATH_MSG_FATAL("Cannot uncompress BLOB! Aborting...");
                        return StatusCode::FAILURE;
                    }
                } else {
                    data = *(static_cast<const std::string*>((atr["data"]).addressOfData()));
                }
                nlohmann::json lines = nlohmann::json::parse(data);
                ATH_CHECK(parseDataFromJSON(lines, *writeCdo));
            }
        }
    }
    ATH_CHECK(writeHandle.record(std::move(writeCdo)));    
    return StatusCode::SUCCESS;
}

StatusCode NswUncertDbAlg::parseDataFromJSON(const nlohmann::json& lines,
                                             NswErrorCalibData& nswErrorCalib) const {
    
       for (auto& corr : lines.items()) {
        nlohmann::json line = corr.value();    
         /// Station Component identification
        const std::string stationType = line["station"];
        const int stationPhi = line["phi"];
        const int stationEta = line["eta"];
        const int multilayer = line["multilayer"];
        const int gasGap = line["gasGap"];
        Identifier errorCalibId{};
        bool isValid{true};
        if (stationType[0] == 'M') {
            if (!m_idHelperSvc->hasMM()) continue;
            errorCalibId = m_idHelperSvc->mmIdHelper().channelID(stationType, stationEta, stationPhi,
                                                                 multilayer, gasGap, 1
#ifndef NDEBUG
        ,isValid
#endif
                                                                );
        } else if (stationType[0] == 'S') {
            if (!m_idHelperSvc->hasSTGC()) continue;
            errorCalibId = m_idHelperSvc->stgcIdHelper().channelID (stationType, stationEta, stationPhi,
                                                                    multilayer, gasGap, 
                                                                    sTgcIdHelper::sTgcChannelTypes::Strip, 1
#ifndef NDEBUG
        ,isValid
#endif
                                                                );

        } else {
            isValid = false;
        }
        if (!isValid) {
            ATH_MSG_ERROR("Failed to construct a valid Identifier from "
                         <<stationType<<", "<<stationEta<<", "<<stationPhi<<", "
                         <<multilayer<<", "<<gasGap);
            return StatusCode::FAILURE;
        }
        
        const uint16_t minStrip = line["minStrip"];
        const uint16_t maxStrip = line["maxStrip"];
        const uint8_t author = line["clusterAuthor"];
        const std::string modelName = line["modelName"];
        std::vector<double> modelPars = line["modelPars"];

        ATH_MSG_VERBOSE("Load uncertainties for channel " <<m_idHelperSvc->toString(errorCalibId)<<" "<<modelPars
                      <<"model name: "<<modelName<<" author: "<<static_cast<int>(author));


        NswErrorCalibData::ErrorConstants constants{modelName, author,
                                                    minStrip, maxStrip, std::move(modelPars)};

        ATH_CHECK(nswErrorCalib.storeConstants(errorCalibId, std::move(constants)));
    }
    
    return StatusCode::SUCCESS;
}
 