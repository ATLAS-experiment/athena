/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigJiveXML/LVL1ResultRetriever.h"
#include "TrigSteeringEvent/Chain.h"


/// Migrated to new TrigDecisionTool described here
///   https://twiki.cern.ch/twiki/bin/view/Atlas/TrigDecisionTool15
namespace JiveXML {


  /**
   * Gaudi default constructor
   */
  LVL1ResultRetriever::LVL1ResultRetriever(const std::string& type, const std::string& name, const IInterface* parent):
    AthAlgTool(type, name, parent)
  {
    declareInterface<IDataRetriever>(this);
  }

  /**
   * Initialize the tool
   * - get handle to TrigDecisionTool
   * - define chain groups
   **/
  StatusCode LVL1ResultRetriever::initialize() {

    //be verbose
    ATH_MSG_VERBOSE("initialize()");

    //Try to retrieve the trig decision tool
    if ( !m_trigDecTool.retrieve() ) {
      ATH_MSG_FATAL("Could not retrieve TrigDecisionTool!");
      return StatusCode::FAILURE;
    }

    // We define the chain groups relying on the name convention (i.e. all L1
    // items start their name from "L1_", etc) In principle we would not have to do
    // so as the TrigDecisionTool jobOptions defines these as public chain
    // groups. This way, we are independant of jobOptions
    m_all   = m_trigDecTool->getChainGroup(".*");
    m_allL1 = m_trigDecTool->getChainGroup("L1_.*");
    m_allL2 = m_trigDecTool->getChainGroup("L2_.*");
    m_allEF = m_trigDecTool->getChainGroup("EF_.*");
    m_allHLT = m_trigDecTool->getChainGroup("HLT_.*");

    return StatusCode::SUCCESS;
  }

  /**
   * Get a long strong with all the item lists and prescales that are passed by
   * the given chain group
   **/
  StatusCode LVL1ResultRetriever::getItemLists(const Trig::ChainGroup* chains,
      std::string& itemList, std::string& prescaleList) {

    std::string sig_name;

    //Get a list of L1 items
    std::vector<std::string> chainList = chains->getListOfTriggers();
    ATH_MSG_DEBUG("Number of items in chain is " << chainList.size());

    for (auto &trigName : chainList){
      // Make sure the item is not empty
      // (can this actually happen ?!?
      if ( trigName.empty() ) continue;

      auto trigChain = m_trigDecTool->getChainGroup(trigName);
      // getPrescale() returns 0.0 for the first event: trigger decision tool needs
      // to internally cache the values prior to the event, but running from RAW the
      // conditions algorithm only runs during the event. Hence the "live" loading of
      // prescales are 1 event late - No trigger decisions for the first event
      if ( std::abs(trigChain->getPrescale()-1.0) > 1e-5 ) continue;

      //Output debug info
      std::string myItem = trigName;
      ATH_MSG_VERBOSE("  * item : name=" << myItem
                      << "; result = " << (trigChain->isPassed() ? "passed" : "failed")
                      << "; prescale = " <<  trigChain->getPrescale());

      // replace HLT with EF (as AtlantisJava doesn't know 'HLT'):
      if ( myItem.find("HLT",0) != std::string::npos){
        myItem.replace(0,4,"EF_");
        ATH_MSG_VERBOSE(trigName << " renamed into: " << myItem);
      }

      // prescale: see TWiki page TrigDecisionTool15

      //Only add passed items
      if ( trigChain->isPassed() ) {

        //Add item to list
        itemList += "-" + myItem;

        // prescale factor
        prescaleList += "-" + DataType(  trigChain->getPrescale() ).toString();
      }
    }


    //Mark empty item lists
    if ( itemList.empty() ){ itemList = "empty"; }
    if ( prescaleList.empty() ){ prescaleList = "empty"; }

    //print debug information
    ATH_MSG_DEBUG(" itemList: " << itemList);
    ATH_MSG_DEBUG(" prescaleList: " << prescaleList);

    return StatusCode::SUCCESS;
  }

  /**
   * Retrieve all trigger info
   * - item lists for L1, L2 and EF, data15: HLT instead of L2,EF
   * - prescale lists for L1, L2 and EF, data15: HLT instead of L2,EF
   * - passed flags for L1, L2 and EF, data15: HLT instead of L2,EF
   **/
  StatusCode LVL1ResultRetriever::retrieve(ToolHandle<IFormatTool> &FormatTool) {

    //be verbose
    ATH_MSG_VERBOSE("retrieve()");

    //Get the item and prescale lists for all levels
    std::string itemListL1="";
    std::string prescaleListL1="";
    std::string itemListL2="";
    std::string prescaleListL2="";
    std::string itemListEF="";
    std::string prescaleListEF="";
    std::string itemListHLT="";
    std::string prescaleListHLT="";

    //Get L1
    getItemLists( m_allL1, itemListL1, prescaleListL1 ).ignore();
    //Summarize L1 result
    int flagL1Passed = m_allL1->isPassed();
    ATH_MSG_DEBUG("Decision : Level-1 " << ((flagL1Passed)? "passed":"failed"));

    //Get L2
    getItemLists( m_allL2, itemListL2, prescaleListL2 ).ignore();
    //Summarize L2 result
    int flagL2Passed = m_allL2->isPassed();
    ATH_MSG_DEBUG("Decision : Level-2 " << ((flagL2Passed)? "passed":"failed"));

    //Get EF
    getItemLists( m_allEF, itemListEF, prescaleListEF ).ignore();
    //Summarize EF result
    int flagEFPassed = m_allEF->isPassed();
    ATH_MSG_DEBUG("Decision : EventFilter " << ((flagEFPassed)? "passed":"failed"));

    //Get HLT
    getItemLists( m_allHLT, itemListHLT, prescaleListHLT ).ignore();
    //Summarize HLT result
    int flagHLTPassed = m_allHLT->isPassed();
    ATH_MSG_DEBUG("Decision : HLT " << ((flagHLTPassed)? "passed":"failed"));

    //Do not write trigger info if we failed to obtain any of it
    if ((itemListL1=="empty") && (itemListL2=="empty") && (itemListEF=="empty") && (itemListHLT=="empty") ){
      ATH_MSG_INFO("All item lists empty, will not write out any data");
      return StatusCode::SUCCESS;
    }
    
    //Store results in data list
    DataVect itemListL1Vec;        itemListL1Vec.push_back( DataType( itemListL1 ));
    DataVect prescaleListL1Vec;    prescaleListL1Vec.push_back( DataType( prescaleListL1 ));
    DataVect itemListL2Vec;        itemListL2Vec.push_back( DataType( itemListL2 ));
    DataVect prescaleListL2Vec;    prescaleListL2Vec.push_back( DataType( prescaleListL2 ));
    DataVect itemListEFVec;        itemListEFVec.push_back( DataType( itemListEF ));
    DataVect prescaleListEFVec;    prescaleListEFVec.push_back( DataType( prescaleListEF ));
    DataVect itemListHLTVec;       itemListHLTVec.push_back( DataType( itemListHLT ));
    DataVect prescaleListHLTVec;   prescaleListHLTVec.push_back( DataType( prescaleListHLT ));
    DataVect passedTrigger;        passedTrigger.push_back(DataType( flagHLTPassed )); //this is just a duplicate
    DataVect passedL1;             passedL1.push_back(DataType( flagL1Passed ));
    DataVect passedL2;             passedL2.push_back(DataType( flagL2Passed ));
    DataVect passedEF;             passedEF.push_back(DataType( flagHLTPassed )); // temporary.
    DataVect passedHLT;            passedHLT.push_back(DataType( flagHLTPassed ));
    // placeholders only ! For backwards compatibility.
    // Trigger energies moved to TriggerInfoRetriever
    DataVect energySumEt; energySumEt.push_back(DataType( -1. ) );
    DataVect energyEx; energyEx.push_back(DataType( -1. ) );
    DataVect energyEy; energyEy.push_back(DataType( -1. ) );
    DataVect energyEtMiss; energyEtMiss.push_back(DataType( -1. ) );

    //Finally create data map and write out results
    DataMap dataMap;
    const std::size_t n = itemListL1Vec.size();
    dataMap["ctpItemList"] = std::move(itemListL1Vec);
    dataMap["prescaleListL1"] = std::move(prescaleListL1Vec);
    dataMap["itemListL2"] = std::move(itemListL2Vec);
    dataMap["prescaleListL2"] = std::move(prescaleListL2Vec);
    dataMap["itemListEF"] = std::move(itemListHLTVec); // temporary. AtlantisJava doesn't know 'HLT' yet. jpt 23Jun15
    dataMap["prescaleListEF"] = std::move(prescaleListHLTVec);
    dataMap["passedTrigger"] = std::move(passedTrigger);
    dataMap["passedL1"] = std::move(passedL1);
    dataMap["passedL2"] = passedHLT; // temporary. AtlantisJava doesn't know 'HLT' yet. jpt 23Jun15
    dataMap["passedEF"] = std::move(passedHLT); // temporary. AtlantisJava doesn't know 'HLT' yet. jpt 23Jun15
    dataMap["energySumEt"] = std::move(energySumEt);
    dataMap["energyEx"] = std::move(energyEx);
    dataMap["energyEy"] = std::move(energyEy);
    dataMap["energyEtMiss"] = std::move(energyEtMiss);

    ATH_MSG_DEBUG(dataTypeName() << ": "<< n);
    //forward data to formating tool
    return FormatTool->AddToEvent(dataTypeName(), "TrigDecision", &dataMap);
  }
}
