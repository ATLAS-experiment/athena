/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MdtCondJsonDumpAlg.h"

#include "StoreGate/ReadCondHandleKey.h"
#include <fstream>



MdtCondJsonDumpAlg::MdtCondJsonDumpAlg(const std::string& name, ISvcLocator* pSvcLocator):
    AthAlgorithm{name, pSvcLocator} {}

StatusCode MdtCondJsonDumpAlg::initialize() {
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_readKey.initialize());
    return StatusCode::SUCCESS;
}
StatusCode MdtCondJsonDumpAlg::execute() {
  const EventContext& ctx{Gaudi::Hive::currentContext()};
  SG::ReadCondHandle<MdtCondDbData> readCondHandle{m_readKey, ctx};
  if (!readCondHandle.isValid()) {
      ATH_MSG_FATAL("Failed to open chamber load conditions "<<m_readKey.fullKey());
      return StatusCode::FAILURE;
  }
  /// Dump the dead channels
  {
     std::ofstream deadChannels{m_deadChannelJSON};
     if (!deadChannels.good()) {
       ATH_MSG_ERROR("Failed to write "<<m_deadChannelJSON);
       return StatusCode::FAILURE;
     }
     deadChannels<<"{"<<std::endl;
     deadChannels<<" \"Chambers\": ";
     dumpDeadChannels(readCondHandle->getDeadChambersId(), deadChannels);
     deadChannels<<","<<std::endl;
     deadChannels<<" \"MultiLayers\": ";
     dumpDeadChannels(readCondHandle->getDeadMultilayersId(), deadChannels, true);
     deadChannels<<","<<std::endl;
     deadChannels<<" \"TubeLayers:\": ";
     dumpDeadChannels(readCondHandle->getDeadLayersId(), deadChannels, true, true);
     deadChannels<<","<<std::endl;
     deadChannels<<" \"Tubes:\": ";
     dumpDeadChannels(readCondHandle->getDeadTubesId(), deadChannels, true, true, true);
     deadChannels<<std::endl<<"}"<<std::endl;
  }
  /// No HV data given
  if (readCondHandle->getAllHvStates().empty() || m_dcsJSON.value().empty()){
      return StatusCode::SUCCESS;
  }
  std::ofstream dcsStates{m_dcsJSON};
  if (!dcsStates.good()) {
    ATH_MSG_ERROR("Failed to write "<<m_dcsJSON);
    return StatusCode::FAILURE;
  }


  dcsStates<<"["<<std::endl;


  const MdtIdHelper& idHelper{m_idHelperSvc->mdtIdHelper()};
  const MuonIdHelper::const_id_iterator begin = idHelper.detectorElement_begin();
  const MuonIdHelper::const_id_iterator end = idHelper.detectorElement_end();
  
  for (MuonIdHelper::const_id_iterator itr = begin; itr != end; ++itr) {
      const Identifier& detElId{*itr};
      const MuonCond::DcsConstants& dcs{readCondHandle->getHvState(detElId)};
      dcsStates<<"    {"<<std::endl;
      dumpIdentifier(detElId, dcsStates, true, false, false, true);
      dcsStates<<"     \"state\": \""<<MuonCond::getFsmStateStrg(dcs.fsmState)<<"\","<<std::endl;
      dcsStates<<"     \"standByVolt\": "<<dcs.standbyVolt<<", "<<std::endl;
      dcsStates<<"     \"readyVolt\": "<<dcs.readyVolt<<std::endl;      
      dcsStates<<"    }";
      if ((itr +1 ) != end) dcsStates<<",";
      dcsStates<<std::endl;
  }
  dcsStates<<"]"<<std::endl;


  return StatusCode::SUCCESS;
}

void MdtCondJsonDumpAlg::dumpIdentifier(const Identifier& id, 
                                        std::ostream& ostr, 
                                        bool dumpMultiLayer, 
                                        bool dumpLayer, 
                                        bool dumpTube,
                                        bool trailingComma) const {
    ostr<<"     \"station\": \""<<m_idHelperSvc->stationNameString(id)<<"\","<<std::endl;
    ostr<<"     \"eta\": "<<m_idHelperSvc->stationEta(id)<<","<<std::endl;
    ostr<<"     \"phi\": "<<m_idHelperSvc->stationPhi(id);
    if (!dumpMultiLayer) {
      ostr<<(trailingComma ? "," : "")<<std::endl;
      return;
    }
    ostr<<","<<std::endl;
    const MdtIdHelper& idHelper{m_idHelperSvc->mdtIdHelper()};
    ostr<<"     \"ml\": "<<idHelper.multilayer(id);
    if (!dumpLayer) {
       ostr<<(trailingComma ? "," : "")<<std::endl;
       return;
    }
    ostr<<","<<std::endl;    
    ostr<<"     \"layer\": "<<idHelper.tubeLayer(id);
    if (!dumpTube) {
      ostr<<(trailingComma ? "," : "")<<std::endl;
      return;
    }
    ostr<<"     \"tube:\": \""<<idHelper.tube(id);
    ostr<<(trailingComma ? "," : "")<<std::endl;
}
void MdtCondJsonDumpAlg::dumpDeadChannels(const std::set<Identifier>& channels, 
                                          std::ostream& ostr, 
                                          bool dumpMultiLayer,
                                          bool dumpLayer, bool dumpTube) const {
   unsigned int processed{0};
   ostr<<"["<<std::endl;
   for (const Identifier& id : channels) {
      ostr<<"    {"<<std::endl;
      dumpIdentifier(id, ostr, dumpMultiLayer, dumpLayer, dumpTube, false);
      ostr<<"    }";
      ++processed;
      if(processed != channels.size()) ostr<<",";
      ostr<<std::endl;
   }
   ostr<<"]";
}