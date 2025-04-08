/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include <utility>
#include "ResolutionPlots.h"
#include "AthContainers/ConstAccessor.h"
#include "TLorentzVector.h"
namespace Tau{

  ResolutionPlots::ResolutionPlots(PlotBase *pParent, const std::string& sDir, std::string sTauJetContainerName):
    PlotBase(pParent, sDir),
    m_ptResolution(nullptr),
    m_etaResolution(nullptr),
    m_phiResolution(nullptr),
    m_sTauJetContainerName(std::move(sTauJetContainerName))
  {
  }
  
    ResolutionPlots::~ResolutionPlots()
    {
    }
  
  
  void ResolutionPlots::initializePlots(){
    m_ptResolution = Book1D("ptResolution",m_sTauJetContainerName + " ptResolution; pt(TauReco)/pt(visTauTruth); # Part",100,0.5,1.5);
    m_etaResolution  = Book1D("etaResolution",m_sTauJetContainerName + " etaResolution; eta(TauReco) - eta(visTauTruth); # Part",100,-0.1,0.1);
    m_phiResolution   = Book1D("phiResolution",m_sTauJetContainerName + " phiResolution; phi(TauReco) - phi(visTauTruth); # Part",100,-0.1,0.1);
  }

  void ResolutionPlots::fill(const xAOD::TauJet& tau, const xAOD::TruthParticle& truthtau, float weight) {
    static const SG::ConstAccessor<double> acc_ptvis("pt_vis");
    static const SG::ConstAccessor<double> acc_etavis("eta_vis");
    static const SG::ConstAccessor<double> acc_phivis("phi_vis");
    static const SG::ConstAccessor<double> acc_mvis("m_vis");
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
  }
  
}
