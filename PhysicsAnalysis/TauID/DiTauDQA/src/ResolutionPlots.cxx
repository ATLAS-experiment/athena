/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include <utility>
#include "ResolutionPlots.h"
#include "AthContainers/ConstAccessor.h"
#include "TLorentzVector.h"
#include "FourMomUtils/xAODP4Helpers.h"

namespace DiTau{

  ResolutionPlots::ResolutionPlots(PlotBase *pParent, const std::string& sDir, const std::string& sDiTauJetContainerName):
    PlotBase(pParent, sDir),
    m_sDiTauJetContainerName{sDiTauJetContainerName} {}

    ResolutionPlots::~ResolutionPlots() = default;  
  
  
  void ResolutionPlots::initializePlots(){
    m_lead_subjet_ptResolution = Book1D("lead_subjet_ptResolution",m_sDiTauJetContainerName + " lead subjet ptResolution; lead subjet pt(TauReco)/pt(visTauTruth); # Part",100,0.5,1.5);
    m_lead_subjet_etaResolution  = Book1D("lead_subjet_etaResolution",m_sDiTauJetContainerName + " lead subjet etaResolution; lead subjet eta(TauReco) - eta(visTauTruth); # Part",100,-0.1,0.1);
    m_lead_subjet_phiResolution   = Book1D("lead_subjet_phiResolution",m_sDiTauJetContainerName + " lead subjet phiResolution; lead subjet  phi(TauReco) - phi(visTauTruth); # Part",100,-0.1,0.1);

    m_sublead_subjet_ptResolution = Book1D("sublead_subjet_ptResolution",m_sDiTauJetContainerName + " sublead subjet ptResolution; sublead subjet pt(TauReco)/pt(visTauTruth); # Part",100,0.5,1.5);
    m_sublead_subjet_etaResolution  = Book1D("sublead_subjet_etaResolution",m_sDiTauJetContainerName + " sublead subjet etaResolution; sublead subjet eta(TauReco) - eta(visTauTruth); # Part",100,-0.1,0.1);
    m_sublead_subjet_phiResolution   = Book1D("sublead_subjet_phiResolution",m_sDiTauJetContainerName + " sublead subjet phiResolution; sublead subjet  phi(TauReco) - phi(visTauTruth); # Part",100,-0.1,0.1);

  }

  void ResolutionPlots::fill(const xAOD::DiTauJet& ditau, float weight) {

    static const SG::ConstAccessor<double> acc_lead_subjet_ptvis("TruthVisLeadPt");
    static const SG::ConstAccessor<double> acc_lead_subjet_etavis("TruthVisLeadEta");
    static const SG::ConstAccessor<double> acc_lead_subjet_phivis("TruthVisLeadPhi");
    
    static const SG::ConstAccessor<double> acc_sublead_subjet_ptvis("TruthVisSubleadPt");
    static const SG::ConstAccessor<double> acc_sublead_subjet_etavis("TruthVisSubleadEta");
    static const SG::ConstAccessor<double> acc_sublead_subjet_phivis("TruthVisSubleadPhi");

    // fill histograms for the leading subjet 
    float ptratio = -999;
    float pt = acc_lead_subjet_ptvis(ditau);
    if(pt>0.) ptratio = ditau.subjetPt(0)/pt;
    float dphi = xAOD::P4Helpers::deltaPhi(ditau.subjetPhi(0),acc_lead_subjet_phivis(ditau)); 

    m_lead_subjet_ptResolution->Fill(ptratio, weight);
    m_lead_subjet_etaResolution->Fill(ditau.subjetEta(0) - acc_lead_subjet_etavis(ditau), weight);
    m_lead_subjet_phiResolution->Fill(dphi, weight); 

    // fill histograms for the subleading subjet 
    pt = acc_sublead_subjet_ptvis(ditau);  
    if(pt>0.) ptratio = ditau.subjetPt(1)/pt;
    dphi = xAOD::P4Helpers::deltaPhi(ditau.subjetPhi(1),acc_sublead_subjet_etavis(ditau));

    m_sublead_subjet_ptResolution->Fill(ptratio, weight);
    m_sublead_subjet_etaResolution->Fill(ditau.subjetEta(1) - acc_sublead_subjet_etavis(ditau), weight);
    m_sublead_subjet_phiResolution->Fill(dphi, weight);

  }
  
}
