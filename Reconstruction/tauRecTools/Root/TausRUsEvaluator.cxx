/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "tauRecTools/TausRUsEvaluator.h"

#include "AsgDataHandles/ReadHandle.h"
#include "TruthUtils/ParticleConstants.h"
#include "xAODTracking/TrackParticle.h"

#include <nlohmann/json.hpp>
#include <onnxruntime_cxx_api.h>

#include <algorithm>
#include <cmath>
#include <map>
#include <numeric>
#include <string>
#include <unordered_map>
#include <utility>

namespace {

constexpr char TAU_ID_SCORE[] = "TausRUsTauIDScore";
constexpr char ELE_REJ_SCORE[] = "TausRUsEleRejScore";
constexpr char DECAY_MODE[] = "TausRUsDecayMode";
constexpr char DECAY_MODE_SCORE_PREFIX[] = "TausRUsDecayModeScore";
constexpr char TAU_P4[] = "TausRUsTauP4";
constexpr char CHARGED_PION_P4[] = "TausRUsChargedPionP4";
constexpr char NEUTRAL_PION_P4[] = "TausRUsNeutralPionP4";
constexpr char VERTEX[] = "TausRUsVertex";
constexpr char TAU_CHARGE[] = "TausRUsTauCharge";

constexpr char TRACK_CLASS[] = "TausRUsTrackClass";
constexpr char TRACK_SCORE_PREFIX[] = "TausRUsTrackScore";

constexpr char METADATA_KEY[] = "metadata";

constexpr char P4_SUFFIXES[4][5] = {"_pt", "_eta", "_phi", "_m"};
constexpr char POSITION_SUFFIXES[3][3] = {"_x", "_y", "_z"};

/// Softmax of @p signal against @p background, the two logits of interest of a
/// classification head. Written as a sigmoid of the difference
float twoClassScore(float signal, float background) {
  return 1.f / (1.f + std::exp(background - signal));
}

/// Index of the largest of @p scores, or DEFAULT_CLASS if empty.
int argMax(std::span<const float> scores) {
  if (scores.empty()) return TausRUsEvaluator::DEFAULT_CLASS;
  const auto largest = std::max_element(scores.begin(), scores.end());
  return static_cast<int>(std::distance(scores.begin(), largest));
}

std::string toString(const std::vector<int64_t>& dims) {
  std::string out = "(";
  for (size_t i = 0; i < dims.size(); ++i) {
    out += (i ? ", " : "") + std::to_string(dims[i]);
  }
  return out + ")";
}

/// Node names and shapes of the inputs or outputs of @p session, with a
/// dynamic batch dimension pinned to one tau.
std::map<std::string, std::vector<int64_t>> nodeShapes(const Ort::Session& session,
                                                       bool isInput) {
  Ort::AllocatorWithDefaultOptions allocator;
  std::map<std::string, std::vector<int64_t>> shapes;
  const size_t nNodes = isInput ? session.GetInputCount() : session.GetOutputCount();
  for (size_t i = 0; i < nNodes; ++i) {
    const std::string name = isInput
      ? session.GetInputNameAllocated(i, allocator).get()
      : session.GetOutputNameAllocated(i, allocator).get();
    const Ort::TypeInfo typeInfo = isInput ? session.GetInputTypeInfo(i)
                                           : session.GetOutputTypeInfo(i);
    std::vector<int64_t> dims = typeInfo.GetTensorTypeAndShapeInfo().GetShape();
    if (!dims.empty() && dims[0] < 0) dims[0] = 1;
    shapes[name] = std::move(dims);
  }
  return shapes;
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

// --------------------------------------------------------------------------
// MARK: Read model metadata
// --------------------------------------------------------------------------

StatusCode TausRUsEvaluator::readModel(const std::string& path) {

  Ort::Env env(ORT_LOGGING_LEVEL_FATAL, "");
  Ort::SessionOptions sessionOptions;
  sessionOptions.SetIntraOpNumThreads(1);
  sessionOptions.SetLogSeverityLevel(4);
  sessionOptions.DisableCpuMemArena();
  const Ort::Session session(env, path.c_str(), sessionOptions);

  Ort::AllocatorWithDefaultOptions allocator;
  const Ort::ModelMetadata modelMetadata = session.GetModelMetadata();
  const Ort::AllocatedStringPtr metadataString =
    modelMetadata.LookupCustomMetadataMapAllocated(METADATA_KEY, allocator);
  if (!metadataString) {
    ATH_MSG_ERROR("Model " << path << " has no '" << METADATA_KEY << "' metadata");
    return StatusCode::FAILURE;
  }

  const auto inputShapes = nodeShapes(session, true);
  const auto outputShapes = nodeShapes(session, false);

  const nlohmann::json metadata = nlohmann::json::parse(metadataString.get());

  std::vector<TausRUsDataLoader::InputConfig> inputs;
  for (const nlohmann::json& node : metadata.at("inputs")) {
    TausRUsDataLoader::InputConfig input;
    input.name = node.at("name").get<std::string>();
    input.collection = node.at("collection").get<std::string>();
    input.maxObjects = node.value("max_objects", size_t{1});
    for (const nlohmann::json& variable : node.at("variables")) {
      input.variables.push_back({variable.at("name").get<std::string>(),
                                  variable.at("offset").get<float>(),
                                  variable.at("scale").get<float>()});
    }
    if (inputShapes.find(input.name) == inputShapes.end()) {
      ATH_MSG_ERROR("The model has no input node '" << input.name << "'");
      return StatusCode::FAILURE;
    }
    inputs.push_back(std::move(input));
  }

  ATH_CHECK(m_loader->initialize(inputs));

  std::map<std::string, nlohmann::json> outputMetadata;
  for (const nlohmann::json& node : metadata.at("outputs")) {
    Output output;
    output.name = node.at("name").get<std::string>();
    output.type = node.value("type", "");
    const auto shape = outputShapes.find(output.name);
    if (shape == outputShapes.end()) {
      ATH_MSG_ERROR("The model has no output node '" << output.name << "'");
      return StatusCode::FAILURE;
    }
    output.dims = shape->second;
    output.size = std::accumulate(output.dims.begin(), output.dims.end(), size_t{1},
                                  std::multiplies<>());
    outputMetadata[output.name] = node;
    m_outputs.push_back(std::move(output));
  }

  // The heads the decoding needs to size itself.
  m_nDecayModes = outputMetadata.at("decay_mode").at("num_classes").get<size_t>();
  m_nTrackClasses = outputMetadata.at("tautrack_class").at("num_classes").get<size_t>();

  // The pt heads share one set of quantiles, and the median is decorated.
  const auto quantiles = outputMetadata.at("tes").at("quantiles").get<std::vector<double>>();
  for (const char* name : {"charged_pion_pt", "neutral_pion_pt"}) {
    if (outputMetadata.at(name).at("quantiles").get<std::vector<double>>() != quantiles) {
      ATH_MSG_ERROR("Output '" << name << "' regresses other quantiles than 'tes'");
      return StatusCode::FAILURE;
    }
  }
  const auto median = std::find_if(quantiles.begin(), quantiles.end(),
                                    [](double q) { return std::abs(q - 0.5) < 1e-6; });
  if (median == quantiles.end()) {
    ATH_MSG_ERROR("The pt heads regress no median quantile");
    return StatusCode::FAILURE;
  }
  m_nQuantiles = quantiles.size();
  m_ptMedianQuantile = std::distance(quantiles.begin(), median);

  // The per-slot heads are decoded against the track and vertex inputs.
  if (m_loader->maxTracks() == 0 || m_loader->maxVertices() == 0) {
    ATH_MSG_ERROR("The model needs both a track and a vertex input");
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}

StatusCode TausRUsEvaluator::initialize() {
  ATH_CHECK(m_inferenceTool.retrieve());
  ATH_CHECK(m_vertexInputContainer.initialize());

  const std::string modelPath = find_file(m_modelFile);
  if (modelPath.empty()) {
    ATH_MSG_ERROR("Could not find the model file '" << m_modelFile.value() << "'");
    return StatusCode::FAILURE;
  }
  m_loader = std::make_unique<TausRUsDataLoader>(name() + "_DataLoader");
  ATH_CHECK(readModel(modelPath));

  // Check the graph against what the decoding below assumes about it, so that a
  // re-exported model with a changed head fails here rather than silently
  // decorating nonsense.
  auto checkNode = [this](const std::string& name, const std::string& type,
                          const std::vector<int64_t>& dims) -> StatusCode {
    auto output = std::find_if(m_outputs.begin(), m_outputs.end(),
                               [&name](const Output& o) { return o.name == name; });
    if (output == m_outputs.end()) {
      ATH_MSG_ERROR("The model metadata has no output '" << name << "'");
      return StatusCode::FAILURE;
    }
    if (output->type != type) {
      ATH_MSG_ERROR("Output '" << name << "' is a '" << output->type
                    << "' head but the tool decodes it as '" << type << "'");
      return StatusCode::FAILURE;
    }
    if (output->dims != dims) {
      ATH_MSG_ERROR("Output '" << name << "' has shape " << toString(output->dims)
                    << " but the tool decodes " << toString(dims));
      return StatusCode::FAILURE;
    }
    return StatusCode::SUCCESS;
  };

  ATH_CHECK(checkNode("primary", "classification", {1, 3}));
  ATH_CHECK(checkNode("decay_mode", "classification",
                      {1, static_cast<int64_t>(m_nDecayModes)}));
  for (const char* name : {"tes", "charged_pion_pt", "neutral_pion_pt"}) {
    ATH_CHECK(checkNode(name, "quantile_regression",
                        {1, static_cast<int64_t>(m_nQuantiles)}));
  }
  for (const char* name : {"tau_eta", "charged_pion_eta", "neutral_pion_eta"}) {
    ATH_CHECK(checkNode(name, "regression", {1, 1}));
  }
  for (const char* name : {"tau_phi", "charged_pion_phi", "neutral_pion_phi"}) {
    ATH_CHECK(checkNode(name, "periodic_regression", {1, 2}));
  }
  ATH_CHECK(checkNode("vertex_classification", "classification",
                      {1, static_cast<int64_t>(m_loader->maxVertices())}));
  ATH_CHECK(checkNode("tautrack_class", "classification",
                      {1, static_cast<int64_t>(m_loader->maxTracks()),
                       static_cast<int64_t>(m_nTrackClasses)}));

  // The decorators, in the order of the suffixes they are named after.
  for (size_t i = 0; i < m_nDecayModes; ++i) {
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
  for (size_t i = 0; i < m_nTrackClasses; ++i) {
    m_trackScores.emplace_back(std::string(TRACK_SCORE_PREFIX) + std::to_string(i));
  }

  ATH_MSG_INFO("TausRUs decorates " << tauDecorationNames().size()
               << " variables on the tau and " << trackDecorationNames().size()
               << " on each of its up to " << m_loader->maxTracks() << " leading tracks");

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

// --------------------------------------------------------------------------
// MARK: Decorations
// --------------------------------------------------------------------------

/// Every decoration written on the tau, and every one written on its tracks,
/// which is what the output data dependencies are declared from.
std::vector<std::string> TausRUsEvaluator::tauDecorationNames() const {
  std::vector<std::string> names{TAU_ID_SCORE, ELE_REJ_SCORE, DECAY_MODE, TAU_CHARGE};
  for (size_t iMode = 0; iMode < m_nDecayModes; ++iMode) {
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

std::vector<std::string> TausRUsEvaluator::trackDecorationNames() const {
  std::vector<std::string> names{TRACK_CLASS};
  for (size_t iClass = 0; iClass < m_nTrackClasses; ++iClass) {
    names.emplace_back(std::string(TRACK_SCORE_PREFIX) + std::to_string(iClass));
  }
  return names;
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

  // All of the tracks, not just the leading ones the network sees, so that a
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

  const float response = ptQuantiles[m_ptMedianQuantile];
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

  const std::vector<const xAOD::TauTrack*> tracks = m_loader->selectTracks(tau);

  int tauCharge = 0; // set default value of charge
  for (size_t iTrack = 0; iTrack < tracks.size(); ++iTrack) {
    const std::span<const float> slot =
      scores.subspan(iTrack * m_nTrackClasses, m_nTrackClasses);
    const int trackClass = argMax(slot);
    m_trackClass(*tracks[iTrack]) = trackClass;
    for (size_t iClass = 0; iClass < m_nTrackClasses; ++iClass) {
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

  AthInfer::InputDataMap inputData = m_loader->loadInputs(tau, *vertexInHandle);

  AthInfer::OutputDataMap outputData;
  for (const Output& output : m_outputs) {
    outputData[output.name] = std::make_pair(output.dims, std::vector<float>{});
  }

  // MARK: run the inference
  ATH_CHECK(m_inferenceTool->inference(inputData, outputData));

  std::unordered_map<std::string, std::span<const float>> raw;
  raw.reserve(m_outputs.size());
  for (const Output& output : m_outputs) {
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
  for (size_t iMode = 0; iMode < m_nDecayModes; ++iMode) {
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
                 m_loader->selectVertices(*vertexInHandle));
  decorateTracks(tau, raw.at("tautrack_class"));

  return StatusCode::SUCCESS;
}
