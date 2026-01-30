/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "NswPassivationDbAlg.h"

#include "CoralBase/Blob.h"
#include "CoralUtilities/blobaccess.h"
#include "AthenaKernel/IOVInfiniteRange.h"
#include "PathResolver/PathResolver.h"

#include <fstream>

namespace Muon{
StatusCode NswPassivationDbAlg::initialize() {
    ATH_MSG_DEBUG( "initializing " << name() );                
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_readKey_data_mm.initialize(m_readFromJSON.value().empty()));
    ATH_CHECK(m_writeKey.initialize());
    return StatusCode::SUCCESS;
}

StatusCode NswPassivationDbAlg::execute(const EventContext& ctx) const {

    // set up write handle
    SG::WriteCondHandle writeHandle{m_writeKey, ctx};
    if (writeHandle.isValid()) {
        ATH_MSG_DEBUG("CondHandle " << writeHandle.fullKey() << " is already valid."
              << " In theory this should not be called, but may happen"
              << " if multiple concurrent events are being processed out of order.");
        return StatusCode::SUCCESS; 
    }
    writeHandle.addDependency(EventIDRange(IOVInfiniteRange::infiniteTime()));
    auto writeCdo{std::make_unique<NswPassivationDbData>(m_idHelperSvc->mmIdHelper())};
    if (!m_readFromJSON.empty()) {
        std::ifstream inStream{PathResolverFindCalibFile(m_readFromJSON)};
        if (!inStream.good()) {
            ATH_MSG_FATAL("No such file or directory");
            return StatusCode::FAILURE;
        }
        nlohmann::json lines;
        inStream >> lines;
        ATH_CHECK(parseData(lines, *writeCdo));
    } else {
        // set up read handle
        SG::ReadCondHandle readHandle{m_readKey_data_mm, ctx};
        if(!readHandle.isValid()){
          ATH_MSG_ERROR("Null pointer to the read conditions object");
          return StatusCode::FAILURE; 
        } 
        writeHandle.addDependency(readHandle);
        ATH_MSG_DEBUG("Size of CondAttrListCollection " << readHandle.fullKey() << " readCdo->size()= " << readHandle->size());
        ATH_MSG_DEBUG("Range of input is " << readHandle.getRange() << ", range of output is " << writeHandle.getRange());
    
        // iterate through data    
        unsigned int nObjs = 0;
        for(CondAttrListCollection::const_iterator itr = readHandle->begin(); 
                                                   itr != readHandle->end(); ++itr) {
            // retrieve data
            const coral::AttributeList& atr = itr->second;
            const std::string& data{*(static_cast<const std::string *>((atr["data"]).addressOfData()))};
 
            // unwrap the json and process the data
            ATH_CHECK(parseData(nlohmann::json::parse(data), *writeCdo));
            ++nObjs;
        }
        ATH_MSG_VERBOSE("Retrieved data for "<<nObjs<<" objects.");
    }
    // insert/write data
    if (writeHandle.record(std::move(writeCdo)).isFailure()) {
        ATH_MSG_FATAL("Could not record NswPassivationDbData " << writeHandle.key() 
              << " with EventRange " << writeHandle.getRange()
              << " into Conditions Store");
        return StatusCode::FAILURE;
    }          
    ATH_MSG_DEBUG("Recorded new " << writeHandle.key() << " with range " << writeHandle.getRange() << " into Conditions Store");

    return StatusCode::SUCCESS;
}
StatusCode NswPassivationDbAlg::parseData(const nlohmann::json & json,
                                         NswPassivationDbData& writeCdo) const {
    unsigned int nChns = 0; 
    for (const auto &kt : json.items()) {
        nlohmann::json jt = kt.value();
        bool isValid=false;
        Identifier channelId = m_idHelperSvc->mmIdHelper().pcbID(int(jt["stationName"]), jt["stationEta"], jt["stationPhi"], jt["multiLayer"], jt["gasGap"], jt["pcbPos"], isValid);
        if(!isValid){
            ATH_MSG_FATAL("Cannot find PCB Id!");
            return StatusCode::FAILURE;
        }
        writeCdo.setData(channelId, jt["pcbPos"], jt["indiv"], jt["extra"], jt["position"]);
        ++nChns;
    }
    ATH_MSG_VERBOSE("Retrieved data for "<<nChns<<" channels.");
    return StatusCode::SUCCESS;
}
}