/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "HLTCaloGlobalCellMaker.h"

#include "LArCabling/LArOnOffIdMapping.h"
#include "CaloIdentifier/CaloCell_ID.h"
#include "CaloConditions/CaloNoise.h"
#include "LArIdentifier/LArOnlineID.h"
#include <memory>
#include <cmath>
#include <map>
#include <algorithm> //for std::sort
HLTCaloGlobalCellMaker::HLTCaloGlobalCellMaker(std::string const& name, ISvcLocator* pSvcLocator)
  : AthReentrantAlgorithm(name, pSvcLocator) {
}

StatusCode HLTCaloGlobalCellMaker::initialize() {

  ATH_CHECK( detStore()->retrieve(m_onlineId,"LArOnlineID") );
  ATH_CHECK( detStore()->retrieve(m_caloCell_ID) );
  ATH_CHECK( m_onOffIdMappingKey.initialize() );
  ATH_CHECK(m_noiseCDOKey.initialize());
  ATH_CHECK(m_inputCellContainerKey.initialize());
  ATH_CHECK(m_outputCellContainerKey.initialize());

  return StatusCode::SUCCESS;
}

StatusCode HLTCaloGlobalCellMaker::execute(EventContext const& context) const {
  ATH_MSG_DEBUG("HLTCaloGlobalCellMaker::execute()");

  SG::ReadHandle<CaloConstCellContainer> inputCellHandle(m_inputCellContainerKey, context);

  SG::WriteHandle<CaloConstCellContainer> outputCellHandle(m_outputCellContainerKey, context);

  SG::ReadCondHandle<CaloNoise> noiseHdl{m_noiseCDOKey,context};
  const CaloNoise* noiseCDO=*noiseHdl;
  SG::ReadCondHandle<LArOnOffIdMapping> onoff (m_onOffIdMappingKey, context);

  auto outputCells = std::make_unique<CaloConstCellContainer>();
  uint32_t n_febs = m_onlineId->febHashMax();
  std::map<HWIdentifier,std::vector<uint16_t> > cells_idx_per_feb;
  std::map<HWIdentifier,uint16_t > maxcell_idx_per_feb;
  constexpr uint16_t maxidx = 129;
  for(uint32_t i=0;i<n_febs;i++){
         HWIdentifier feb_id = m_onlineId->feb_Id(IdentifierHash(i));
         maxcell_idx_per_feb[feb_id]=maxidx;
  }

  // Get all channels which are above a given sigma cut (usually 2) and 
  // organize them in vectors.
  for (auto const* cell : *inputCellHandle) {
    if (!cell->caloDDE()->is_tile() ){
       Identifier cellID = cell->ID();
       float noiseSigma = noiseCDO->getNoise(cellID,cell->gain());   
       IdentifierHash offhashid = m_caloCell_ID->calo_cell_hash(cellID);
       HWIdentifier hwid = (*onoff)->createSignalChannelIDFromHash(offhashid);
       HWIdentifier hwid_feb = m_onlineId->feb_Id(hwid);
       uint16_t channel = m_onlineId->channel(hwid);
       if ( cell->energy() > m_NumberOfSigma*noiseSigma ) {
         cells_idx_per_feb[hwid_feb].push_back(channel);
       }
    }
  }

  // impose the channel order. One might want to have other organizations.
  // Here, I assume channel order. Only worth to manipulate if above
  // a given number
  for(uint32_t i=0;i<n_febs;i++){
         HWIdentifier hwid_feb = m_onlineId->feb_Id(IdentifierHash(i));
	 std::vector<uint16_t>& indexes = cells_idx_per_feb[hwid_feb];
         if ( indexes.size() < m_MaxNCellsPerFEB ) continue;
         std::sort(indexes.begin(),indexes.end());
         // This is the maximal channel number that will be accepted in
         // a given FEB
         maxcell_idx_per_feb[hwid_feb]=indexes[m_MaxNCellsPerFEB-1];
  }

  for (auto const* cell : *inputCellHandle) {
    if (cell->caloDDE()->is_tile() ){
       std::unique_ptr<CaloCell> copy = cell->clone();
       outputCells->push_back(std::move(copy));
    } else {
       Identifier cellID = cell->ID();
       float noiseSigma = noiseCDO->getNoise(cellID,cell->gain());
       IdentifierHash offhashid = m_caloCell_ID->calo_cell_hash(cellID);
       HWIdentifier hwid = (*onoff)->createSignalChannelIDFromHash(offhashid);
       HWIdentifier hwid_feb = m_onlineId->feb_Id(hwid);
       uint16_t channel = m_onlineId->channel(hwid);
       // remember, by default, no channel number is above 128
       if ( (cell->energy() > m_NumberOfSigma*noiseSigma) && (channel <= maxcell_idx_per_feb[hwid_feb]) ) {
         std::unique_ptr<CaloCell> copy = cell->clone();
         outputCells->push_back(std::move(copy));
       }
    }
  }

  for (auto const id : { CaloCell_ID::LAREM, CaloCell_ID::LARHEC, CaloCell_ID::LARFCAL, CaloCell_ID::TILE })
    if (inputCellHandle->hasCalo(id))
      outputCells->setHasCalo(id);

  outputCells->updateCaloIterators();

  ATH_CHECK(outputCellHandle.record(std::move(outputCells)));

  return StatusCode::SUCCESS;
}
