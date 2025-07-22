/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MSVTXVALIDATIONALG_H
#define MSVTXVALIDATIONALG_H

#include <memory>
#include <vector>
#include <string>

#include <TLorentzVector.h>
#include <TH1.h>
#include <TMath.h>

#include "AthenaBaseComps/AthHistogramAlgorithm.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/ReadHandleKey.h"

#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ITHistSvc.h"
#include "GaudiKernel/SystemOfUnits.h"

#include "TrigDecisionTool/TrigDecisionTool.h"
#include "TriggerMatchingTool/R3MatchingTool.h"

#include "xAODEventInfo/EventInfo.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTracking/VertexContainer.h"
#include "xAODMuon/MuonSegmentContainer.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODJet/JetContainer.h"
#include "xAODMissingET/MissingETContainer.h"

#include "MuonTesterTree/MuonTesterTree.h"
#include "MuonTesterTree/EventInfoBranch.h"
#include "MuonTesterTree/IParticleFourMomBranch.h"
#include "MuonTesterTree/ThreeVectorBranch.h"
#include "MuonPRDTest/SegmentVariables.h"


class MSVtxValidationAlg: public ::AthHistogramAlgorithm {
   public:
      // using the AthHistogramAlgorithm constructor which creates a THistSvc, whose pointer is accessible via histSvc()
      using AthHistogramAlgorithm::AthHistogramAlgorithm;

      virtual StatusCode initialize() override final;
      virtual StatusCode execute() override final;
      virtual StatusCode finalize() override final;

   private:
      SG::ReadHandleKey<xAOD::EventInfo> m_evtKey{this, "EvtKey", "EventInfo"};
      SG::ReadHandleKey<xAOD::TruthParticleContainer> m_TruthParticleKey{this, "TruthParticleKey", "TruthParticles"};
      SG::ReadHandleKey<xAOD::TrackParticleContainer> m_TrackParticleKey{this, "TrackParticleKey", "InDetTrackParticles"};
      SG::ReadHandleKey<xAOD::JetContainer> m_JetKey{this, "JetKey", "HLT_AntiKt4EMTopoJets_nojcalib"};
      SG::ReadHandleKey<xAOD::MissingETContainer> m_MetKey{this, "MetKey", "STCalibMET"};
      SG::ReadHandleKey<xAOD::TrackParticleContainer> m_TrackletKey{this, "TrackletKey", "MSonlyTracklets"};
      SG::ReadHandleKey<xAOD::VertexContainer> m_MSVtxKey{this, "MSVertexKey", "MSDisplacedVertex"};

      /** Tool handle for the trigger decision and matching tools */
      PublicToolHandle<Trig::TrigDecisionTool> m_trigDec{this, "TrigDecisionTool", "Trig::TrigDecisionTool/TrigDecisionTool"};
      PublicToolHandle<Trig::R3MatchingTool> m_matchingTool{this, "R3MatchingTool", "Trig::R3MatchingTool", "R3MatchingTool"};

      // tree
      MuonVal::MuonTesterTree m_tree{"MSVtxValidTree", "MSVtxValidation"};

      // Algorithm properties
      Gaudi::Property<std::string> m_MuonSegKey{this, "MuonSegmentsKey", "MuonSegments"};

      Gaudi::Property<bool> m_readJets{this, "readJets", false, "add jet information to the tree"};
      Gaudi::Property<bool> m_computeJetVtx{this, "computeJetVtx", false, "compute the displaced vertex for a jet"};
      Gaudi::Property<bool> m_readMET{this, "readMET", false, "add met information to the tree"};
      Gaudi::Property<bool> m_computeIso{this, "computeIso", false, "add vertex isolation variables to the tree"};

      // truth particles
      Gaudi::Property<int> m_pdgId_portal{this, "pdgId_portal", 25, "Truth portal PDGId"};
      Gaudi::Property<int> m_pdgId_llp{this, "pdgId_llp", 35, "Truth LLP PDGId"};
      Gaudi::Property<bool> m_llp_genStableChildren{this, "llp_genStableChildren", false, "Restricts the TruthParticle-LLP linking to generator stable children rather than all stable children"};

      // isolation variables
      Gaudi::Property<float> m_trackIso_pT{this, "trackIso_pT", 5.0*Gaudi::Units::GeV, "minimum track pT [GeV] to be considered for isolation"};
      Gaudi::Property<float> m_softTrackIso_R{this, "softTrackIso_R", 0.2, "cone radius around vertex to sum up track pT"};
      Gaudi::Property<float> m_jetIso_pT{this, "jetIso_pT", 20.0*Gaudi::Units::GeV, "minimum jet pT [GeV] to be considered for isolation"};
      Gaudi::Property<float> m_jetIso_LogRatio{this, "jetIso_LogRatio", 0.5, "max log ECal/HCal ratio for jet to be considered for isolation"};

      // jet triggers
      Gaudi::Property<std::string> m_triggerString{this, "TriggerString", "HLT_j400.*"}; // accepts single trigger name, comma separated list of trigger names, or regex
      Gaudi::Property<float> m_jetTriggerMatchingDR{this, "jetTriggerMatching", 0.1, "max dR distance for jet trigger matching"};

      // filling routines
      StatusCode fillTruth(const EventContext& ctx);
      StatusCode fillMSVtx(const EventContext& ctx);
      StatusCode fillMSVtxIso(const EventContext& ctx);
      StatusCode fillTracklets(const EventContext& ctx);
      StatusCode fillMuonSegments(const EventContext& ctx);
      StatusCode fillJet(const EventContext& ctx);
      StatusCode fillMet(const EventContext& ctx);
      // helpers
      StatusCode fillJetVtx(const EventContext& ctx, const xAOD::Jet* jet);
      void fillHits(const xAOD::Vertex* vtx, const std::string& decorator_str, MuonVal::VectorBranch<int>& branch);

      // --- //
      // n-tuple branches with kinematics filled in units of GeV
      // additional variables are added to IParticleFourMomBranch in initialize()
      // --- //
      using ParticleBranchPtr_t = std::shared_ptr<MuonVal::IParticleFourMomBranch>;

      // truth TrackParticles
      ParticleBranchPtr_t m_truthParticle{nullptr};
      MuonVal::VectorBranch<int>& m_truthParticle_llpVtx_link{m_tree.newVector<int>("truthParticle_llpVtx_link")};
      std::unique_ptr<std::vector<int>> m_truthParticle_llpVtx_link_tmp = std::make_unique<std::vector<int>>(); // temporary vector to hold the vertex link information before filling the branch
      MuonVal::VectorBranch<int>& m_truthParticle_jetVtx_link{m_tree.newVector<int>("truthParticle_jetVtx_link")};
      std::unique_ptr<std::vector<int>> m_truthParticle_jetVtx_link_tmp = std::make_unique<std::vector<int>>();

      // portal particle
      ParticleBranchPtr_t m_portal{nullptr};
      MuonVal::ScalarBranch<int>& m_portal_N{m_tree.newScalar<int>("portal_N",0)};

      // truth LLPs and their children
      ParticleBranchPtr_t m_llp{nullptr};
      MuonVal::ScalarBranch<int>& m_llp_N{m_tree.newScalar<int>("llp_N",0)};
      MuonVal::VectorBranch<int>& m_llp_Nchildren{m_tree.newVector<int>("llp_Nchildren")};

      // truth displaced vertex
      MuonVal::ThreeVectorBranch m_llpVtx_pos{m_tree, "llpVtx_"};
      MuonVal::ScalarBranch<int>& m_llpVtx_N{m_tree.newScalar<int>("llpVtx_N",0)};
      MuonVal::VectorBranch<float>& m_llpVtx_Lxy{m_tree.newVector<float>("llpVtx_Lxy")};
      MuonVal::VectorBranch<float>& m_llpVtx_ctau{m_tree.newVector<float>("llpVtx_ctau")};

      // reconstructed MS vertex
      MuonVal::ThreeVectorBranch m_msVtx_pos{m_tree, "msVtx_"};
      MuonVal::ScalarBranch<int>& m_msVtx_N{m_tree.newScalar<int>("msVtx_N",0)};
      MuonVal::VectorBranch<float>& m_msVtx_chi2{m_tree.newVector<float>("msVtx_chi2")};
      MuonVal::VectorBranch<int>& m_msVtx_nDoF{m_tree.newVector<int>("msVtx_nDoF")};
      MuonVal::VectorBranch<int>& m_msVtx_Ntrklet{m_tree.newVector<int>("msVtx_Ntrklet")};
      // hits near the vertex
      MuonVal::VectorBranch<int>& m_msVtx_nMDT{m_tree.newVector<int>("msVtx_nMDT")};
      MuonVal::VectorBranch<int>& m_msVtx_nMDT_inwards{m_tree.newVector<int>("msVtx_nMDT_inwards")};
      MuonVal::VectorBranch<int>& m_msVtx_nMDT_I{m_tree.newVector<int>("msVtx_nMDT_I")};
      MuonVal::VectorBranch<int>& m_msVtx_nMDT_E{m_tree.newVector<int>("msVtx_nMDT_E")};
      MuonVal::VectorBranch<int>& m_msVtx_nMDT_M{m_tree.newVector<int>("msVtx_nMDT_M")};
      MuonVal::VectorBranch<int>& m_msVtx_nMDT_O{m_tree.newVector<int>("msVtx_nMDT_O")};
      MuonVal::VectorBranch<int>& m_msVtx_nRPC{m_tree.newVector<int>("msVtx_nRPC")};
      MuonVal::VectorBranch<int>& m_msVtx_nRPC_inwards{m_tree.newVector<int>("msVtx_nRPC_inwards")};
      MuonVal::VectorBranch<int>& m_msVtx_nRPC_I{m_tree.newVector<int>("msVtx_nRPC_I")};
      MuonVal::VectorBranch<int>& m_msVtx_nRPC_E{m_tree.newVector<int>("msVtx_nRPC_E")};
      MuonVal::VectorBranch<int>& m_msVtx_nRPC_M{m_tree.newVector<int>("msVtx_nRPC_M")};
      MuonVal::VectorBranch<int>& m_msVtx_nRPC_O{m_tree.newVector<int>("msVtx_nRPC_O")};
      MuonVal::VectorBranch<int>& m_msVtx_nTGC{m_tree.newVector<int>("msVtx_nTGC")};
      MuonVal::VectorBranch<int>& m_msVtx_nTGC_inwards{m_tree.newVector<int>("msVtx_nTGC_inwards")};
      MuonVal::VectorBranch<int>& m_msVtx_nTGC_I{m_tree.newVector<int>("msVtx_nTGC_I")};
      MuonVal::VectorBranch<int>& m_msVtx_nTGC_E{m_tree.newVector<int>("msVtx_nTGC_E")};
      MuonVal::VectorBranch<int>& m_msVtx_nTGC_M{m_tree.newVector<int>("msVtx_nTGC_M")};
      MuonVal::VectorBranch<int>& m_msVtx_nTGC_O{m_tree.newVector<int>("msVtx_nTGC_O")};
      // vertex isolation variables
      MuonVal::VectorBranch<float>& m_msVtx_isoTracks_mindR{m_tree.newVector<float>("msVtx_isoTracks_mindR")};
      MuonVal::VectorBranch<float>& m_msVtx_isoTracks_pTsum{m_tree.newVector<float>("msVtx_isoTracks_pTsum")};
      MuonVal::VectorBranch<float>& m_msVtx_isoJets_mindR{m_tree.newVector<float>("msVtx_isoJets_mindR")};

      // tracklets
      MuonVal::ThreeVectorBranch m_trklet_pos{m_tree, "trklet_"};
      MuonVal::ThreeVectorBranch m_trklet_mom{m_tree, "trklet_p"};
      MuonVal::ScalarBranch<int>& m_trklet_N{m_tree.newScalar<int>("trklet_N",0)};
      MuonVal::VectorBranch<float>& m_trklet_d0{m_tree.newVector<float>("trklet_d0")};
      MuonVal::VectorBranch<float>& m_trklet_z0{m_tree.newVector<float>("trklet_z0")};
      MuonVal::VectorBranch<float>& m_trklet_theta{m_tree.newVector<float>("trklet_theta")};
      MuonVal::VectorBranch<float>& m_trklet_eta{m_tree.newVector<float>("trklet_eta")};
      MuonVal::VectorBranch<float>& m_trklet_phi{m_tree.newVector<float>("trklet_phi")};
      MuonVal::VectorBranch<float>& m_trklet_qOverP{m_tree.newVector<float>("trklet_qOverP")};
      MuonVal::VectorBranch<float>& m_trklet_q{m_tree.newVector<float>("trklet_q")};
      MuonVal::VectorBranch<int>& m_trklet_vtxLink{m_tree.newVector<int>("trklet_vtxLink")};

      // muon segments
      std::shared_ptr<MuonPRDTest::SegmentVariables> m_muonSeg{nullptr};

      // jet
      ParticleBranchPtr_t m_jet{nullptr};
      MuonVal::ScalarBranch<int>& m_jet_N{m_tree.newScalar<int>("jet_N",0)};
      MuonVal::VectorBranch<int>& m_jet_triggers{m_tree.newVector<int>("jet_triggers")};
      MuonVal::VectorBranch<int>& m_jet_jetVtx_link{m_tree.newVector<int>("jet_jetVtx_link")};
      MuonVal::ThreeVectorBranch m_jetVtx_pos{m_tree, "jetVtx_"};
      MuonVal::VectorBranch<float>& m_jetVtx_jet_dEta{m_tree.newVector<float>("jetVtx_jet_dEta")};
      MuonVal::VectorBranch<float>& m_jetVtx_jet_dPhi{m_tree.newVector<float>("jetVtx_jet_dPhi")};
      MuonVal::VectorBranch<int>& m_jetVtx_NChildren{m_tree.newVector<int>("jetVtx_NChildren")};
      MuonVal::VectorBranch<int>& m_jetVtx_chainDepth{m_tree.newVector<int>("jetVtx_chainDepth")};

      // met
      MuonVal::ScalarBranch<float>& m_met{m_tree.newScalar<float>("met",-1)};
      MuonVal::ScalarBranch<float>& m_met_x{m_tree.newScalar<float>("met_X",-99999)};
      MuonVal::ScalarBranch<float>& m_met_y{m_tree.newScalar<float>("met_Y",-99999)};
      MuonVal::ScalarBranch<float>& m_met_phi{m_tree.newScalar<float>("met_phi",-99999)};
      MuonVal::ScalarBranch<float>& m_sumEt{m_tree.newScalar<float>("sumEt",-1)};


      // --- //
      // histograms //
      // --- //
      TH1F* m_h_LLP1LLP2dR{nullptr};
      TH1F* m_h_diLLPMass{nullptr};
      TH1F* m_h_leadLLPLxy{nullptr};
      TH1F* m_h_leadLLPLz{nullptr};
      TH1F* m_h_leadLLPctau{nullptr};
      TH1F* m_h_leadLLPpt{nullptr};
      TH1F* m_h_subleadLLPLxy{nullptr};
      TH1F* m_h_subleadLLPLz{nullptr};
      TH1F* m_h_subleadLLPctau{nullptr};
      TH1F* m_h_subleadLLPpt{nullptr};
};

#endif // MSVTXVALIDATIONALG_H
