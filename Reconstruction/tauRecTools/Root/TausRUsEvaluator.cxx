/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "tauRecTools/TausRUsEvaluator.h"

#include "AsgDataHandles/ReadHandle.h"
#include "TruthUtils/ParticleConstants.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <unordered_map>
#include <utility>

/// The heads, their class orders and the constants needed to invert them.
using namespace TausRUsModel;

namespace {

// The decorations, documented in TausRUsEvaluator.h 

/// On the tau.
constexpr char TAU_ID_SCORE[] = "TausRUsTauIDScore";
constexpr char ELE_REJ_SCORE[] = "TausRUsEleRejScore";
constexpr char DECAY_MODE[] = "TausRUsDecayMode";
constexpr char DECAY_MODE_SCORE_PREFIX[] = "TausRUsDecayModeScore";
constexpr char TAU_P4[] = "TausRUsTauP4";
constexpr char CHARGED_PION_P4[] = "TausRUsChargedPionP4";
constexpr char NEUTRAL_PION_P4[] = "TausRUsNeutralPionP4";
constexpr char VERTEX[] = "TausRUsVertex";
constexpr char TAU_CHARGE[] = "TausRUsTauCharge";

/// On each of the tau's tracks.
constexpr char TRACK_CLASS[] = "TausRUsTrackClass";
constexpr char TRACK_SCORE_PREFIX[] = "TausRUsTrackScore";

/// Suffixes of the four floats a four-momentum is stored as, in the order
/// TLorentzVector::SetPtEtaPhiM() takes them, and of the three a position is.
constexpr char P4_SUFFIXES[4][5] = {"_pt", "_eta", "_phi", "_m"};
constexpr char POSITION_SUFFIXES[3][3] = {"_x", "_y", "_z"};

/// Every decoration written on the tau, and every one written on its tracks,
/// which is what the output data dependencies are declared from.
std::vector<std::string> tauDecorationNames() {
  std::vector<std::string> names{TAU_ID_SCORE, ELE_REJ_SCORE, DECAY_MODE, TAU_CHARGE};
  for (size_t iMode = 0; iMode < N_DECAY_MODES; ++iMode) {
    names.emplace_back(std::string(DECAY_MODE_SCORE_PREFIX) + std::to_string(iMode));
  }
  for (const char* base : {TAU_P4, CHARGED_PION_P4, NEUTRAL_PION_P4}) {
    for (const auto& suffix : P4_SUFFIXES) {
      names.emplace_back(std::string(base) + suffix);
    }
  }
  for (const auto& suffix : POSITION_SUFFIXES) {
    names.emplace_back(std::string(VERTEX) + suffix);
  }
  return names;
}

std::vector<std::string> trackDecorationNames() {
  std::vector<std::string> names{TRACK_CLASS};
  for (size_t iClass = 0; iClass < N_TRACK_CLASSES; ++iClass) {
    names.emplace_back(std::string(TRACK_SCORE_PREFIX) + std::to_string(iClass));
  }
  return names;
}

/// Softmax of @p signal against @p background, the two logits of interest of a
/// classification head. Written as a sigmoid of the difference, which is the
/// same thing and cannot overflow: a large difference saturates at 0 or 1.
float twoClassScore(float signal, float background) {
  return 1.f / (1.f + std::exp(background - signal));
}

/// Index of the largest of @p scores, or DEFAULT_CLASS if empty.
int argMax(std::span<const float> scores) {
  if (scores.empty()) return TausRUsEvaluator::DEFAULT_CLASS;
  const auto largest = std::max_element(scores.begin(), scores.end());
  return static_cast<int>(std::distance(scores.begin(), largest));
}

} // anonymous namespace

TausRUsEvaluator::TausRUsEvaluator(const std::string& name)
  : TauRecToolBase(name),
    m_tauIDScore(TAU_ID_SCORE),
    m_eleRejScore(ELE_REJ_SCORE),
    m_decayMode(DECAY_MODE),
    m_tauCharge(TAU_CHARGE),
    m_trackClass(TRACK_CLASS) {}

TausRUsEvaluator::~TausRUsEvaluator() {}

StatusCode TausRUsEvaluator::initialize() {
  m_loader = std::make_unique<TausRUsDataLoader>(name() + "_DataLoader");
  ATH_CHECK(m_loader->initialize());

  ATH_CHECK(m_inferenceTool.retrieve());
  ATH_CHECK(m_vertexInputContainer.initialize());

  m_outputs = TausRUsModel::outputs();

  // Check the graph against what the decoding below assumes about it, so that a
  // re-exported model with a changed head fails here rather than silently
  // decorating nonsense.
  auto checkNode = [this](const std::string& name, size_t minSize) -> StatusCode {
    const TausRUsModel::Output* output = TausRUsModel::findOutput(m_outputs, name);
    if (!output) {
      ATH_MSG_ERROR("The model description has no output node '" << name << "'");
      return StatusCode::FAILURE;
    }
    if (output->size < minSize) {
      ATH_MSG_ERROR("Output '" << name << "' has " << output->size
                    << " values but the tool needs at least " << minSize);
      return StatusCode::FAILURE;
    }
    return StatusCode::SUCCESS;
  };

  ATH_CHECK(checkNode("primary", 3));
  ATH_CHECK(checkNode("decay_mode", N_DECAY_MODES));
  for (const char* name : {"tes", "charged_pion_pt", "neutral_pion_pt"}) {
    ATH_CHECK(checkNode(name, PT_MEDIAN_QUANTILE + 1));
  }
  for (const char* name : {"tau_eta", "charged_pion_eta", "neutral_pion_eta"}) {
    ATH_CHECK(checkNode(name, 1));
  }
  for (const char* name : {"tau_phi", "charged_pion_phi", "neutral_pion_phi"}) {
    ATH_CHECK(checkNode(name, 2));
  }

  // The per-constituent heads carry their own slot count, which is the
  // truncation the matching input tensor was built with.
  const TausRUsModel::Output* trackClass = TausRUsModel::findOutput(m_outputs, "tautrack_class");
  if (!trackClass || trackClass->dims.size() != 3
      || static_cast<size_t>(trackClass->dims[2]) != N_TRACK_CLASSES) {
    ATH_MSG_ERROR("Output 'tautrack_class' is expected to be (batch, tracks, "
                  << N_TRACK_CLASSES << ")");
    return StatusCode::FAILURE;
  }
  m_maxTracks = static_cast<size_t>(trackClass->dims[1]);

  const TausRUsModel::Output* vertexClass =
    TausRUsModel::findOutput(m_outputs, "vertex_classification");
  if (!vertexClass || vertexClass->dims.size() != 2) {
    ATH_MSG_ERROR("Output 'vertex_classification' is expected to be (batch, vertices)");
    return StatusCode::FAILURE;
  }
  m_maxVertices = static_cast<size_t>(vertexClass->dims[1]);

  // The decorators, in the order of the suffixes they are named after.
  for (size_t i = 0; i < N_DECAY_MODES; ++i) {
    m_decayModeScores.emplace_back(std::string(DECAY_MODE_SCORE_PREFIX)
                                   + std::to_string(i));
  }
  for (const auto& suffix : P4_SUFFIXES) {
    m_tauP4.emplace_back(std::string(TAU_P4) + suffix);
    m_chargedPionP4.emplace_back(std::string(CHARGED_PION_P4) + suffix);
    m_neutralPionP4.emplace_back(std::string(NEUTRAL_PION_P4) + suffix);
  }
  for (const auto& suffix : POSITION_SUFFIXES) {
    m_vertexPosition.emplace_back(std::string(VERTEX) + suffix);
  }
  for (size_t i = 0; i < N_TRACK_CLASSES; ++i) {
    m_trackScores.emplace_back(std::string(TRACK_SCORE_PREFIX) + std::to_string(i));
  }

  ATH_MSG_INFO("TausRUs decorates " << tauDecorationNames().size()
               << " variables on the tau and " << trackDecorationNames().size()
               << " on each of its up to " << m_maxTracks << " leading tracks");

  // Create the decoration keys to enforce data dependencies in the scheduler
  if (!m_tauContainerName.empty()) {
    for (const std::string& decoration : tauDecorationNames()) {
      m_decorKeys.emplace_back(m_tauContainerName + "." + decoration);
    }
    for (SG::WriteDecorHandleKey<xAOD::TauJetContainer>& key : m_decorKeys) {
      ATH_CHECK(key.initialize());
    }
  }
  // The per-track decorations are written through handles on these keys, so
  // unlike the tau ones the container has to be named.
  if (m_tauTrackContainerName.empty()) {
    ATH_MSG_ERROR("TauTrackContainerName is not set, but the per-track "
                  << trackDecorationNames().size() << " decorations are written on it");
    return StatusCode::FAILURE;
  }
  for (const std::string& decoration : trackDecorationNames()) {
    m_trackDecorKeys.emplace_back(m_tauTrackContainerName + "." + decoration);
  }
  for (SG::WriteDecorHandleKey<xAOD::TauTrackContainer>& key : m_trackDecorKeys) {
    ATH_CHECK(key.initialize());
  }

  return StatusCode::SUCCESS;
}

void TausRUsEvaluator::setDefaults(xAOD::TauJet& tau) const {
  m_tauIDScore(tau) = DEFAULT_VALUE;
  m_eleRejScore(tau) = DEFAULT_VALUE;
  m_decayMode(tau) = DEFAULT_VALUE;
  m_tauCharge(tau) = DEFAULT_VALUE;
  for (const SG::Accessor<float>& decorator : m_decayModeScores) {
    decorator(tau) = DEFAULT_VALUE;
  }
  for (const FourMomDecorators* decorators : {&m_tauP4, &m_chargedPionP4, &m_neutralPionP4}) {
    for (const SG::Accessor<float>& decorator : *decorators) {
      decorator(tau) = DEFAULT_VALUE;
    }
  }
  for (const SG::Accessor<float>& decorator : m_vertexPosition) {
    decorator(tau) = DEFAULT_VALUE;
  }

  // All of the tracks, not just the m_maxTracks the network sees, so that a
  // track dropped by the truncation is still decorated.
  for (const xAOD::TauTrack* track : tau.allTracks()) {
    if (!track) continue;
    m_trackClass(*track) = DEFAULT_CLASS;
    for (const SG::Decorator<float>& decorator : m_trackScores) {
      decorator(*track) = DEFAULT_VALUE;
    }
  }
}

void TausRUsEvaluator::decorateFourMomentum(xAOD::TauJet& tau,
                                            const FourMomDecorators& decorators,
                                            std::span<const float> ptQuantiles,
                                            std::span<const float> etaValues,
                                            std::span<const float> phiValues,
                                            float mass) const {
  // The pt heads regress quantiles of the log response, log(ptJetSeed / pt), so
  // the seed pt is needed to undo it and a seed with no pt leaves the defaults.
  const float ptJetSeed = tau.ptJetSeed();
  if (ptJetSeed <= 0.f) {
    ATH_MSG_DEBUG("Seed jet pt is " << ptJetSeed << ", leaving the regressed "
                  "four-momenta at their defaults");
    return;
  }

  const float response = ptQuantiles[PT_MEDIAN_QUANTILE];
  decorators[0](tau) = std::exp(std::log(ptJetSeed) - response);
  decorators[1](tau) = etaValues[0];
  decorators[2](tau) = std::atan2(phiValues[PHI_SIN], phiValues[PHI_COS]);
  decorators[3](tau) = mass;
}

void TausRUsEvaluator::decorateVertex(xAOD::TauJet& tau,
                                      std::span<const float> scores,
                                      const std::vector<const xAOD::Vertex*>& vertices) const {
  // Only the slots backed by a real vertex carry a meaningful score: the rest
  // score zero-padded inputs, and an event with no vertex at all leaves the
  // defaults.
  const size_t nSlots = std::min(vertices.size(), scores.size());
  if (nSlots == 0) {
    ATH_MSG_DEBUG("No vertex to choose from, leaving the vertex position at its default");
    return;
  }

  const int selected = argMax(scores.first(nSlots));
  const xAOD::Vertex* vertex = vertices[selected];
  m_vertexPosition[0](tau) = vertex->x();
  m_vertexPosition[1](tau) = vertex->y();
  m_vertexPosition[2](tau) = vertex->z();
}

void TausRUsEvaluator::decorateTracks(xAOD::TauJet& tau,
                                      std::span<const float> scores) const {

  const std::vector<const xAOD::TauTrack*> tracks = m_loader->selectTracks(tau, m_maxTracks);

  int tauCharge = 0; // set default value of charge 
  for (size_t iTrack = 0; iTrack < tracks.size(); ++iTrack) {
    const std::span<const float> slot =
      scores.subspan(iTrack * N_TRACK_CLASSES, N_TRACK_CLASSES);
    const int trackClass = argMax(slot);
    m_trackClass(*tracks[iTrack]) = trackClass;
    for (size_t iClass = 0; iClass < N_TRACK_CLASSES; ++iClass) {
      m_trackScores[iClass](*tracks[iTrack]) = slot[iClass];
    }
    // only add charge to tau charge if the track is the correct one.
    if (trackClass == TAU_TRACK_CLASS) {
      tauCharge += static_cast<int>(tracks[iTrack]->track()->charge());
    }
  }

  // Only append a valid charge
  if (tauCharge == -1 || tauCharge == 1) {
    m_tauCharge(tau) = tauCharge;
  }
}

StatusCode TausRUsEvaluator::execute(xAOD::TauJet& tau) const {
  // Set the defaults before any early return, so that a tau skipped below still
  // carries every decoration and no consumer has to test for their presence.

  setDefaults(tau);

  if (tau.pt() < m_minTauPt) {
    return StatusCode::SUCCESS;
  }

  SG::ReadHandle<xAOD::VertexContainer> vertexInHandle(m_vertexInputContainer);
  if (!vertexInHandle.isValid()) {
    ATH_MSG_ERROR("Could not retrieve vertex container "
                  << m_vertexInputContainer.key());
    return StatusCode::FAILURE;
  }

  const xAOD::TauJet* tauPtr = &tau;
  AthInfer::InputDataMap inputData =
    m_loader->loadInputs(std::span<const xAOD::TauJet* const>{&tauPtr, 1}, *vertexInHandle);

  AthInfer::OutputDataMap outputData;
  for (const TausRUsModel::Output& output : m_outputs) {
    outputData[output.name] = std::make_pair(output.dims, std::vector<float>{});
  }

  // MARK: run the inference
  ATH_CHECK(m_inferenceTool->inference(inputData, outputData));

  std::unordered_map<std::string, std::span<const float>> raw;
  raw.reserve(m_outputs.size());
  for (const TausRUsModel::Output& output : m_outputs) {
    const std::vector<float>& values =
      std::get<std::vector<float>>(outputData.at(output.name).second);
    if (values.size() != output.size) {
      ATH_MSG_ERROR("Output '" << output.name << "' returned " << values.size()
                    << " values but the tool expects " << output.size);
      return StatusCode::FAILURE;
    }
    raw[output.name] = values;
  }

  const std::span<const float> primary = raw.at("primary");
  m_tauIDScore(tau) = twoClassScore(primary[Tau], primary[QCD]);
  m_eleRejScore(tau) = twoClassScore(primary[Tau], primary[Electron]);

  const std::span<const float> decayMode = raw.at("decay_mode");
  m_decayMode(tau) = static_cast<float>(argMax(decayMode));
  for (size_t iMode = 0; iMode < N_DECAY_MODES; ++iMode) {
    m_decayModeScores[iMode](tau) = decayMode[iMode];
  }

  // The network gives pt, eta and phi only, so the four-momenta are completed
  // with the PDG masses.
  decorateFourMomentum(tau, m_tauP4, raw.at("tes"), raw.at("tau_eta"),
                       raw.at("tau_phi"),
                       ParticleConstants::PDG2024::tauMassInMeV);
  decorateFourMomentum(tau, m_chargedPionP4, raw.at("charged_pion_pt"),
                       raw.at("charged_pion_eta"), raw.at("charged_pion_phi"),
                       ParticleConstants::PDG2024::chargedPionMassInMeV);
  decorateFourMomentum(tau, m_neutralPionP4, raw.at("neutral_pion_pt"),
                       raw.at("neutral_pion_eta"), raw.at("neutral_pion_phi"),
                       ParticleConstants::PDG2024::piZeroMassInMeV);

  decorateVertex(tau, raw.at("vertex_classification"),
                 m_loader->selectVertices(*vertexInHandle, m_maxVertices));
  decorateTracks(tau, raw.at("tautrack_class"));

  return StatusCode::SUCCESS;
}
