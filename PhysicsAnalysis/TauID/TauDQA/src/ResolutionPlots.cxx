/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include <utility>
#include "ResolutionPlots.h"
#include "AthContainers/ConstAccessor.h"
#include "TLorentzVector.h"
namespace Tau{

  ResolutionPlots::ResolutionPlots(PlotBase *pParent, const std::string& sDir, std::string sTauJetContainerName):
    PlotBase(pParent, sDir),
    m_sTauJetContainerName(std::move(sTauJetContainerName))
  {
  }
  
    ResolutionPlots::~ResolutionPlots()
    {
    }
  
  
  void ResolutionPlots::initializePlots(){
    m_ptResolution = Book1D("ptResolution",m_sTauJetContainerName + " pt Resolution; pt(TauReco)/pt(visTauTruth); # Part",100,0.5,1.5);
    m_etaResolution  = Book1D("etaResolution",m_sTauJetContainerName + " eta Resolution; eta(TauReco) - eta(visTauTruth); # Part",100,-0.1,0.1);
    m_phiResolution   = Book1D("phiResolution",m_sTauJetContainerName + " phi Resolution; phi(TauReco) - phi(visTauTruth); # Part",100,-0.1,0.1);
    m_chargeResolution = Book1D("chargeResolution",m_sTauJetContainerName + " charge Resolution; charge(TauReco) - charge(TauTruth); # Part",12,-6,6);
  }

  void ResolutionPlots::fill(const xAOD::TauJet& tau, const xAOD::TruthParticle& truthtau, float weight) {

    static const SG::ConstAccessor<double> acc_ptvis("pt_vis");
    static const SG::ConstAccessor<double> acc_etavis("eta_vis");
    static const SG::ConstAccessor<double> acc_phivis("phi_vis");
    static const SG::ConstAccessor<double> acc_mvis("m_vis");
    static const SG::ConstAccessor<int> acc_pdgID("pdgId");
    float ptratio = -999;
    float pt = acc_ptvis(truthtau);
    if(pt>0.) ptratio = tau.pt()/pt; 
    float eta = acc_etavis(truthtau);
    float phi = acc_phivis(truthtau);
    float m = acc_mvis(truthtau);
    m_ptResolution->Fill(ptratio, weight);
    m_etaResolution->Fill(tau.eta() - eta, weight);
    TLorentzVector LV_TauJet(0,0,0,0);
    TLorentzVector LV_TruthTau(0,0,0,0);
    LV_TauJet.SetPtEtaPhiM(tau.pt(), tau.eta(), tau.phi(), tau.m());
    LV_TruthTau.SetPtEtaPhiM(pt, eta, phi, m);
    m_phiResolution->Fill(LV_TauJet.DeltaPhi(LV_TruthTau), weight);

    int pdgID = acc_pdgID(truthtau);
    float truth_charge = 0.;
    if(pdgID > 0) truth_charge = -1.;
    else if(pdgID < 0) truth_charge = 1.;
    float charge_diff = tau.charge() - truth_charge;

    m_chargeResolution->Fill(charge_diff, weight);
     
  }
  
}
