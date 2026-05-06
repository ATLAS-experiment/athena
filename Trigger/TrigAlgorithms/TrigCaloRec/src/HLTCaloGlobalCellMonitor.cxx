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
  for(size_t i=0;i<25; i++) {
    std::string number("layer");
    number+=std::to_string(i);
    m_layerNames.push_back(number);
  }
  for(size_t i=0;i<2; i++) {
    std::string number("layerIW");
    number+=std::to_string(i+1);
    m_layerNamesIW.push_back(number);
  }

  return StatusCode::SUCCESS;
}

StatusCode HLTCaloGlobalCellMonitor::execute(EventContext const& context) const {
  ATH_MSG_DEBUG("HLTCaloGlobalCellMonitor::execute()");

  SG::ReadHandle<CaloConstCellContainer> inputCellHandle(m_inputCellContainerKey, context);

  SG::ReadCondHandle<CaloNoise> noiseHdl{m_noiseCDOKey,context};
  const CaloNoise* noiseCDO=*noiseHdl;
  SG::ReadCondHandle<LArOnOffIdMapping> onoff (m_onOffIdMappingKey, context);

  const auto bcid = context.eventID().bunch_crossing_id();

  uint32_t n_febs = m_onlineId->febHashMax();
  std::map<HWIdentifier,uint32_t> cells_per_feb;
  for(uint32_t i=0;i<n_febs;i++){
         HWIdentifier feb_id = m_onlineId->feb_Id(IdentifierHash(i));
         cells_per_feb[feb_id]=0;
  }

  auto mon_bcid = Monitored::Scalar<int>("BCID",0);
  auto mon_inputSize = Monitored::Scalar<int>("inputContSize", 0);
  auto mon_outputSize = Monitored::Scalar<int>("outputContSize", 0);
  auto mon_larSize = Monitored::Scalar<int>("larContSize", 0);
  auto mon_larAboveSigmaSize = Monitored::Scalar<int>("larAboveSigmaContSize", 0);
  
  
  std::vector<uint32_t> febID;
  febID.reserve(2000);
  std::vector<uint32_t> cellCount;
  cellCount.reserve(2000);
  std::vector<uint32_t> cellCountPerLayer[25];
  for(size_t i=0;i<25;i++)cellCountPerLayer[i].reserve(200);
  std::vector<uint32_t> cellCountPerLayerIW[2];
  for(size_t i=0;i<2;i++)cellCountPerLayerIW[i].reserve(200);
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
  for(uint32_t i=0;i<n_febs;i++){
         HWIdentifier feb_id = m_onlineId->feb_Id(IdentifierHash(i));
         febID.push_back( feb_id.get_identifier32().get_compact() );
         HWIdentifier hw = m_onlineId->channel_Id(feb_id,0); // first channel layer
	 Identifier id = (*onoff)->cnvToIdentifier(hw);
         cellCount.push_back(cells_per_feb[feb_id]);
         int calosample = m_caloCell_ID->calo_sample(id);
         if (calosample < 5 ) {
             cellCountPerLayer[calosample].push_back(cells_per_feb[feb_id]);
         } else {
           if ( !m_onlineId->isEMECIW(hw) ){
             if ( calosample<25)
                cellCountPerLayer[calosample].push_back(cells_per_feb[feb_id]);
           }else{
             if ( calosample<8)
               cellCountPerLayerIW[calosample-6].push_back(cells_per_feb[feb_id]);
	   }
         }
  }

  mon_bcid = bcid;
  mon_inputSize = (*inputCellHandle).size();
  mon_outputSize = outputSize;
  mon_larSize = larSize;
  mon_larAboveSigmaSize = larAboveSizeSize;
  auto mon_febID = Monitored::Collection("FEBID",febID);
  auto mon_cells = Monitored::Collection("CellsPerFEB",cellCount);
  auto mon0 = Monitored::Collection(m_layerNames[0],cellCountPerLayer[0]);
  auto mon1 = Monitored::Collection(m_layerNames[1],cellCountPerLayer[1]);
  auto mon2 = Monitored::Collection(m_layerNames[2],cellCountPerLayer[2]);
  auto mon3 = Monitored::Collection(m_layerNames[3],cellCountPerLayer[3]);
  auto mon4 = Monitored::Collection(m_layerNames[4],cellCountPerLayer[4]);
  auto mon5 = Monitored::Collection(m_layerNames[5],cellCountPerLayer[5]);
  auto mon6 = Monitored::Collection(m_layerNames[6],cellCountPerLayer[6]);
  auto mon7 = Monitored::Collection(m_layerNames[7],cellCountPerLayer[7]);
  auto mon8 = Monitored::Collection(m_layerNamesIW[0],cellCountPerLayerIW[0]);
  auto mon9 = Monitored::Collection(m_layerNamesIW[1],cellCountPerLayerIW[1]);
  std::vector<std::reference_wrapper<Monitored::IMonitoredVariable>> variables;
  variables.reserve(50);
  variables.push_back(std::ref(mon_bcid));
  variables.push_back(std::ref(mon_inputSize));
  variables.push_back(std::ref(mon_outputSize));
  variables.push_back(std::ref(mon_larSize));
  variables.push_back(std::ref(mon_larAboveSigmaSize) );
  variables.push_back(std::ref(mon_cells) );
  variables.push_back(std::ref(mon_febID) );
  variables.push_back(std::ref(mon0));
  variables.push_back(std::ref(mon1));
  variables.push_back(std::ref(mon2));
  variables.push_back(std::ref(mon3));
  variables.push_back(std::ref(mon4));
  variables.push_back(std::ref(mon5));
  variables.push_back(std::ref(mon6));
  variables.push_back(std::ref(mon7));
  variables.push_back(std::ref(mon8));
  variables.push_back(std::ref(mon9));
  auto monitorIt = Monitored::Group( m_moniTool, variables);
  variables.clear();

  return StatusCode::SUCCESS;
}
