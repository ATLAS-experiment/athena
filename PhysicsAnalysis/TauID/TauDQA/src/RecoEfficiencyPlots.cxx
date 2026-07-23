/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "RecoEfficiencyPlots.h"
#include "AthenaKernel/Units.h"

namespace Tau{

  RecoEfficiencyPlots::RecoEfficiencyPlots(PlotBase* pParent, const std::string& sDir, std::string sTauJetContainerName):
    PlotBase(pParent, sDir),
    m_sTauJetContainerName(std::move(sTauJetContainerName))	
  {	
  }

  void RecoEfficiencyPlots::initializePlots()
  {
    m_eff_truth_pt_all       = BookTProfile("RecoEff_TruthPt_All"," Matched Truth-Reco taus eff in truth pt; truth pt; eff", 26, 0., 260.0);
    m_eff_truth_eta_all      = BookTProfile("RecoEff_TruthEta_All"," Matched Truth-Reco taus eff in truth eta; truth eta; eff", 10, -2.5,2.5);
    m_eff_mu_all             = BookTProfile("RecoEff_Mu_All", "Matched Truth-Reco taus eff in mu; mu; eff", 8, 10, 90); 

    m_eff_truth_pt_1p       = BookTProfile("RecoEff_TruthPt_1P"," Matched 1P Truth-Reco taus eff in truth pt; truth pt; eff", 26, 0., 260.0);
    m_eff_truth_eta_1p      = BookTProfile("RecoEff_TruthEta_1P"," Matched 1P Truth-Reco taus eff in truth eta; truth eta; eff", 10, -2.5,2.5);
    m_eff_mu_1p             = BookTProfile("RecoEff_Mu_1P", "Matched 1P Truth-Reco taus eff in mu; mu; eff", 8, 10, 90);

    m_eff_truth_pt_3p       = BookTProfile("RecoEff_TruthPt_3P"," Matched 3P Truth-Reco taus eff in truth pt; truth pt; eff", 26, 0., 260.0);
    m_eff_truth_eta_3p      = BookTProfile("RecoEff_TruthEta_3P"," Matched 3P Truth-Reco taus eff in truth eta; truth eta; eff", 10, -2.5,2.5);
    m_eff_mu_3p             = BookTProfile("RecoEff_Mu_3P", "Matched 3P Truth-Reco taus eff in mu; mu; eff", 8, 10, 90);

  }

  void RecoEfficiencyPlots::fill(std::vector<const xAOD::TruthParticle*> & truth_taus, std::vector<const xAOD::TauJet*> & reco_taus, float weight, float avg_mu)
  {

    static const SG::ConstAccessor<double> acc_ptvis("pt_vis");
    static const SG::ConstAccessor<double> acc_etavis("eta_vis");
    static const SG::ConstAccessor<char> accIsHadronicTau ("IsHadronicTau");
    static const SG::ConstAccessor<unsigned long> acc_ntracks("numChargedPion");

    for (auto truth_tau : truth_taus) {

      // check that truth tau is not a null pointer 
      if(truth_tau == nullptr){ continue; }   	     
      // place some minimum kinematic requirements on the truth taus
      if((acc_ptvis(*truth_tau) < 15 * Athena::Units::GeV) || (std::abs(acc_etavis(*truth_tau)) > 2.5)){ continue; }
      // select only truth hadronic taus
      if(!(accIsHadronicTau(*truth_tau))) { continue; }

      bool isMatched = false;    
      for (auto reco_tau : reco_taus) { 
        if(truth_tau->p4().DeltaR(reco_tau->p4()) < 0.2) {
           isMatched = true;
	   break;
	}
      }
      m_eff_truth_pt_all->Fill(acc_ptvis(*truth_tau)/Athena::Units::GeV,isMatched,weight); 
      m_eff_truth_eta_all->Fill(acc_etavis(*truth_tau),isMatched,weight);
      m_eff_mu_all->Fill(avg_mu,isMatched,weight);

      if(acc_ntracks(*truth_tau) == 1){
        m_eff_truth_pt_1p->Fill(acc_ptvis(*truth_tau)/Athena::Units::GeV,isMatched,weight);
        m_eff_truth_eta_1p->Fill(acc_etavis(*truth_tau),isMatched,weight);
        m_eff_mu_1p->Fill(avg_mu,isMatched,weight);  
      }

      if(acc_ntracks(*truth_tau) == 3){
        m_eff_truth_pt_3p->Fill(acc_ptvis(*truth_tau)/Athena::Units::GeV,isMatched,weight);
        m_eff_truth_eta_3p->Fill(acc_etavis(*truth_tau),isMatched,weight);
        m_eff_mu_3p->Fill(avg_mu,isMatched,weight); 
      }
    } 
  }

}
