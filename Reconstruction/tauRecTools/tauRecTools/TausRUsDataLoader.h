/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TAURECTOOLS_TAUSRUSDATALOADER_H
#define TAURECTOOLS_TAUSRUSDATALOADER_H

#include "tauRecTools/ConstituentsLoaderTauCluster.h"
#include "tauRecTools/ConstituentsLoaderTauTrack.h"
#include "tauRecTools/TauGNNDataLoader.h"

#include "AthOnnxInterfaces/IAthInferenceTool.h"

#include "AsgMessaging/AsgMessaging.h"
#include "AsgMessaging/StatusCode.h"

#include "xAODCaloEvent/CaloVertexedTopoCluster.h"
#include "xAODTau/TauJet.h"
#include "xAODTau/TauTrack.h"
#include "xAODTracking/Vertex.h"
#include "xAODTracking/VertexContainer.h"

#include <cstddef>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

/// TausRUs input variables that no other tau network uses. Where one exists,
/// the variables of TauClusterVars, TauTrackVars and TauScalarVars are used.
/// A log_ prefix is the natural log of the training preprocessing,
/// log(max(x, 1e-8)) with zero kept at zero.
namespace TausRUsClusterVars {
  // Kinematics of the raw cluster, clust(), not of the vertex-corrected p4
  bool dEtaRaw  (const xAOD::TauJet& tau, const xAOD::CaloVertexedTopoCluster& cluster, float& out);
  bool dPhiRaw  (const xAOD::TauJet& tau, const xAOD::CaloVertexedTopoCluster& cluster, float& out);
  bool etaRaw   (const xAOD::TauJet& tau, const xAOD::CaloVertexedTopoCluster& cluster, float& out);
  bool phiRaw   (const xAOD::TauJet& tau, const xAOD::CaloVertexedTopoCluster& cluster, float& out);
  bool log_etRaw(const xAOD::TauJet& tau, const xAOD::CaloVertexedTopoCluster& cluster, float& out);
  bool log_eRaw (const xAOD::TauJet& tau, const xAOD::CaloVertexedTopoCluster& cluster, float& out);
} // namespace TausRUsClusterVars

namespace TausRUsTrackVars {
  bool log_pt(const xAOD::TauJet& tau, const xAOD::TauTrack& track, float& out);
  bool log_e (const xAOD::TauJet& tau, const xAOD::TauTrack& track, float& out);
  bool z0    (const xAOD::TauJet& tau, const xAOD::TauTrack& track, float& out);
  // Read off the TrackParticle, unlike TauTrackVars::eProbabilityNN which reads the TauTrack
  bool eProbabilityNN_trackParticle(const xAOD::TauJet& tau, const xAOD::TauTrack& track, float& out);
} // namespace TausRUsTrackVars

namespace TausRUsVertexVars {
  bool sumPt2(const xAOD::TauJet& tau, const xAOD::Vertex& vertex, float& out);
  bool x     (const xAOD::TauJet& tau, const xAOD::Vertex& vertex, float& out);
  bool y     (const xAOD::TauJet& tau, const xAOD::Vertex& vertex, float& out);
  bool z     (const xAOD::TauJet& tau, const xAOD::Vertex& vertex, float& out);
} // namespace TausRUsVertexVars

/**
 * @brief Builds the fixed-size input tensors of the TausRUs network.
 *
 * The inputs are described by the model metadata: per input node, the
 * collection it is built from, its truncation and its variables. Each variable
 * name selects one function from the map of its collection, so a model with
 * other inputs needs no code change unless it uses a new variable. The
 * constituents are sorted and truncated, the remaining slots zero-padded.
 */
class TausRUsDataLoader : public asg::AsgMessaging {
public:
  struct VariableConfig {
    std::string name;
    float offset{0.f};   ///< value is (raw + offset) * scale
    float scale{1.f};
  };

  struct InputConfig {
    std::string name;          ///< model input node name
    std::string collection;    ///< clusters, tracks, vertices or seedjet
    size_t maxObjects{1};      ///< truncation, 1 for seedjet
    std::vector<VariableConfig> variables;
  };

  explicit TausRUsDataLoader(const std::string& name);

  /// Resolve the variables of @p inputs to their functions.
  StatusCode initialize(const std::vector<InputConfig>& inputs);

  AthInfer::InputDataMap loadInputs(const xAOD::TauJet& tau,
                                    const xAOD::VertexContainer& vertices) const;

  /// The tracks and vertices the input tensors are built from, in slot order,
  /// for decoding the per-slot heads.
  std::vector<const xAOD::TauTrack*> selectTracks(const xAOD::TauJet& tau) const;
  std::vector<const xAOD::Vertex*> selectVertices(const xAOD::VertexContainer& vertices) const;

  size_t maxTracks() const { return m_tracks.maxObjects; }
  size_t maxVertices() const { return m_vertices.maxObjects; }

private:
  using ClusterFunc_t = std::function<bool(const xAOD::TauJet&, const xAOD::CaloVertexedTopoCluster&, float&)>;
  using TrackFunc_t   = std::function<bool(const xAOD::TauJet&, const xAOD::TauTrack&, float&)>;
  using VertexFunc_t  = std::function<bool(const xAOD::TauJet&, const xAOD::Vertex&, float&)>;
  using ScalarFunc_t  = std::function<bool(const xAOD::TauJet&, float&)>;

  /// One input node, its variables resolved to their functions.
  template <class Func>
  struct Sequence {
    std::string name;
    size_t maxObjects{0};
    std::vector<Func> funcs;
    std::vector<float> offsets;
    std::vector<float> scales;
  };

  template <class Func>
  StatusCode resolve(const InputConfig& input,
                     const std::unordered_map<std::string, Func>& funcMap,
                     Sequence<Func>& sequence) const;

  std::vector<const xAOD::CaloVertexedTopoCluster*> selectClusters(
    const xAOD::TauJet& tau,
    std::vector<xAOD::CaloVertexedTopoCluster>& storage) const;

  Sequence<ClusterFunc_t> m_clusters;
  Sequence<TrackFunc_t> m_tracks;
  Sequence<VertexFunc_t> m_vertices;
  Sequence<ScalarFunc_t> m_scalars;

  inline static const std::unordered_map<std::string, ClusterFunc_t> m_clusterFuncs = {
    {"dEtaRaw",        TausRUsClusterVars::dEtaRaw},
    {"dPhiRaw",        TausRUsClusterVars::dPhiRaw},
    {"etaRaw",         TausRUsClusterVars::etaRaw},
    {"phiRaw",         TausRUsClusterVars::phiRaw},
    {"log_etRaw",      TausRUsClusterVars::log_etRaw},
    {"log_eRaw",       TausRUsClusterVars::log_eRaw},
    {"FIRST_ENG_DENS", TauClusterVars::FIRST_ENG_DENS},
    {"SECOND_R",       TauClusterVars::SECOND_R},
    {"EM_PROBABILITY", TauClusterVars::EM_PROBABILITY},
    {"SECOND_LAMBDA",  TauClusterVars::SECOND_LAMBDA},
    {"CENTER_LAMBDA",  TauClusterVars::CENTER_LAMBDA},
    {"CENTER_MAG",     TauClusterVars::CENTER_MAG},
  };

  inline static const std::unordered_map<std::string, TrackFunc_t> m_trackFuncs = {
    {"dEta",                            TauTrackVars::dEta},
    {"dPhi",                            TauTrackVars::dPhi},
    {"log_pt",                          TausRUsTrackVars::log_pt},
    {"log_e",                           TausRUsTrackVars::log_e},
    {"trackEta",                        TauTrackVars::trackEta},
    {"trackPhi",                        TauTrackVars::trackPhi},
    {"charge",                          TauTrackVars::charge},
    {"qOverP",                          TauTrackVars::qOverP},
    {"d0_old",                          TauTrackVars::d0_old},
    {"z0",                              TausRUsTrackVars::z0},
    {"z0sinthetaTJVA",                  TauTrackVars::z0sinthetaTJVA},
    {"numberOfTRTHits",                 TauTrackVars::numberOfTRTHits},
    {"numberOfTRTHighThresholdHits",    TauTrackVars::numberOfTRTHighThresholdHits},
    {"numberOfSCTHits",                 TauTrackVars::numberOfSCTHits},
    {"numberOfPixelHits",               TauTrackVars::numberOfPixelHits},
    {"numberOfInnermostPixelLayerHits", TauTrackVars::numberOfInnermostPixelLayerHits},
    {"eProbabilityNN_trackParticle",    TausRUsTrackVars::eProbabilityNN_trackParticle},
  };

  inline static const std::unordered_map<std::string, VertexFunc_t> m_vertexFuncs = {
    {"sumPt2", TausRUsVertexVars::sumPt2},
    {"x",      TausRUsVertexVars::x},
    {"y",      TausRUsVertexVars::y},
    {"z",      TausRUsVertexVars::z},
  };

  inline static const std::unordered_map<std::string, ScalarFunc_t> m_scalarFuncs = {
    {"ptJetSeed",  TauScalarVars::ptJetSeed},
    {"etaJetSeed", TauScalarVars::etaJetSeed},
    {"phiJetSeed", TauScalarVars::phiJetSeed},
    {"mJetSeed",   TauScalarVars::mJetSeed},
  };
};

#endif // TAURECTOOLS_TAUSRUSDATALOADER_H
