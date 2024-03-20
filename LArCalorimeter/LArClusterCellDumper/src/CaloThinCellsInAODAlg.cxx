/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file LArClusterCellDumper/src/CaloThinCellsInAODAlg.h
 * @author mateus hufnagel <mateus.hufnagel@cern.ch>
 * @date mar, 2024
 * @brief Thin calorimeter cells, digits and raw channels associated with clusters, after some cut, and forward them to AOD.
 */

#include "LArClusterCellDumper/CaloThinCellsInAODAlg.h"
#include "StoreGate/ReadHandle.h"

/**
 * @brief Gaudi initialize method.
 */
StatusCode CaloThinCellsInAODAlg::initialize()
{
  ATH_MSG_INFO("Thinning cells from '"<< m_clusterCntKey.key()<<"' container with "<< m_clusterPtCut <<" MeV and "<< m_clusterEtaCut <<".");
  if    (!m_isMC) ATH_MSG_INFO("Outputs are: CaloCells ('"<< m_caloCellOutputKey.key() <<"'), rawCh ('"<< m_rawChOutputKey.key()<<"') and digits ('"<< m_digitsOutputKey.key() <<"').");
  else  ATH_MSG_INFO("Outputs are: CaloCells ('"<< m_caloCellOutputKey.key() <<"'), rawCh ('"<< m_rawChOutputKey.key()<<"'), digits ('"<< m_digitsOutputKey.key() <<"') and hits ('"<< m_hitsOutputKey.key() <<"')");
  
  ATH_CHECK(m_cablingKey.initialize());
  
  // inputs
  ATH_CHECK(m_clusterCntKey.initialize());
  ATH_CHECK(m_hitsInputKey.initialize(m_isMC));
  ATH_CHECK(m_digitsInputKey.initialize());
  ATH_CHECK(m_rawChInputKey.initialize());
  ATH_CHECK(m_caloCellInputKey.initialize());
  
  // outputs
  ATH_CHECK(m_hitsOutputKey.initialize());
  ATH_CHECK(m_digitsOutputKey.initialize());
  ATH_CHECK(m_rawChOutputKey.initialize());
  ATH_CHECK(m_caloCellOutputKey.initialize());

  // calo id helpers
  ATH_CHECK(detStore()->retrieve(m_onlineID, "LArOnlineID"));
  ATH_CHECK(detStore()->retrieve(m_caloCellId,"CaloCell_ID"));

  return StatusCode::SUCCESS;
}

/**
 * @brief Execute the algorithm.
 * @param ctx Current event context.
 */
StatusCode CaloThinCellsInAODAlg::execute (const EventContext& ctx) const
{
  ATH_MSG_DEBUG("Executing CaloThinCellsInAODAlg...");

  SG::ReadCondHandle<LArOnOffIdMapping> cablingHdl(m_cablingKey,ctx);
  const LArOnOffIdMapping* larCabling=*cablingHdl;

  //Get event inputs from read handles:
  SG::ReadHandle<xAOD::CaloClusterContainer> clusterContainer(m_clusterCntKey,ctx);
  SG::ReadHandle<CaloCellContainer> inputCellContainer(m_caloCellInputKey,ctx);
  SG::ReadHandle<LArRawChannelContainer> inputRawChContainer(m_rawChInputKey,ctx);
  SG::ReadHandle<LArDigitContainer> inputDigitsContainer(m_digitsInputKey,ctx);

  // Write to the new containers
  SG::WriteHandle<ConstCaloCellCont_t> outputCells(m_caloCellOutputKey,ctx);
  ATH_CHECK(outputCells.record(std::make_unique<ConstCaloCellCont_t>(SG::VIEW_ELEMENTS)));

  SG::WriteHandle<ConstLArDigitCont_t> outputDigits(m_digitsOutputKey,ctx);
  ATH_CHECK(outputDigits.record(std::make_unique<ConstLArDigitCont_t>(SG::VIEW_ELEMENTS)));

  SG::WriteHandle<LArRawChannelContainer> outputRawChannels(m_rawChOutputKey,ctx);
  ATH_CHECK(outputRawChannels.record(std::make_unique<LArRawChannelContainer>()));

  SG::WriteHandle<LArHitContainer> outputHits = SG::makeHandle(m_hitsOutputKey, ctx);
  ATH_CHECK(outputHits.record(std::make_unique<LArHitContainer>()));

  std::bitset<200000> keepCellSet;
  size_t nCellsAllClus = 0;

  for (const xAOD::CaloCluster* clus : *clusterContainer){

    // cluster filters
    if (clus->pt() < m_clusterPtCut) continue;
    if (std::abs(clus->eta()) > m_clusterEtaCut) continue;

    ATH_MSG_DEBUG("Cluster pt: "<< clus->pt()<<", eta/phi: "<< clus->eta() <<" / "<< clus->phi());

    const CaloClusterCellLink* cellLinks = clus->getCellLinks();

    if (!cellLinks){
      ATH_MSG_DEBUG( "  Cluster without cell links found  in collection: " << m_clusterCntKey.key() << "   ===> cells cannot be written in AOD as requested ! " );
      continue;
    }
    if (cellLinks->getCellContainerLink().dataID() != m_caloCellInputKey.key()) {
       ATH_MSG_DEBUG( "  Cluster points to cell container " << cellLinks->getCellContainerLink().dataID() << " which is different from the cell container being thinned: " << m_caloCellInputKey.key() << "; cluster skipped.");
       continue;
    }

    nCellsAllClus += cellLinks->size();
    CaloClusterCellLink::const_iterator it = cellLinks->begin();
    CaloClusterCellLink::const_iterator end = cellLinks->end();
    for (; it != end; ++it) {
      if (it.index() >= inputCellContainer->size()) {
        ATH_MSG_DEBUG( "  Cell index " << it.index() << " is larger than the number of cells in " << m_caloCellInputKey.key() << " (" << inputCellContainer->size() << ")" );
        continue;
      }
      const CaloCell* cell = (*it); //get the caloCells

      Identifier cellId = cell->ID();

      if (m_caloCellId->is_em_barrel(cellId)){ // cells belong to EM Barrel
        HWIdentifier chhwid = larCabling->createSignalChannelID(cellId);
        IdentifierHash chOnlHash =  m_onlineID->channel_Hash(chhwid);

        if (!keepCellSet.test(chOnlHash)){
          keepCellSet.set(chOnlHash);
          outputCells->push_back(cell);
        }
      }    
    }//end loop over calo cell links
  } // end loop over clusters
  ATH_MSG_DEBUG("\tTotal Copied " << outputCells->size() << " of " << nCellsAllClus << " calo cells, linked to CaloCluster.");

  if (keepCellSet.any()){  
    //start loop over raw channels
    for(const LArRawChannel& chan : *inputRawChContainer) {
      const IdentifierHash onlHash=m_onlineID->channel_Hash(chan.hardwareID());
      if (keepCellSet.test(onlHash)) {
        outputRawChannels->push_back(chan);
      }
    }
    ATH_MSG_DEBUG("\tCopied " << outputRawChannels->size() << " of " << (*inputRawChContainer).size() << " raw channels.");

    //start loop over digits
    for (const LArDigit* dig : *inputDigitsContainer) {
      const IdentifierHash onlHash=m_onlineID->channel_Hash(dig->hardwareID());
      if (keepCellSet.test(onlHash)) {
        outputDigits->push_back(dig);
      }
    } //end loop over input container
    ATH_MSG_DEBUG("\tCopied " << outputDigits->size() << " of " << inputDigitsContainer->size() << " digits.");
  }
  
  //(MC) start loop over hits container
  if (m_isMC){
    if (keepCellSet.any()){  
      SG::ReadHandle<LArHitContainer> inputHitsContainer(m_hitsInputKey,ctx);

      for (const LArHit* hit : *inputHitsContainer) {
        const HWIdentifier   hwid    = larCabling->createSignalChannelID(hit->cellID());
        const IdentifierHash onlHash = m_onlineID->channel_Hash(hwid);

        if (keepCellSet.test(onlHash)) {
          LArHit* clusHit = new LArHit(hit->cellID(),hit->energy(),hit->time());
          clusHit->finalize();
          outputHits->push_back(clusHit);
        }
      } //end loop over input container

    ATH_MSG_DEBUG("\tCopied " << outputHits->size() << " of " << inputHitsContainer->size() << " hits.");
    } // end keepCellSet.any()
  } // end-if MC

  return StatusCode::SUCCESS;
}
