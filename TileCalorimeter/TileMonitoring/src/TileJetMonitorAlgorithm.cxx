/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "TileJetMonitorAlgorithm.h"

#include "TileEvent/TileCell.h"
#include "TileIdentifier/TileHWID.h"
#include "TileIdentifier/TileFragHash.h"
#include "TileCalibBlobObjs/TileCalibUtils.h"
#include "TileConditions/TileCablingSvc.h"
#include "TileConditions/TileCablingService.h"

#include "CaloIdentifier/TileID.h"
#include "CaloEvent/CaloCell.h"

#include "StoreGate/ReadHandle.h"
#include <xAODCore/ShallowCopy.h>
#include "xAODCaloEvent/CaloCluster.h"


TileJetMonitorAlgorithm::TileJetMonitorAlgorithm( const std::string& name, ISvcLocator* pSvcLocator )
  : AthMonitorAlgorithm(name, pSvcLocator)
  , m_tileID{nullptr}
  , m_tileHWID{nullptr}
  , m_cabling{nullptr}
{}


TileJetMonitorAlgorithm::~TileJetMonitorAlgorithm() {}


StatusCode TileJetMonitorAlgorithm::initialize() {

  ATH_CHECK( AthMonitorAlgorithm::initialize() );

  ATH_MSG_INFO("in initialize()");

  ATH_CHECK( detStore()->retrieve(m_tileID) );
  ATH_CHECK( detStore()->retrieve(m_tileHWID) );

  //=== get TileCablingSvc
  ServiceHandle<TileCablingSvc> cablingSvc("TileCablingSvc", name());
  ATH_CHECK( cablingSvc.retrieve() );

  //=== cache pointers to cabling helpers
  m_cabling = cablingSvc->cablingService();
 
  if (!m_cabling) {
    ATH_MSG_ERROR( "Pointer to TileCablingService is zero: " << m_cabling);
    return StatusCode::FAILURE;
  }

  ATH_MSG_INFO("value of m_doJetCleaning: " << m_doJetCleaning);

  //=== get TileBadChanTool
  ATH_MSG_DEBUG("::Retrieving tile bad channel tool");
  ATH_CHECK(m_tileBadChanTool.retrieve());
  ATH_MSG_DEBUG("::Retrieved tile bad channel tool");

  if (m_doJetCleaning) {
    ATH_MSG_DEBUG("::initializing JVT updater");
    ATH_CHECK(m_jvt.retrieve());
    ATH_MSG_DEBUG("::initialized JVT updater");
    
    ATH_MSG_DEBUG("::initializing JetCleaningTool");
    ATH_CHECK(m_jetCleaningTool.retrieve());
    ATH_CHECK(m_eventCleaningTool.retrieve());
    ATH_MSG_DEBUG("::initialized JetCleaningTool");
  } else {
    m_jvt.disable();
    m_jetCleaningTool.disable();
    m_eventCleaningTool.disable();
  }

  ATH_CHECK( m_jetContainerKey.initialize() );
  ATH_CHECK( m_caloCellContainerKey.initialize() );

  /* Initialize Gain and Min/Max cell energies for E-cells:
     if they are not explicitly given, set them as for the ordinary cells
  */
  if (m_energyE1Min < 0) m_energyE1Min = m_energyChanMin;
  if (m_energyE1Max < 0) m_energyE1Max = m_energyChanMax;
  if (m_gainE1 < 0) m_gainE1 = m_gain;
  if (m_energyE2Min < 0) m_energyE2Min = m_energyE1Min;
  if (m_energyE2Max < 0) m_energyE2Max = m_energyE1Max;
  if (m_gainE2 < 0) m_gainE2 = m_gainE1;
  if (m_energyE3Min < 0) m_energyE3Min = m_energyE2Min;
  if (m_energyE3Max < 0) m_energyE3Max = m_energyE2Max;
  if (m_gainE3 < 0) m_gainE3 = m_gainE2;
  if (m_energyE4Min < 0) m_energyE4Min = m_energyE3Min;
  if (m_energyE4Max < 0) m_energyE4Max = m_energyE3Max;
  if (m_gainE4 < 0) m_gainE4 = m_gainE3;
  
  return StatusCode::SUCCESS;
}


StatusCode TileJetMonitorAlgorithm::fillHistograms( const EventContext& ctx ) const {

  // In case you want to measure the execution time
  auto timer = Monitored::Timer("TIME_execute");

  if (!isGoodEvent(ctx)) {
    ATH_MSG_DEBUG("::fillHistograms(), event skipped ");
    return StatusCode::SUCCESS;
  } else {
    ATH_MSG_DEBUG("::fillHistograms(), event accepted ");
  }

  SG::ReadHandle<xAOD::JetContainer> jetContainer(m_jetContainerKey, ctx);
  if (!jetContainer.isValid()) {
    ATH_MSG_WARNING("Can't retrieve Jet Container: " << m_jetContainerKey.key());
    return StatusCode::SUCCESS;
  }

  uint32_t lumiBlock = GetEventInfo(ctx)->lumiBlock();

  ATH_MSG_VERBOSE("::fillHistograms(), lumiblock " << lumiBlock);

  std::set<Identifier> usedCells;  // cells already used in the given event

  int iJet = 0;
  for (const xAOD::Jet* jet : *jetContainer) {
    if ((jet->pt() > m_jetPtMin) && (fabs(jet->eta()) < m_jetEtaMax)) {
      if (isGoodJet(*jet)) {
        ATH_MSG_DEBUG("::fillHistograms, jet " << iJet
			                << ", eta " << jet->eta()
			                << ", phi " << jet->phi()
			                << ", constituents " << jet->numConstituents());
        CHECK(fillTimeHistograms(*jet, lumiBlock, usedCells));
      } else {
        ATH_MSG_DEBUG("::fillHistogram, BAD jet " << iJet
                      << ", eta " << jet->eta()
                      << ", phi " << jet->phi()
                      << ", constituents " << jet->numConstituents());
      }
    }
    ++iJet;
  }


  fill("TileJetMonExecuteTime", timer);

  ATH_MSG_VERBOSE("::fillHistograms(), end-of-loops ");

  return StatusCode::SUCCESS;
}


/*---------------------------------------------------------*/  
StatusCode TileJetMonitorAlgorithm::fillTimeHistograms(const xAOD::Jet& jet, uint32_t lumiBlock, std::set<Identifier>& usedCells) const {
/*---------------------------------------------------------*/  

  ATH_MSG_VERBOSE( "in fillTimeHistograms()" );

  if( jet.numConstituents() == 0 || !jet.getConstituents().isValid()) return StatusCode::SUCCESS;
     
  int cellIndex(-1);

  ToolHandle<GenericMonitoringTool> tileJetChannTimeDQTool = getGroup("TileJetChanTimeDQ");

  std::array<std::string, 2> gainName{"LG", "HG"};
  std::array<std::string, 5> partitionName{"AUX", "LBA", "LBC", "EBA", "EBC"};

  for (const xAOD::JetConstituent* jet_constituent : jet.getConstituents()) {
    if( jet_constituent->type() == xAOD::Type::CaloCluster ){
      const xAOD::CaloCluster* calo_cluster = static_cast<const xAOD::CaloCluster*>(jet_constituent->rawConstituent());
      if (calo_cluster && calo_cluster->getCellLinks()) {
        for (const CaloCell* cell : *calo_cluster) {
          cellIndex++;
          if (cell->caloDDE()->getSubCalo() == CaloCell_ID::TILE) { // a Tile Cell
            ATH_MSG_DEBUG("Cell " << cellIndex << " IS TILECAL !!");
            const TileCell *tilecell = (TileCell*) cell;
            Identifier id = tilecell->ID();
            if (usedCells.find(id) == usedCells.end()) {
              usedCells.insert(id);
            } else {
              continue;
            }
            //      int section= m_tileID->section(id);
            // int module = m_tileID->module(id); // ranges 0..63
            auto module = Monitored::Scalar<int>("module", m_tileID->module(id));
            int sample = m_tileID->sample(id); // ranges 0..3 (A, BC, D, E)
	    int tower = m_tileID->tower(id); 
            int ros1 = 0;
            int ros2 = 0;
            int chan1 = -1;
            int chan2 = -1;
            uint32_t bad1 = 0;
            uint32_t bad2 = 0;
            int gain1 = tilecell->gain1();
            int gain2 = tilecell->gain2();
            unsigned int qbit1 = tilecell->qbit1();
            unsigned int qbit2 = tilecell->qbit2();
            
            const CaloDetDescrElement * caloDDE = tilecell->caloDDE();
            IdentifierHash hash1 = caloDDE->onl1();
            if (hash1 != TileHWID::NOT_VALID_HASH) {
              HWIdentifier adc_id = m_tileHWID->adc_id(hash1, gain1);
              ros1 = m_tileHWID->ros(adc_id);
              chan1 = m_tileHWID->channel(adc_id);
              bad1 = m_tileBadChanTool->encodeStatus(m_tileBadChanTool->getAdcStatus(adc_id));
            }
            
            // How is it here with partition? D0 spans two partitions....
            // It should be ok to treat it in this way:
            IdentifierHash hash2 = caloDDE->onl2();
            if (hash2 != TileHWID::NOT_VALID_HASH) {
              HWIdentifier adc_id = m_tileHWID->adc_id(hash2, gain2);
              ros2 = m_tileHWID->ros(adc_id);
              chan2 = m_tileHWID->channel(adc_id);
              bad2 = m_tileBadChanTool->encodeStatus(m_tileBadChanTool->getAdcStatus(adc_id));
            }

            bool is_good1 = isGoodChannel(ros1, module, chan1, bad1, qbit1, id);
            bool is_good2 = isGoodChannel(ros2, module, chan2, bad2, qbit2, id);
            float ene1 = is_good1 ? tilecell->ene1() : -1;
            float ene2 = is_good2 ? tilecell->ene2() : -1;
            
            ATH_MSG_DEBUG(".... " << TileCalibUtils::getDrawerString(ros1, module)
                          << ", ch1 " << chan1
                          << ", ch2 " << chan2
                          << ", qbit " << qbit1 << "/" << qbit2
                          << ", is_bad " << bad1 << "/" << bad2
                          << ", isGood " << is_good1
                          << "/" << is_good2
                          << ", ene " << tilecell->energy());
            /*
              Now really fill the histograms time vs lumiblock and 1dim time
            */
            
            // first channel
            if (is_good1 && matchesEnergyRange(sample, tower, ene1, gain1)) {
              if (m_do1DHistograms) {
                std::string name = TileCalibUtils::getDrawerString(ros1, module) + "_ch_" + std::to_string(chan1) + "_1d";
                auto channelTime = Monitored::Scalar<float>(name, tilecell->time1());
                fill("TileJetChanTime1D", channelTime);
              }

              if (m_do2DHistograms) {
                ATH_MSG_WARNING("This histograms are not implemented yet!");
              }

              // info for DQ histograms
              auto moduleDQ = Monitored::Scalar<int>("module" + partitionName[ros1], module + 1);
              auto channelDQ = Monitored::Scalar<int>("channel" + partitionName[ros1], chan1);
              auto timeDQ = Monitored::Scalar<float>("time" + partitionName[ros1], tilecell->time1());
              Monitored::fill(tileJetChannTimeDQTool, moduleDQ, channelDQ, timeDQ);

              // general histograms, only require non-affected channels
              if (bad1 < 2) {
                ATH_MSG_DEBUG( "Filling in time1 for " << TileCalibUtils::getDrawerString(ros1, module)
                               << ", ch " << chan1
                               << ", ene " << ene1
                               << ", LB " << lumiBlock
                               << ", time: " << tilecell->time1());

                std::string name("channelTime" + partitionName[ros1]);
                auto channelTime = Monitored::Scalar<float>(name, tilecell->time1());
                fill("TileJetChanTime", channelTime);

                if ((ros1 > 2) && (sample < TileID::SAMP_E)) {
                  std::string nameNoScint("channelTime" + partitionName[ros1] + "_NoScint");
                  auto channelTimeNoScint = Monitored::Scalar<float>(nameNoScint, tilecell->time1());
                  fill("TileJetChanTime", channelTimeNoScint);
                }
              }
            }

            // second channel
            if (is_good2 && matchesEnergyRange(sample, tower, ene2, gain2)) {
              if (m_do1DHistograms) {
                std::string name = TileCalibUtils::getDrawerString(ros2, module) + "_ch_" + std::to_string(chan2) + "_1d";
                auto channelTime = Monitored::Scalar<float>(name, tilecell->time2());
                fill("TileJetChanTime1D", channelTime);
              }

              if (m_do2DHistograms) {
                ATH_MSG_WARNING("This histograms are not implemented yet!");
              }

              // info for DQ histograms
              auto moduleDQ = Monitored::Scalar<int>("module" + partitionName[ros2], module + 1);
              auto channelDQ = Monitored::Scalar<int>("channel" + partitionName[ros2], chan2);
              auto timeDQ = Monitored::Scalar<float>("time" + partitionName[ros2], tilecell->time2());
              Monitored::fill(tileJetChannTimeDQTool, moduleDQ, channelDQ, timeDQ);

              // general histograms, only require non-affected channels
              if (bad2 < 2) {
                ATH_MSG_DEBUG( "Filling in time2 for " << TileCalibUtils::getDrawerString(ros2, module)
                               << ", ch " << chan2
                               << ", ene " << ene2
                               << ", LB " << lumiBlock
                               << ", time: " << tilecell->time2()
                               <<" (qbit2 " << qbit2 << ", ch1 " << chan1 << ", ene1 " << ene1 << ", bad1 " << bad1 << ", qbit1 " << qbit1 << ")" );

                std::string name("channelTime" + partitionName[ros2]);
                auto channelTime = Monitored::Scalar<float>(name, tilecell->time2());
                fill("TileJetChanTime", channelTime);

                if ((ros2 > 2) && (sample < TileID::SAMP_E)) {
                  std::string nameNoScint("channelTime" + partitionName[ros2] + "_NoScint");
                  auto channelTimeNoScint = Monitored::Scalar<float>(nameNoScint, tilecell->time2());
                  fill("TileJetChanTime", channelTimeNoScint);
                }
              }
            }

            /* 
               Now filling the cell-based histograms,
               HG-HG and LG-LG combinations only for normal cells, 
	       also include E-cells
            */
            if ((is_good1) && (((is_good2) && (gain1 == gain2)) || (sample == TileID::SAMP_E)))   {
	      // E-cells are read-out by one channel only, so is_good2 = false for E-cells

              if (m_doEnergyDiffHistograms && (tilecell->energy() > m_energyDiffThreshold))  { 
                // EneDiff histograms

                int evenChannnel = (chan1 % 2 == 0) ? chan1 : chan2;
                std::string name = TileCalibUtils::getDrawerString(ros1, module) + "_enediff_" 
                                   + gainName[gain1] + "_ch1_" + std::to_string(evenChannnel);
                auto energyDifference = Monitored::Scalar<float>(name, tilecell->eneDiff() / tilecell->energy());
                fill("TileJetEnergyDiff", energyDifference);
              }

              if ((bad1 < 2) && (bad2 < 2)) {

                // cell-time histograms, only overall, require not affected channels
		int index = findIndex(gain1, tilecell->energy());
		ATH_MSG_DEBUG( "Filling in cell-time for " << TileCalibUtils::getDrawerString(ros1, module)
			       << ", ch1 " << chan1  
			       << ", ch2 " << chan2  
			       << ", ene " << tilecell->energy()
			       << ", index " << index
			       << ", time: " << tilecell->time());
                  
		// TD adding histograms per partition and per radial sampling
		std::string name1("Cell_time_" + partitionName[ros1] + "_" + sampleName(ros1, sample, tower) + "_" + gainName[gain1] + "_slice_" + std::to_string(index));
		auto cellTime1 = Monitored::Scalar<float>(name1, tilecell->time());
		fill("TileJetCellTime", cellTime1);
                  
		if (m_doEnergyProfiles) {
		  // TD adding energy profiles per partition and per radial sampling
		  std::string indexName1("index_" + partitionName[ros1] + "_" + sampleName(ros1, sample, tower) + "_" + gainName[gain1]);
		  auto energyIndex1 = Monitored::Scalar<float>(indexName1, index);
                  
		  std::string energyName1("energy_" + partitionName[ros1] + "_" + sampleName(ros1, sample, tower) + "_" + gainName[gain1]);
		  auto cellEnergy1 = Monitored::Scalar<float>(energyName1, tilecell->energy());
                  
		  fill("TileJetCellEnergyProfile", energyIndex1, cellEnergy1);
		} else {
		  // TD adding energy histograms per partition and per radial sampling
		  std::string name1("Cell_ene_" + partitionName[ros1] + "_" + sampleName(ros1, sample, tower) + "_" + gainName[gain1] + "_slice_" + std::to_string(index));
		  auto cellEnergy1 = Monitored::Scalar<float>(name1, tilecell->energy());
		  fill("TileJetCellEnergy", cellEnergy1);
		}
	      }
            }
          } else {
            ATH_MSG_DEBUG("Cell " << cellIndex << " is NOT Tilecal");
          }
        }
      }
    }
  }

  return StatusCode::SUCCESS;
}

/*---------------------------------------------------------*/  
std::string TileJetMonitorAlgorithm::sampleName(const int ros, const int sample, const int tower) const {
/*---------------------------------------------------------*/
  std::array<std::string, 3> sample_Name_LB{"A", "BC", "D"};
  std::array<std::string, 4> sample_Name_EB{"A", "B", "D", "E"};
  std::string s_name;
  if (ros < 3) { // LBA, LBC
    s_name = sample_Name_LB[sample]; // default, standard cells
    if ((sample == TileID::SAMP_BC) && (tower == 8)) { // cell B9
      s_name = "B9";
    }
  } else { // EBA, EBC
    s_name = sample_Name_EB[sample]; //default, standard cells
    if ((sample == TileID::SAMP_C) && (tower == 9)) { // cell C10
      s_name = "C10";
    }
    if ((sample == TileID::SAMP_D) && (tower == 8)) { // cell D4
      s_name = "D4";
    }
    if ((sample == TileID::SAMP_E) && (tower == 10)) { // cell E1	
      s_name = "E1";
    }
    if ((sample == TileID::SAMP_E) && (tower == 11)) { // cell E2	
      s_name = "E2";
    }
    if ((sample == TileID::SAMP_E) && (tower == 13)) { // cell E3	
      s_name = "E3";
    }
    if ((sample == TileID::SAMP_E) && (tower == 15)) { // cell E4	
      s_name = "E4";
    }
  }
  return s_name;
}

/*---------------------------------------------------------*/    
bool TileJetMonitorAlgorithm::matchesEnergyRange(const int sample, const int tower, const float energy, const int gain) const {
/*---------------------------------------------------------*/    
  /* Want to separate E-cells, D4 and C10 from the rest:
     ros: 1-2 LBA/LBC, 3-4 EBA/EBC
     sample: 0 = A, 1 = B/BC/C, D = 2, E = 3
     tower: 10 = E1, 11 = E2, 13 = E3, 14 = E4, 8 = D4, 9 = C10
     Nevertheless, C10 and D4 have the same limits, since these are ordinary cells
  */    
  if (sample != TileID::SAMP_E) {
    return((energy > m_energyChanMin) && (energy < m_energyChanMax) && (gain == m_gain));
  } else {
    switch (tower) {
    case 10:
      return((energy > m_energyE1Min) && (energy < m_energyE1Max) && (gain == m_gainE1));
      break;
    case 11:
      return((energy > m_energyE2Min) && (energy < m_energyE2Max) && (gain == m_gainE2));
      break;
    case 13:
      return((energy > m_energyE3Min) && (energy < m_energyE3Max) && (gain == m_gainE3));
      break;
    case 15:
      return((energy > m_energyE4Min) && (energy < m_energyE4Max) && (gain == m_gainE4));
      break;
    default:
      return false;
    }
  }
}

bool TileJetMonitorAlgorithm::isGoodChannel(int ros, int module, int channel, uint32_t bad, unsigned int qbit, Identifier id) const {

  if ((ros < 1) || (ros >= (int) TileCalibUtils::MAX_ROS)) return false; // invalid partition

  if ((module < 0) || (module >= (int) TileCalibUtils::MAX_DRAWER)) return false;  // invalid module number

  if (m_cabling->isDisconnected(ros, module, channel)) {
    return false; // non-existing PMT (empty hole)
  }

  if (((qbit & TileCell::MASK_BADCH) != 0) || // masked as bad
      ((qbit & TileCell::MASK_TIME) != TileCell::MASK_TIME) ||  // flagged
      ((qbit & TileCell::MASK_ALGO) == TileFragHash::OptFilterDsp)) // in DSP

    return false;
  /* 
   bad is the status in the DB (see http://alxr.usatlas.bnl.gov/lxr-stb6/source/atlas/TileCalorimeter/TileConditions/src/TileBadChanTool.cxx#390).
   Meaning:
   0 = good, 1 = noisy, 2 = affected, 3 = bad, 4 = otherwise
   */
  if (bad > 2) return false;

  /* 
     Now check for special C10, merged E1, E4'
     C10 spec is ok only if channel = 5 (i.e. pmt=6). The other is pmt=5
     E1 merged and E4' should be dropped if channel = 12 (i.e. pmt=13)
  */
  return ((( channel != 4) && (channel != 12)) || m_cabling->TileGap_connected(id));
}


bool TileJetMonitorAlgorithm::isGoodEvent(const EventContext& ctx) const {
  /* check for errors in LAr and Tile, see https://twiki.cern.ch/twiki/bin/viewauth/Atlas/DataPreparationCheckListForPhysicsAnalysis
   */
  if (! m_doEventCleaning) return true;

  ATH_MSG_DEBUG("::isGoodEvent()....");

  SG::ReadHandle<xAOD::EventInfo> eventInfo = GetEventInfo(ctx);

  if (eventInfo->errorState(xAOD::EventInfo::LAr) == xAOD::EventInfo::Error) return false;
  if (eventInfo->errorState(xAOD::EventInfo::Tile) == xAOD::EventInfo::Error) return false;

  /* see https://twiki.cern.ch/twiki/bin/view/AtlasProtected/HowToCleanJets2017
  */
  if (! m_doJetCleaning) return true;

  const xAOD::JetContainer* jetContainer = SG::get(m_jetContainerKey);
  if (! jetContainer){
    ATH_MSG_INFO("Cannot retrieve " << m_jetContainerKey << ". However, returning true.");
    return true;
  }

  auto jetsSC = xAOD::shallowCopyContainer(*jetContainer);
  std::unique_ptr< xAOD::JetContainer > jetsCopy(jetsSC.first);
  std::unique_ptr< xAOD::ShallowAuxContainer > jetsCopyAux(jetsSC.second);

  // We're attaching decorations here to a temporary object that
  // is not recorded, so we shouldn't use a WriteDecorHandle.
  static const SG::AuxElement::Decorator<char> passOR ("passOR");
  static const SG::AuxElement::Decorator<char> passJvt ("passJvt");

  int iJet = 0;
  for (auto jet : *jetsCopy) {
    ATH_MSG_DEBUG("Jet " << iJet << ", pT " << jet->pt()/1000.0 << " GeV, eta " 
		  << jet->eta());
    passJvt(*jet) = passesJvt(*jet);
    passOR(*jet) = true;
    ATH_MSG_DEBUG("... done with jet " << iJet);
    ++iJet;
  }

  bool accept = m_eventCleaningTool->acceptEvent(&*jetsCopy);

  return accept;
}


bool TileJetMonitorAlgorithm::passesJvt(const xAOD::Jet& jet) const {

  if (jet.pt() > m_jetPtMin
      && jet.pt() < m_jetPtMax
      && fabs(jet.getAttribute<float>("DetectorEta")) < m_jetTrackingEtaLimit
      && m_jvt->updateJvt(jet) < m_jvtThreshold) {

    return false;

  } else {
    return true;
  }

}

bool TileJetMonitorAlgorithm::isGoodJet(const xAOD::Jet& jet) const {

  if (m_doJetCleaning) {

    if (jet.pt() >= m_jetPtMin && passesJvt(jet) && m_jetCleaningTool->keep(jet)) {
      return true;
    } else {
      return false;
    }

  } else {
    return true;
  }

}

unsigned int TileJetMonitorAlgorithm::findIndex(const int gain, const float energy) const {

  if (gain == 1) {
    return (std::upper_bound(m_cellEnergyUpperLimitsHG.begin(), m_cellEnergyUpperLimitsHG.end(), energy) 
            - m_cellEnergyUpperLimitsHG.begin());
  } else {
    return (std::upper_bound(m_cellEnergyUpperLimitsLG.begin(), m_cellEnergyUpperLimitsLG.end(), energy) 
            - m_cellEnergyUpperLimitsLG.begin());
  }

}
