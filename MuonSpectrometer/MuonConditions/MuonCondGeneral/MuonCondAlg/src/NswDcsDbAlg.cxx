/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "NswDcsDbAlg.h"

#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "AthenaKernel/IOVInfiniteRange.h"
#include <string>
#include <regex>
#include "nlohmann/json.hpp"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "GeoModelKernel/throwExcept.h"

namespace Muon{
// Initialize
StatusCode
NswDcsDbAlg::initialize(){

    // retrievals
    ATH_MSG_DEBUG( "initializing " << name() );                
    ATH_CHECK(m_idHelperSvc.retrieve());
    
    // initialize read keys
    ATH_CHECK(m_readKey_mmg_hv.initialize(!m_readKey_mmg_hv.empty() && m_idHelperSvc->hasMM() ));
    ATH_CHECK(m_readKey_stg_hv.initialize(!m_readKey_stg_hv.empty() && m_idHelperSvc->hasSTGC() ));
    ATH_CHECK(m_readKey_mmg_tdaq.initialize(!m_readKey_mmg_tdaq.empty() && m_loadTdaq && m_idHelperSvc->hasMM() ));
    ATH_CHECK(m_readKey_stg_tdaq.initialize(!m_readKey_stg_tdaq.empty() && m_loadTdaq && m_idHelperSvc->hasSTGC() ));
    ATH_CHECK(m_readKey_mmg_eltx.initialize(!m_readKey_mmg_eltx.empty() && m_loadEltx && m_idHelperSvc->hasMM() ));
    ATH_CHECK(m_readKey_stg_eltx.initialize(!m_readKey_stg_eltx.empty() && m_loadEltx && m_idHelperSvc->hasSTGC() ));
    
    // write key for time/charge data
    ATH_CHECK(m_writeKey.initialize());

	CHECK(m_muonManagerKey.initialize());

    ATH_MSG_INFO("NswDcsAlg is using tdaq "<< m_loadTdaq << " and eltx " << m_loadEltx);
    
    return StatusCode::SUCCESS;
}

// execute
StatusCode 
NswDcsDbAlg::execute(const EventContext& ctx) const {

    ATH_MSG_DEBUG( "execute " << name() );   

    // set up write handles
    SG::WriteCondHandle wrHdl{m_writeKey, ctx};
    if (wrHdl.isValid()) {
        ATH_MSG_DEBUG("CondHandle " << wrHdl.fullKey() << " is already valid."
            << " In theory this should not be called, but may happen"
            << " if multiple concurrent events are being processed out of order.");
        return StatusCode::SUCCESS;
    }
    wrHdl.addDependency(EventIDRange(IOVInfiniteRange::infiniteMixed()));
    ATH_MSG_DEBUG("Range of time/charge output is " << wrHdl.getRange());

	SG::ReadCondHandle<MuonGM::MuonDetectorManager> detMgr{m_muonManagerKey, ctx};
    if (!detMgr.isValid()) {
        ATH_MSG_FATAL(
            "Failed to retrieve the detector manager from the conditions store "
            << m_muonManagerKey.fullKey());
        return StatusCode::FAILURE;
    }
	wrHdl.addDependency(detMgr);
    std::unique_ptr<NswDcsDbData> wrCdo{std::make_unique<NswDcsDbData>(m_idHelperSvc.get(), detMgr.cptr())};

    // load HV data
    if(!m_readKey_mmg_hv.empty()){
        ATH_CHECK(loadHvData(ctx, m_readKey_mmg_hv, DcsTechType::MMG, wrHdl, wrCdo.get()));
        ATH_CHECK(loadHvData(ctx, m_readKey_mmg_hv, DcsTechType::MMD, wrHdl, wrCdo.get()));
    }
    if(!m_readKey_stg_hv.empty()){
        ATH_CHECK(loadHvData(ctx, m_readKey_stg_hv, DcsTechType::STG, wrHdl, wrCdo.get()));
    }

    // load TDAQ data
    if(m_loadTdaq){
        if(!m_readKey_mmg_tdaq.empty()) {
            ATH_CHECK(loadTDaqData(ctx, m_readKey_mmg_tdaq, DcsTechType::MMG, wrHdl, wrCdo.get()));
        }
        if(!m_readKey_stg_tdaq.empty()) {
            ATH_CHECK(loadTDaqData(ctx, m_readKey_stg_tdaq, DcsTechType::STG, wrHdl, wrCdo.get()));
        }
    }
    if(m_loadEltx){
        if(!m_readKey_mmg_eltx.empty()) {
            ATH_CHECK(loadELTXData(ctx, m_readKey_mmg_eltx, DcsTechType::MMG, wrHdl, wrCdo.get()));
        }
        if(!m_readKey_stg_eltx.empty()) {
            ATH_CHECK(loadELTXData(ctx, m_readKey_stg_eltx, DcsTechType::STG, wrHdl, wrCdo.get()));
        }
    }
  
    // insert/write data
    if (wrHdl.record(std::move(wrCdo)).isFailure()) {
        ATH_MSG_FATAL("Could not record " << wrHdl.key() 
               << " with EventRange " << wrHdl.getRange()
               << " into Conditions Store");
        return StatusCode::FAILURE;
    }      
    ATH_MSG_DEBUG("Recorded new " << wrHdl.key() << " with range " << wrHdl.getRange() << " into Conditions Store");
    
    return StatusCode::SUCCESS;

}

// loadHvData
StatusCode 
NswDcsDbAlg::loadHvData(const EventContext& ctx, const readKey_t& readKey, const DcsTechType tech, writeHandleDcs_t& writeHandle, NswDcsDbData* writeCdo) const {

    // set up read handle
    SG::ReadCondHandle readCdo{readKey, ctx};
    if(!readCdo.isValid()){
        ATH_MSG_ERROR("Null pointer to the read conditions object");
        return StatusCode::FAILURE; 
    } 
    writeHandle.addDependency(readCdo);
    ATH_MSG_DEBUG("Size of CondAttrListCollection " << readKey.fullKey() << " readCdo->size()= " << readCdo->size());
    ATH_MSG_DEBUG("Range of input is " << readCdo.getRange() << ", range of output is " << writeHandle.getRange());

    // iterate through data
    CondAttrListCollection::const_iterator itr;
    unsigned int nChns = 0;
    for(itr = readCdo->begin(); itr != readCdo->end(); ++itr) {

        // channel ID and name
        const unsigned int chanNum  = itr->first;
        const std::string& chanName = readCdo->chanName(chanNum);
        if(chanName.empty()){
            ATH_MSG_DEBUG("Channel number "<< chanNum <<"has empty name");
            continue;
        }
        Identifier channelId{0};
        bool isOK = false;
        bool found = buildChannelIdForHv(channelId, tech, chanName, isOK);
        if(!found){
            if(!isOK){
                ATH_MSG_ERROR("Could not identify valid channelId for channel "<<chanNum<<" with name "<< chanName<<"!");
                THROW_EXCEPTION("NswDcsDbAlg: Could not identify valid channelId for HV channel");
            }
        continue;
        }

        // payload
        const coral::AttributeList& atr = itr->second;

        NswDcsDbData::DcsConstants dcs_data{};
        dcs_data.standbyVolt     = *(static_cast<const float*>((atr["v0Set"]).addressOfData()));
        dcs_data.readyVolt     = *(static_cast<const float*>((atr["v1Set"]).addressOfData()));
        dcs_data.fsmState  = MuonCond::getFsmStateEnum(*(static_cast<const std::string*>((atr["fsmCurrentState"]).addressOfData())));
        ATH_MSG_DEBUG("channel " << chanName << " has fsm state " << *(static_cast<const std::string*>((atr["fsmCurrentState"]).addressOfData()))<< " has v0 state " << *(static_cast<const float*>( (atr["v0Set"]).addressOfData()))<< " has v1 " << *(static_cast<const float*>((atr["v1Set"]).addressOfData())));
        
        writeCdo->setDataHv(tech, channelId, dcs_data);
        ++nChns;
    }
    ATH_MSG_VERBOSE("Retrieved data for "<<nChns<<" channels.");

    return StatusCode::SUCCESS;
}


// loadTDaqData
StatusCode 
NswDcsDbAlg::loadTDaqData(const EventContext& ctx, const readKey_t& readKey, const DcsTechType tech, writeHandleDcs_t& writeHandle, NswDcsDbData* writeCdo) const {
    // set up read handle
    SG::ReadCondHandle readCdo{readKey, ctx};
    if(!readCdo.isValid()){
        ATH_MSG_ERROR("Null pointer to the read conditions object");
        return StatusCode::FAILURE; 
    } 
    writeHandle.addDependency(readCdo);
    ATH_MSG_DEBUG("Size of CondAttrListCollection " << readKey.fullKey() << " readCdo->size()= " << readCdo->size());
    ATH_MSG_DEBUG("Range of input is " << readCdo.getRange() << ", range of output is " << writeHandle.getRange());

    // iterate through data
    CondAttrListCollection::const_iterator itr;
    for(itr = readCdo->begin(); itr != readCdo->end(); ++itr) {

        // retrieve data
        const coral::AttributeList& atr = itr->second;
        std::string data = *(static_cast<const std::string *>((atr["data_array"]).addressOfData()));

        // unwrap the json and process the data
        nlohmann::json jx = nlohmann::json::parse(data);
        unsigned int nLB = 0; 

        // loop over lumi blocks and channels
        for (auto &yy : jx["holes"].items()) {
            nlohmann::json jt = yy.value();

            // channel ID and name
            Identifier channelId{0};
            bool isOK = false;
            uint elink{0};
            bool found = buildChannelIdForTDaq(channelId, elink, tech, jt["channelName"], isOK);
            if(!found){
                if(!isOK)
                    ATH_MSG_DEBUG("Could not identify valid channelId for channel "<<jt["channelId"]<<" with name "<< jt["channelName"]<<"! Skipping...");
                continue;
            }
            // write data
            int channelDead = jt["channelDead"];
            writeCdo->setDataTDaq(tech, channelId, jt["hole_iovSince"], jt["hole_iovUntil"], elink, channelDead );
            ATH_MSG_VERBOSE(m_idHelperSvc->toString(channelId)<<" " << jt["channelName"] << " " << jt["hole_iovSince"]<<" " <<jt["hole_iovUntil"]<<" " <<  elink<<" "<<channelDead );
            ++nLB;
        }
        ATH_MSG_VERBOSE("Retrieved data for "<<nLB<<" entries (combinations of lumi block and channel).");
    }

    return StatusCode::SUCCESS;
}

StatusCode NswDcsDbAlg::loadELTXData(const EventContext& ctx, const readKey_t& readKey, const DcsTechType tech, writeHandleDcs_t& writeHandle, NswDcsDbData* writeCdo) const {

    // set up read handle
    SG::ReadCondHandle readCdo{readKey, ctx};
    if(!readCdo.isValid()){
        ATH_MSG_ERROR("Null pointer to the read conditions object");
        return StatusCode::FAILURE; 
    } 
    writeHandle.addDependency(readCdo);
    ATH_MSG_DEBUG("Size of CondAttrListCollection " << readKey.fullKey() << " readCdo->size()= " << readCdo->size());
    ATH_MSG_DEBUG("Range of input is " << readCdo.getRange() << ", range of output is " << writeHandle.getRange());

    // This code is under development so lets mute unused variable warnings for now
    (void) writeCdo;
    (void) tech;

    // iterate through data
    CondAttrListCollection::const_iterator itr;
    for(itr = readCdo->begin(); itr != readCdo->end(); ++itr) {

        // channel ID and name
        const unsigned int chanNum  = itr->first;
        const std::string& chanName = readCdo->chanName(chanNum);
        if(chanName.empty()){
            ATH_MSG_DEBUG("Channel number "<< chanNum <<"has empty name");
            continue;
        }
        const coral::AttributeList& atr = itr->second;
        bool online = *(static_cast<const bool*>((atr["online"]).addressOfData()));
        ATH_MSG_DEBUG("found SCA " << chanName << " with status " << atr["online"] << " " << online);
        if(online) continue; // we only want to record things that are dead
        Identifier channelId{};
        bool isOk{false};
        if(!buildChannelIdForEltx(channelId, tech, chanName, isOk)){
          continue;
        }
        ATH_MSG_DEBUG(__FILE__<<":"<<__LINE__<<" "<<m_idHelperSvc->toString(channelId));
        writeCdo->setDataEltx(tech, channelId);
    }
    return StatusCode::SUCCESS;

}


// buildChannelIdForHv
bool
NswDcsDbAlg::buildChannelIdForHv(Identifier& channelId, const DcsTechType tech0, const std::string& chanName, bool& isOK) const {

    // prepare regex
    std::regex reMMG("^([A-Za-z]{1})([0-9]{2})_ML([0-9])P([0-9])_(IP|HO)R([0-9])__HV");
    std::regex reMMD("^([A-Za-z]{1})([0-9]{2})_DRIFT_(IP|HO)R([0-9])__HV");
    std::regex reSTG("^([A-Za-z]{1})([0-9]{2})_ML([0-9])_(IP|HO)R([0-9])__HV");
    std::regex re;
    DcsTechType tech = DcsTechType::MMG; 

    // match regex
    if(std::regex_match(chanName, reMMG)) {
        re   = reMMG;
        tech = DcsTechType::MMG;
    }
    else if(std::regex_match(chanName, reMMD)) {
        re   = reMMD;
        tech = DcsTechType::MMD;
    }
    else if(std::regex_match(chanName, reSTG)) {
        re   = reSTG;
        tech = DcsTechType::STG;
    }
    else {
        ATH_MSG_ERROR("Could not identify channel with name "<<chanName);
        isOK = false;
        return false;
    }

    // sanity check
    if(tech0!=tech) {
        isOK = true; // need to distinguish error in parsing and simple difference between required technologies
        return false;
    }

    // build channel Id
    bool isValid{false};
    std::smatch m;
    std::regex_match(chanName, m, re);

    // copy to vec of strings
    std::vector<std::string> res;
    for(unsigned int i=0; i<m.size(); ++i) res.push_back(m[i].str());

    // MMG Channel
    if(tech==DcsTechType::MMG){
        int wheel         = res[1]=="A"? 1 : -1;
        int sector        = std::stoi(res[2]);
        const std::string stationName   = sector%2==0 ? "MMS" : "MML";
        int stationEta    = wheel*std::stoi(res[6]);
        int stationPhi    = (sector-1)/2+1;
        int multiLayer    = res[5]=="IP" ? 1 : 2;
        int gasGap        = std::stoi(res[3]);
        int pcb           = std::stoi(res[4]);
        Identifier chnlId = m_idHelperSvc->mmIdHelper().pcbID(stationName, stationEta, stationPhi, multiLayer, gasGap, pcb, isValid);
        if(!isValid){
            ATH_MSG_DEBUG("Could not extract valid channelId for MMG channel "<<chanName);
            ATH_MSG_DEBUG("Fields: "<< wheel << " "<<sector<<" " << stationName<< " " << stationEta<<" "<<stationPhi<<" "<<multiLayer);
            isOK = false;
            return false;
        }
        channelId = chnlId;
    }

    // MMG Drift Channel
    else if(tech==DcsTechType::MMD){
        int wheel         = res[1]=="A"? 1 : -1;
        int sector        = std::stoi(res[2]);
        const std::string stationName   = sector%2==0 ? "MMS" : "MML";
        int stationEta    = wheel*std::stoi(res[4]);
        int stationPhi    = (sector-1)/2+1;
        int multiLayer    = res[3]=="IP" ? 1 : 2;
        Identifier modId  = m_idHelperSvc->mmIdHelper().elementID(stationName, stationEta, stationPhi, isValid);
        if(!isValid){
            ATH_MSG_DEBUG("Could not extract valid elementId for MMGD channel "<<chanName);
            ATH_MSG_DEBUG("Fields: "<< wheel << " "<<sector<<" " << stationName<< " " << stationEta<<" "<<stationPhi<<" "<<multiLayer);
            isOK = false;
            return false;
        }
        Identifier chnlId = m_idHelperSvc->mmIdHelper().multilayerID(modId, multiLayer, isValid);
        if(!isValid){
            ATH_MSG_DEBUG("Could not extract valid multilayerId for MMG channel "<<chanName);
            ATH_MSG_DEBUG("Fields: "<< wheel << " "<<sector<<" " << stationName<< " " << stationEta<<" "<<stationPhi<<" "<<multiLayer);
            isOK = false;
            //return false;
        }
        channelId = chnlId;
    }

    // STG
    else if(tech==DcsTechType::STG){
        int wheel         = res[1]=="A"? 1 : -1;
        int sector        = std::stoi(res[2]);
        const std::string stationName   = sector%2==0 ? "STS" : "STL";
        int radius        = std::stoi(res[5]);
        int stationEta    = wheel*(radius<=2 ? 1 : radius-1);
        int stationPhi    = (sector-1)/2+1;
        int multiLayer    = res[4]=="IP" ? 1 : 2;
        int gasGap        = std::stoi(res[3]);
        Identifier chnlId = m_idHelperSvc->stgcIdHelper().hvID(stationName, stationEta, stationPhi, multiLayer, gasGap, radius == 1 /*radius 1 is the inner hv section*/ , isValid);
        if(!isValid){
            ATH_MSG_DEBUG("Could not extract valid channelId for STG channel "<<chanName);
            ATH_MSG_DEBUG("Fields: "<< wheel << " "<<sector<<" " << stationName<< " " << stationEta<<" "<<stationPhi<<" "<<multiLayer);
            isOK = false;
            return false;
        }
        channelId = chnlId;
    }

    return true;
}


// buildChannelIdForTDaq
bool
NswDcsDbAlg::buildChannelIdForTDaq(Identifier& channelId, uint& elink ,const DcsTechType tech0, const std::string& chanName, bool& isOK) const {

    // prepare regex
    std::regex reMMG("^ELink-MM-(A|C)/V([0-9]{1})/L1A/Strip/S([0-9]{1,2})/L([0-9]{1})/R([0-9]{1,2})/E([0-9]{1})");
    std::regex reSTG("^ELink-sTGC-(A|C)/V([0-9]{1})/L1A/(Strip|Pad)/S([0-9]{1,2})/L([0-9]{1})/R([0-9]{1})/E([0-9]{1})");
    std::regex reSTGTrigProc("^ELink-sTGC-A/V0/L1A/TrigProc/");
    std::regex reSTGPadTrig("^ELink-sTGC-A/V0/L1A/PadTrig/");
    
    std::regex re;
    DcsTechType tech = DcsTechType::MMG; 

    // match regex
    if(std::regex_match(chanName, reMMG)) {
        re   = reMMG;
        tech = DcsTechType::MMG;
    }
    else if(std::regex_match(chanName, reSTG)) {
        re   = reSTG;
        tech = DcsTechType::STG;
    } else if(std::regex_match(chanName, reSTGPadTrig) || std::regex_match(chanName, reSTGTrigProc)){ // those are trigger elinks that are not needed in athena
        isOK = true;
        return false;
    }
    else {
        ATH_MSG_DEBUG("Could not identify channel with name "<<chanName);
        isOK = false;
        return false;
    }

    // sanity check
    if(tech0!=tech) {
        isOK = true; // need to distinguish error in parsing and simple difference between required technologies
        return false;
    }

    // build channel Id
    bool isValid{false};
    std::smatch m;
    std::regex_match(chanName, m, re);

    // copy to vec of strings
    std::vector<std::string> res;
    for(unsigned int i=0; i<m.size(); ++i) res.push_back(m[i].str());

    // MMG Channel
    if(tech==DcsTechType::MMG){
        int wheel         = res[1]=="A"? 1 : -1;
        int sector        = std::stoi(res[3])+1; // elx counts from 0 athena from 1 -->need a +1
        int stationName   = sector%2==0 ? 55 : 56;
        /*
        res[4] -> L
        res[5] -> R
        res[6] -> E
        */
        int stationEta    = wheel*(std::stoi(res[5])<10? 1 : 2); // boards 0-9 are und the first quad, boards 10-15 und the second one
        int stationPhi    = (sector-1)/2+1;
        int multiLayer    = std::stoi(res[4])< 4 ? 1 : 2; // layers 0-3 are on multilayer 1 and layers 4-7 are on multilayer 2
        int gasGap        = (std::stoi(res[4])%4) + 1; // identifies layer within multilayer --> counts from  0-3; +1 because athena counts from 1-4 
        int radius = std::stoi(res[5]);
        Identifier chnlId = m_idHelperSvc->mmIdHelper().febID(stationName, stationEta, stationPhi, multiLayer, gasGap, radius, isValid);
        elink = std::stoi(res[6]);
        if(!isValid){
            ATH_MSG_DEBUG("Could not extract valid channelId for MMG channel "<<chanName);
            ATH_MSG_DEBUG("Fields: "<< wheel << " "<<sector<<" " << stationName<< " " << stationEta<<" "<<stationPhi<<" "<<multiLayer<<" " << gasGap<<" " << radius<<" " << elink);
            isOK = false;
            return false;
        }
        channelId = chnlId;
    }

    // STG
    else if(tech==DcsTechType::STG){
        int wheel         = res[1]=="A"? 1 : -1;
        int sector        = std::stoi(res[4]) + 1;
        std::string stationName   = sector%2==0 ? "STS" : "STL";
        
        /*
        res[3] -> Strip/Pad
        res[5] -> L
        res[6] -> R
        res[7] -> E
        */
        
        int    radius        = std::stoi(res[6]);
        int stationEta    = wheel*(radius+1);
        int stationPhi    = (sector-1)/2+1;
        int multiLayer    = (std::stoi(res[5]) < 4 ?  1 : 2);
        int gasGap        = ((std::stoi(res[5]))%4)+1;
        uint channelType  = (res[3] == "Pad" ? sTgcIdHelper::sTgcChannelTypes::Pad : sTgcIdHelper::sTgcChannelTypes::Strip);
        Identifier chnlId = m_idHelperSvc->stgcIdHelper().febID(stationName, stationEta, stationPhi, multiLayer, gasGap, channelType, isValid);
        if(!isValid){
            ATH_MSG_DEBUG("Could not extract valid channelId for STG channel "<<chanName);
            ATH_MSG_DEBUG("Fields: "<< wheel << " "<<sector<<" " << stationName<< " " << stationEta<<" "<<stationPhi<<" "<<multiLayer<<" " << gasGap<<" " << radius<<" " << elink);
            isOK = false;
            return false;
        }
        channelId = chnlId;
        elink = std::stoi(res[7]);    
    }

    return true;
}

// buildChannelIdForElx
bool
NswDcsDbAlg::buildChannelIdForEltx(Identifier& channelId, const DcsTechType tech, const std::string& chanName, bool& isOK) const {

    // prepare regex
    std::regex re("^(A|C)_([0-9]{2})_L([0-9])_B([0-9]{2})");
    

    // match regex
    if(!std::regex_match(chanName, re)) {
      ATH_MSG_WARNING("Could not identify channel with name "<<chanName);
      isOK = false;
      return false;
    }

    // build channel Id
    bool isValid{false};
    std::smatch m;
    std::regex_match(chanName, m, re);

    // copy to vec of strings
    std::vector<std::string> res;
    for(unsigned int i=0; i<m.size(); ++i) res.push_back(m[i].str());

    //extract field common to MM and stgc
    int wheel         = res[1]=="A"? 1 : -1;
    int sector        = std::stoi(res[2])+1;
    int board         = std::stoi(res[4]);
    int layer         = std::stoi(res[3]);
    int stationPhi    = (sector-1)/2+1;
    int multiLayer    = layer< 5 ? 1 : 2; // layers 1-4 are on multilayer 1 and layers 5-8 are on multilayer 2
    int gasGap        = ((layer-1)%4) + 1; // identifies layer within multilayer


    // MMG Channel
    if(tech==DcsTechType::MMG){
        int stationName   = sector%2==0 ? 56 : 55;
    
        board -= (layer%2==1 ? 1 : 2); // for odd layers (counting 1-8 here) the first board is on position 1 for even ones on position 2. https://mattermost.web.cern.ch/files/i84ghkjsfbrzje7c3kr5h7ccdy/public?h=xs3cQQ38yZJDrct75eY73G9d1deOaEtJCHmk2Qcni4s
        int radius{0};
        if(board%4==0){
            radius = 2*(board/4);
        } else if (board%4==3) {
            radius = 2*(board/4) + 1;
        } else { // not a readout board
            isOK = true;
            return false;
        }

        int stationEta    = wheel*(radius<10? 1 : 2); // boards 0-9 are on the first quad, boards 10-15 on the second one
        Identifier chnlId = m_idHelperSvc->mmIdHelper().febID(stationName, stationEta, stationPhi, multiLayer, gasGap, radius, isValid);
        if(!isValid){
            ATH_MSG_WARNING("Could not extract valid channelId for MMG channel "<<chanName);
            ATH_MSG_WARNING("Fields: "<< wheel << " "<<sector<<" " << stationName<< " " << stationEta<<" "<<stationPhi<<" "<<multiLayer<<" " << gasGap<<" " << radius<<" " << board);
            isOK = false;
            return false;
        }
        channelId = chnlId;
    } else if(tech==DcsTechType::STG){
        int stationName   = sector%2==0 ? 58 : 57;
        board -= 1; // count from 0 while input counts from one
        uint radius = board/2;
        int stationEta = wheel*(radius+1);
        uint channelType{0};
        if(board>=6){ // not a STG readout board
            isOK = true;
            return false;
        }    
        else if((layer&1)==1){// for odd layers (counting 1-8 here) the boards on even positions are reading the strips. https://mattermost.web.cern.ch/files/bdh8wwjzf7yiiggtad3u8bttrr/public?h=2fFnVki1EUefrMEa3tb8AZkTiC-tF3L11qdq43dBbJc 
            if(board%2==0){
                channelType=sTgcIdHelper::sTgcChannelTypes::Strip;
            } else {
                channelType=sTgcIdHelper::sTgcChannelTypes::Pad;
            }
        } else if(layer%1==0){// for even layers (counting 1-8 here) the boards on even positions are reading the pads and wires. https://mattermost.web.cern.ch/files/taro34muwpb18pqgiufhwf5a5c/public?h=3YlA-w0NfEuCV2JIGUjiWwpnpDlxvIaEkzrTXWfo71M 
            if(board%2==0){
                channelType=sTgcIdHelper::sTgcChannelTypes::Pad;
            } else {
                channelType=sTgcIdHelper::sTgcChannelTypes::Strip;
            }
        }
        Identifier chnlId = m_idHelperSvc->stgcIdHelper().febID(stationName, stationEta, stationPhi, multiLayer, gasGap, channelType ,isValid);
        if(!isValid){
            ATH_MSG_WARNING("Could not extract valid channelId for STG channel "<<chanName);
            ATH_MSG_WARNING("Fields: "<< wheel << " "<<sector<<" " << stationName<< " " << stationEta<<" "<<stationPhi<<" "<<multiLayer<<" " << gasGap<<" " << radius<<" " << board << " " << channelType);
            isOK = false;
            return false;
        }
        channelId = chnlId;
    }
    return true;
}
}