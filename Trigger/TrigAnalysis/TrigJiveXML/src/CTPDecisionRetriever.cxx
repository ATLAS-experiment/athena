/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigJiveXML/CTPDecisionRetriever.h"
#include "CLHEP/Units/SystemOfUnits.h"
#include "AnalysisTriggerEvent/CTP_Decision.h"

namespace JiveXML {

  //--------------------------------------------------------------------------

  CTPDecisionRetriever::CTPDecisionRetriever(const std::string& type, const std::string& name, const IInterface* parent):
    AthAlgTool(type, name, parent),
    m_typeName("LVL1Result")
  {
    declareInterface<IDataRetriever>(this);
  }

  //--------------------------------------------------------------------------

  StatusCode CTPDecisionRetriever::retrieve(ToolHandle<IFormatTool> &FormatTool) {

    DataVect itemListL1Vec;
    DataVect prescaleListL1Vec;
    DataVect itemListL2Vec;
    DataVect prescaleListL2Vec;
    DataVect itemListEFVec;
    DataVect prescaleListEFVec;
    DataVect passedTrigger;
    DataVect passedL1;
    DataVect passedL2;
    DataVect passedEF;

// placeholders only ! For backwards compatibility.
// Trigger energies moved to TriggerInfoRetriever
    DataVect energySumEt;
    DataVect energyEx;
    DataVect energyEy;
    DataVect energyEtMiss;

    energySumEt.push_back(DataType( -1. ) );
    energyEx.push_back(DataType( -1. ) );
    energyEy.push_back(DataType( -1. ) );
    energyEtMiss.push_back(DataType( -1. ) );

// CTP_Decision has no 'passed' info
    passedTrigger.push_back(DataType( -1. ) );
    passedL1.push_back(DataType( -1. ) );
    passedL2.push_back(DataType( -1. ) );
    passedEF.push_back(DataType( -1. ) );

// end of placeholders

    std::string itemListL1="";
    std::string prescaleListL1="";
    std::string itemListL2="n_a_CTPDecOnly";
    std::string prescaleListL2="0";
    std::string itemListEF="n_a_CTPDecOnly";
    std::string prescaleListEF="0";

//// according to Twiki page LevelOneCentralTriggerData (David Berge)
//// comments taken from there

     const CTP_Decision * ctpDecision;

     // L1JetObject -not- available
     if ( evtStore()->retrieve(ctpDecision,"CTP_Decision").isFailure() ) {
       if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG) <<  "CTP_Decision retrieval from Storegate failed" << endmsg;
       return StatusCode::SUCCESS;
     } 
     if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG) << "Found CTP_Decision in StoreGate !" << endmsg;

     CTP_Decision::items_type::const_iterator itCTP  = (ctpDecision->getItems()).begin();
     CTP_Decision::items_type::const_iterator itCTPe = (ctpDecision->getItems()).end();

     for (; itCTP != itCTPe; ++itCTP){
       itemListL1 += "-"+*itCTP;
       prescaleListL1 += "-0";
     }

    itemListL1Vec.emplace_back( std::move( itemListL1 )); 
    itemListL2Vec.emplace_back( std::move( itemListL2 ));
    itemListEFVec.emplace_back( std::move( itemListEF ));
    prescaleListL1Vec.emplace_back( std::move( prescaleListL1 ));
    prescaleListL2Vec.emplace_back( std::move( prescaleListL2 ));
    prescaleListEFVec.emplace_back( std::move( prescaleListEF ));

    DataMap myDataMap;
    const int nEntries = itemListL1Vec.size();
    myDataMap["ctpItemList"] = std::move(itemListL1Vec);
    myDataMap["prescaleListL1"] = std::move(prescaleListL1Vec);
    myDataMap["itemListL2"] = std::move(itemListL2Vec);
    myDataMap["prescaleListL2"] = std::move(prescaleListL2Vec);
    myDataMap["itemListEF"] = std::move(itemListEFVec);
    myDataMap["prescaleListEF"] = std::move(prescaleListEFVec);
    myDataMap["passedTrigger"] = std::move(passedTrigger);
    myDataMap["passedL1"] = std::move(passedL1);
    myDataMap["passedL2"] = std::move(passedL2);
    myDataMap["passedEF"] = std::move(passedEF);
    myDataMap["energySumEt"] = std::move(energySumEt);
    myDataMap["energyEx"] = std::move(energyEx);
    myDataMap["energyEy"] = std::move(energyEy);
    myDataMap["energyEtMiss"] = std::move(energyEtMiss);
    
    if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG) << dataTypeName() << ": "<< nEntries << endmsg;

    //forward data to formating tool
    return FormatTool->AddToEvent(dataTypeName(), "CTP_Decision", &myDataMap);
  }
}
