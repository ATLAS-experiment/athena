/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TAURECTOOLS_TAUSRUSEVALUATOR_H
#define TAURECTOOLS_TAUSRUSEVALUATOR_H

#include "tauRecTools/TauRecToolBase.h"
#include "tauRecTools/TausRUsDataLoader.h"

#include "AthOnnxInterfaces/IAthInferenceTool.h"

#include "AsgDataHandles/ReadHandleKey.h"
#include "AsgDataHandles/WriteDecorHandleKey.h"
#include "AsgTools/PropertyWrapper.h"
#include "AsgTools/ToolHandle.h"

#include "AthContainers/Accessor.h"
#include "AthContainers/Decorator.h"

#include "xAODTau/TauJet.h"
#include "xAODTau/TauJetContainer.h"
#include "xAODTau/TauTrack.h"
#include "xAODTau/TauTrackContainer.h"
#include "xAODTracking/Vertex.h"
#include "xAODTracking/VertexContainer.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <vector>

/**
 * @brief Evaluates the TausRUs network and decorates its predictions on the tau
 * and on its tracks.
 *
 * The input and output nodes are read from the "metadata" JSON the model
 * carries. Inputs are built by TausRUsDataLoader.
 *
 * Nothing is decorated as it comes out of the graph. This tool owns
 * the whole translation into quantities that stand on their own:
 *
 *   TausRUsTauIDScore      tau vs QCD, a two-class softmax over those two logits of the 'primary' head
 *   TausRUsEleRejScore     tau vs electron, the same over that pair
 *   TausRUsDecayMode       decay mode decision (0=1p0n, 1=1p1n, 2=1pXn, 3=3p0n, 4=3pXn),
 *   TausRUsDecayModeScore<i>   the raw scores (logits) it is the argmax of, and
 *   TausRUsDecayModeProb<i>    their softmax
 *   TausRUsTauP4_*         the regressed tau four-momentum, and
 *   TausRUsChargedPionP4_* the charged and neutral pion-sum four-momenta, each four floats,
 *   TausRUsNeutralPionP4_* _pt, _eta, _phi and _m. The tau gets the PDG tau mass; the pion sums the
 *                          mass of the pions the predicted decay mode holds, counted as PanTau does
 *                          (nCharged x m(pi+-), nPi0 x m(pi0) with 1pXn = 2 pi0, 3pXn = 1 pi0, 0n = 0).
 *                          pt is undone from the log response w.r.t. ptJetSeed; eta and phi
 *                          heads flagged as relative to the seed jet get etaJetSeed / phiJetSeed added.
 *   TausRUsVertex_x/_y/_z  position of the vertex the 'vertex_classification' head picks out of the
 *                          event vertex collection; left at the default if the model has no vertex head
 *   TausRUsTauCharge       sum of the charges of the tracks classified as tau tracks
 *
 * Then, per xAOD::TauTrack the following are parsed
 *
 *   TausRUsTrackClass      track class decision (0=TT, 1=CT, 2=IT, 3=FT),
 *   TausRUsTrackScore<i>   the raw scores (logits) it is the argmax of, and
 *   TausRUsTrackProb<i>    their softmax
 */
class TausRUsEvaluator : public TauRecToolBase {
public:
  ASG_TOOL_CLASS2(TausRUsEvaluator, TauRecToolBase, ITauToolBase)

  TausRUsEvaluator(const std::string& name = "TausRUsEvaluator");
  virtual ~TausRUsEvaluator();

  virtual StatusCode initialize() override;

  using TauRecToolBase::executeTool;
  virtual StatusCode executeTool(xAOD::TauJet& tau,
				 const EventContext& ctx) const override;

  /// initialized with these default values
  static constexpr float DEFAULT_VALUE = -1111.0f;
  static constexpr int DEFAULT_CLASS = -1;

  struct Output {
    std::string name;                   ///< model output node name
    std::string type;                   ///< head type from the metadata
    std::vector<int64_t> dims;
    size_t size{0};                     ///< product of dims
    bool seedRelative{false};           ///< eta / phi head regressed w.r.t. the seed-jet axis
  };

  /// Class order of the 'primary' head, which is three raw logits.
  enum PrimaryClass : size_t { QCD = 0, Tau = 1, Electron = 2 };

  /// Class for true-tau track from tau_track_class head
  static constexpr int TAU_TRACK_CLASS = 0;

  /// The 'decay_mode' classes, in PanTau order, and the charged and neutral
  /// pion multiplicities PanTau gives each of them (Tool_DetailsArranger: one
  /// pi0 for 3pXn, two for 1pXn). They set the masses of the pion sums.
  static constexpr size_t N_DECAY_MODES = 5;
  static constexpr int N_CHARGED_PIONS[N_DECAY_MODES] = {1, 1, 1, 3, 3};
  static constexpr int N_NEUTRAL_PIONS[N_DECAY_MODES] = {0, 1, 2, 0, 1};

  /// The two components of a phi head, which regresses the angle as a point on
  /// the unit circle: phi = atan2(sin, cos).
  enum PhiComponent : size_t { PHI_SIN = 0, PHI_COS = 1 };

private:

  using FourMomDecorators = std::vector<SG::Accessor<float>>;

  /// Configure the data loader and build m_outputs from the metadata and node
  /// shapes of the model.
  StatusCode readModel(const std::string& path);

  void setDefaults(xAOD::TauJet& tau) const;

  /// Decorate the four floats of @p decorators from the pt, eta and phi heads
  /// named @p prefix + "_pt", "_eta", "_phi" (or "tes", "tau_eta", "tau_phi"),
  /// taking the mass from @p mass.
  void decorateFourMomentum(xAOD::TauJet& tau, const FourMomDecorators& decorators,
                            const Output& ptHead, std::span<const float> ptQuantiles,
                            const Output& etaHead, std::span<const float> etaValues,
                            const Output& phiHead, std::span<const float> phiValues,
                            float mass) const;

  const Output* findOutput(const std::string& name) const;

  /// Decorate the position of the vertex that @p scores picks out of
  /// @p vertices, which must be the list the input tensor was built from.
  void decorateVertex(xAOD::TauJet& tau,
                      std::span<const float> scores,
                      const std::vector<const xAOD::Vertex*>& vertices) const;

  /// Decorate each track of @p tau with its slot of the per-track head.
  void decorateTracks(xAOD::TauJet& tau, std::span<const float> scores) const;

  std::vector<std::string> tauDecorationNames() const;
  std::vector<std::string> trackDecorationNames() const;

  ToolHandle<AthInfer::IAthInferenceTool> m_inferenceTool{
    this, "InferenceTool", "", "ONNX Runtime or Triton inference backend"};

  SG::ReadHandleKey<xAOD::VertexContainer> m_vertexInputContainer{
    this, "Key_vertexInputContainer", "PrimaryVertices", "Input vertex container key"};

  Gaudi::Property<std::string> m_modelFile{
    this, "ModelFile", "",
    "ONNX model the input and output nodes are read from, also when inference runs on Triton"};
  Gaudi::Property<std::string> m_tauContainerName{
    this, "TauContainerName", "",
    "Name of the TauJetContainer, needed to declare the output data dependencies"};
  Gaudi::Property<std::string> m_tauTrackContainerName{
    this, "TauTrackContainerName", "",
    "Name of the TauTrackContainer, needed to declare the per-track output data dependencies"};
  Gaudi::Property<float> m_minTauPt{
    this, "MinTauPt", 15., "Skip taus below this pt to save CPU"};

  std::unique_ptr<TausRUsDataLoader> m_loader;

  /// The model's output nodes, from its metadata.
  std::vector<Output> m_outputs;

  /// Read off the metadata at initialize().
  size_t m_nDecayModes{0};
  size_t m_nTrackClasses{0};
  size_t m_nQuantiles{0};
  /// Whether the model has a vertex input and 'vertex_classification' head
  bool m_hasVertexHead{false};
  /// The pt heads ('tes', 'charged_pion_sum_pt', 'neutral_pion_sum_pt') regress
  /// quantiles of the log response, log(pt_seedjet / pt_true). This is the index
  /// of the median, the one to take as the point estimate; the pt then follows as
  ///   pt = exp(log(ptJetSeed) - quantile[m_ptMedianQuantile]).
  size_t m_ptMedianQuantile{0};

  /// The decorators. The scalar ones are named in the constructor and the rest,
  /// which are one decoration per class or per component, in initialize().
  SG::Accessor<float> m_tauIDScore;
  SG::Accessor<float> m_eleRejScore;
  SG::Accessor<float> m_decayMode;
  SG::Accessor<float> m_tauCharge;
  std::vector<SG::Accessor<float>> m_decayModeScores;
  std::vector<SG::Accessor<float>> m_decayModeProbs;
  FourMomDecorators m_tauP4;
  FourMomDecorators m_chargedPionP4;
  FourMomDecorators m_neutralPionP4;
  std::vector<SG::Accessor<float>> m_vertexPosition;
  // decorators to the tracks
  SG::Decorator<int> m_trackClass;
  std::vector<SG::Decorator<float>> m_trackScores;
  std::vector<SG::Decorator<float>> m_trackProbs;

  std::vector<SG::WriteDecorHandleKey<xAOD::TauJetContainer>> m_decorKeys;
  std::vector<SG::WriteDecorHandleKey<xAOD::TauTrackContainer>> m_trackDecorKeys;
};

#endif // TAURECTOOLS_TAUSRUSEVALUATOR_H
