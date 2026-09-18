#ifndef HYPERANALYSISALGORITHMS_RUNHYPERALG_H
#define HYPERANALYSISALGORITHMS_RUNHYPERALG_H

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/// @author Diego Baron
/// @author Zihan Zhang

#include <memory>
#include <string>
#include <vector>

// Algorithm includes
#include <AnaAlgorithm/AnaAlgorithm.h>
#include <AsgTools/PropertyWrapper.h>
#include <AsgTools/ToolHandle.h>
#include <SelectionHelpers/SysReadSelectionHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysWriteHandle.h>

// Framework includes
#include <xAODEgamma/ElectronContainer.h>
#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>
#include <xAODMissingET/MissingETContainer.h>
#include <xAODMuon/MuonContainer.h>

#include "AthContainers/ConstDataVector.h"

// HyPER includes
#include "HyPERAnalysisAlgorithms/HyPERGraph.h"
#include "HyPERAnalysisAlgorithms/HyPERModel.h"
#include "HyPERAnalysisAlgorithms/HyPERParser.h"

namespace EventReco {
using ROOT::Math::PtEtaPhiMVector;

/**
 * @struct InputsPack
 * @brief This struct stores the input objects needed to build the graph after
 * reading and filtering the xAOD containers.
 */
struct InputsPack {
  ConstDataVector<xAOD::ElectronContainer> m_electrons;
  ConstDataVector<xAOD::MuonContainer> m_muons;
  ConstDataVector<xAOD::JetContainer> m_jets;
  const xAOD::MissingETContainer* m_met = nullptr;

  InputsPack() = default;

  void clear() {
    m_electrons.clear();
    m_muons.clear();
    m_jets.clear();
  }
};

/**
 *  @class RunHyPERAlg
 *  @brief This class is in charge of building the HyPER graph and running the
 * HyPER algorithm: The user-inputs are:
 *    - The HyPER topology
 *    - The b-tagger to use
 *    - The object containers to use
 *    - The event selection
 *    - The algortithm debug level.
 *  Apart from the usual handles to the xAOD containers and tools, the class
 * stores pointers to:
 *    - The HyPER ONNX model to be used -> Implemented in HyPER<TOPOLOGY>Model.h
 *    - The graph class -> Implemented in HyPERGraph.h
 *    - A parser object -> Implemented in HyPER<TOPOLOGY>Parser.h
 */
class RunHyPERAlg final : public EL::AnaAlgorithm {
 public:
  using EL::AnaAlgorithm::AnaAlgorithm;

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) override;
  virtual StatusCode finalize() override;

  /**
   * @brief These methods build the HyPER graph based on the user-input
   * topology.
   */
  StatusCode buildGraph();
  StatusCode buildTtbarAllHadronicGraph();
  StatusCode buildTtbarLJetsGraph();
  StatusCode buildTtbarDiLeptonGraph();
  bool buildDileptonPartialCandidate(const std::vector<int>& indices,
                                     const std::vector<int>& ids,
                                     PtEtaPhiMVector& bJetP4,
                                     PtEtaPhiMVector& leptonP4,
                                     float& analyserCharge) const;
  void buildTopP4TtbarAllHadronic(const std::vector<int>& topIndices,
                                  const std::vector<int>& wIndices,
                                  PtEtaPhiMVector& top_b_p4,
                                  PtEtaPhiMVector& top_W_decay0_p4,
                                  PtEtaPhiMVector& top_W_decay1_p4);
  void buildTopP4TtbarLJets(const std::vector<int>& topHadIndices,
                            const std::vector<int>& wHadIndices,
                            const std::vector<int>& topLepIndices,
                            const std::vector<int>& topLepIDs,
                            PtEtaPhiMVector& tophad_b_p4,
                            PtEtaPhiMVector& tophad_w_decay0_p4,
                            PtEtaPhiMVector& tophad_w_decay1_p4,
                            PtEtaPhiMVector& toplep_b_p4,
                            PtEtaPhiMVector& toplep_lep_p4);


 private:
  // Configurable properties from python/yml
  Gaudi::Property<std::string> m_topology{
      this, "topology", "",
      "HyPER topology. Choose between: 'TtbarLJets', 'TtbarLJetsNoBTag', "
      "'TtbarAllHadronic' and 'TtbarDiLepton'."};
  Gaudi::Property<std::string> m_btagger{
      this, "btagger", "GN2v01_Continuous",
      "Name of the b-tagger and working point for jets. The "
      "'ftag_quantile_' prefix is added internally."};

  // The b-tagging decoration name derived from m_btagger
  std::string m_btagDecorName;

  // The Athena ONNX inference tools, one per cross-validation fold. The model
  // files themselves are configured on each tool's session tool.
  ToolHandle<AthOnnx::IOnnxRuntimeInferenceTool> m_onnxToolTrainedOnEven{
      this, "onnxToolTrainedOnEven", "",
      "ONNX inference tool holding the model trained on even-numbered events"};
  ToolHandle<AthOnnx::IOnnxRuntimeInferenceTool> m_onnxToolTrainedOnOdd{
      this, "onnxToolTrainedOnOdd", "",
      "ONNX inference tool holding the model trained on odd-numbered events"};

  // The onnxruntime model
  std::unique_ptr<HyPERModel> m_hyperModel;
  // The HyPER Graph object
  std::unique_ptr<HyPERGraph> m_hyperGraph;
  // The inputs from xAOD containers
  InputsPack m_hyperInputs{};
  // The HyPER parser object
  std::unique_ptr<HyPERParser> m_hyperParser;
  // Number for full log event
  Gaudi::Property<long unsigned int> m_fullLogEventNumber{
      this, "fullLogEventNumber", 0,
      "Number of the event to log in full detail"};

  // Cached topology enum (avoid re-parsing from string every event)
  HyPERTopology m_hyperTopology{HyPERTopology::NotSelected};
  // Whether the l+jets topology uses b-tagging features
  bool m_ljetsUseBTag{false};

  // systematics
  CP::SysListHandle m_systematicsList{this};

  // Input objects needed for reconstruction
  // Electrons
  CP::SysReadHandle<xAOD::ElectronContainer> m_electronsHandle{
      this, "electrons", "", "The electron container to use."};
  CP::SysReadSelectionHandle m_electronSelection{
      this, "electronSelection", "", "The selection on the input electrons."};
  // Muons
  CP::SysReadHandle<xAOD::MuonContainer> m_muonsHandle{
      this, "muons", "", "The muon container to use."};
  CP::SysReadSelectionHandle m_muonSelection{
      this, "muonSelection", "", "The selection on the input muons."};
  // Jets
  CP::SysReadHandle<xAOD::JetContainer> m_jetsHandle{
      this, "jets", "", "The jet container to use."};
  CP::SysReadSelectionHandle m_jetSelection{this, "jetSelection", "",
                                            "The selection on the input jets."};
  // MET
  CP::SysReadHandle<xAOD::MissingETContainer> m_metHandle{
      this, "met", "", "The MET container to use."};
  // EventInfo
  CP::SysReadHandle<xAOD::EventInfo> m_eventInfoHandle{
      this, "eventInfo", "EventInfo",
      "The EventInfo container to read selection decisions from."};
  // Event pre-selection
  CP::SysReadSelectionHandle m_selection{
      this, "eventSelection", "",
      "Name of the selection on which this HyPER instance is allowed to run."};

  // Output decorations

  // For TtbarAllHadronic
  // Indices
  CP::SysWriteDecorHandle<std::vector<int>>
      m_hyper_TtbarAllHadronic_Top1_Indices{
          this, "TtbarAllHadronic_HyPER_Top1_Indices",
          "TtbarAllHadronic_HyPER_Top1_Indices_%SYS%",
          "Indices of the reconstructed top1 in the ttbar all-hadronic "
          "topology"};
  CP::SysWriteDecorHandle<std::vector<int>>
      m_hyper_TtbarAllHadronic_Top2_Indices{
          this, "TtbarAllHadronic_HyPER_Top2_Indices",
          "TtbarAllHadronic_HyPER_Top2_Indices_%SYS%",
          "Indices of the reconstructed top2 in the ttbar all-hadronic "
          "topology"};
  CP::SysWriteDecorHandle<std::vector<int>> m_hyper_TtbarAllHadronic_W1_Indices{
      this, "TtbarAllHadronic_HyPER_W1_Indices",
      "TtbarAllHadronic_HyPER_W1_Indices_%SYS%",
      "Indices of the reconstructed W1 in the ttbar all-hadronic topology"};
  CP::SysWriteDecorHandle<std::vector<int>> m_hyper_TtbarAllHadronic_W2_Indices{
      this, "TtbarAllHadronic_HyPER_W2_Indices",
      "TtbarAllHadronic_HyPER_W2_Indices_%SYS%",
      "Indices of the reconstructed W2 in the ttbar all-hadronic topology"};
  // Scores
  CP::SysWriteDecorHandle<float> m_hyper_TtbarAllHadronic_Top1_Score{
      this, "TtbarAllHadronic_HyPER_Top1_Score",
      "TtbarAllHadronic_HyPER_Top1_Score_%SYS%",
      "Score of the reconstructed top1 in the ttbar all-hadronic topology"};
  CP::SysWriteDecorHandle<float> m_hyper_TtbarAllHadronic_Top2_Score{
      this, "TtbarAllHadronic_HyPER_Top2_Score",
      "TtbarAllHadronic_HyPER_Top2_Score_%SYS%",
      "Score of the reconstructed top2 in the ttbar all-hadronic topology"};
  CP::SysWriteDecorHandle<float> m_hyper_TtbarAllHadronic_W1_Score{
      this, "TtbarAllHadronic_HyPER_W1_Score",
      "TtbarAllHadronic_HyPER_W1_Score_%SYS%",
      "Score of the reconstructed W1 in the ttbar all-hadronic topology"};
  CP::SysWriteDecorHandle<float> m_hyper_TtbarAllHadronic_W2_Score{
      this, "TtbarAllHadronic_HyPER_W2_Score",
      "TtbarAllHadronic_HyPER_W2_Score_%SYS%",
      "Score of the reconstructed W2 in the ttbar all-hadronic topology"};
  // 4-vector publication for TtbarAllHadronic
  // The b-quark jets definition is in the TtbarDiLepton tology 4-vectors.
  CP::SysWriteDecorHandle<PtEtaPhiMVector> m_top_Wplus_decay0_p4{
      this, "top_Wplus_decay0_p4", "",
      "Visible top W+ leading decay product four-vector associated to the top "
      "quark"};
  CP::SysWriteDecorHandle<PtEtaPhiMVector> m_top_Wplus_decay1_p4{
      this, "top_Wplus_decay1_p4", "",
      "Visible top W+ subleading decay product four-vector associated to the "
      "top quark"};
  CP::SysWriteDecorHandle<PtEtaPhiMVector> m_topbar_Wminus_decay0_p4{
      this, "topbar_Wminus_decay0_p4", "",
      "Visible anti-top W- leading decay product four-vector associated to the "
      "anti-top quark"};
  CP::SysWriteDecorHandle<PtEtaPhiMVector> m_topbar_Wminus_decay1_p4{
      this, "topbar_Wminus_decay1_p4", "",
      "Visible anti-top W- subleading decay product four-vector associated to "
      "the anti-top quark"};

  // For TtbarLJets
  // Classification score
  CP::SysWriteDecorHandle<float> m_hyper_TtbarLJets_Classification_Score{
      this, "TtbarLJets_HyPER_Classification_Score",
      "TtbarLJets_HyPER_Classification_Score_%SYS%",
      "Classification score of the ttbar single lepton topology"};
  // Indices
  CP::SysWriteDecorHandle<std::vector<int>> m_hyper_TtbarLJets_TopHad_Indices{
      this, "TtbarLJets_HyPER_TopHad_Indices",
      "TtbarLJets_HyPER_TopHad_Indices_%SYS%",
      "Indices of the reconstructed hadronic top in the ttbar single "
      "lepton topology"};
  CP::SysWriteDecorHandle<std::vector<int>> m_hyper_TtbarLJets_TopLep_Indices{
      this, "TtbarLJets_HyPER_TopLep_Indices",
      "TtbarLJets_HyPER_TopLep_Indices_%SYS%",
      "Indices of the reconstructed leptonic top in the ttbar single "
      "lepton topology"};
  CP::SysWriteDecorHandle<std::vector<int>> m_hyper_TtbarLJets_WHad_Indices{
      this, "TtbarLJets_HyPER_WHad_Indices",
      "TtbarLJets_HyPER_WHad_Indices_%SYS%",
      "Indices of the reconstructed hadronic W in the ttbar single lepton "
      "topology"};
  CP::SysWriteDecorHandle<std::vector<int>> m_hyper_TtbarLJets_WLep_Indices{
      this, "TtbarLJets_HyPER_WLep_Indices",
      "TtbarLJets_HyPER_WLep_Indices_%SYS%",
      "Indices of the reconstructed leptonic W in the ttbar single lepton "
      "topology"};
  // Scores
  CP::SysWriteDecorHandle<float> m_hyper_TtbarLJets_TopHad_Score{
      this, "TtbarLJets_HyPER_TopHad_Score",
      "TtbarLJets_HyPER_TopHad_Score_%SYS%",
      "Score of the reconstructed hadronic top in the ttbar single lepton "
      "topology"};
  CP::SysWriteDecorHandle<float> m_hyper_TtbarLJets_TopLep_Score{
      this, "TtbarLJets_HyPER_TopLep_Score",
      "TtbarLJets_HyPER_TopLep_Score_%SYS%",
      "Score of the reconstructed leptonic top in the ttbar single lepton "
      "topology"};
  CP::SysWriteDecorHandle<float> m_hyper_TtbarLJets_WHad_Score{
      this, "TtbarLJets_HyPER_WHad_Score", "TtbarLJets_HyPER_WHad_Score_%SYS%",
      "Score of the reconstructed hadronic W in the ttbar single lepton "
      "topology"};
  CP::SysWriteDecorHandle<float> m_hyper_TtbarLJets_WLep_Score{
      this, "TtbarLJets_HyPER_WLep_Score", "TtbarLJets_HyPER_WLep_Score_%SYS%",
      "Score of the reconstructed leptonic W in the ttbar single lepton "
      "topology"};
  // IDs
  CP::SysWriteDecorHandle<std::vector<int>> m_hyper_TtbarLJets_TopHad_IDs{
      this, "TtbarLJets_HyPER_TopHad_IDs", "TtbarLJets_HyPER_TopHad_IDs_%SYS%",
      "IDs of the reconstructed hadronic top in the ttbar single lepton "
      "topology"};
  CP::SysWriteDecorHandle<std::vector<int>> m_hyper_TtbarLJets_TopLep_IDs{
      this, "TtbarLJets_HyPER_TopLep_IDs", "TtbarLJets_HyPER_TopLep_IDs_%SYS%",
      "IDs of the reconstructed leptonic top in the ttbar single lepton "
      "topology"};
  // 4-vector publication for TtbarLJets
  CP::SysWriteDecorHandle<PtEtaPhiMVector> m_toplep_b_p4{
      this, "toplep_b_p4", "",
      "Visible toplep b-jet four-vector associated to the leptonic top quark"};
  CP::SysWriteDecorHandle<PtEtaPhiMVector> m_toplep_lep_p4{
      this, "toplep_lep_p4", "",
      "Lepton four-vector associated to the leptonic top quark"};
  CP::SysWriteDecorHandle<PtEtaPhiMVector> m_tophad_b_p4{
      this, "tophad_b_p4", "",
      "Visible tophad b-jet four-vector associated to the hadronic top quark"};
  CP::SysWriteDecorHandle<PtEtaPhiMVector> m_tophad_w_decay0_p4{
      this, "tophad_w_decay0_p4", "",
      "Visible hadronic top W leading decay product four-vector"};
  CP::SysWriteDecorHandle<PtEtaPhiMVector> m_tophad_w_decay1_p4{
      this, "tophad_w_decay1_p4", "",
      "Visible hadronic top W subleading decay product four-vector"};

  // For TtbarDiLepton
  // Classification score
  CP::SysWriteDecorHandle<float> m_hyper_TtbarDiLepton_Classification_Score{
      this, "TtbarDiLepton_HyPER_Classification_Score",
      "TtbarDiLepton_HyPER_Classification_Score_%SYS%",
      "Classification score of the ttbar di-lepton topology"};
  // Indices
  CP::SysWriteDecorHandle<std::vector<int>> m_hyper_TtbarDiLepton_Top1_Indices{
      this, "TtbarDiLepton_HyPER_Top1_Indices",
      "TtbarDiLepton_HyPER_Top1_Indices_%SYS%",
      "Indices of the reconstructed top1 in the ttbar di-lepton "
      "topology"};
  CP::SysWriteDecorHandle<std::vector<int>> m_hyper_TtbarDiLepton_Top2_Indices{
      this, "TtbarDiLepton_HyPER_Top2_Indices",
      "TtbarDiLepton_HyPER_Top2_Indices_%SYS%",
      "Indices of the reconstructed top2 in the ttbar di-lepton "
      "topology"};
  // Scores
  CP::SysWriteDecorHandle<float> m_hyper_TtbarDiLepton_Top1_Score{
      this, "TtbarDiLepton_HyPER_Top1_Score",
      "TtbarDiLepton_HyPER_Top1_Score_%SYS%",
      "Score of the reconstructed top1 in the ttbar di-lepton topology"};
  CP::SysWriteDecorHandle<float> m_hyper_TtbarDiLepton_Top2_Score{
      this, "TtbarDiLepton_HyPER_Top2_Score",
      "TtbarDiLepton_HyPER_Top2_Score_%SYS%",
      "Score of the reconstructed top2 in the ttbar di-lepton topology"};
  // IDs
  CP::SysWriteDecorHandle<std::vector<int>> m_hyper_TtbarDiLepton_Top1_IDs{
      this, "TtbarDiLepton_HyPER_Top1_IDs",
      "TtbarDiLepton_HyPER_Top1_IDs_%SYS%",
      "IDs of the reconstructed top1 in the ttbar di-lepton topology"};
  CP::SysWriteDecorHandle<std::vector<int>> m_hyper_TtbarDiLepton_Top2_IDs{
      this, "TtbarDiLepton_HyPER_Top2_IDs",
      "TtbarDiLepton_HyPER_Top2_IDs_%SYS%",
      "IDs of the reconstructed top2 in the ttbar di-lepton topology"};
  // HE score
  CP::SysWriteDecorHandle<float> m_hyper_TtbarDiLepton_HE_Score{
      this, "TtbarDiLepton_HyPER_HE_Score",
      "TtbarDiLepton_HyPER_HE_Score_%SYS%",
      "HE score of the ttbar di-lepton topology"};
  // 4-vector publication for TtbarDiLepton
  CP::SysWriteDecorHandle<PtEtaPhiMVector> m_top_b_p4{
      this, "top_b_p4", "",
      "Visible top b-jet four-vector for the positive-charge lepton branch"};
  CP::SysWriteDecorHandle<PtEtaPhiMVector> m_topbar_bbar_p4{
      this, "topbar_bbar_p4", "",
      "Visible anti-top bbar-jet four-vector for the negative-charge lepton "
      "branch"};
  CP::SysWriteDecorHandle<PtEtaPhiMVector> m_top_lep_p4{
      this, "top_lep_p4", "",
      "Top lepton four-vector for the positive-charge lepton branch"};
  CP::SysWriteDecorHandle<PtEtaPhiMVector> m_topbar_lepbar_p4{
      this, "topbar_lepbar_p4", "",
      "Anti-top lepton four-vector for the negative-charge lepton branch"};

  // Tools and functions for btagging
  std::unique_ptr<SG::ConstAccessor<int>> m_bTagDecoAcc;
};

}  // namespace EventReco

#endif
