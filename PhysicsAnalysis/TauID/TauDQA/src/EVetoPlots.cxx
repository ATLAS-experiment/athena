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
	
EVetoPlots::~EVetoPlots()
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
}

  void EVetoPlots::fill(const xAOD::TauJet& tau, float weight) {

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
}

}
