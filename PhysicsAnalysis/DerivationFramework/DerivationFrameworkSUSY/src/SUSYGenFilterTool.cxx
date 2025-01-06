/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "DerivationFrameworkSUSY/SUSYGenFilterTool.h"

#include "xAODEventInfo/EventInfo.h"
#include "xAODJet/JetContainer.h"
#include "TruthUtils/MagicNumbers.h"
#include "TruthUtils/HepMCHelpers.h"

namespace DerivationFramework {

  using namespace MCTruthPartClassifier;

  static const SG::AuxElement::Decorator<float> dec_genFiltHT("GenFiltHT");
  static const SG::AuxElement::Decorator<float> dec_genFiltMET("GenFiltMET");

  SUSYGenFilterTool::SUSYGenFilterTool(const std::string& t, const std::string& n, const IInterface* p):
    AthAlgTool(t,n,p),
    m_classif("MCTruthClassifier/SUSYGenFilt_MCTruthClassifier")
  {
    
    declareInterface<DerivationFramework::IAugmentationTool>(this);
    
    declareProperty("EventInfoName",m_eventInfoName="EventInfo");
    declareProperty("MCCollectionName",m_mcName="TruthParticles");
    declareProperty("TruthJetCollectionName",m_truthJetsName="AntiKt4TruthWZJets");
    declareProperty("MinJetPt",m_MinJetPt = 35e3);  
    declareProperty("MaxJetEta",m_MaxJetEta = 2.5);
    declareProperty("MinLeptonPt",m_MinLepPt = 25e3);
    declareProperty("MaxLeptonEta",m_MaxLepEta = 2.5);
  }
  
  SUSYGenFilterTool::~SUSYGenFilterTool(){}
  
  StatusCode SUSYGenFilterTool::initialize(){
    ATH_MSG_INFO("Initialize " );
    return StatusCode::SUCCESS;
  }

  bool SUSYGenFilterTool::isPrompt( const xAOD::TruthParticle* tp ) const
  {
    ParticleOrigin orig = m_classif->particleTruthClassifier( tp ).second;
    ATH_MSG_VERBOSE("Particle has origin " << orig);

    switch(orig) {
    case PhotonConv:
    case DalitzDec:
    case ElMagProc:
    case Mu:
    case TauLep:
    case LightMeson:
    case StrangeMeson:
    case CharmedMeson:
    case BottomMeson:
    case CCbarMeson:
    case JPsi:
    case BBbarMeson: 
    case LightBaryon:
    case StrangeBaryon:
    case CharmedBaryon: 
    case BottomBaryon:
    case PionDecay:
    case KaonDecay:
      return false;
    default:
      break;
    }
    return true;
  }

  StatusCode SUSYGenFilterTool::addBranches() const{
    ATH_MSG_VERBOSE("SUSYGenFilterTool::addBranches()");
    
    const xAOD::EventInfo* eventInfo;
    if (evtStore()->retrieve(eventInfo,m_eventInfoName).isFailure()) {
      ATH_MSG_ERROR("could not retrieve event info " <<m_eventInfoName);
      return StatusCode::FAILURE;
    }
    
    const xAOD::TruthParticleContainer* truthPC = 0;
    if (evtStore()->retrieve(truthPC,m_mcName).isFailure()) {
      ATH_MSG_ERROR("WARNING could not retrieve TruthParticleContainer " <<m_mcName);
      return StatusCode::FAILURE;
    }

    float genFiltHT(0.), genFiltMET(0.);
    ATH_CHECK( getGenFiltVars(truthPC, genFiltHT, genFiltMET) );

    ATH_MSG_DEBUG("Computed generator filter quantities: HT " << genFiltHT/1e3 << ", MET " << genFiltMET/1e3 );

    dec_genFiltHT(*eventInfo) = genFiltHT;
    dec_genFiltMET(*eventInfo) = genFiltMET;

    return StatusCode::SUCCESS;
  }

  StatusCode SUSYGenFilterTool::getGenFiltVars(const xAOD::TruthParticleContainer* tpc, float& genFiltHT, float& genFiltMET) const {
    // Get jet container out
    const xAOD::JetContainer* truthjets = 0;
    if ( evtStore()->retrieve( truthjets, m_truthJetsName).isFailure() || !truthjets ){
      ATH_MSG_ERROR( "No xAOD::JetContainer found in StoreGate with key " << m_truthJetsName ); 
      return StatusCode::FAILURE;
    }

    // Get HT
    genFiltHT = -1;
    for (const auto tj : *truthjets) {
      if ( tj->pt()>m_MinJetPt && fabs(tj->eta())<m_MaxJetEta ) {
	ATH_MSG_VERBOSE("Adding truth jet with pt " << tj->pt()
			<< ", eta " << tj->eta()
			<< ", phi " << tj->phi()
			<< ", nconst = " << tj->numConstituents());
	genFiltHT += tj->pt();
      }
    }

    float MEx(0.), MEy(0.);
    for (const auto tp : *tpc){
      int pdgid = tp->pdgId();
      if (HepMC::is_simulation_particle(tp)) continue; // Particle is from G4
      if (MC::isZeroEnergyPhoton(tp)) continue; // Work around for an old generator bug
      if ( !MC::isStable(tp) ) continue; // Stable!

      if ((MC::isElectron(pdgid) || MC::isMuon(pdgid)) && tp->pt()>m_MinLepPt && std::fabs(tp->eta())<m_MaxLepEta) {
	if( isPrompt(tp) ) {
	  ATH_MSG_VERBOSE("Adding prompt lepton " << tp);
	  genFiltHT += tp->pt();
	}
      }

      if (MC::isSpecialNonInteracting(tp) && isPrompt(tp) ) {
	ATH_MSG_VERBOSE("Found prompt nonInteracting particle " << tp);
	MEx += tp->px();
	MEy += tp->py();
      }
    }
    genFiltMET = std::sqrt(MEx*MEx+MEy*MEy);

    return StatusCode::SUCCESS;
  }

} /// namespace
