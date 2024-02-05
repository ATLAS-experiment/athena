/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "MMCablingTestAlg.h"
#include "StoreGate/ReadCondHandle.h"
#include "MuonReadoutGeometry/MdtReadoutElement.h"
#include "MuonCablingData/MdtMezzanineCard.h"

#include <fstream>

MMCablingTestAlg::MMCablingTestAlg(const std::string& name, ISvcLocator* pSvcLocator):
    AthAlgorithm(name,pSvcLocator) {}


StatusCode MMCablingTestAlg::initialize(){
  ATH_CHECK(m_idHelperSvc.retrieve());
  ATH_CHECK(m_DetectorManagerKey.initialize());
  ATH_CHECK(m_cablingKey.initialize());  
  return StatusCode::SUCCESS;
} 

StatusCode MMCablingTestAlg::execute(){
  const EventContext& ctx = Gaudi::Hive::currentContext();
  std::unique_ptr<std::fstream> f_dump = !m_dumpFile.value().empty() ? 
                                         std::make_unique<std::fstream>(m_dumpFile, std::fstream::out) : nullptr;
  ATH_MSG_INFO("Start validation of the MM cabling. Dump complete mapping into "<<m_dumpFile);
  
  SG::ReadCondHandle<MuonGM::MuonDetectorManager> detectorMgr{m_DetectorManagerKey, ctx};
  if (!detectorMgr.isValid()){
      ATH_MSG_FATAL("Failed to retrieve the Detector manager "<<m_DetectorManagerKey.fullKey());
      return StatusCode::FAILURE;
  }

  SG::ReadCondHandle<Nsw_CablingMap> cabling{m_cablingKey,ctx};
  if (!cabling.isValid()) {
     ATH_MSG_ERROR("Failed to retrieve the Mdt cabling "<<m_cablingKey.fullKey());
     return StatusCode::FAILURE;
  }
  Identifier chId1 = m_idHelperSvc->mmIdHelper().channelID("MML", 1, 1, 1, 1, 1);
  cabling->correctChannel(chId1, msgStream());

  Identifier chId2 = m_idHelperSvc->mmIdHelper().channelID("MMS", -2, 3, 1, 3, 1280);
  cabling->correctChannel(chId2, msgStream());
  
  return StatusCode::SUCCESS;  
} 
   
