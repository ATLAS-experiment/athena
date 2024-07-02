/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigJiveXML/TriggerInfoRetriever.h"

#include <string>

#include "CLHEP/Units/SystemOfUnits.h"

#include "AnalysisTriggerEvent/LVL1_ROI.h"

#include "EventInfo/EventInfo.h"
#include "EventInfo/EventID.h"
#include "EventInfo/TriggerInfo.h"
#include <cmath> //std::sqrt

namespace JiveXML {

  //--------------------------------------------------------------------------

  TriggerInfoRetriever::TriggerInfoRetriever(const std::string& type, const std::string& name, const IInterface* parent):
    AthAlgTool(type, name, parent),
    m_typeName("TriggerInfo")
  {

    declareInterface<IDataRetriever>(this);
  }

  //--------------------------------------------------------------------------

  StatusCode TriggerInfoRetriever::retrieve(ToolHandle<IFormatTool> &FormatTool) {

// retrieve TriggerInfo
    const EventInfo* eventInfo;
    const TriggerInfo* trigger_info;
    
    if ( evtStore()->retrieve(eventInfo).isFailure() ) {
      if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG) << "No EventInfo found in SG" << endmsg;
      return StatusCode::SUCCESS;
    } 
    trigger_info = eventInfo->trigger_info();
    if (trigger_info == 0){
      if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG) << "No TriggerInfo in EventInfo" << endmsg;
      return StatusCode::SUCCESS;
    }
    if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG) << " Retrieved EventInfo and TriggerInfo" << endmsg;

    std::string triggerInfoStrL1="0";
    std::string triggerInfoStrL2="0";
    std::string triggerInfoStrEF="0";
    std::string triggerInfoStreamTag="0";

// Level-1 Missing-ET from LVL1_ROI: 
    DataVect energySumEtVec; 
    DataVect energyExVec;
    DataVect energyEyVec;
    DataVect energyEtMissVec;

// TriggerInfo from EventInfo: 
    DataVect trigInfoStatusVec;
    DataVect trigInfoExtL1IDVec;
    DataVect trigInfoLvl1TypeVec;
    DataVect trigInfoL1Vec;
    DataVect trigInfoL2Vec;
    DataVect trigInfoEFVec;
    DataVect trigInfoStreamTagVec;

// from: AtlasTest/DatabaseTest/AthenaPoolTest/AthenaPoolTestDataReader

    trigInfoStatusVec.push_back(DataType( trigger_info->statusElement() ) );
    trigInfoExtL1IDVec.push_back(DataType( trigger_info->extendedLevel1ID() ) );
    trigInfoLvl1TypeVec.push_back(DataType( trigger_info->level1TriggerType() ) );
        if ( trigger_info->level1TriggerInfo().size() > 0 ){
	    triggerInfoStrL1 = "-"; // item seperator
        }
        for (unsigned int i = 0; i < trigger_info->level1TriggerInfo().size(); ++i) {
            triggerInfoStrL1 += DataType( trigger_info->level1TriggerInfo()[i]).toString() + "-";
        }

        if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG) << " lvl2Info " << endmsg;    
        if ( trigger_info->level2TriggerInfo().size() > 0 ){
	    triggerInfoStrL2 = "-"; // item seperator
        }
        for (unsigned int i = 0; i < trigger_info->level2TriggerInfo().size(); ++i) {
            triggerInfoStrL2 += DataType( trigger_info->level2TriggerInfo()[i] ).toString() + "-";
        }

        if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG) << "EventFilterInfo " << endmsg;    
        if ( trigger_info->eventFilterInfo().size() > 0 ){
	    triggerInfoStrEF = "-"; // item seperator
        }
        for (unsigned int i = 0; i < trigger_info->eventFilterInfo().size(); ++i) {
            triggerInfoStrEF += DataType( trigger_info->eventFilterInfo()[i]).toString() + "-";
        }

        if ( trigger_info->streamTags().size() > 0 ){
	    triggerInfoStreamTag = "-"; // item seperator
        }
        for (unsigned int i = 0; i < trigger_info->streamTags().size(); ++i) {
           if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG)  << " i " << i << " name " << trigger_info->streamTags()[i].name()
          << " type " << trigger_info->streamTags()[i].type()
          << " ObeysLumi " << trigger_info->streamTags()[i].obeysLumiblock();
            //// normal code without mask is this:
	    triggerInfoStreamTag += trigger_info->streamTags()[i].name() + "_" +
            trigger_info->streamTags()[i].type() + "_";
          if ( trigger_info->streamTags()[i].obeysLumiblock() ){ // is a boolean
             triggerInfoStreamTag += "ObeysLumi";
          }else{
             triggerInfoStreamTag += "notObeysLumi";
	  }
          if ( eventInfo->event_ID()->lumi_block() ){
	     triggerInfoStreamTag += "_lumiBlock";
	     triggerInfoStreamTag += DataType(eventInfo->event_ID()->lumi_block()).toString();
          }else{
	     triggerInfoStreamTag += "_lumiBlockUnknown";
          }
          triggerInfoStreamTag += "-";
	}

    if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG) << "trigInfoL1: " << triggerInfoStrL1 << endmsg;
    if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG) << "trigInfoL2: " << triggerInfoStrL2 << endmsg;
    if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG) << "trigInfoEF: " << triggerInfoStrEF << endmsg;
    if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG) << "triggerInfoStreamTag: " << triggerInfoStreamTag << endmsg;

    trigInfoL1Vec.emplace_back(std::move(triggerInfoStrL1));
    trigInfoL2Vec.emplace_back(std::move(triggerInfoStrL2 ));
    trigInfoEFVec.emplace_back(std::move( triggerInfoStrEF ));
    trigInfoStreamTagVec.emplace_back(std::move( triggerInfoStreamTag ));

    // Retrieve LVL1_ROI for trigger energies
    // assume that eventInfo->triggerInfo _has_ to be present,
    // but LVL1_ROI not present under all circumstances (ACR: muoncomm)
    // Don't exit upon failed SG retrieval as usual.

    const LVL1_ROI * roi;
    if ( evtStore()->retrieve(roi,"LVL1_ROI").isFailure() ) {
       if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG) << "No LVL1_ROI for Trigger-MissingET found in SG, setting placeholders." << endmsg;
       // placeholders:
        energySumEtVec.push_back(DataType( -1. ) ); // means n/a
        energyExVec.push_back(DataType( -1. ) );
        energyEyVec.push_back(DataType( -1. ) );
        energyEtMissVec.push_back(DataType( -1. ) );
    }else{
      if ( (roi->getEnergySumROIs()).size() != 0 ){ // catch empty container
        if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG) << "TriggerInfo: LVL1_ROI for EnergySum retrieved" << endmsg;

        LVL1_ROI::energysums_type::const_iterator itES  = (roi->getEnergySumROIs()).begin();
        LVL1_ROI::energysums_type::const_iterator itESe = (roi->getEnergySumROIs()).end();
        for (; itES != itESe; ++itES){
	  float Et = itES->getEnergyT()/CLHEP::GeV;
	  float Ex = itES->getEnergyX()/CLHEP::GeV;
	  float Ey = itES->getEnergyY()/CLHEP::GeV;
	  energySumEtVec.push_back(DataType( Et ) );
	  energyExVec.push_back(DataType( Ex ) );
	  energyEyVec.push_back(DataType( Ey ) );
	  float EtMiss = static_cast<long>(sqrt(static_cast<double>(Ex*Ex + Ey*Ey)));
	  energyEtMissVec.push_back(DataType( EtMiss ) );

	  if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG) << "EnergySum from LVL1_ROI: X:" << Ex << ", Y:"
	      << Ey << ", T:" << Et << ", ETMiss:" << EtMiss << endmsg;
        }
      }else{ // empty container
        if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG) << "TriggerInfo: LVL1_ROI for EnergySum empty !" << endmsg;

	  energySumEtVec.push_back(DataType( "-1" ) ); // means n/a
	  energyExVec.push_back(DataType( "-1" ) );
	  energyEyVec.push_back(DataType( "-1" ) );
	  energyEtMissVec.push_back(DataType( "-1" ) );
      }	  
    } // LVL1_ROI available ?

    DataMap myDataMap;
    const int nEntries = trigInfoStatusVec.size();
    myDataMap["energySumEt"] = std::move(energySumEtVec);
    myDataMap["energyEx"] = std::move(energyExVec);
    myDataMap["energyEy"] = std::move(energyEyVec);
    myDataMap["energyEtMiss"] = std::move(energyEtMissVec);
    myDataMap["trigInfoStatus"] = std::move(trigInfoStatusVec);
    myDataMap["trigInfoExtL1ID"] = std::move(trigInfoExtL1IDVec);
    myDataMap["trigInfoLvl1Type"] = std::move(trigInfoLvl1TypeVec);
    myDataMap["trigInfoL1"] = std::move(trigInfoL1Vec);
    myDataMap["trigInfoL2"] = std::move(trigInfoL2Vec);
    myDataMap["trigInfoEF"] = std::move(trigInfoEFVec);
    myDataMap["trigInfoStreamTag"] = std::move(trigInfoStreamTagVec);

    if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG) << dataTypeName() << ": "<< nEntries << endmsg;

    //forward data to formating tool
    std::string emptyStr=""; // eventInfo has no SGKey
    return FormatTool->AddToEvent(dataTypeName(), emptyStr, &myDataMap);
  }
}
