/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "PanTauAlgs/Tool_TauConstituentSelector.h"
#include "PanTauAlgs/HelperFunctions.h"

PanTau::Tool_TauConstituentSelector::Tool_TauConstituentSelector(const std::string& name) :
  asg::AsgTool(name)
{}


PanTau::Tool_TauConstituentSelector::~Tool_TauConstituentSelector() = default;


StatusCode PanTau::Tool_TauConstituentSelector::initialize() {

  ATH_MSG_INFO(" initialize()");
  m_init=true;
    
  ATH_CHECK( HelperFunctions::bindToolHandle( m_Tool_InformationStore, m_Tool_InformationStoreName ) );
  ATH_CHECK( m_Tool_InformationStore.retrieve() );
    
  ATH_CHECK( m_Tool_InformationStore->getInfo_Double("TauConstituents_MaxEta", m_MaxEta) );
    
  //eta bin edges
  ATH_CHECK( m_Tool_InformationStore->getInfo_VecDouble("Common_BinEdges_Eta", m_BinEdges_Eta) );
    
  //et cuts for types used in mode reco
  ATH_CHECK( m_Tool_InformationStore->getInfo_VecDouble("TauConstituents_Selection_Neutral_EtaBinned_EtCut", m_Selection_Neutral_EtaBinned_EtCut) );
    
  return StatusCode::SUCCESS;
} 


double PanTau::Tool_TauConstituentSelector::getEtCut(double eta, PanTau::TauConstituent::Type constituentType) const {
    
  for (unsigned int iEtaBin=0; iEtaBin<m_BinEdges_Eta.size()-1; iEtaBin++) {
    if (m_BinEdges_Eta[iEtaBin] <= eta && eta < m_BinEdges_Eta[iEtaBin+1]) {
      switch(constituentType) {
      case PanTau::TauConstituent::t_Neutral:  return m_Selection_Neutral_EtaBinned_EtCut[iEtaBin];
      default:
	return 9999999.;
      }
    }
  }
    
  ATH_MSG_WARNING("Eta value of " << eta << " could not be matched to any eta bin!");
  return 9999999.;
}


/**
 * Function to further select PFOs of the various categories (basically apply additional ET cuts): 
 * 
 */
StatusCode PanTau::Tool_TauConstituentSelector::SelectTauConstituents(const std::vector<TauConstituent*>& inputList,
								      std::vector<TauConstituent*>& outputList) const {    

  for (unsigned int iConst=0; iConst<inputList.size(); iConst++) {

    PanTau::TauConstituent* curConstituent = inputList[iConst];
        
    //general preselection:
    double curEta = std::abs( curConstituent->p4().Eta() );
    if (curEta > m_MaxEta) {
      ATH_MSG_DEBUG("\tNot using constituent with eta of " << curEta);
      continue;
    }
        
    bool passesSelection = false;

    // check if constituent is charged:
    if (curConstituent->isOfType(PanTau::TauConstituent::t_Charged)) {
      // we want to use all tracks
      passesSelection = true;

      // check if constituent is neutral, assign correctly pi0neut and neut flags:
    } else if (curConstituent->isOfType(PanTau::TauConstituent::t_Neutral)) {
      passesSelection = passesSelection_NeutralConstituent(curConstituent);
            
      //special treatment for the testing neutral flags
      if (!passesSelection) {
	curConstituent->removeTypeFlag(PanTau::TauConstituent::t_Neutral);
	curConstituent->removeTypeFlag(PanTau::TauConstituent::t_Pi0Neut);
      }            
    } else {
      ATH_MSG_DEBUG("Unhandled constituent type (" << curConstituent->getTypeNameString() 
		      << ") when trying to apply constituent selection - constituent will not be selected!");
      passesSelection = false;
    }
        
    if (!passesSelection) continue;
        
    outputList.push_back(inputList[iConst]);
  }
    
  return StatusCode::SUCCESS;
}


bool PanTau::Tool_TauConstituentSelector::passesSelection_NeutralConstituent(PanTau::TauConstituent* tauConstituent) const {

  TLorentzVector tlv_Constituent = tauConstituent->p4();

  if (tlv_Constituent.Et() < getEtCut(std::abs(tlv_Constituent.Eta()), PanTau::TauConstituent::t_Neutral)) {
    ATH_MSG_DEBUG("\tNot using constituent at eta " << tlv_Constituent.Eta() << " with et of " << tlv_Constituent.Et());
    return false;
  }

  return true;
}

