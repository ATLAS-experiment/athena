/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "PanTauAlgs/Tool_InputConverter.h"
#include "PanTauAlgs/Tool_InformationStore.h"
#include "PanTauAlgs/TauConstituent.h"
#include "PanTauAlgs/PanTauSeed.h"
#include "PanTauAlgs/HelperFunctions.h"
#include "xAODTau/TauJet.h"
#include "xAODPFlow/PFO.h"
#include "xAODPFlow/PFODefs.h"


PanTau::Tool_InputConverter::Tool_InputConverter( const std::string& name ) :
  asg::AsgTool(name)
{
}

PanTau::Tool_InputConverter::~Tool_InputConverter() = default;

StatusCode PanTau::Tool_InputConverter::initialize() {

  ATH_MSG_INFO(" initialize()");
  m_init=true;

  ATH_CHECK( HelperFunctions::bindToolHandle( m_Tool_InformationStore, m_Tool_InformationStoreName) ); //ABR only

  ATH_CHECK( m_Tool_InformationStore.retrieve() );
    
  ATH_CHECK( m_Tool_InformationStore->getInfo_Int("TauConstituents_UsePionMass", m_Config_UsePionMass) );
  ATH_CHECK( m_Tool_InformationStore->getInfo_Double("TauConstituents_Types_DeltaRCore", m_Config_TauConstituents_Types_DeltaRCore) );
  ATH_CHECK( m_Tool_InformationStore->getInfo_Double("TauConstituents_PreselectionMinEnergy", m_Config_TauConstituents_PreselectionMinEnergy) );
  ATH_CHECK( m_Tool_InformationStore->getInfo_VecDouble("CellBased_BinEdges_Eta", m_Config_CellBased_BinEdges_Eta) );
  ATH_CHECK( m_Tool_InformationStore->getInfo_VecDouble("CellBased_EtaBinned_Pi0MVACut_1prong", m_Config_CellBased_EtaBinned_Pi0MVACut_1prong) );
  ATH_CHECK( m_Tool_InformationStore->getInfo_VecDouble("CellBased_EtaBinned_Pi0MVACut_3prong", m_Config_CellBased_EtaBinned_Pi0MVACut_3prong) );  

  return StatusCode::SUCCESS;
}


bool PanTau::Tool_InputConverter::passesPreselectionEnergy(double energy) const {
  return energy >= m_Config_TauConstituents_PreselectionMinEnergy;
}


// PFO converter
StatusCode PanTau::Tool_InputConverter::ConvertToTauConstituent(const xAOD::PFO* pfo,
								PanTau::TauConstituent* &tauConstituent,
								const xAOD::TauJet* tauJet) const {
    
  // check for invalid eta, phi, e "NAN" values -- likely no longer needed, raise severity to be sure
  if (pfo->eta() != pfo->eta() || pfo->phi() != pfo->phi() || pfo->e() != pfo->e()) {
    ATH_MSG_ERROR("Will not convert PFO with eta value of " << pfo->eta() << " -> return to Tool_TauConstituentGetter");
    return StatusCode::FAILURE;
  }
  
  // Check whether neutral input pfo has pion mass (it may have if xAOD is being reprocessed)
  // If it does, 
  // - old (R21) approach: make it massless again and use that
  // - new (R22) approach: complain and abort, reconstruction from AOD implies rebuilding PFOs upstream, so the mass should not have been set at that point
  if (!pfo->isCharged() && pfo->m() != 0) {
    ATH_MSG_ERROR("Input neutral PFO should have null mass. If reconstruction runs from AOD, PFOs must be rebuilt!");
    return StatusCode::FAILURE;
  }
    
  // preselection to veto very low energy PFOs
  if (!passesPreselectionEnergy(pfo->e())) {
    ATH_MSG_DEBUG("EFO of charge " << static_cast<int>(pfo->charge()) << " and energy " << pfo->e() << " does not pass presel Energy cut of " << m_Config_TauConstituents_PreselectionMinEnergy);
    return StatusCode::SUCCESS;
  }
        
  // get the mass correct & build 4-vector
  double constituentMass = pfo->m();
  if (m_Config_UsePionMass) {
    
    // clusters: don't touch the measured energy. set mass to pion mass, so momentum will be altered
    if (!pfo->isCharged()) {
      constituentMass = 134.98;
    }
  }
 
  TLorentzVector momentum; 
  PanTau::SetP4EEtaPhiM( momentum, pfo->e(), pfo->eta(), pfo->phi(), constituentMass);
    
  // get type (based on charge and DR to tau)
  std::vector<int> typeFlags = std::vector<int>((unsigned int)PanTau::TauConstituent::t_nTypes, 0);
  if (typeFlags.size() < (unsigned int)PanTau::TauConstituent::t_nTypes) std::abort(); // suppress cppcheck warning
  typeFlags.at((int)PanTau::TauConstituent::t_NoType) = 1;
    
  double mvaValue = PanTau::TauConstituent::DefaultBDTValue();

  TLorentzVector tlv_intAxis = tauJet->p4(xAOD::TauJetParameters::IntermediateAxis);
  double deltaR_toTauJet = tlv_intAxis.DeltaR( pfo->p4() );
    
  if (deltaR_toTauJet > m_Config_TauConstituents_Types_DeltaRCore) {
    if (pfo->isCharged()) {
      typeFlags.at((int)PanTau::TauConstituent::t_Charged) = 1;
    }
    else {
      typeFlags.at((int)PanTau::TauConstituent::t_OutNeut) = 1;
      mvaValue = pfo->bdtPi0Score();
    }
  }//end if pfo is not in core
    
  if (deltaR_toTauJet <= m_Config_TauConstituents_Types_DeltaRCore) {
    
    if (pfo->isCharged()) {
      typeFlags.at((int)PanTau::TauConstituent::t_Charged) = 1;
    }
    else {
      typeFlags.at((int)PanTau::TauConstituent::t_Neutral) = 1;
            
      //neutral PFO arranging --- check for pi0 tag
      mvaValue = pfo->bdtPi0Score();
            
      int nPi0sPerCluster = 0;
      if ( !pfo->attribute(xAOD::PFODetails::nPi0Proto, nPi0sPerCluster) ) {
	ATH_MSG_WARNING("WARNING: Could not retrieve nPi0Proto. Will set it to 1.");
	nPi0sPerCluster = 1;
      }
      if (nPi0sPerCluster > 0) typeFlags.at((int)PanTau::TauConstituent::t_Pi0Neut) = 1;

    }
  }//end if pfo is in core
    
  // create the tau constituent
  tauConstituent = new PanTau::TauConstituent(momentum, static_cast<int>(pfo->charge()), typeFlags, mvaValue, pfo);
  tauConstituent->makePrivateStore();
    
  // Check if the pfo object has shots:
  std::vector<const xAOD::IParticle*> list_TauShots = std::vector<const xAOD::IParticle*>(0);
  if (!(pfo->associatedParticles(xAOD::PFODetails::TauShot, list_TauShots))) {
    ATH_MSG_DEBUG("WARNING: Could not get shots from current pfo");
  }
    
  for (unsigned int iShot=0; iShot<list_TauShots.size(); iShot++) {
    
    if (list_TauShots.at(iShot) == nullptr) {
      ATH_MSG_WARNING("Shot number " << iShot << " points to 0! Skip it");
      continue;
    }
    
    const xAOD::PFO*        curShot         = dynamic_cast<const xAOD::PFO*>(list_TauShots.at(iShot));
    TLorentzVector          shotMomentum;
    PanTau::SetP4EEtaPhiM( shotMomentum, curShot->e(), curShot->eta(), curShot->phi(), curShot->m());
    std::vector<int>        shotTypeFlags   = std::vector<int>((unsigned int)PanTau::TauConstituent::t_nTypes, 0);
    PanTau::TauConstituent* shotConstituent = new PanTau::TauConstituent(shotMomentum, 0, typeFlags, PanTau::TauConstituent::DefaultBDTValue(), curShot);
    shotConstituent->makePrivateStore();
    
    int nPhotons = 0;
    if (!curShot->attribute(xAOD::PFODetails::tauShots_nPhotons, nPhotons)) {
      nPhotons = -1;
      ATH_MSG_DEBUG("WARNING: Could not get nPhotons for this shot! Set to -1");
    }
    shotConstituent->setNPhotonsInShot(nPhotons);
    tauConstituent->addShot(shotConstituent);            
  }//end loop over shots
    
  return StatusCode::SUCCESS;
}
