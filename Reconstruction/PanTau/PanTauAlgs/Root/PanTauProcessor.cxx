/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "PanTauAlgs/PanTauProcessor.h"
#include "PanTauAlgs/Tool_InformationStore.h"
#include "PanTauAlgs/TauConstituent.h"
#include "PanTauAlgs/HelperFunctions.h"
#include "PanTauAlgs/PanTauSeed.h"

#include "xAODTau/TauJet.h"
#include "xAODParticleEvent/Particle.h"
#include "xAODParticleEvent/ParticleContainer.h"

#include <string>


PanTau::PanTauProcessor::PanTauProcessor(const std::string& name)
  : TauRecToolBase(name)
{
}


PanTau::PanTauProcessor::~PanTauProcessor() = default;


StatusCode PanTau::PanTauProcessor::initialize() {
    
  ATH_MSG_INFO(name() << " initialize()");
    
  // retrieve StoreGate Service and the tools
  ATH_CHECK( HelperFunctions::bindToolHandle( m_Tool_InformationStore, m_Tool_InformationStoreName ) );
  ATH_CHECK( HelperFunctions::bindToolHandle( m_Tool_TauConstituentGetter, m_Tool_TauConstituentGetterName ) );
  ATH_CHECK( HelperFunctions::bindToolHandle( m_Tool_TauConstituentSelector, m_Tool_TauConstituentSelectorName ) );
  ATH_CHECK( HelperFunctions::bindToolHandle( m_Tool_FeatureExtractor, m_Tool_FeatureExtractorName ) );
  ATH_CHECK( HelperFunctions::bindToolHandle( m_Tool_DecayModeDeterminator, m_Tool_DecayModeDeterminatorName ) );
  ATH_CHECK( HelperFunctions::bindToolHandle( m_Tool_DetailsArranger, m_Tool_DetailsArrangerName ) );

  // retrieve StoreGate Service and the tools
  ATH_CHECK( m_Tool_InformationStore.retrieve() );
  ATH_CHECK( m_Tool_TauConstituentGetter.retrieve() );
  ATH_CHECK( m_Tool_TauConstituentSelector.retrieve() );
  ATH_CHECK( m_Tool_FeatureExtractor.retrieve() );
  ATH_CHECK( m_Tool_DecayModeDeterminator.retrieve() );
  ATH_CHECK( m_Tool_DetailsArranger.retrieve() );
    
  ATH_CHECK( m_Tool_InformationStore->getInfo_VecDouble("ModeDiscriminator_BinEdges_Pt", m_Config_PtBins) );
  m_Config_MinPt = m_Config_PtBins.front();
  m_Config_MaxPt = m_Config_PtBins.back();
    
  return StatusCode::SUCCESS;
}


StatusCode PanTau::PanTauProcessor::finalize() {
  return StatusCode::SUCCESS;
}


StatusCode PanTau::PanTauProcessor::executePanTau(xAOD::TauJet& pTau, xAOD::ParticleContainer& pi0Container, xAOD::PFOContainer& neutralPFOContainer) const {
    
  //get the current TauJet
  xAOD::TauJet* curTauJet = &pTau;

  //check for null pointer
  if(curTauJet == nullptr) {
    ATH_MSG_WARNING("Pointer to xAOD::TauJet is 0!");
    return StatusCode::FAILURE;
  }

  fillDefaultValuesToTau(curTauJet);

  //keep track of the technical quality of the pantau seed to be created
  std::vector<int> pantauSeed_TechnicalQuality = std::vector<int>((unsigned int)PanTau::PanTauSeed::t_nTechnicalQualities, 0);
    
  //Only process taus with 1 <= ntracks <= 5:
  int numTrack = curTauJet->nTracks();
  if(numTrack == 0 || numTrack > 5) {
    ATH_MSG_DEBUG("Flagging tau for algorithm CellBased as invalid, because numTrack is " << numTrack);
    pantauSeed_TechnicalQuality.at((int)PanTau::PanTauSeed::t_NoValidInputTau) = 1;
  }

  // if there is substructure info available, get constituents, perform selection and recombination
  //these vectors will be owned by the PanTauSeed object.
  std::vector<TauConstituent*> list_TauConstituents = std::vector<TauConstituent*>(0);
  std::vector<TauConstituent*> list_SelectedTauConstituents = std::vector<TauConstituent*>(0);
    
  if(pantauSeed_TechnicalQuality.at((int)PanTau::PanTauSeed::t_NoValidInputTau) == 0) {
    // Get the constituents for the current tau
    ATH_CHECK( m_Tool_TauConstituentGetter->GetTauConstituents(curTauJet, list_TauConstituents) );
    if(list_TauConstituents.empty())  {
      pantauSeed_TechnicalQuality.at((int)PanTau::PanTauSeed::t_NoConstituentsAtAll) = 1;
      ATH_MSG_DEBUG("Seed has no associated constituents!");
    }
      
    // Call the TauConstituentSelector tool to throw out bad constituents
    ATH_CHECK(m_Tool_TauConstituentSelector->SelectTauConstituents(list_TauConstituents, list_SelectedTauConstituents) );
    if(list_SelectedTauConstituents.empty()) {
      pantauSeed_TechnicalQuality.at((int)PanTau::PanTauSeed::t_NoSelectedConstituents) = 1;
      ATH_MSG_DEBUG("Seed has no associated constituents that pass selection!");
    }
      
    //check if there are core objects in the selected constituents
    bool hasCoreConstituents = false;
    for(unsigned int iConst=0; iConst<list_SelectedTauConstituents.size(); iConst++) {
      PanTau::TauConstituent* curConst = list_SelectedTauConstituents.at(iConst);
      if(curConst->isOfType(PanTau::TauConstituent::t_Charged)) {hasCoreConstituents = true; break;};
      if(curConst->isOfType(PanTau::TauConstituent::t_Neutral)) {hasCoreConstituents = true; break;};
      if(curConst->isOfType(PanTau::TauConstituent::t_Pi0Neut)) {hasCoreConstituents = true; break;};
    }
      
    if(!hasCoreConstituents) {
      pantauSeed_TechnicalQuality.at((int)PanTau::PanTauSeed::t_NoSelectedConstituents) = 1; //use this flag at the moment as a quick hack
    }      
  }
        
  //to be consistent, taus without substructure algs run on them also have no constituents
  if(pantauSeed_TechnicalQuality.at((int)PanTau::PanTauSeed::t_NoValidInputTau) == 1) {
    pantauSeed_TechnicalQuality.at((int)PanTau::PanTauSeed::t_NoConstituentsAtAll) = 1;
    pantauSeed_TechnicalQuality.at((int)PanTau::PanTauSeed::t_NoSelectedConstituents) = 1;
  }
    
  //check for the pT flag
  double tauJet_pT = curTauJet->ptIntermediateAxis();
  if(tauJet_pT < m_Config_MinPt || tauJet_pT > m_Config_MaxPt) {      
    pantauSeed_TechnicalQuality.at((int)PanTau::PanTauSeed::t_BadPtValue) = 1;
  }
    
  // Now build the PanTauSeed with the new Constituents
  PanTau::PanTauSeed* curPanTauSeed = new PanTau::PanTauSeed(curTauJet,
							     list_SelectedTauConstituents,
							     list_TauConstituents,
							     pantauSeed_TechnicalQuality);

  curPanTauSeed->makePrivateStore();

  // Get the features for this PanTauSeed
  ATH_CHECK(m_Tool_FeatureExtractor->execute(curPanTauSeed) );

  // Seed finalizing:
  //  1. Pass the seed to the decay mode determination tool
  //  2. Pass the seed to the DetailsArranger tool to calculate the four momentum and add the details to tauJet

  // 1. call decay mode determinator for this seed
  ATH_CHECK( m_Tool_DecayModeDeterminator->execute(curPanTauSeed) );
  
  // 2. calculate the four momentum and link the details to the tauJet
  ATH_CHECK( m_Tool_DetailsArranger->execute(curPanTauSeed, pi0Container, neutralPFOContainer) );
  
  delete curPanTauSeed;
  return StatusCode::SUCCESS;
}


void PanTau::PanTauProcessor::fillDefaultValuesToTau(xAOD::TauJet* tauJet) {
  //default four momentum set to previous calibration state: TauEnergyScale
  TLorentzVector defaultP4 = tauJet->p4(xAOD::TauJetParameters::TauEnergyScale);
  tauJet->setP4(xAOD::TauJetParameters::PanTauCellBased, defaultP4.Pt(), defaultP4.Eta(), defaultP4.Phi(), defaultP4.M());
    
  //charged
  std::vector< ElementLink< xAOD::PFOContainer > > chrgPFOLinks = tauJet->protoChargedPFOLinks();
  tauJet->setChargedPFOLinks(chrgPFOLinks);
    
  //pi0
  std::vector< ElementLink< xAOD::PFOContainer > > pi0PFOLinks = tauJet->protoPi0PFOLinks();
  tauJet->setPi0PFOLinks(pi0PFOLinks);
    
  //neutrals
  std::vector< ElementLink< xAOD::PFOContainer > > neutralPFOLinks = tauJet->protoNeutralPFOLinks();
  tauJet->setNeutralPFOLinks(neutralPFOLinks);
}
