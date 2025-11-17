/*
 Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
//***************************************************************************
//             Interface for jFEXCondAlgo - Tool to read the COOL DB for jFEX
//                              -------------------
//     begin                : 01 08 2023
//     email                : Sergi.Rodriguez@cern.ch
//***************************************************************************
#include "jFEXCondAlgo.h"
#include "nlohmann/json.hpp"
#include "CoralBase/Blob.h"
#include "AthenaKernel/IOVInfiniteRange.h"


#include "jFEXCoolDBDefaults.h"



#include <fstream>

namespace jDBdefaults = LVL1::jFEXCoolDBDefaults;

namespace LVL1 {
    
//Default Constructor
jFEXCondAlgo::jFEXCondAlgo(const std::string& name, ISvcLocator* svc) : AthReentrantAlgorithm(name, svc){}

StatusCode jFEXCondAlgo::initialize() {
    
    ATH_MSG_INFO("Loading jFEX parameters from DB");
    
    ATH_CHECK( m_JfexModuleSettingsKey.initialize(SG::AllowEmpty) );
    ATH_CHECK( m_JfexNoiseCutsKey.initialize(SG::AllowEmpty) );
    ATH_CHECK( m_JfexSystemSettingsKey.initialize(SG::AllowEmpty) );
    ATH_CHECK( m_jFEXDBParamsKey.initialize() );
    
    return StatusCode::SUCCESS;
}


StatusCode jFEXCondAlgo::execute(const EventContext& ctx) const {
   
 
    // Construct the output Cond Object and fill it in
    std::unique_ptr<jFEXDBCondData> writeDBTool(std::make_unique<jFEXDBCondData>() );
    
    //Write handle 
    SG::WriteCondHandle<jFEXDBCondData> writeCHandle(m_jFEXDBParamsKey, ctx);
    if (writeCHandle.isValid()) {
        ATH_MSG_DEBUG("Existing jfex condition data is still valid");
        return StatusCode::SUCCESS;
    }

    // Date from which jFEX database parameters should be used (MC should always use DB)
    // PU fix: 2023-10-27
    bool validTimeStamp = (ctx.eventID().time_stamp() < m_dbBeginTimestamp) ? false : true;
    if (m_isMC)
      validTimeStamp = true;

    // Set DB to false if any of keys not provided
    bool anyKeyEmpty = ( m_JfexModuleSettingsKey.empty() ||  m_JfexNoiseCutsKey.empty() || m_JfexSystemSettingsKey.empty() );

    bool useDBparams = (!anyKeyEmpty && validTimeStamp);
    
    /***************************************/
    /*                                     */
    /*    Jet calibration Parameters       */
    /*                                     */
    /***************************************/
    std::vector<std::vector<int>> jJCalibParams(6, std::vector<int>(25, 0)); //6 modules, up to 25 calib factors
    
    if(!m_JfexModuleSettingsKey.empty() && useDBparams){
        SG::ReadCondHandle <CondAttrListCollection> load_jFexModuleSet{m_JfexModuleSettingsKey, ctx };
        
        // we should check if it is valid and the size is 6 (corresponding to the 6 jfex modules)
        if (load_jFexModuleSet.isValid() and load_jFexModuleSet->size() == 6) {
            
            //setting the validity
            writeCHandle.addDependency(load_jFexModuleSet);
            
            unsigned int mod = 0;
            for (auto itr = load_jFexModuleSet->begin(); itr != load_jFexModuleSet->end(); ++itr) {
                
                const coral::Blob& blob = (itr->second["json"]).data<coral::Blob>();
                const std::string s((char*)blob.startingAddress(),blob.size());       
                nlohmann::json attrList = nlohmann::json::parse(s);
                
                if(attrList["JetCalib"] == nullptr) {
                    ATH_MSG_ERROR("Not loaded jFEX JetCalib from "<<m_JfexModuleSettingsKey);
                    return StatusCode::FAILURE;
                } 
                
                const std::vector<int> v_tmp(attrList["JetCalib"]);
                if ( not(v_tmp.size() == 9 || v_tmp.size() == 8 || v_tmp.size() == 25 ) ){ //9 ET bins (before 2024), 8 (ctrl) / 25 (fwd) eta bins (from 2024 on)
                    ATH_MSG_ERROR("Not loaded jFEX JetCalib from "<<m_JfexModuleSettingsKey<<" (unexpected size: "<<v_tmp.size()<<")");
                    return StatusCode::FAILURE;
                }
                
                jJCalibParams[mod] = std::move(v_tmp);
                mod++;
            }
            
        }
        else{
            ATH_MSG_ERROR("Values from "<<m_JfexModuleSettingsKey<< " not loaded. Wrong key?");
            return StatusCode::FAILURE;
        }        
    }
    else{
        
        // LOADING default values!
        for(unsigned int fex=0; fex<6; fex++){
            for(unsigned int range=0; range<25; range++){
                jJCalibParams[fex][range] = jDBdefaults::jJCalibParams[fex][range];
            }
        }
    }    
    
    writeDBTool->set_jJCalibParam(jJCalibParams);


    /***************************************/
    /*                                     */
    /*    System Settings Parameters       */
    /*                                     */
    /***************************************/    

    // Loading defaults
    bool PileUpCorrectionJet              = jDBdefaults::PileUpCorrectionJet;
    bool PileUpCorrectionMET              = jDBdefaults::PileUpCorrectionMET;
    int PileUpThresholdLowEm             = jDBdefaults::PileUpThresholdLowEm;
    int PileUpThresholdHighEm            = jDBdefaults::PileUpThresholdHighEm;
    int PileUpThresholdLowHadLar         = jDBdefaults::PileUpThresholdLowHadLar;
    int PileUpThresholdHighHadLar        = jDBdefaults::PileUpThresholdHighHadLar;
    int PileUpThresholdLowHadHecOverlap  = jDBdefaults::PileUpThresholdLowHadHecOverlap;
    int PileUpThresholdHighHadHecOverlap = jDBdefaults::PileUpThresholdHighHadHecOverlap;
    int PileUpThresholdLowHadTrex        = jDBdefaults::PileUpThresholdLowHadTrex;
    int PileUpThresholdHighHadTrex       = jDBdefaults::PileUpThresholdHighHadTrex;
    int PileUpThresholdLowFcal           = jDBdefaults::PileUpThresholdLowFcal;
    int PileUpThresholdHighFcal          = jDBdefaults::PileUpThresholdHighFcal;    
      

    if(!m_JfexSystemSettingsKey.empty() && useDBparams) {
        SG::ReadCondHandle <CondAttrListCollection> load_SystemSet{m_JfexSystemSettingsKey, ctx};

        const std::vector<std::string> myStrings{ "PileUpCorrectionJet", "PileUpCorrectionMET", "PileUpThresholdLowEm", "PileUpThresholdHighEm", "PileUpThresholdLowHadLar", "PileUpThresholdHighHadLar", "PileUpThresholdLowHadHecOverlap", "PileUpThresholdHighHadHecOverlap", "PileUpThresholdLowHadTrex", "PileUpThresholdHighHadTrex", "PileUpThresholdLowFcal", "PileUpThresholdHighFcal" };

        if (load_SystemSet.isValid()) {
            
            //setting the validity
            writeCHandle.addDependency(load_SystemSet);
            
            for (auto itr = load_SystemSet->begin(); itr != load_SystemSet->end(); ++itr) {

                const coral::Blob& blob = (itr->second["json"]).data<coral::Blob>();
                const std::string s((char*)blob.startingAddress(),blob.size());
                nlohmann::json attrList = nlohmann::json::parse(s);

                try {

                    //checking if all the string are present in the json file
                    bool allitemsPresent = true;
                    for(const auto & name:myStrings ) {
                        allitemsPresent = allitemsPresent && attrList.contains(name);
                    }
                    if(allitemsPresent) {
                        PileUpCorrectionJet              = (bool) attrList[myStrings.at( 0)];
                        PileUpCorrectionMET              = (bool) attrList[myStrings.at( 1)];
                        PileUpThresholdLowEm             = (int)  attrList[myStrings.at( 2)];
                        PileUpThresholdHighEm            = (int)  attrList[myStrings.at( 3)];
                        PileUpThresholdLowHadLar         = (int)  attrList[myStrings.at( 4)];
                        PileUpThresholdHighHadLar        = (int)  attrList[myStrings.at( 5)];
                        PileUpThresholdLowHadHecOverlap  = (int)  attrList[myStrings.at( 6)];
                        PileUpThresholdHighHadHecOverlap = (int)  attrList[myStrings.at( 7)];
                        PileUpThresholdLowHadTrex        = (int)  attrList[myStrings.at( 8)];
                        PileUpThresholdHighHadTrex       = (int)  attrList[myStrings.at( 9)];
                        PileUpThresholdLowFcal           = (int)  attrList[myStrings.at(10)];
                        PileUpThresholdHighFcal          = (int)  attrList[myStrings.at(11)];

                    }
                    else {
                        throw (uint16_t) itr->first;
                    }
                }
                catch(uint16_t errTower) {
                    ATH_MSG_DEBUG("Loading Pileup values for jFEX ID A:"<<errTower <<". Some towers are not used anymore. Skipping");
                }
            }
        }
        else {
            ATH_MSG_ERROR("Values from "<<m_JfexSystemSettingsKey<< " not loaded. Wrong key?");
            return StatusCode::FAILURE;
        }
    }

    writeDBTool->set_doPileUpJet(PileUpCorrectionJet);
    writeDBTool->set_doPileUpMet(PileUpCorrectionMET);
    writeDBTool->set_PUThrLowEm(PileUpThresholdLowEm);
    writeDBTool->set_PUThrHighEm(PileUpThresholdHighEm);
    writeDBTool->set_PUThrLowHadLar(PileUpThresholdLowHadLar);
    writeDBTool->set_PUThrHighHadLar(PileUpThresholdHighHadLar);
    writeDBTool->set_PUThrLowHadHecOverlap(PileUpThresholdLowHadHecOverlap);
    writeDBTool->set_PUThrHighHadHecOverlap(PileUpThresholdHighHadHecOverlap);
    writeDBTool->set_PUThrLowHadTrex(PileUpThresholdLowHadTrex);
    writeDBTool->set_PUThrHighHadTrex(PileUpThresholdHighHadTrex);
    writeDBTool->set_PUThrLowFcal(PileUpThresholdLowFcal);
    writeDBTool->set_PUThrHighFcal(PileUpThresholdHighFcal);

    /***************************************/
    /*                                     */
    /*        Noise Cut Parameters         */
    /*                                     */
    /***************************************/ 

    // Noise values for EM and HAD 
    // the map contains: key: OnlineID and the array contains {CutJetEM, CutJetHad, CutMetEM, CutMetHad} in that order!
    std::unordered_map< uint16_t, std::array<uint16_t,4> > NoiseCuts;
    
    // PileUp values for EM and HAD 
    // the map contains: key: OnlineID and the array contains {CutJetEM, CutJetHad, CutMetEM, CutMetHad} in that order!
    std::unordered_map< uint16_t, std::array<uint16_t,4> > PileUpWeight;    
      
    
    if(!m_JfexNoiseCutsKey.empty() && useDBparams) {
        
        SG::ReadCondHandle <CondAttrListCollection> load_jFexNoiseCut{m_JfexNoiseCutsKey, ctx };

        std::vector<std::string> myStringsNoise;
 
        if (PileUpCorrectionJet == 0 and PileUpCorrectionMET == 0)
            myStringsNoise = { "CutJetEM", "CutJetHad", "CutMetEM", "CutMetHad"             };
        else if (PileUpCorrectionJet == 1 and PileUpCorrectionMET == 0)
            myStringsNoise = { "CutJetPUEM", "CutJetPUHad", "CutMetEM", "CutMetHad"         };
        else if (PileUpCorrectionJet == 0 and PileUpCorrectionMET == 1)
            myStringsNoise = { "CutJetEM", "CutJetHad", "CutMetPUEM", "CutMetPUHad"         };
        else // (PileUpCorrectionJet == 1 and PileUpCorrectionMET == 1)
            myStringsNoise = { "CutJetPUEM", "CutJetPUHad", "CutMetPUEM", "CutMetPUHad"     };
       
        const std::vector<std::string> myStringsPileup{ "PileUpWeightEM", "PileUpWeightHad", "InverseWeightEM", "InverseWeightHad" };

        // we should check if it is valid and the size is 6 (corresponding to the 6 jfex modules)
        if (load_jFexNoiseCut.isValid()) {
            
            //setting the validity
            writeCHandle.addDependency(load_jFexNoiseCut);
            
            writeDBTool->set_sendDefaults(false);

            for (auto itr = load_jFexNoiseCut->begin(); itr != load_jFexNoiseCut->end(); ++itr) {

                const coral::Blob& blob = (itr->second["json"]).data<coral::Blob>();
                const std::string s((char*)blob.startingAddress(),blob.size());
                nlohmann::json attrList = nlohmann::json::parse(s);


                //Trying to update Noise cut values
                try {

                    bool allitemsPresent = true;
                    for(const auto & name:myStringsNoise ) {
                        allitemsPresent = allitemsPresent && attrList.contains(name);
                    }

                    if( allitemsPresent ) {
                        NoiseCuts[ (uint16_t) itr->first ]= { {(uint16_t) attrList[myStringsNoise.at(0)],(uint16_t) attrList[myStringsNoise.at(1)],(uint16_t) attrList[myStringsNoise.at(2)],(uint16_t) attrList[myStringsNoise.at(3)]} };
                    }
                    else {
                        throw (uint16_t) itr->first;
                    }
                }
                catch(uint16_t errTower) {
                    ATH_MSG_DEBUG("Loading Pileup values for jFEX ID B:"<<errTower <<". Some towers are not used anymore. Skipping");
                    NoiseCuts[ (uint16_t) itr->first ] = {0, 0, 0, 0};
                }

                //Trying to update the PileUp values
                //Some jTower are expected to throw and exception, wrong in DB
                try {

                    bool allitemsPresent = true;
                    for(const auto & name:myStringsPileup ) {
                        allitemsPresent = allitemsPresent && attrList.contains(name);
                    }

                    if( allitemsPresent ) {
                        PileUpWeight[ (uint16_t) itr->first ]= { {(uint16_t) attrList[myStringsPileup.at(0)],(uint16_t) attrList[myStringsPileup.at(1)],(uint16_t) attrList[myStringsPileup.at(2)],(uint16_t) attrList[myStringsPileup.at(3)]} };
                    }
                    else {
                        throw (uint16_t) itr->first;
                    }
                }
                catch(uint16_t errTower) {
                    ATH_MSG_DEBUG("Loading Pileup values for jFEX ID C:"<<errTower <<". Some towers are not used anymore. Skipping");
                    PileUpWeight[ (uint16_t) itr->first ] = {0, 0, 0, 0};
                }
            }
            
        }
        else {
            ATH_MSG_ERROR("Values from "<<m_JfexNoiseCutsKey<< " not loaded. Wrong key?");
            return StatusCode::FAILURE;
        }
    }
    else{
        NoiseCuts[0x0000] = jDBdefaults::PileUpWeight_default; // Unused - filled with 0s
        NoiseCuts[0x00f0] = jDBdefaults::NoiseCuts_LATOME_TILE;
        NoiseCuts[0x0f00] = jDBdefaults::NoiseCuts_LATOME_HEC;
        NoiseCuts[0xf000] = jDBdefaults::NoiseCuts_FCAL;
        
        PileUpWeight[0x0000] = jDBdefaults::PileUpWeight_default;
        PileUpWeight[0x00f0] = jDBdefaults::PileUpWeight_default;
        PileUpWeight[0x0f00] = jDBdefaults::PileUpWeight_default;
        PileUpWeight[0xf000] = jDBdefaults::PileUpWeight_default;
    }
    
    
    writeDBTool->set_NoiseCuts(NoiseCuts);
    writeDBTool->set_PileUpValues(PileUpWeight);    
    
    if(msgLvl(MSG::DEBUG)){

        std::stringstream myprint;
	if (useDBparams)
	  myprint << "Parameters obtained from m_JfexModuleSettingsKey: "<< m_JfexModuleSettingsKey << std::endl;
	else
	  myprint << "JfexModuleSettings obtained from jDBdefaults"<< std::endl;
        myprint << "jJCalibParam:" << std::endl;

        for(int mod=0; mod<6; mod++) {
            myprint << "jFEX"<<mod<<" - ";
            for(int range=0; range<25; range++) {
                myprint << writeDBTool->get_jJCalibParam(mod,range)<< " ";
            }
            myprint << std::endl;
        }
        myprint << std::endl;

        ATH_MSG_DEBUG(myprint.str());


        std::stringstream myprint1;

	if (useDBparams)
	  myprint1 << "Parameters obtained from m_JfexSystemSettingsKey: "<< m_JfexSystemSettingsKey << std::endl;
	else
	  myprint1 << "JfexSystemSettings obtained from jDBdefaults"<< std::endl;

        myprint1 << "System setting parameters: " <<std::endl;
        myprint1 << "PileUpCorrectionJet: "             << writeDBTool->get_doPileUpJet()           <<std::endl;
        myprint1 << "PileUpCorrectionMET: "             << writeDBTool->get_doPileUpMet()           <<std::endl;
        myprint1 << "PileUpThresholdLowEm: "            << writeDBTool->get_PUThrLowEm()            <<std::endl;
        myprint1 << "PileUpThresholdHighEm: "           << writeDBTool->get_PUThrHighEm()           <<std::endl;
        myprint1 << "PileUpThresholdLowHadLar: "        << writeDBTool->get_PUThrLowHadLar()        <<std::endl;
        myprint1 << "PileUpThresholdHighHadLar: "       << writeDBTool->get_PUThrHighHadLar()       <<std::endl;
        myprint1 << "PileUpThresholdLowHadHecOverlap: " << writeDBTool->get_PUThrLowHadHecOverlap() <<std::endl;
        myprint1 << "PileUpThresholdHighHadHecOverlap: "<< writeDBTool->get_PUThrHighHadHecOverlap()<<std::endl;
        myprint1 << "PileUpThresholdLowHadTrex: "       << writeDBTool->get_PUThrLowHadTrex()       <<std::endl;
        myprint1 << "PileUpThresholdHighHadTrex: "      << writeDBTool->get_PUThrHighHadTrex()      <<std::endl;
        myprint1 << "PileUpThresholdLowFcal: "          << writeDBTool->get_PUThrLowFcal()          <<std::endl;
        myprint1 << "PileUpThresholdHighFcal: "         << writeDBTool->get_PUThrHighFcal()         <<std::endl;

        ATH_MSG_DEBUG(myprint1.str());

        std::stringstream myprint2;
        
	if (useDBparams)
	  myprint2 << "Parameters obtained from m_JfexNoiseCutsKey: "<< m_JfexNoiseCutsKey << std::endl;
	else
	  myprint2 << "JfexNoiseCuts obtained from jDBdefaults"<< std::endl;

        for( const auto& [key, value] : NoiseCuts) {
            const auto [CutJetEM, CutJetHad, CutMetEM, CutMetHad] = value;
            const auto [PileUpWeightEM, PileUpWeightHad, InverseWeightEM, InverseWeightHad] = PileUpWeight[key];

            myprint2 <<"OnlineID: 0x"<< std::hex<<key<<std::dec<< " - CutJetEM: "<< CutJetEM<< ", CutJetHad:"<< CutJetHad<< ", CutMetEM:"<< CutMetEM<< ", CutMetHad:"<< CutMetHad<< ", PileUpWeightEM: "<< PileUpWeightEM<< ", PileUpWeightHad:"<< PileUpWeightHad<< ", InverseWeightEM:"<< InverseWeightEM<< ", InverseWeightHad:"<< InverseWeightHad<< std::endl;
        }

        ATH_MSG_DEBUG(myprint2.str());

    }


    
    // If DB parameters not loaded, valid for infinity and beyond!
    if (!useDBparams) {
        writeCHandle.addDependency(IOVInfiniteRange::infiniteRunLB()); // Use infinite IOV    
    }
    
    //Wrinting into SG!
    if (writeCHandle.record(std::move(writeDBTool)).isFailure()) {
        ATH_MSG_ERROR("Could not record " << writeCHandle.key() << " with EventRange " << writeCHandle.getRange() << " into Conditions Store");
        return StatusCode::FAILURE;
    }
    ATH_MSG_INFO("Recorded " << writeCHandle.key() << " with EventRange " << writeCHandle.getRange() << " into Conditions Store");
    
    return StatusCode::SUCCESS;
    
}



}// end of namespace LVL1
