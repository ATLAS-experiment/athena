/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include <utility>

#include "EfficiencyPlots.h"
#include "AthenaKernel/Units.h"

namespace Tau{

  EfficiencyPlots::EfficiencyPlots(PlotBase* pParent, const std::string& sDir, std::string sTauJetContainerName):
    PlotBase(pParent, sDir),
    m_sTauJetContainerName(std::move(sTauJetContainerName))
  {	
  }

  EfficiencyPlots::~EfficiencyPlots()
  {
  }

  void EfficiencyPlots::initializePlots()
  {
    m_eff_pt_jetRNNloose       = BookTProfile("Eff_Pt_jetRNNloose"," Matched Tau loose RNN eff in pt; pt; eff",20, 0., 150.0);
    m_eff_pt_jetRNNmed         = BookTProfile("Eff_Pt_jetRNNmed","Matched Tau med RNN eff in pt; pt; eff", 20, 0.0, 150.0);
    m_eff_pt_jetRNNtight       = BookTProfile("Eff_Pt_jetRNNtight","Matched Tau tight RNN eff in pt; pt; eff", 20, 0.0, 150.0);
    m_eff_pt_jetRNNlooseHighPt = BookTProfile("Eff_Pt_jetRNNlooseHightPt"," Matched Tau loose RNN eff in pt; pt; eff", 20, 0.0, 1500.0);
    m_eff_pt_jetRNNmedHighPt   = BookTProfile("Eff_Pt_jetRNNmedHightPt","Matched Tau med RNN eff in pt; pt; eff", 20, 0.0, 1500.0);
    m_eff_pt_jetRNNtightHighPt = BookTProfile("Eff_Pt_jetRNNtightHightPt","Matched Tau tight RNN eff in pt; pt; eff", 20, 0.0, 1500.0);
    m_eff_jetRNNloose          = BookTProfile("Eff_jetRNNloose"," Matched Tau loose RNN eff total; bin; eff",3,-1.5,1.5);
    m_eff_jetRNNmed            = BookTProfile("Eff_jetRNNmed","Matched Tau med RNN eff total; bin; eff",3,-1.5,1.5);
    m_eff_jetRNNtight          = BookTProfile("Eff_jetRNNtight","Matched Tau tight RNN eff total; bin; eff",3,-1.5,1.5);

    m_eff_pt_jetGNTauloose       = BookTProfile("Eff_Pt_jetGNTauloose"," Matched Tau loose GNTau eff in pt; pt; eff", 20, 0., 150.0);
    m_eff_pt_jetGNTaumed         = BookTProfile("Eff_Pt_jetGNTaumed","Matched Tau med GNTau eff in pt; pt; eff", 20, 0.0, 150.0);
    m_eff_pt_jetGNTautight       = BookTProfile("Eff_Pt_jetGNTautight","Matched Tau tight GNTau eff in pt; pt; eff", 20, 0.0, 150.0);
    m_eff_pt_jetGNTaulooseHighPt = BookTProfile("Eff_Pt_jetGNTaulooseHightPt"," Matched Tau loose GNTau eff in pt; pt; eff", 20, 0.0, 1500.0);
    m_eff_pt_jetGNTaumedHighPt   = BookTProfile("Eff_Pt_jetGNTaumedHightPt","Matched Tau med GNTau eff in pt; pt; eff", 20, 0.0, 1500.0);
    m_eff_pt_jetGNTautightHighPt = BookTProfile("Eff_Pt_jetGNTautightHightPt","Matched Tau tight GNTau eff in pt; pt; eff", 20, 0.0, 1500.0);
    m_eff_jetGNTauloose          = BookTProfile("Eff_jetGNTauloose"," Matched Tau loose GNTau eff total; bin; eff",3,-1.5,1.5);
    m_eff_jetGNTaumed            = BookTProfile("Eff_jetGNTaumed","Matched Tau med GNTau eff total; bin; eff",3,-1.5,1.5);
    m_eff_jetGNTautight          = BookTProfile("Eff_jetGNTautight","Matched Tau tight GNTau eff total; bin; eff",3,-1.5,1.5);

    m_eff_pt_eVetoloose       = BookTProfile("Eff_Pt_eVetoloose"," loose eVeto eff in pt; pt; eff", 7, 0.0, 140.0);
    m_eff_pt_eVetomed         = BookTProfile("Eff_Pt_eVetomed"," med eVeto eff in pt; pt; eff", 7, 0.0, 140.0);
    m_eff_pt_eVetotight       = BookTProfile("Eff_Pt_eVetotight","tight eVeto eff in pt; pt; eff", 7, 0.0, 140.0);
    m_eff_pt_eVetolooseHighPt = BookTProfile("Eff_Pt_eVetolooseHightPt"," loose eVeto eff in pt; pt; eff", 20, 0.0, 1500.0);
    m_eff_pt_eVetomedHighPt   = BookTProfile("Eff_Pt_eVetomedHightPt","med eVeto eff in pt; pt; eff", 20, 0.0, 1500.0);
    m_eff_pt_eVetotightHighPt = BookTProfile("Eff_Pt_eVetotightHightPt","tight eVeto eff in pt; pt; eff", 20, 0.0, 1500.0);
    m_eff_eta_eVetoloose       = BookTProfile("Eff_Eta_eVetoloose"," loose eVeto eff in eta; eta; eff", 15, -3.0,3.0);
    m_eff_eta_eVetomed         = BookTProfile("Eff_Eta_eVetomed"," med eVeto eff in eta; eta; eff", 15, -3.0,3.0);
    m_eff_eta_eVetotight       = BookTProfile("Eff_Eta_eVetotight","tight eVeto eff in eta; eta; eff", 15, -3.0,3.0);
    m_eff_eVetoloose          = BookTProfile("Eff_eVetoloose"," loose eVeto eff total; bin; eff",3,-1.5,1.5);
    m_eff_eVetomed            = BookTProfile("Eff_eVetomed","med eVeto eff total; bin; eff",3,-1.5,1.5);
    m_eff_eVetotight          = BookTProfile("Eff_eVetotight","tight eVeto eff total; bin; eff",3,-1.5,1.5);

  }

  void EfficiencyPlots::fill(const xAOD::TauJet& tau, float weight)
  {
    if ( tau.isTau(xAOD::TauJetParameters::JetRNNSigLoose) ) {
      m_eff_pt_jetRNNloose      ->Fill(tau.pt()/Athena::Units::GeV, 1., weight);
      m_eff_pt_jetRNNlooseHighPt->Fill(tau.pt()/Athena::Units::GeV, 1., weight);
      m_eff_jetRNNloose         ->Fill(0., 1., weight);
    }
    else {
      m_eff_pt_jetRNNloose      ->Fill(tau.pt()/Athena::Units::GeV, 0., weight);
      m_eff_pt_jetRNNlooseHighPt->Fill(tau.pt()/Athena::Units::GeV, 0., weight);
      m_eff_jetRNNloose         ->Fill(0., 0., weight);
    }
   
    if ( tau.isTau(xAOD::TauJetParameters::JetRNNSigMedium) ) {
      m_eff_pt_jetRNNmed      ->Fill(tau.pt()/Athena::Units::GeV, 1., weight);
      m_eff_pt_jetRNNmedHighPt->Fill(tau.pt()/Athena::Units::GeV, 1., weight);
      m_eff_jetRNNmed         ->Fill(0., 1., weight);
    }
    else {
      m_eff_pt_jetRNNmed      ->Fill(tau.pt()/Athena::Units::GeV, 0., weight);
      m_eff_pt_jetRNNmedHighPt->Fill(tau.pt()/Athena::Units::GeV, 0., weight);
      m_eff_jetRNNmed         ->Fill(0., 0., weight);
    }

    if ( tau.isTau(xAOD::TauJetParameters::JetRNNSigTight) ) {
      m_eff_pt_jetRNNtight      ->Fill(tau.pt()/Athena::Units::GeV, 1., weight);
      m_eff_pt_jetRNNtightHighPt->Fill(tau.pt()/Athena::Units::GeV, 1., weight);
      m_eff_jetRNNtight         ->Fill(0., 1., weight);
    }
    else {
      m_eff_pt_jetRNNtight      ->Fill(tau.pt()/Athena::Units::GeV, 0., weight);
      m_eff_pt_jetRNNtightHighPt->Fill(tau.pt()/Athena::Units::GeV, 0., weight);
      m_eff_jetRNNtight         ->Fill(0., 0., weight);
    }


    static const SG::ConstAccessor<char> acc_GNTauL("GNTauL_v0prune");
    double pass_loose = acc_GNTauL.withDefault(tau,false);
    m_eff_pt_jetGNTauloose->Fill(tau.pt()/Athena::Units::GeV, pass_loose, weight);
    m_eff_pt_jetGNTaulooseHighPt->Fill(tau.pt()/Athena::Units::GeV, pass_loose, weight);
    m_eff_jetGNTauloose->Fill(0.,pass_loose , weight);


    static const SG::ConstAccessor<char> acc_GNTauM("GNTauM_v0prune");
    double pass_medium = acc_GNTauM.withDefault(tau,false);
    m_eff_pt_jetGNTaumed->Fill(tau.pt()/Athena::Units::GeV, pass_medium, weight);
    m_eff_pt_jetGNTaumedHighPt->Fill(tau.pt()/Athena::Units::GeV, pass_medium, weight);
    m_eff_jetGNTaumed->Fill(0., pass_medium, weight);

    static const SG::ConstAccessor<char> acc_GNTauT("GNTauT_v0prune");
    double pass_tight = acc_GNTauT.withDefault(tau,false);
    m_eff_pt_jetGNTautight->Fill(tau.pt()/Athena::Units::GeV, pass_tight, weight);
    m_eff_pt_jetGNTautightHighPt->Fill(tau.pt()/Athena::Units::GeV, pass_tight, weight);
    m_eff_jetGNTautight->Fill(0., pass_tight, weight);


    static const SG::ConstAccessor<char> acc_RNNEleLoose("EleRNNLoose_v1");
    double pass_eVeto_loose = acc_RNNEleLoose.withDefault(tau,false);
    m_eff_pt_eVetoloose->Fill(tau.pt()/Athena::Units::GeV, pass_eVeto_loose, weight);
    m_eff_pt_eVetolooseHighPt->Fill(tau.pt()/Athena::Units::GeV, pass_eVeto_loose, weight);
    m_eff_eVetoloose->Fill(0., pass_eVeto_loose, weight);
    m_eff_eta_eVetoloose->Fill(tau.eta(), pass_eVeto_loose, weight);


    static const SG::ConstAccessor<char> acc_RNNEleMedium("EleRNNMedium_v1");
    double pass_eVeto_medium = acc_RNNEleMedium.withDefault(tau,false);
    m_eff_pt_eVetomed->Fill(tau.pt()/Athena::Units::GeV, pass_eVeto_medium,  weight);
    m_eff_pt_eVetomedHighPt->Fill(tau.pt()/Athena::Units::GeV, pass_eVeto_medium, weight);
    m_eff_eVetomed->Fill(0., pass_eVeto_medium, weight);
    m_eff_eta_eVetomed->Fill(tau.eta(), pass_eVeto_medium, weight);

    static const SG::ConstAccessor<char> acc_RNNEleTight("EleRNNTight_v1");
    double pass_eVeto_tight = acc_RNNEleTight.withDefault(tau,false);
    m_eff_pt_eVetotight->Fill(tau.pt()/Athena::Units::GeV, pass_eVeto_tight,  weight);
    m_eff_pt_eVetotightHighPt->Fill(tau.pt()/Athena::Units::GeV, pass_eVeto_tight, weight);
    m_eff_eVetotight->Fill(0., pass_eVeto_tight, weight);
    m_eff_eta_eVetotight->Fill(tau.eta(), pass_eVeto_tight, weight);
    
  }
  

}
