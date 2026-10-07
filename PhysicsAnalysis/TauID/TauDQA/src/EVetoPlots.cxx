/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "EVetoPlots.h"
#include "AthenaKernel/Units.h"

namespace Tau{

EVetoPlots::EVetoPlots(PlotBase* pParent, const std::string& sDir, std::string sTauJetContainerName):
   PlotBase(pParent, sDir),
   m_sTauJetContainerName(std::move(sTauJetContainerName))
{
}
	
void EVetoPlots::initializePlots(){

  m_id_RNNEleScore         = Book1D("id_RNNEleScore",m_sTauJetContainerName + " RNNEleScore ; RNNEleScore; # Tau",20,0.,1.00);
  m_id_RNNEleScoreSigTrans = Book1D("id_RNNEleScoreSigTrans",m_sTauJetContainerName + " RNNEleScoreSigTrans ; RNNEleScoreSigTrans; # Tau",20,0.,1.00);
  m_pt_eleRNNloose      = Book1D("Pt_eleRNNloose",m_sTauJetContainerName + " Tau pt; pt; # Taus",20,0.,150.);
  m_pt_eleRNNmed        = Book1D("Pt_eleRNNmed",m_sTauJetContainerName + " Tau pt; pt; # Taus",20,0.,150.);
  m_pt_eleRNNtight      = Book1D("Pt_eleRNNtight",m_sTauJetContainerName + " Tau pt; pt; # Taus",20,0.,150.);
  m_pt_eleRNNlooseHighPt = Book1D("Pt_eleRNNlooseHighPt",m_sTauJetContainerName + " Tau pt; pt; # Taus",20,0.,1500.);
  m_pt_eleRNNmedHighPt   = Book1D("Pt_eleRNNmedHighPt",m_sTauJetContainerName + " Tau pt; pt; # Taus",20,0.,1500.);
  m_pt_eleRNNtightHighPt = Book1D("Pt_eleRNNtightHighPt",m_sTauJetContainerName + " Tau pt; pt; # Taus",20,0.,1500.);

  m_id_GNNEleScore         = Book1D("id_GNNEleScore",m_sTauJetContainerName + " GNNEleScore ; GNNEleScore; # Tau",75,0.,15.);
  m_id_GNNEleScoreSigTrans = Book1D("id_GNNEleScoreSigTrans",m_sTauJetContainerName + " GNNEleScoreSigTrans ; GNNEleScoreSigTrans; # Tau",20,0.,1.00);
  m_pt_eleGNNloose      = Book1D("Pt_eleGNNloose",m_sTauJetContainerName + " Tau pt; pt; # Taus",20,0.,150.);
  m_pt_eleGNNmed        = Book1D("Pt_eleGNNmed",m_sTauJetContainerName + " Tau pt; pt; # Taus",20,0.,150.);
  m_pt_eleGNNtight      = Book1D("Pt_eleGNNtight",m_sTauJetContainerName + " Tau pt; pt; # Taus",20,0.,150.);
  m_pt_eleGNNlooseHighPt = Book1D("Pt_eleGNNlooseHighPt",m_sTauJetContainerName + " Tau pt; pt; # Taus",20,0.,1500.);
  m_pt_eleGNNmedHighPt   = Book1D("Pt_eleGNNmedHighPt",m_sTauJetContainerName + " Tau pt; pt; # Taus",20,0.,1500.);
  m_pt_eleGNNtightHighPt = Book1D("Pt_eleGNNtightHighPt",m_sTauJetContainerName + " Tau pt; pt; # Taus",20,0.,1500.);

}

  void EVetoPlots::fill(const xAOD::TauJet& tau, float weight) {

  // RNN section	  
  static const SG::ConstAccessor<float> RNNEleScoreAcc("RNNEleScore");
  if(RNNEleScoreAcc.isAvailable(tau)) {
    m_id_RNNEleScore->Fill(tau.discriminant(xAOD::TauJetParameters::RNNEleScore), weight);

    static const SG::ConstAccessor<float> RNNEleScoreSigTrans_v1Acc("RNNEleScoreSigTrans_v1");
    if(RNNEleScoreSigTrans_v1Acc.isAvailable(tau)) {
       m_id_RNNEleScoreSigTrans->Fill(RNNEleScoreSigTrans_v1Acc(tau), weight);	    
    }  

    static const SG::ConstAccessor<char> acc_RNNEleLoose("EleRNNLoose_v1");
    if(acc_RNNEleLoose.isAvailable(tau) && acc_RNNEleLoose(tau)){ 
      m_pt_eleRNNloose->Fill(tau.pt()/Athena::Units::GeV, weight);
      m_pt_eleRNNlooseHighPt->Fill(tau.pt()/Athena::Units::GeV, weight);
    }

    static const SG::ConstAccessor<char> acc_RNNEleMedium("EleRNNMedium_v1");
    if(acc_RNNEleMedium.isAvailable(tau) && acc_RNNEleMedium(tau)){
      m_pt_eleRNNmed->Fill(tau.pt()/Athena::Units::GeV, weight);
      m_pt_eleRNNmedHighPt->Fill(tau.pt()/Athena::Units::GeV, weight);
    }

    static const SG::ConstAccessor<char> acc_RNNEleTight("EleRNNTight_v1");
    if(acc_RNNEleTight.isAvailable(tau) && acc_RNNEleTight(tau)){
      m_pt_eleRNNtight->Fill(tau.pt()/Athena::Units::GeV, weight);
      m_pt_eleRNNtightHighPt->Fill(tau.pt()/Athena::Units::GeV, weight);
    }
  }

  // GNN section
  static const SG::ConstAccessor<float> GNNEleScoreAcc("TauGNNeVetoScore");
  if(GNNEleScoreAcc.isAvailable(tau)) {
    m_id_GNNEleScore->Fill(GNNEleScoreAcc(tau), weight);

    static const SG::ConstAccessor<float> GNNEleScoreSigTrans_v1Acc("TauGNNeVetoSigTrans");
    m_id_GNNEleScoreSigTrans->Fill(GNNEleScoreSigTrans_v1Acc(tau), weight);

    static const SG::ConstAccessor<char> acc_GNNEleLoose("TauGNNeVeto_L");
    if(acc_GNNEleLoose(tau)){
      m_pt_eleGNNloose->Fill(tau.pt()/Athena::Units::GeV, weight);
      m_pt_eleGNNlooseHighPt->Fill(tau.pt()/Athena::Units::GeV, weight);
    }

    static const SG::ConstAccessor<char> acc_GNNEleMedium("TauGNNeVeto_M");
    if(acc_GNNEleMedium(tau)){
      m_pt_eleGNNmed->Fill(tau.pt()/Athena::Units::GeV, weight);
      m_pt_eleGNNmedHighPt->Fill(tau.pt()/Athena::Units::GeV, weight);
    }

    static const SG::ConstAccessor<char> acc_GNNEleTight("TauGNNeVeto_T");
    if(acc_GNNEleTight(tau)){
      m_pt_eleGNNtight->Fill(tau.pt()/Athena::Units::GeV, weight);
      m_pt_eleGNNtightHighPt->Fill(tau.pt()/Athena::Units::GeV, weight);
    }
  }
}

}
