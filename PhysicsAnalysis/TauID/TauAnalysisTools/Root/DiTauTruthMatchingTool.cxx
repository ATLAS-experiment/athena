/**
 * @copyright Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
 *
 * @file DiTauTruthMatchingTool.cxx
 * @brief Class for ditau truth matching
 * @date 2021-02-18
 * 
 */

// Local include(s)
#include "TauAnalysisTools/DiTauTruthMatchingTool.h"

// Core include(s):
#include "AthLinks/ElementLink.h"
#include "TruthUtils/HepMCHelpers.h"
#include "AthContainers/ConstAccessor.h"
#include "AthContainers/Decorator.h"

// EDM include(s):
#include "xAODEgamma/ElectronContainer.h"
#include "xAODMuon/MuonContainer.h"

#include "MCTruthClassifier/MCTruthClassifier.h"

using namespace TauAnalysisTools;

//=================================PUBLIC-PART==================================
//______________________________________________________________________________
DiTauTruthMatchingTool::DiTauTruthMatchingTool( const std::string& name )
  : BuildTruthTaus(name)
  , m_accPtVis("pt_vis")
  , m_accEtaVis("eta_vis")
  , m_accPhiVis("phi_vis")
  , m_accMVis("m_vis")
{
}

//______________________________________________________________________________
DiTauTruthMatchingTool::~DiTauTruthMatchingTool( )
{

}

//______________________________________________________________________________
StatusCode DiTauTruthMatchingTool::initialize()
{
  ATH_MSG_INFO( "Initializing DiTauTruthMatchingTool" );

  // configure BuildTruthTaus in truth matching mode, not truth tau building mode
  DiTauTruthMatchingTool::BuildTruthTaus::setTruthMatchingMode();

  if (DiTauTruthMatchingTool::BuildTruthTaus::initialize().isFailure())
    {
      ATH_MSG_FATAL("Failed initializing BuildTruthTaus");
      return StatusCode::FAILURE;
    }
  return StatusCode::SUCCESS;
}

//______________________________________________________________________________
void DiTauTruthMatchingTool::getTruth(const xAOD::DiTauJet& xDiTau)
{
  if (retrieveTruthTaus().isFailure())
    return;

  if (findTruthTau(xDiTau).isFailure())
    ATH_MSG_WARNING("There was a failure in finding the matched truth tau");

  return;
}

//______________________________________________________________________________
void DiTauTruthMatchingTool::getTruth(const std::vector<const xAOD::DiTauJet*>& vDiTaus)
{
  for (auto xDiTau : vDiTaus)
    getTruth(*xDiTau);
  return;
}


////////////////////////////////////////////////////////////////////////////////
//                                 Private Part                               //
////////////////////////////////////////////////////////////////////////////////

//______________________________________________________________________________
StatusCode DiTauTruthMatchingTool::findTruthTau(const xAOD::DiTauJet& xDiTau)
{
  // check if decorations were already added to the first passed tau
  if (!m_bIsTruthMatchedAvailable.isValid()) {
    static const SG::ConstAccessor<char> accIsTruthMatched("IsTruthMatched");
    m_bIsTruthMatchedAvailable.set (accIsTruthMatched.isAvailable(xDiTau));
  }
  if (*m_bIsTruthMatchedAvailable.ptr())
    return StatusCode::SUCCESS;

  if (m_bTruthTauAvailable)
    return checkTruthMatch(xDiTau, *m_truthTausEvent.m_xTruthTauContainerConst);
  else
    return checkTruthMatch(xDiTau, *m_truthTausEvent.m_xTruthTauContainer);
}

//______________________________________________________________________________
StatusCode DiTauTruthMatchingTool::checkTruthMatch (const xAOD::DiTauJet& xDiTau, const xAOD::TruthParticleContainer& xTruthTauContainer) const
{
  std::vector<const xAOD::TruthParticle*> vTruthMatch;
  std::vector<TruthMatchedParticleType> vTruthMatchedParticleType;

  xAOD::TruthParticleContainer xRemainingTruthTaus = xTruthTauContainer;

  static const SG::Decorator<char> decIsTruthMatched("IsTruthMatched");
  static const SG::Decorator<char> decIsTruthHadronic("IsTruthHadronic");
  static const SG::Decorator<char> decIsTruthHadMu("IsTruthHadMu");
  static const SG::Decorator<char> decIsTruthHadEl("IsTruthHadEl");
  static const SG::ConstAccessor<int> accNSubjets("n_subjets");
  static const SG::ConstAccessor<char> accIsTruthHadronic("IsTruthHadronic");

  // set default values for each subjet
  for (int i = 0; i < accNSubjets(xDiTau); ++i)
    {
      const xAOD::TruthParticle* xTruthMatch = nullptr;
      TruthMatchedParticleType eTruthMatchedParticleType = Unknown;

      vTruthMatch.push_back(xTruthMatch);
      vTruthMatchedParticleType.push_back(eTruthMatchedParticleType);
    }

  // truthmatching for subjets:
  for (int i = 0; i < accNSubjets(xDiTau); ++i)
    {
      TLorentzVector vSubjetTLV;
      vSubjetTLV.SetPtEtaPhiE(xDiTau.subjetPt(i),
			      xDiTau.subjetEta(i),
			      xDiTau.subjetPhi(i),
			      xDiTau.subjetE(i));
      if ( truthMatch(vSubjetTLV,
		      xRemainingTruthTaus,
		      vTruthMatch.at(i),
		      vTruthMatchedParticleType.at(i)).isFailure() )
	{
	  ATH_MSG_WARNING("There was a failure in matching truth taus with subjet " << i);
	  return StatusCode::FAILURE;
	}
      if (vTruthMatch.at(i) && vTruthMatchedParticleType.at(i) == TruthHadronicTau)
	{
	  xRemainingTruthTaus.erase( std::find(xRemainingTruthTaus.begin(),
					       xRemainingTruthTaus.end(),
					       vTruthMatch.at(i)) );
	}
    }
  
  bool bTruthMatched = true;

  // create link to the original TruthParticle
  std::vector< ElementLink < xAOD::TruthParticleContainer > > vTruthLinks;
  for (int i = 0; i < accNSubjets(xDiTau); ++i)
    {
      const xAOD::TruthParticle* xTruthMatch = vTruthMatch.at(i);
      TruthMatchedParticleType eTruthMatchedParticleType = vTruthMatchedParticleType.at(i);
      if (xTruthMatch)
	{
	  if (eTruthMatchedParticleType == TruthHadronicTau)
	    {
	      ElementLink < xAOD::TruthParticleContainer > lTruthParticleLink(xTruthMatch, xTruthTauContainer);
	      vTruthLinks.push_back(lTruthParticleLink);  
	    }
	  else if (eTruthMatchedParticleType == TruthMuon)
	    {
	      ElementLink <xAOD::TruthParticleContainer> lTruthParticleLink(xTruthMatch, *m_truthTausEvent.m_xTruthMuonContainerConst);
	      vTruthLinks.push_back(lTruthParticleLink);
	    }
	  else if (eTruthMatchedParticleType  == TruthElectron)
	    {
	      ElementLink <xAOD::TruthParticleContainer> lTruthParticleLink(xTruthMatch, *m_truthTausEvent.m_xTruthElectronContainerConst);
	      vTruthLinks.push_back(lTruthParticleLink);
	    }
	}
      else
	{
	  ElementLink < xAOD::TruthParticleContainer > lTruthParticleLink;
	  vTruthLinks.push_back(lTruthParticleLink);  

	  // ditau is not truth matched if one of the two leading subjets is not truth matched
	  if (i == 0 || i == 1) bTruthMatched = false;
	}
    }

  static const SG::Decorator<std::vector<ElementLink<xAOD::TruthParticleContainer>>>
    decTruthParticleLinks ("truthParticleLinks");
  decTruthParticleLinks(xDiTau) = vTruthLinks;
  if (!m_bTruthTauAvailable)
    {
      static const SG::Decorator<std::vector<ElementLink<xAOD::TruthParticleContainer>>>
        decTruthTaus ("TruthTaus");
      decTruthTaus(xDiTau) = vTruthLinks;
    }
  
  ElementLink<xAOD::TruthParticleContainer> lTruthLeptonLink;
  static const SG::Decorator<unsigned int> decClassifierParticleType("classifierParticleTypeTruthLepton");
  static const SG::Decorator<unsigned int> decClassifierParticleOrigin("classifierParticleOriginTruthLepton");
  static const SG::Decorator<ElementLink<xAOD::TruthParticleContainer>> decTruthLeptonLink("truthLeptonLink");
  
  int mcTruthType = MCTruthPartClassifier::ParticleType::Unknown;
  int mcTruthOrigin = MCTruthPartClassifier::ParticleOrigin::NonDefined;
  static const SG::ConstAccessor<int> accTruthType("truthType");
  static const SG::ConstAccessor<int> accTruthOrigin("truthOrigin");
  static const SG::ConstAccessor<ElementLink<xAOD::ElectronContainer>> accElLink("elLink");
  static const SG::ConstAccessor<ElementLink<xAOD::MuonContainer>> accMuLink("muonLink");
  if(accElLink.isAvailable(xDiTau) && accMuLink.isAvailable(xDiTau))
    ATH_MSG_ERROR("Links to reco electron and reco muon available for one ditau candidate.");
  if(accElLink.isAvailable(xDiTau)){
    const xAOD::Electron* pElectron = *accElLink(xDiTau);
    if ((accTruthType.isAvailable(*pElectron) && accTruthOrigin.isAvailable(*pElectron)))
      {
	mcTruthType = accTruthType(*pElectron);
	mcTruthOrigin = accTruthOrigin(*pElectron);
      }
    lTruthLeptonLink = checkTruthLepton(pElectron);
  }
  if(accMuLink.isAvailable(xDiTau)){
    const xAOD::Muon* pMuon = *accMuLink(xDiTau);
    if (accTruthType.isAvailable(*pMuon) && accTruthOrigin.isAvailable(*pMuon))
      {
	mcTruthType = accTruthType(*pMuon);
	mcTruthOrigin = accTruthOrigin(*pMuon);
      }
    lTruthLeptonLink = checkTruthLepton(pMuon);
  }

  decIsTruthHadEl(xDiTau) = (char)(mcTruthType == MCTruthPartClassifier::ParticleType::IsoElectron && accNSubjets(xDiTau) != 0 && vTruthMatchedParticleType[0] == TruthHadronicTau);
  decIsTruthHadMu(xDiTau) = (char)(mcTruthType == MCTruthPartClassifier::ParticleType::IsoMuon && accNSubjets(xDiTau) != 0 && vTruthMatchedParticleType[0] == TruthHadronicTau);
  decClassifierParticleType(xDiTau) = mcTruthType;
  decClassifierParticleOrigin(xDiTau) = mcTruthOrigin;
  decTruthLeptonLink(xDiTau) = lTruthLeptonLink;

  static const SG::Decorator<float> decTruthLeadPt("TruthVisLeadPt");
  static const SG::Decorator<float> decTruthLeadEta("TruthVisLeadEta");
  static const SG::Decorator<float> decTruthLeadPhi("TruthVisLeadPhi");
  static const SG::Decorator<float> decTruthLeadM("TruthVisLeadM");
  static const SG::Decorator<float> decTruthLeadPdgID("TruthLeadPdgID");
  static const SG::Decorator<float> decTruthSubleadPt("TruthVisSubleadPt");
  static const SG::Decorator<float> decTruthSubleadEta("TruthVisSubleadEta");
  static const SG::Decorator<float> decTruthSubleadPhi("TruthVisSubleadPhi");
  static const SG::Decorator<float> decTruthSubleadM("TruthVisSubleadM");
  static const SG::Decorator<float> decTruthSubleadPdgID("TruthSubleadPdgID");
  static const SG::Decorator<float> decTruthDeltaR("TruthVisDeltaR");
  static const SG::Decorator<float> decTruthMass("TruthVisMass");

  // the ditau candidate should have at least 2 subjets to be truth matched
  if ( accNSubjets(xDiTau) < 2) {
    decIsTruthMatched(xDiTau) = (char)false;
    decIsTruthHadronic(xDiTau) = (char)false;
    decTruthLeadPt(xDiTau) = -1234.;
    decTruthLeadEta(xDiTau) = -1234.;
    decTruthLeadPhi(xDiTau) = -1234.;
    decTruthLeadM(xDiTau) = -1234.;
    decTruthLeadPdgID(xDiTau) = -1234.;
    decTruthSubleadPt(xDiTau) = -1234.;
    decTruthSubleadEta(xDiTau) = -1234.;
    decTruthSubleadPhi(xDiTau) = -1234.;
    decTruthSubleadM(xDiTau) = -1234.;
    decTruthSubleadPdgID(xDiTau) = -1234.;
    decTruthDeltaR(xDiTau) = -1234.;
    decTruthMass(xDiTau) = -1234.;
    return StatusCode::SUCCESS;
  }

  decIsTruthMatched(xDiTau) = (char)bTruthMatched;
  if (bTruthMatched)
    {
      // ditau is hadronic if two leading subjets are truth matched with hadronic decay
      decIsTruthHadronic(xDiTau) = (char)(vTruthMatchedParticleType[0]==TruthHadronicTau
					  && vTruthMatchedParticleType[1]==TruthHadronicTau );
    }
  else 
    decIsTruthHadronic(xDiTau) = (char)false;

  if (accIsTruthHadronic(xDiTau))
    {
      TLorentzVector tlvTruthTau1;
      TLorentzVector tlvTruthTau2;
      tlvTruthTau1.SetPtEtaPhiM(m_accPtVis(*(*vTruthLinks.at(0))),
				m_accEtaVis(*(*vTruthLinks.at(0))),
				m_accPhiVis(*(*vTruthLinks.at(0))),
				m_accMVis(*(*vTruthLinks.at(0))));
      tlvTruthTau2.SetPtEtaPhiM(m_accPtVis(*(*vTruthLinks.at(1))),
				m_accEtaVis(*(*vTruthLinks.at(1))),
				m_accPhiVis(*(*vTruthLinks.at(1))),
				m_accMVis(*(*vTruthLinks.at(1))));

      decTruthLeadPt(xDiTau) = std::max(tlvTruthTau1.Pt(), tlvTruthTau2.Pt());
      decTruthLeadEta(xDiTau) = (tlvTruthTau1.Pt() > tlvTruthTau2.Pt()) ? tlvTruthTau1.Eta() : tlvTruthTau2.Eta();
      decTruthLeadPhi(xDiTau) = (tlvTruthTau1.Pt() > tlvTruthTau2.Pt()) ? tlvTruthTau1.Phi() : tlvTruthTau2.Phi();
      decTruthLeadM(xDiTau) = (tlvTruthTau1.Pt() > tlvTruthTau2.Pt()) ? tlvTruthTau1.M() : tlvTruthTau2.M();
      decTruthLeadPdgID(xDiTau) = (*vTruthLinks.at(0))->pdgId();
      decTruthSubleadPt(xDiTau) = std::min(tlvTruthTau1.Pt(), tlvTruthTau2.Pt());
      decTruthSubleadEta(xDiTau) = (tlvTruthTau1.Pt() > tlvTruthTau2.Pt()) ? tlvTruthTau2.Eta() : tlvTruthTau1.Eta();
      decTruthSubleadPhi(xDiTau) = (tlvTruthTau1.Pt() > tlvTruthTau2.Pt()) ? tlvTruthTau2.Phi() : tlvTruthTau1.Phi();
      decTruthSubleadM(xDiTau) = (tlvTruthTau1.Pt() > tlvTruthTau2.Pt()) ? tlvTruthTau2.M() : tlvTruthTau1.M();
      decTruthSubleadPdgID(xDiTau) = (*vTruthLinks.at(1))->pdgId();
      decTruthDeltaR(xDiTau) = tlvTruthTau1.DeltaR(tlvTruthTau2);
      decTruthMass(xDiTau) = (tlvTruthTau1 + tlvTruthTau2).M();
    }
  else {
      // set to a default value 	  
      decTruthLeadPt(xDiTau) = -1234.; 
      decTruthLeadEta(xDiTau) = -1234.;
      decTruthLeadPhi(xDiTau) = -1234.;
      decTruthLeadM(xDiTau) = -1234.;
      decTruthLeadPdgID(xDiTau) = -1234.;
      decTruthSubleadPt(xDiTau) = -1234.;
      decTruthSubleadEta(xDiTau) = -1234.; 
      decTruthSubleadPhi(xDiTau) = -1234.;
      decTruthSubleadM(xDiTau) = -1234.;
      decTruthSubleadPdgID(xDiTau) = -1234.;
      decTruthDeltaR(xDiTau) = -1234.;
      decTruthMass(xDiTau) = -1234.; 
    }	  
  
  return StatusCode::SUCCESS;
}

//______________________________________________________________________________
ElementLink<xAOD::TruthParticleContainer> DiTauTruthMatchingTool::checkTruthLepton(const xAOD::IParticle* pLepton) const {
  ElementLink<xAOD::TruthParticleContainer> truthParticleLink;
  static const SG::ConstAccessor<ElementLink<xAOD::TruthParticleContainer>> accTruthParticleLink("truthParticleLink");
  if(!accTruthParticleLink.isAvailable(*pLepton)){
    return truthParticleLink;
  }
  truthParticleLink = accTruthParticleLink(*pLepton);
  return truthParticleLink;
}

//______________________________________________________________________________
StatusCode DiTauTruthMatchingTool::truthMatch(const TLorentzVector& vSubjetTLV,
                                              const xAOD::TruthParticleContainer& xTruthTauContainer,
                                              const xAOD::TruthParticle* &xTruthMatch,
                                              TruthMatchedParticleType &eTruthMatchedParticleType) const
{
  for (auto xTruthTauIt : xTruthTauContainer)
    {
      TLorentzVector vTruthVisTLV;
      vTruthVisTLV.SetPtEtaPhiM(m_accPtVis(*xTruthTauIt),
				m_accEtaVis(*xTruthTauIt),
				m_accPhiVis(*xTruthTauIt),
				m_accMVis(*xTruthTauIt));
      if (vSubjetTLV.DeltaR(vTruthVisTLV) <= m_dMaxDeltaR)
	{
	  static const SG::ConstAccessor<char> accIsHadronicTau("IsHadronicTau");
	  if ((bool)accIsHadronicTau(*xTruthTauIt))
	    eTruthMatchedParticleType = TruthHadronicTau;
	  else
	    continue; // don't let leptonic taus steal truthmatch just by chance

	  xTruthMatch = xTruthTauIt;
	  break;
	}
    }

  if (!xTruthMatch and m_truthTausEvent.m_xTruthMuonContainerConst)
    {
      double dPtMax = 0.;
      for (auto xTruthMuonIt : *m_truthTausEvent.m_xTruthMuonContainerConst)
	{
	  if (vSubjetTLV.DeltaR(xTruthMuonIt->p4()) <= m_dMaxDeltaR)
	    {
	      if (xTruthMuonIt->pt()<dPtMax)
		continue;
	      eTruthMatchedParticleType = TruthMuon;

	      xTruthMatch = xTruthMuonIt;
	      dPtMax = xTruthMuonIt->pt();
	    }
	}
    }

  if (!xTruthMatch and m_truthTausEvent.m_xTruthElectronContainerConst)
    {
      double dPtMax = 0.;
      for (auto xTruthElectronIt : *m_truthTausEvent.m_xTruthElectronContainerConst)
	{
	  if (vSubjetTLV.DeltaR(xTruthElectronIt->p4()) <= m_dMaxDeltaR)
	    {
	      if (xTruthElectronIt->pt()<dPtMax)
		continue;
	      eTruthMatchedParticleType = TruthElectron;
	      xTruthMatch = xTruthElectronIt;
	      dPtMax = xTruthElectronIt->pt();
	    }
	}
    }

  return StatusCode::SUCCESS;
}
