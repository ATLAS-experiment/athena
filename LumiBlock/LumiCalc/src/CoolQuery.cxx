/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include <limits.h>
#include "LumiCalc/CoolQuery.h"

namespace{
  const std::string chainNameStr{"ChainName"};
  const std::string chainCounterStr{"ChainCounter"};
  const std::string itemNameStr{"ItemName"};
  const std::string prescaleStr{"Prescale"};
  const std::string lvl1PrescaleStr{"Lvl1Prescale"};
  const std::string lowerChainNameStr{"LowerChainName"};
  const std::string validStr{"Valid"};
  const std::string beforePrescaleStr{"BeforePrescale"};
  const std::string afterPrescaleStr{"AfterPrescale"};
  const std::string l1AcceptStr{"L1Accept"};
  const std::string lbAvInstLumiStr{"LBAvInstLumi"};
  const std::string lbAvEvtsPerBXStr{"LBAvEvtsPerBX"};
}

//===========================================================================
CoolQuery::CoolQuery(const std::string& database, const std::string& triggerchain): 
  m_repsort(0),
  m_database(database), 
  m_triggerchain(triggerchain),
  m_VKstart(0),
  m_VKstop(0),
  m_logger( "CoolQuery" ),
  m_valid(false)
{

}

//===========================================================================
CoolQuery::~CoolQuery() {
  if ( m_sourceDbPtr.use_count()>0 && m_sourceDbPtr->isOpen() ) {
     m_logger << Root::kINFO << "Closing database '" << m_sourceDbPtr->databaseName() << Root::GEndl;
     m_sourceDbPtr->closeDatabase();
  }
}

//===========================================================================
bool CoolQuery::openDbConn() {
  m_logger << Root::kINFO << "Trying to connect to database " << m_database << "..." << Root::GEndl;
  cool::IDatabaseSvc& databasesvc = cool::DatabaseSvcFactory::databaseService();
  try {
    m_repsort=new ReplicaSorter();
    coral::IConnectionServiceConfiguration& csconfig=m_coralsvc.configuration();
    csconfig.setReplicaSortingAlgorithm(*m_repsort);

    m_sourceDbPtr = databasesvc.openDatabase(m_database,true);// true --> readonly
    //    m_sourceCoraPtr=corasvc.openDatabase(m_database,true);// true --> readonly
    //std::cout << "....database connections open OK" << std::endl;
    return true;
   }
  catch (std::exception&e) {
    m_logger << Root::kERROR << "Problem opening CORAL database: " << e.what() << Root::GEndl;
    return false;
  }
  return false;
}

//===========================================================================
std::string CoolQuery::transConn(const std::string& inconn) {
  // translate simple connection string (no slash) to mycool.db with given                                                                                                              
  // instance name, all others are left alone                                                                                                                                           
  if (inconn.find('/')==std::string::npos) {
    return "sqlite://X;schema=mycool.db;dbname="+inconn;
  } else {
    return inconn;
  }
}

//===========================================================================
unsigned int CoolQuery::getTriggerLevel(const std::string& triggername){

  size_t found = triggername.find_first_of('_');
  if(found != std::string::npos){
    std::string s_lvl = triggername.substr(0,found);
    if(s_lvl == "EF") return 3;
    if(s_lvl == "L2") return 2;
    if(s_lvl == "L1") return 1;
    if(s_lvl == "HLT") return 2;
  }

  // Indicate no valid trigger name passed
  return 0;
    
}

//===========================================================================
void CoolQuery::setIOV(const cool::ValidityKey start, const cool::ValidityKey stop){
  m_VKstart = start;
  m_VKstop = stop;
}
  
void CoolQuery::setIOVForRun(unsigned int runnum) {
  cool::ValidityKey run = runnum;
  m_VKstart = (run << 32);
  m_VKstop = ((run+1) << 32) - 1;
}

cool::Int32 CoolQuery::getL1PrescaleFromChannelId(const std::string& folder_name, const cool::ChannelId& id ){

  cool::IFolderPtr folder_ptr = m_sourceDbPtr->getFolder(folder_name);
  cool::IObjectIteratorPtr itr = folder_ptr->browseObjects(m_VKstart, m_VKstop,id);

  // Need to iterate once to get to first valid record, do it this way to avoid Coverity warning
  if (itr->goToNext()) {
    const cool::IRecord& payload=itr->currentRef().payload();    
    return payload[lvl1PrescaleStr].data<cool::Int32>();
  }

  // Nonsense value
  return UINT_MAX;

}

cool::Float CoolQuery::getHLTPrescaleFromChannelId(const std::string& folder_name, const cool::ChannelId& id ){

  cool::IFolderPtr folder_ptr = m_sourceDbPtr->getFolder(folder_name);
  cool::IObjectIteratorPtr itr = folder_ptr->browseObjects(m_VKstart, m_VKstop,id);

  // Need to iterate once to get to first valid record, do it this way to avoid Coverity warning
  if (itr->goToNext()) {
    const cool::IRecord& payload=itr->currentRef().payload();    
    return payload[prescaleStr].data<cool::Float>();
  }

  // Nonsense value
  return -1.;

}

cool::ChannelId CoolQuery::getL1ChannelId(const std::string& trigger, const std::string& folder_name){
  m_valid = false;

  if (trigger == "") return UINT_MAX;
 
  cool::IFolderPtr folder_ptr = m_sourceDbPtr->getFolder(folder_name);
  cool::IObjectIteratorPtr obj_itr=folder_ptr->browseObjects(m_VKstart,m_VKstart, cool::ChannelSelection::all());
  // loop through all triggers
  while (obj_itr->goToNext()){
    const cool::IRecord& payload=obj_itr->currentRef().payload();
    // find the L1 trigger chain
    if(payload[itemNameStr].data<std::string>() == trigger){
      m_valid = true;
      return obj_itr->currentRef().channelId();
    }
  }
  if(!m_valid){
    m_logger << Root::kERROR << "Couldn't find L1 trigger [" << trigger << "] in folder [" << folder_name << "]" << Root::GEndl;
  }
  // Nonsense value
  return UINT_MAX;
}

//===========================================================================
cool::ChannelId CoolQuery::getLumiChannelId(const std::string& lumimethod, const std::string& folder_name){
  m_valid = false;
  if (lumimethod == "") return UINT_MAX;
  cool::IFolderPtr folder_ptr = m_sourceDbPtr->getFolder(folder_name);
  if(folder_ptr->existsChannel(lumimethod)){
    m_valid = true;
    return folder_ptr->channelId(lumimethod);
  }else{
    m_logger << Root::kWARNING << "Couldn't find lumimethod: " << lumimethod << " in COOL database!" << Root::GEndl;
  }
  // Nonsense value
  return UINT_MAX;
}

//===========================================================================
cool::ChannelId CoolQuery::getHLTChannelId(const std::string& trigger, const std::string& folder_name){
  m_valid = false;
  if (trigger == "") return UINT_MAX;
  cool::IFolderPtr folder_ptr = m_sourceDbPtr->getFolder(folder_name);
  cool::IObjectIteratorPtr obj_itr=folder_ptr->browseObjects(m_VKstart,m_VKstart, cool::ChannelSelection::all());
  // loop through all triggers
  // loop through all triggers
  while (obj_itr->goToNext()){
    const cool::IRecord& payload=obj_itr->currentRef().payload();
    if(payload[chainNameStr].data<std::string>() == trigger){
      m_valid = true;
      return payload[chainCounterStr].data<cool::UInt32>();
    }
  }
  if(!m_valid){
    m_logger << Root::kERROR << "Couldn't find HLT trigger [" << trigger << "] in folder [" << folder_name << "]" << Root::GEndl;
  }
  // Nonsense value
  return UINT_MAX;
}

void
CoolQuery::printL1Triggers(const std::string& folder_name) {
  m_logger << Root::kINFO << "Listing available triggers [triggername(prescale, chanid)]: " << Root::GEndl;
  cool::IFolderPtr folder_ptr = m_sourceDbPtr->getFolder(folder_name);
  cool::IObjectIteratorPtr obj_itr=folder_ptr->browseObjects(m_VKstart,m_VKstart, cool::ChannelSelection::all());
  while (obj_itr->goToNext()){
    const cool::IRecord& payload=obj_itr->currentRef().payload();
    m_logger << Root::kINFO << payload[itemNameStr].data<std::string>()  << "(" << this->getL1PrescaleFromChannelId("/TRIGGER/LVL1/Prescales",this->getL1ChannelId(payload[itemNameStr].data<std::string>(), folder_name)) << ", " << obj_itr->currentRef().channelId() << "), ";
  }
  m_logger << Root::kINFO << Root::GEndl;
}

void
CoolQuery::printHLTTriggers(const std::string& folder_name) {

  m_logger << Root::kINFO << "Listing available triggers [triggername(prescale, chanid)]: " << Root::GEndl;

  cool::IFolderPtr folder_ptr = m_sourceDbPtr->getFolder(folder_name);
  cool::IObjectIteratorPtr obj_itr2=folder_ptr->browseObjects(m_VKstart,m_VKstart, cool::ChannelSelection::all());
  while (obj_itr2->goToNext()){
    const cool::IRecord& payload2=obj_itr2->currentRef().payload();
    m_logger << Root::kINFO << payload2[chainNameStr].data<std::string>()  << "(" << payload2[prescaleStr].data<cool::Float>() << ", " << payload2[chainCounterStr].data<cool::UInt32>() << "), ";
  }
  m_logger << Root::kINFO << Root::GEndl;
}

bool
CoolQuery::channelIdValid() {
  return m_valid;
}

//===========================================================================
std::string CoolQuery::getHLTLowerChainName(const std::string& trigger, const std::string& folder_name){
  bool found = false;
  cool::IFolderPtr folder_ptr = m_sourceDbPtr->getFolder(folder_name);
  cool::IObjectIteratorPtr obj_itr=folder_ptr->browseObjects(m_VKstart,m_VKstart, cool::ChannelSelection::all());
  // loop through all triggers
  while (obj_itr->goToNext()){
    const cool::IRecord& payload=obj_itr->currentRef().payload();
    if(payload[chainNameStr].data<std::string>() == trigger){
      found = true;
      return payload[lowerChainNameStr].data<std::string>();

    }
  }
  if (!found) {
    m_logger << Root::kERROR << "Couldn't find HLT trigger [" << trigger << "] in folder [" << folder_name << "]" << Root::GEndl;
  }

  return "";

}

//===========================================================================
std::map<cool::ValidityKey, CoolQuery::LumiFolderData> 
CoolQuery::getLumiFolderData(const std::string& folder_name, const std::string& tag, const cool::ChannelId& id ){

  std::map<cool::ValidityKey, LumiFolderData> mymap;
  LumiFolderData folderData;

  cool::IFolderPtr folder_ptr = m_sourceDbPtr->getFolder(folder_name);
  if (!folder_ptr->existsChannel(id)) {
    m_logger << Root::kWARNING << "Lumi channel id " << id << " does not exist in database " << folder_name << "!" << Root::GEndl;
    return mymap;
  }

  cool::IObjectIteratorPtr itr;
  if(folder_ptr->existsUserTag(tag)) {
    itr = folder_ptr->browseObjects(m_VKstart, m_VKstop, id, tag);
  } else {
    // Try without specifying tag
    itr = folder_ptr->browseObjects(m_VKstart, m_VKstop, id);
  }

  while (itr->goToNext()) {
    const cool::IRecord& payload=itr->currentRef().payload();
    folderData.LBAvInstLumi = payload[lbAvInstLumiStr].data<float>();
    folderData.LBAvEvtsPerBX = payload[lbAvEvtsPerBXStr].data<float>();
    folderData.Valid = payload[validStr].data<cool::UInt32>();
    mymap.insert( std::pair<cool::ValidityKey, LumiFolderData>(itr->currentRef().since(), folderData));
  }
  
  return mymap;

}

//===========================================================================
std::map<cool::ValidityKey, CoolQuery::L1CountFolderData> 
CoolQuery::getL1CountFolderData(const std::string& folder_name, const cool::ChannelId& id ){

  std::map<cool::ValidityKey, L1CountFolderData> mymap;
  L1CountFolderData folderData;

  cool::IFolderPtr folder_ptr = m_sourceDbPtr->getFolder(folder_name);
  if (!folder_ptr->existsChannel(id)) {
    m_logger << Root::kWARNING << "Lumi channel id " << id << " does not exist in database " << folder_name << "!" << Root::GEndl;
    return mymap;
  }

  cool::IObjectIteratorPtr itr;

  itr = folder_ptr->browseObjects(m_VKstart, m_VKstop, id);

  while (itr->goToNext()) {
    const cool::IRecord& payload=itr->currentRef().payload();
    folderData.BeforePrescale = payload[beforePrescaleStr].data<cool::UInt63>();
    folderData.AfterPrescale = payload[afterPrescaleStr].data<cool::UInt63>();
    folderData.L1Accept = payload[l1AcceptStr].data<cool::UInt63>();
    mymap.insert( std::pair<cool::ValidityKey, L1CountFolderData>(itr->currentRef().since(), folderData));
  }
  
  return mymap;

}



