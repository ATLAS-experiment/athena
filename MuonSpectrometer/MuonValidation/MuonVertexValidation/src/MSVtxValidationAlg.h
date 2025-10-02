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
#include "xAODTruth/TruthVertexContainer.h"
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


namespace defaults {
   inline constexpr int    Int = -99999;
   inline constexpr float  Float = -99999.;
}


class MSVtxValidationAlg: public ::AthHistogramAlgorithm {
   public:
      // using the AthHistogramAlgorithm constructor which creates a THistSvc, whose pointer is accessible via histSvc()
      using AthHistogramAlgorithm::AthHistogramAlgorithm;

      virtual StatusCode initialize() override final;
      virtual StatusCode execute() override final;
      virtual StatusCode finalize() override final;

   private:
      // ================
      // input containers
      // ================
      SG::ReadHandleKey<xAOD::EventInfo> m_evtKey{this, "EvtKey", "EventInfo"};
      SG::ReadHandleKey<xAOD::TruthParticleContainer> m_TruthParticleKey{this, "TruthParticleKey", "TruthParticles"};
      SG::ReadHandleKey<xAOD::TruthVertexContainer> m_TruthVertexKey{this, "TruthVertexKey", "TruthVertices"};
      SG::ReadHandleKey<xAOD::TrackParticleContainer> m_TrackParticleKey{this, "TrackParticleKey", "InDetTrackParticles"};
      SG::ReadHandleKey<xAOD::JetContainer> m_JetKey{this, "JetKey", "HLT_AntiKt4EMTopoJets_nojcalib"};
      SG::ReadHandleKey<xAOD::MissingETContainer> m_MetKey{this, "MetKey", "STCalibMET"};
      SG::ReadHandleKey<xAOD::MuonSegmentContainer> m_MuonSegKey{this, "MuonSegmentsKey", "MuonSegments"};
      SG::ReadHandleKey<xAOD::TrackParticleContainer> m_TrackletKey{this, "TrackletKey", "MSonlyTracklets"};
      SG::ReadHandleKey<xAOD::VertexContainer> m_MSVtxKey{this, "MSVertexKey", "MSDisplacedVertex"};
      
      // ========
      // triggers 
      // ========
      // Tool names are passed from the python configuration
      PublicToolHandle<Trig::TrigDecisionTool> m_trigDec{this, "TrigDecisionTool", "", "TrigDecisionTool (optional)"};
      PublicToolHandle<Trig::R3MatchingTool> m_matchingTool{this, "R3MatchingTool", "", "R3MatchingTool (optional)"};
      // struct for easy passing of trigger decisions
      struct TriggerInfo {
         std::string triggerString;
         std::vector<std::string> names;
         std::vector<bool> passed;
      };
      // jet trigger configuration
      Gaudi::Property<std::string> m_triggerStringJets{this, "TriggerString", "HLT_j400.*"}; // accepts single trigger name, comma separated list of trigger names, or regex
      Gaudi::Property<float> m_jetTriggerMatchingDR{this, "jetTriggerMatching", 0.1, "max dR distance for jet trigger matching"};


      // Algorithm properties
      Gaudi::Property<bool> m_isMC{this, "isMC", true, "is the input data simulated (MC) or real data?"};
      Gaudi::Property<bool> m_readTriggers{this, "readTriggers", false, "should trigger information be read?"};
      // what variables to fill
      Gaudi::Property<bool> m_dumpTruthParticles{this, "dumpTruthParticles", false, "dump truth particles to the tree"};
      Gaudi::Property<bool> m_dumpTruthVertices{this, "dumpTruthVertices", false, "dump truth vertices to the tree"};
      Gaudi::Property<bool> m_fillMET{this, "fillMET", false, "add met information to the tree"};
      Gaudi::Property<bool> m_fillJets{this, "fillJets", false, "add jet information to the tree"};
      Gaudi::Property<bool> m_computeActiveVertices{this, "computeActiveVertices", false, "extracts active vertices in the vicinity of a jet by looping through truth particles. Requires isMC."};
      Gaudi::Property<bool> m_computeVertexIso{this, "computeVertexIso", false, "add vertex isolation variables to the tree"};
      // truth particles
      Gaudi::Property<int> m_pdgId_portal{this, "pdgId_portal", 25, "Truth portal PDGId"};
      Gaudi::Property<int> m_pdgId_llp{this, "pdgId_llp", 35, "Truth LLP PDGId"};
      Gaudi::Property<bool> m_llp_genStableChildren{this, "llp_genStableChildren", false, "Restricts the TruthParticle-LLP linking to generator stable children rather than detector stable children"};
      // isolation variables
      Gaudi::Property<float> m_trackIso_pT{this, "trackIso_pT", 5.0*Gaudi::Units::GeV, "minimum track pT [GeV] to be considered for isolation"};
      Gaudi::Property<float> m_softTrackIso_R{this, "softTrackIso_R", 0.2, "cone radius around vertex to sum up track pT"};
      Gaudi::Property<float> m_jetIso_pT{this, "jetIso_pT", 20.0*Gaudi::Units::GeV, "minimum jet pT [GeV] to be considered for isolation"};
      Gaudi::Property<float> m_jetIso_LogRatio{this, "jetIso_LogRatio", 0.5, "max log ECal/HCal ratio for jet to be considered for isolation"};

      // ================
      // filling routines
      // ================
      StatusCode fillTruth(const EventContext& ctx);
      StatusCode fillTruthVertex(const EventContext& ctx);
      StatusCode fillTruthParticle(const EventContext& ctx);
      StatusCode fillJet(const EventContext& ctx);
      StatusCode fillMet(const EventContext& ctx);
      StatusCode fillTracklets(const EventContext& ctx);
      StatusCode fillMSVtx(const EventContext& ctx);
      // filling helpers
      StatusCode fillLLP(std::vector<const xAOD::TruthParticle*>& llps);
      void fillHits(const xAOD::Vertex* vtx, const std::string& decorator_str, MuonVal::VectorBranch<int>& branch);
      TriggerInfo getTriggerDecisions(std::string triggerString);
      template <typename T>
      bool getTriggerMatchingDecision(const T* object, TriggerInfo& triggerInfo, float dRmatching);
      StatusCode fillActiveVertices(const EventContext& ctx, const xAOD::Jet* jet);
      StatusCode fillMSVtxIsolation(const xAOD::Vertex* msVtx, const xAOD::TrackParticleContainer* tracks, const xAOD::JetContainer* jets);


      // ========================================================================
      // tree and branches
      // n-tuple branches with kinematics filled in units of GeV
      // additional variables are added to IParticleFourMomBranch in initialize()
      // ========================================================================
      MuonVal::MuonTesterTree m_tree{"MSVtxValidTree", "MSVtxValidation"};
      using ParticleBranchPtr_t = std::shared_ptr<MuonVal::IParticleFourMomBranch>;
      
      // truth vertex
      // taken from https://geant4.kek.jp/lxr/source/processes/hadronic/management/include/G4HadronicProcessType.hh#L46
      std::unordered_set<int> m_hadronicProcessEnum = {111, 116, 121, 131, 132, 141, 151, 152, 161, 165, 166, 167, 210, 310};
      // taken from https://geant4.kek.jp/lxr/source/processes/electromagnetic/utils/include/G4EmProcessSubType.hh#L43
      std::unordered_set<int> m_emProcessEnum = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,21,22,23,24,25,26,30,31,40,49};
      MuonVal::ThreeVectorBranch m_truthVtx_pos{m_tree, "truthVtx_"};
      MuonVal::VectorBranch<int>& m_truthVtx_status{m_tree.newVector<int>("truthVtx_status")};
      MuonVal::VectorBranch<int>& m_truthVtx_interactionType{m_tree.newVector<int>("truthVtx_ interactionType")}; // 0 for hadronic, 1 for EM, -1 for other
      MuonVal::VectorBranch<int>& m_truthVtx_NchildrenDirect{m_tree.newVector<int>("truthVtx_NchildrenDirect")}; // number of outgoing particles from the vertex
      MuonVal::VectorBranch<int>& m_truthVtx_Nchildren{m_tree.newVector<int>("truthVtx_Nchildren")}; // number of detector stable decay products
      MuonVal::VectorBranch<int>& m_truthVtx_NchildrenCharged{m_tree.newVector<int>("truthVtx_NchildrenCharged")}; // number of charged detector stable decay products
      MuonVal::MatrixBranch<int>& m_truthVtx_truthParticle_link{m_tree.newMatrix<int>("truthVtx_truthParticle_link")};

      // truth particles
      ParticleBranchPtr_t m_truthParticle{nullptr};
      MuonVal::VectorBranch<int>& m_truthParticle_llpVtx_link{m_tree.newVector<int>("truthParticle_llpVtx_link")};
      std::unique_ptr<std::vector<int>> m_truthParticle_llpVtx_link_tmp = std::make_unique<std::vector<int>>(); // temporary vector to hold the vertex link information before filling the branch
      MuonVal::VectorBranch<int>& m_truthParticle_actVtx_link{m_tree.newVector<int>("truthParticle_actVtx_link")};
      std::unique_ptr<std::vector<int>> m_truthParticle_actVtx_link_tmp = std::make_unique<std::vector<int>>();
      MuonVal::VectorBranch<int>& m_truthParticle_truthVtx_link{m_tree.newVector<int>("truthParticle_truthVtx_link")};
      std::unique_ptr<std::vector<int>> m_truthParticle_truthVtx_link_tmp = std::make_unique<std::vector<int>>();

      // portal particle
      ParticleBranchPtr_t m_portal{nullptr};
      MuonVal::ScalarBranch<int>& m_portal_N{m_tree.newScalar<int>("portal_N",defaults::Int)};

      // truth LLPs and their children
      ParticleBranchPtr_t m_llp{nullptr};
      MuonVal::ScalarBranch<int>& m_llp_N{m_tree.newScalar<int>("llp_N",defaults::Int)};
      MuonVal::VectorBranch<int>& m_llp_Nchildren{m_tree.newVector<int>("llp_Nchildren")};
      MuonVal::VectorBranch<int>& m_llp_NchildrenCharged{m_tree.newVector<int>("llp_NchildrenCharged")};

      // truth displaced vertex
      MuonVal::ThreeVectorBranch m_llpVtx_pos{m_tree, "llpVtx_"};
      MuonVal::ScalarBranch<int>& m_llpVtx_N{m_tree.newScalar<int>("llpVtx_N",defaults::Int)};
      MuonVal::VectorBranch<float>& m_llpVtx_Lxy{m_tree.newVector<float>("llpVtx_Lxy")};
      MuonVal::VectorBranch<float>& m_llpVtx_ctau{m_tree.newVector<float>("llpVtx_ctau")};
      MuonVal::MatrixBranch<int>& m_llpVtx_truthParticle_link{m_tree.newMatrix<int>("llpVtx_truthParticle_link")};

      // reconstructed MS vertex
      MuonVal::ThreeVectorBranch m_msVtx_pos{m_tree, "msVtx_"};
      MuonVal::ScalarBranch<int>& m_msVtx_N{m_tree.newScalar<int>("msVtx_N",defaults::Int)};
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
      MuonVal::ScalarBranch<int>& m_trklet_N{m_tree.newScalar<int>("trklet_N",defaults::Int)};
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
      MuonVal::ScalarBranch<int>& m_jet_N{m_tree.newScalar<int>("jet_N",defaults::Int)};
      MuonVal::VectorBranch<int>& m_jet_triggers{m_tree.newVector<int>("jet_triggers")};
      MuonVal::VectorBranch<int>& m_jet_NactVtx{m_tree.newVector<int>("jet_NactVtx")};
      MuonVal::MatrixBranch<int>& m_jet_actVtx_link{m_tree.newMatrix<int>("jet_actVtx_link")};

      // active vertices associated to jets
      MuonVal::ThreeVectorBranch m_actVtx_pos{m_tree, "actVtx_"};
      MuonVal::VectorBranch<int>& m_actVtx_NChildren{m_tree.newVector<int>("actVtx_NChildren")};
      MuonVal::VectorBranch<int>& m_actVtx_chainDepth{m_tree.newVector<int>("actVtx_chainDepth")};
      MuonVal::VectorBranch<float>& m_actVtx_energy{m_tree.newVector<float>("actVtx_energy")};
      MuonVal::VectorBranch<float>& m_actVtx_mass{m_tree.newVector<float>("actVtx_mass")};
      MuonVal::VectorBranch<float>& m_actVtx_pT{m_tree.newVector<float>("actVtx_pT")};
      MuonVal::VectorBranch<float>& m_actVtx_scalarPtSum{m_tree.newVector<float>("actVtx_scalarPtSum")};
      MuonVal::VectorBranch<int>& m_actVtx_jet_link{m_tree.newVector<int>("actVtx_jet_link")};
      MuonVal::VectorBranch<float>& m_actVtx_jet_dEta{m_tree.newVector<float>("actVtx_jet_dEta")};
      MuonVal::VectorBranch<float>& m_actVtx_jet_dPhi{m_tree.newVector<float>("actVtx_jet_dPhi")};
      MuonVal::MatrixBranch<int>& m_actVtx_truthParticle_link{m_tree.newMatrix<int>("actVtx_truthParticle_link")};

      // met
      MuonVal::ScalarBranch<float>& m_met{m_tree.newScalar<float>("met",defaults::Float)};
      MuonVal::ScalarBranch<float>& m_met_x{m_tree.newScalar<float>("met_X",defaults::Float)};
      MuonVal::ScalarBranch<float>& m_met_y{m_tree.newScalar<float>("met_Y",defaults::Float)};
      MuonVal::ScalarBranch<float>& m_met_phi{m_tree.newScalar<float>("met_phi",defaults::Float)};
      MuonVal::ScalarBranch<float>& m_sumEt{m_tree.newScalar<float>("sumEt",defaults::Float)};


      // ==========
      // histograms
      // ==========
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
