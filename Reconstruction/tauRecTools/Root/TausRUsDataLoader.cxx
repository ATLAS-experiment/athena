/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "tauRecTools/TausRUsDataLoader.h"

#include "AthContainers/ConstAccessor.h"
#include "FourMomUtils/xAODP4Helpers.h"
#include "xAODTracking/TrackParticle.h"

#include <algorithm>
#include <cmath>

namespace {

float logNonzero(double raw) {
  return raw == 0. ? 0.f : static_cast<float>(std::log(std::max(raw, 1e-8)));
}

/// Write the variables of @p sequence for each of @p constituents into a
/// zero-padded (maxObjects, nVariables) tensor. A function returning false
/// leaves its slot at zero.
template <class Sequence, class Constituent>
std::vector<float> fillTensor(const Sequence& sequence, const xAOD::TauJet& tau,
                              const std::vector<const Constituent*>& constituents) {
  const size_t nVariables = sequence.funcs.size();
  std::vector<float> tensor(sequence.maxObjects * nVariables, 0.f);
  for (size_t iObject = 0; iObject < constituents.size(); ++iObject) {
    for (size_t iVar = 0; iVar < nVariables; ++iVar) {
      float value = 0.f;
      if (sequence.funcs[iVar](tau, *constituents[iObject], value)) {
        tensor[iObject * nVariables + iVar] =
          (value + sequence.offsets[iVar]) * sequence.scales[iVar];
      }
    }
  }
  return tensor;
}

} // anonymous namespace

// --------------------------------------------------------------------------
// MARK: TausRUs-only variables
// --------------------------------------------------------------------------

// uncorrected cluster variables
namespace TausRUsClusterVars {

bool dEtaRaw(const xAOD::TauJet& tau, const xAOD::CaloVertexedTopoCluster& cluster, float& out) {
  out = cluster.clust().eta() - tau.eta();
  return true;
}

bool dPhiRaw(const xAOD::TauJet& tau, const xAOD::CaloVertexedTopoCluster& cluster, float& out) {
  out = xAOD::P4Helpers::deltaPhi(cluster.clust().phi(), tau.phi());
  return true;
}

bool etaRaw(const xAOD::TauJet&, const xAOD::CaloVertexedTopoCluster& cluster, float& out) {
  out = cluster.clust().eta();
  return true;
}

bool phiRaw(const xAOD::TauJet&, const xAOD::CaloVertexedTopoCluster& cluster, float& out) {
  out = cluster.clust().phi();
  return true;
}

bool log_etRaw(const xAOD::TauJet&, const xAOD::CaloVertexedTopoCluster& cluster, float& out) {
  out = logNonzero(cluster.clust().et());
  return true;
}

bool log_eRaw(const xAOD::TauJet&, const xAOD::CaloVertexedTopoCluster& cluster, float& out) {
  out = logNonzero(cluster.clust().e());
  return true;
}

} // namespace TausRUsClusterVars

namespace TausRUsTrackVars {

bool log_pt(const xAOD::TauJet&, const xAOD::TauTrack& track, float& out) {
  out = logNonzero(track.track()->pt());
  return true;
}

bool log_e(const xAOD::TauJet&, const xAOD::TauTrack& track, float& out) {
  out = logNonzero(track.track()->e());
  return true;
}

bool z0(const xAOD::TauJet&, const xAOD::TauTrack& track, float& out) {
  out = track.track()->z0();
  return true;
}

bool eProbabilityNN_trackParticle(const xAOD::TauJet&, const xAOD::TauTrack& track, float& out) {
  static const SG::ConstAccessor<float> acc("eProbabilityNN");
  if (!acc.isAvailable(*track.track())) return false;
  out = acc(*track.track());
  return true;
}

} // namespace TausRUsTrackVars

namespace TausRUsVertexVars {

// A padded vertex slot stays all zero; the model counts a slot as a real
// vertex if any of its variables is nonzero.
bool sumPt2(const xAOD::TauJet&, const xAOD::Vertex& vertex, float& out) {
  static const SG::ConstAccessor<float> acc("sumPt2");
  if (!acc.isAvailable(vertex)) return false;
  out = acc(vertex);
  return true;
}

bool x(const xAOD::TauJet&, const xAOD::Vertex& vertex, float& out) {
  out = vertex.x();
  return true;
}

bool y(const xAOD::TauJet&, const xAOD::Vertex& vertex, float& out) {
  out = vertex.y();
  return true;
}

bool z(const xAOD::TauJet&, const xAOD::Vertex& vertex, float& out) {
  out = vertex.z();
  return true;
}

} // namespace TausRUsVertexVars

// --------------------------------------------------------------------------
// MARK: TausRUsDataLoader
// --------------------------------------------------------------------------

TausRUsDataLoader::TausRUsDataLoader(const std::string& name)
  : asg::AsgMessaging(name) {}

template <class Func> StatusCode TausRUsDataLoader::resolve(const InputConfig& input,
                                      const std::unordered_map<std::string, Func>& funcMap,
                                      Sequence<Func>& sequence) const {
  if (!sequence.name.empty()) {
    ATH_MSG_ERROR("Inputs '" << sequence.name << "' and '" << input.name
                  << "' are both built from " << input.collection);
    return StatusCode::FAILURE;
  }
  sequence.name = input.name;
  sequence.maxObjects = input.maxObjects;
  for (const VariableConfig& variable : input.variables) {
    const auto func = funcMap.find(variable.name);
    if (func == funcMap.end()) {
      ATH_MSG_ERROR("Variable '" << variable.name << "' of input '" << input.name
                    << "' is not defined for " << input.collection);
      return StatusCode::FAILURE;
    }
    sequence.funcs.push_back(func->second);
    sequence.offsets.push_back(variable.offset);
    sequence.scales.push_back(variable.scale);
  }
  ATH_MSG_INFO("TausRUs input '" << input.name << "' (" << input.collection << "): "
               << input.maxObjects << " objects x " << input.variables.size() << " variables");
  return StatusCode::SUCCESS;
}

StatusCode TausRUsDataLoader::initialize(const std::vector<InputConfig>& inputs) {
  for (const InputConfig& input : inputs) {
    if (input.collection == "clusters") {
      ATH_CHECK(resolve(input, m_clusterFuncs, m_clusters));
    } else if (input.collection == "tracks") {
      ATH_CHECK(resolve(input, m_trackFuncs, m_tracks));
    } else if (input.collection == "vertices") {
      ATH_CHECK(resolve(input, m_vertexFuncs, m_vertices));
    } else if (input.collection == "seedjet") {
      ATH_CHECK(resolve(input, m_scalarFuncs, m_scalars));
    } else {
      ATH_MSG_ERROR("Input '" << input.name << "' is built from unknown collection '"
                    << input.collection << "'");
      return StatusCode::FAILURE;
    }
  }
  return StatusCode::SUCCESS;
}

std::vector<const xAOD::CaloVertexedTopoCluster*> TausRUsDataLoader::selectClusters(
    const xAOD::TauJet& tau,
    std::vector<xAOD::CaloVertexedTopoCluster>& storage) const {
  storage = tau.vertexedClusters();

  std::vector<const xAOD::CaloVertexedTopoCluster*> selected;
  selected.reserve(storage.size());
  for (const xAOD::CaloVertexedTopoCluster& cluster : storage) {
    selected.push_back(&cluster);
  }
  std::sort(selected.begin(), selected.end(),
            [](const xAOD::CaloVertexedTopoCluster* lhs,
               const xAOD::CaloVertexedTopoCluster* rhs) {
              return lhs->clust().e() > rhs->clust().e();
            });
  if (selected.size() > m_clusters.maxObjects) selected.resize(m_clusters.maxObjects);
  return selected;
}

std::vector<const xAOD::TauTrack*> TausRUsDataLoader::selectTracks(const xAOD::TauJet& tau) const {
  std::vector<const xAOD::TauTrack*> tracks = tau.allTracks();
  std::erase_if(tracks, [](const xAOD::TauTrack* track) {
    return track == nullptr || track->track() == nullptr;
  });
  std::sort(tracks.begin(), tracks.end(),
            [](const xAOD::TauTrack* lhs, const xAOD::TauTrack* rhs) {
              return lhs->track()->pt() > rhs->track()->pt();
            });
  if (tracks.size() > m_tracks.maxObjects) tracks.resize(m_tracks.maxObjects);
  return tracks;
}

std::vector<const xAOD::Vertex*> TausRUsDataLoader::selectVertices(
    const xAOD::VertexContainer& vertices) const {
  std::vector<const xAOD::Vertex*> selected;
  selected.reserve(std::min(vertices.size(), m_vertices.maxObjects));
  for (const xAOD::Vertex* vertex : vertices) {
    if (selected.size() == m_vertices.maxObjects) break;
    if (!vertex) continue;
    const int type = vertex->vertexType();
    if (type == 0 || type == -99) continue;
    selected.push_back(vertex);
  }
  return selected;
}

AthInfer::InputDataMap TausRUsDataLoader::loadInputs(const xAOD::TauJet& tau,
                              const xAOD::VertexContainer& vertices) const {
  AthInfer::InputDataMap inputData;

  auto add = [&inputData](const auto& sequence, std::vector<float> tensor, bool isScalar) {
    const auto nVariables = static_cast<int64_t>(sequence.funcs.size());
    std::vector<int64_t> shape = isScalar
      ? std::vector<int64_t>{1, nVariables}
      : std::vector<int64_t>{1, static_cast<int64_t>(sequence.maxObjects), nVariables};
    inputData[sequence.name] = std::make_pair(std::move(shape), std::move(tensor));
  };

  if (!m_clusters.name.empty()) {
    std::vector<xAOD::CaloVertexedTopoCluster> storage;
    add(m_clusters, fillTensor(m_clusters, tau, selectClusters(tau, storage)), false);
  }
  if (!m_tracks.name.empty()) {
    add(m_tracks, fillTensor(m_tracks, tau, selectTracks(tau)), false);
  }
  if (!m_vertices.name.empty()) {
    add(m_vertices, fillTensor(m_vertices, tau, selectVertices(vertices)), false);
  }
  if (!m_scalars.name.empty()) {
    std::vector<float> tensor(m_scalars.funcs.size(), 0.f);
    for (size_t iVar = 0; iVar < m_scalars.funcs.size(); ++iVar) {
      float value = 0.f;
      if (m_scalars.funcs[iVar](tau, value)) {
        tensor[iVar] = (value + m_scalars.offsets[iVar]) * m_scalars.scales[iVar];
      }
    }
    add(m_scalars, std::move(tensor), true);
  }
  return inputData;
}
