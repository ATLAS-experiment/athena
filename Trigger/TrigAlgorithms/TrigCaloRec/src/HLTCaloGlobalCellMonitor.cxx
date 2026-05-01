/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "HLTCaloGlobalCellMonitor.h"

#include "LArCabling/LArOnOffIdMapping.h"
#include "CaloIdentifier/CaloCell_ID.h"
#include "CaloConditions/CaloNoise.h"
#include "LArIdentifier/LArOnlineID.h"
#include "AthenaMonitoringKernel/MonitoredCollection.h"
#include "AthenaMonitoringKernel/MonitoredGroup.h"
#include "AthenaMonitoringKernel/MonitoredScalar.h"
#include "AthenaMonitoringKernel/MonitoredTimer.h"
#include <memory>
#include <cmath>

HLTCaloGlobalCellMonitor::HLTCaloGlobalCellMonitor(std::string const& name, ISvcLocator* pSvcLocator)
  : AthReentrantAlgorithm(name, pSvcLocator) {
}

StatusCode HLTCaloGlobalCellMonitor::initialize() {

  ATH_CHECK( detStore()->retrieve(m_onlineId,"LArOnlineID") );
  ATH_CHECK( detStore()->retrieve(m_caloCell_ID) );
  ATH_CHECK( m_onOffIdMappingKey.initialize() );
  ATH_CHECK(m_noiseCDOKey.initialize());
  ATH_CHECK(m_inputCellContainerKey.initialize());
  if (!m_moniTool.empty()) {
     ATH_CHECK(m_moniTool.retrieve());
  }

  return StatusCode::SUCCESS;
}

StatusCode HLTCaloGlobalCellMonitor::execute(EventContext const& context) const {
  ATH_MSG_DEBUG("HLTCaloGlobalCellMonitor::execute()");

  SG::ReadHandle<CaloConstCellContainer> inputCellHandle(m_inputCellContainerKey, context);

  SG::ReadCondHandle<CaloNoise> noiseHdl{m_noiseCDOKey,context};
  const CaloNoise* noiseCDO=*noiseHdl;
  SG::ReadCondHandle<LArOnOffIdMapping> onoff (m_onOffIdMappingKey, context);

  uint32_t n_febs = m_onlineId->febHashMax();
  std::map<HWIdentifier,uint32_t> cells_per_feb;
  for(uint32_t i=0;i<n_febs;i++){
         HWIdentifier feb_id = m_onlineId->feb_Id(IdentifierHash(i));
         cells_per_feb[feb_id]=0;
  }

  auto mon_inputSize = Monitored::Scalar("inputContSize", 0.);
  auto mon_outputSize = Monitored::Scalar("outputContSize", 0.);
  auto mon_larSize = Monitored::Scalar("larContSize", 0.);
  auto mon_larAboveSigmaSize = Monitored::Scalar("larAboveSigmaContSize", 0.);
  int larSize=0;
  int larAboveSizeSize=0;
  int outputSize=0;

  for (auto const* cell : *inputCellHandle) {
    if (!cell->caloDDE()->is_tile() ){
       larSize++;
       Identifier cellID = cell->ID();
       float noiseSigma = noiseCDO->getNoise(cellID,cell->gain());   
       IdentifierHash offhashid = m_caloCell_ID->calo_cell_hash(cellID);
       HWIdentifier hwid = (*onoff)->createSignalChannelIDFromHash(offhashid);
       HWIdentifier hwid_feb = m_onlineId->feb_Id(hwid);
       uint32_t ncells_in_feb = cells_per_feb[hwid_feb];
       if (cell->energy() > m_NumberOfSigma*noiseSigma) {
         larAboveSizeSize++;
         if( ncells_in_feb < m_MaxNCellsPerFEB ) {
            cells_per_feb[hwid_feb]=ncells_in_feb+1;
            outputSize++;
         }
       }
    } else outputSize++;
  }

  mon_inputSize = (*inputCellHandle).size();
  mon_outputSize = outputSize;
  mon_larSize = larSize;
  mon_larAboveSigmaSize = larAboveSizeSize;
  auto monitorIt = Monitored::Group( m_moniTool, mon_inputSize, mon_outputSize, mon_larSize, mon_larAboveSigmaSize);

  return StatusCode::SUCCESS;
}
