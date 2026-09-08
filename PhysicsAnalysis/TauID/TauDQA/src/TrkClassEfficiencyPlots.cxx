/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TrkClassEfficiencyPlots.h"
#include "AthenaKernel/Units.h"

namespace Tau{

  TrkClassEfficiencyPlots::TrkClassEfficiencyPlots(PlotBase* pParent, const std::string& sDir, std::string sTauJetContainerName):
    PlotBase(pParent, sDir),
    m_sTauJetContainerName(std::move(sTauJetContainerName))	
  {	
  }

  void TrkClassEfficiencyPlots::initializePlots()
  {
    m_eff_truth_pt_1p       = BookTProfile("TrkClassEff_TruthPt_1P"," Matched 1P Truth-Reco taus TrkClass eff in truth pt; truth pt; eff", 26, 0., 260.0);
    m_eff_truth_eta_1p      = BookTProfile("TrkClassEff_TruthEta_1P"," Matched 1P Truth-Reco taus TrkClass eff in truth eta; truth eta; eff", 10, -2.5,2.5);
    m_eff_mu_1p             = BookTProfile("TrkClassEff_Mu_1P", "Matched 1P Truth-Reco taus TrkClass eff in mu; mu; eff", 8, 10, 90);

    m_eff_truth_pt_3p       = BookTProfile("TrkClassEff_TruthPt_3P"," Matched 3P Truth-Reco taus TrkClass eff in truth pt; truth pt; eff", 26, 0., 260.0);
    m_eff_truth_eta_3p      = BookTProfile("TrkClassEff_TruthEta_3P"," Matched 3P Truth-Reco taus TrkClass eff in truth eta; truth eta; eff", 10, -2.5,2.5);
    m_eff_mu_3p             = BookTProfile("TrkClassEff_Mu_3P", "Matched 3P Truth-Reco taus TrkClass eff in mu; mu; eff", 8, 10, 90);
  }

  void TrkClassEfficiencyPlots::fill(std::vector<const xAOD::TruthParticle*> & truth_taus, std::vector<const xAOD::TauJet*> & reco_taus, float weight, float avg_mu)
  {

    static const SG::ConstAccessor<double> acc_ptvis("pt_vis");
    static const SG::ConstAccessor<double> acc_etavis("eta_vis");
    static const SG::ConstAccessor<char> accIsHadronicTau ("IsHadronicTau");
    static const SG::ConstAccessor<unsigned long> acc_ntracks("numChargedPion");

    for (auto reco_tau : reco_taus) {

      for (auto truth_tau : truth_taus) {
        // check that truth tau is not a null pointer 
        if(truth_tau == nullptr){ continue; }
        // place some minimum kinematic requirements on the truth taus
        if((acc_ptvis(*truth_tau) < 15 * Athena::Units::GeV) || (std::abs(acc_etavis(*truth_tau)) > 2.5)){ continue; }
        // select only truth hadronic taus
        if(!(accIsHadronicTau(*truth_tau))) { continue; }	      

	if(truth_tau->p4().DeltaR(reco_tau->p4()) < 0.2) { 
           bool track_num_match = false;
	   if( acc_ntracks(*truth_tau) == 1){
              if( reco_tau->nTracks() == 1 ) { track_num_match = true; }
              else { track_num_match = false;}
             
	      m_eff_truth_pt_1p->Fill(acc_ptvis(*truth_tau)/Athena::Units::GeV,track_num_match,weight);
              m_eff_truth_eta_1p->Fill(acc_etavis(*truth_tau),track_num_match,weight);
              m_eff_mu_1p->Fill(avg_mu,track_num_match,weight);

   	   } else if ( acc_ntracks(*truth_tau) == 3){
  
              if( reco_tau->nTracks() == 3 ) { track_num_match = true; }
              else { track_num_match = false;}

              m_eff_truth_pt_3p->Fill(acc_ptvis(*truth_tau)/Athena::Units::GeV,track_num_match,weight);
              m_eff_truth_eta_3p->Fill(acc_etavis(*truth_tau),track_num_match,weight);
              m_eff_mu_3p->Fill(avg_mu,track_num_match,weight);
    	   }
	   break;
        }
      }
    }   
  }
}
