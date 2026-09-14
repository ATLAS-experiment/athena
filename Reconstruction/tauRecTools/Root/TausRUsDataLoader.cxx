/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "tauRecTools/TausRUsDataLoader.h"

#include <algorithm>

namespace {

using Source = TausRUsModel::Source;

/// Write one constituent (or the single scalar slot) into @p slot, which must
/// point at variables.size() floats. @p args are whatever the getters of this
/// tensor take ahead of the output value.
template <class Variable, class... Args>
void fillSlot(const std::vector<Variable>& variables, float* slot, const Args&... args) {
  for (size_t iVar = 0; iVar < variables.size(); ++iVar) {
    float value = 0.f;
    if (variables[iVar].get(args..., value)) {
      slot[iVar] = variables[iVar].norm.apply(value);
    }
  }
}

} // anonymous namespace

TausRUsDataLoader::TausRUsDataLoader(const std::string& name)
  : asg::AsgMessaging(name) {}

StatusCode TausRUsDataLoader::initialize() {
  m_inputs = TausRUsModel::inputs();

  for (const TausRUsModel::Input& input : m_inputs) {
    if (input.nVariables() == 0) {
      ATH_MSG_ERROR("Input '" << input.name << "' declares no variables");
      return StatusCode::FAILURE;
    }
    if (input.source != Source::Scalar && input.maxConstituents == 0) {
      ATH_MSG_ERROR("Input '" << input.name << "' is a constituent tensor and needs a "
                    "non-zero maxConstituents");
      return StatusCode::FAILURE;
    }
    ATH_MSG_INFO("TausRUs input '" << input.name << "': "
                 << input.maxConstituents << " constituents x "
                 << input.nVariables() << " variables");
  }

  return StatusCode::SUCCESS;
}

std::vector<int64_t> TausRUsDataLoader::inputShape(const TausRUsModel::Input& input,
                                                   size_t nTaus) const {
  const auto nVariables = static_cast<int64_t>(input.nVariables());
  if (input.source == Source::Scalar) {
    return {static_cast<int64_t>(nTaus), nVariables};
  }
  return {static_cast<int64_t>(nTaus),
          static_cast<int64_t>(input.maxConstituents),
          nVariables};
}

std::vector<const xAOD::CaloVertexedTopoCluster*>
TausRUsDataLoader::selectClusters(const xAOD::TauJet& tau, size_t maxConstituents,
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
  if (selected.size() > maxConstituents) selected.resize(maxConstituents);
  return selected;
}

std::vector<const xAOD::TauTrack*>
TausRUsDataLoader::selectTracks(const xAOD::TauJet& tau, size_t maxConstituents) const {
  std::vector<const xAOD::TauTrack*> tracks = tau.allTracks();
  std::erase_if(tracks, [](const xAOD::TauTrack* track) {
    return track == nullptr || track->track() == nullptr;
  });
  std::sort(tracks.begin(), tracks.end(),
            [](const xAOD::TauTrack* lhs, const xAOD::TauTrack* rhs) {
              return lhs->track()->pt() > rhs->track()->pt();
            });
  if (tracks.size() > maxConstituents) tracks.resize(maxConstituents);
  return tracks;
}

std::vector<const xAOD::Vertex*>
TausRUsDataLoader::selectVertices(const xAOD::VertexContainer& vertices,
                                  size_t maxConstituents) const {
  std::vector<const xAOD::Vertex*> selected;
  selected.reserve(std::min(vertices.size(), maxConstituents));
  for (const xAOD::Vertex* vertex : vertices) {
    if (!vertex) continue;
    const int type = vertex->vertexType();
    if (type == 0 || type == -99) continue;
    selected.push_back(vertex);
    if (selected.size() == maxConstituents) break;
  }
  return selected;
}

void TausRUsDataLoader::fillTensor(const TausRUsModel::Input& input,
                                   std::span<const xAOD::TauJet* const> taus,
                                   const xAOD::VertexContainer& vertices,
                                   std::vector<float>& tensor) const {
  const size_t nVariables = input.nVariables();
  const size_t nSlots = (input.source == Source::Scalar) ? 1 : input.maxConstituents;

  // Zero-padded by construction: only the slots backed by a real constituent are written
  tensor.assign(taus.size() * nSlots * nVariables, 0.f);

  size_t offset = 0;
  for (const xAOD::TauJet* tau : taus) {
    const size_t tauOffset = offset;
    offset += nSlots * nVariables;
    if (!tau) continue;

    switch (input.source) {
      case Source::Scalar: {
        fillSlot(input.scalarVariables, &tensor[tauOffset], *tau);
        break;
      }
      case Source::Cluster: {
        std::vector<xAOD::CaloVertexedTopoCluster> storage;
        const auto clusters = selectClusters(*tau, nSlots, storage);
        for (size_t iSlot = 0; iSlot < clusters.size(); ++iSlot) {
          fillSlot(input.clusterVariables, &tensor[tauOffset + iSlot * nVariables],
                   *tau, *clusters[iSlot]);
        }
        break;
      }
      case Source::TauTrack: {
        const auto tracks = selectTracks(*tau, nSlots);
        for (size_t iSlot = 0; iSlot < tracks.size(); ++iSlot) {
          fillSlot(input.trackVariables, &tensor[tauOffset + iSlot * nVariables],
                   *tau, *tracks[iSlot]);
        }
        break;
      }
      case Source::Vertex: {
        const auto selected = selectVertices(vertices, nSlots);
        for (size_t iSlot = 0; iSlot < selected.size(); ++iSlot) {
          fillSlot(input.vertexVariables, &tensor[tauOffset + iSlot * nVariables],
                   *tau, *selected[iSlot]);
        }
        break;
      }
    }
  }
}

AthInfer::InputDataMap
TausRUsDataLoader::loadInputs(std::span<const xAOD::TauJet* const> taus,
                              const xAOD::VertexContainer& vertices) const {
  AthInfer::InputDataMap inputData;
  for (const TausRUsModel::Input& input : m_inputs) {
    std::vector<float> tensor;
    fillTensor(input, taus, vertices, tensor);
    inputData[input.name] =
      std::make_pair(inputShape(input, taus.size()), std::move(tensor));
  }
  return inputData;
}
