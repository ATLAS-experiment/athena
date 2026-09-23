/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TAURECTOOLS_TAUSRUSDATALOADER_H
#define TAURECTOOLS_TAUSRUSDATALOADER_H

#include "tauRecTools/TausRUsModel.h"

#include "AthOnnxInterfaces/IAthInferenceTool.h"

#include "AsgMessaging/AsgMessaging.h"
#include "AsgMessaging/StatusCode.h"

#include "xAODCaloEvent/CaloVertexedTopoCluster.h"
#include "xAODTau/TauJet.h"
#include "xAODTau/TauTrack.h"
#include "xAODTracking/Vertex.h"
#include "xAODTracking/VertexContainer.h"

#include <cstdint>
#include <span>
#include <string>
#include <vector>

/**
 * @brief Builds the fixed-size input tensors for the TausRUs network.
 *
 * The network takes four separate tensors with pinned shapes (clusters, tau
 * tracks, event vertices, seed-jet scalars). Constituents are sorted and
 * truncated to a fixed maximum and the remaining slots are zero-padded, making
 * each tensor rectangular. The variables names and order come directly from TausRUsModel.
 */
class TausRUsDataLoader : public asg::AsgMessaging {
public:
  explicit TausRUsDataLoader(const std::string& name);


  StatusCode initialize();

  AthInfer::InputDataMap loadInputs(std::span<const xAOD::TauJet* const> taus,
                                    const xAOD::VertexContainer& vertices) const;

  std::vector<int64_t> inputShape(const TausRUsModel::Input& input, size_t nTaus) const;

  /// Tracks of @p tau, sorted by descending pt and truncated
  std::vector<const xAOD::TauTrack*> selectTracks(const xAOD::TauJet& tau,
                                                  size_t maxConstituents) const;
  /// Event vertices, dummy/unset types removed and truncated.
  std::vector<const xAOD::Vertex*> selectVertices(const xAOD::VertexContainer& vertices,
                                                  size_t maxConstituents) const;

private:

  /// Clusters of @p tau, sorted by descending energy and truncated. 
  std::vector<const xAOD::CaloVertexedTopoCluster*> selectClusters(
    const xAOD::TauJet& tau, size_t maxConstituents,
    std::vector<xAOD::CaloVertexedTopoCluster>& storage) const;

  void fillTensor(const TausRUsModel::Input& input,
                  std::span<const xAOD::TauJet* const> taus,
                  const xAOD::VertexContainer& vertices,
                  std::vector<float>& tensor) const;

  std::vector<TausRUsModel::Input> m_inputs;
};

#endif // TAURECTOOLS_TAUSRUSDATALOADER_H
