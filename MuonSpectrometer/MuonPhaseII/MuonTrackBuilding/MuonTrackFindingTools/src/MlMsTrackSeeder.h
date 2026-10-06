/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONTRACKFINDINGALGS_MLMSTRACKSEEDER_H
#define MUONTRACKFINDINGALGS_MLMSTRACKSEEDER_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "MuonRecToolInterfacesR4/ITrackSeedingTool.h"
#include "StoreGate/ReadDecorHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "xAODMuon/MuonSegmentContainer.h"

#include <span>
#include <string>

namespace MuonR4 {

/** @brief ITrackSeedingTool implementation which constructs seeds directly
 *         from sufficiently large ML-nominated segment components.
 *
 * The wrapped conventional tool is used only for initial-parameter estimation;
 * it never constructs seeds for this tool. Consequently MsTrackFindingAlg sees
 * only the standard ITrackSeedingTool interface and selects this implementation
 * entirely through job configuration.
 */
class MlMsTrackSeeder final :
    public extends<AthAlgTool, ITrackSeedingTool> {
 public:
  using base_class::base_class;

  StatusCode initialize() override final;

  /** @copydoc ITrackSeedingTool::findTrackSeeds */
  StatusCode findTrackSeeds(const EventContext& ctx,
                            std::vector<MsTrackSeed>& outSeeds) const override final;

  /** @copydoc ITrackSeedingTool::estimateStartParameters */
  Acts::Result<Acts::BoundTrackParameters> estimateStartParameters(
      const EventContext& ctx,
      const MsTrackSeed& seed) const override final;

 private:
  ToolHandle<ITrackSeedingTool> m_seedParameterEstimator{
      this, "BaselineSeeder", "",
      "Tool providing initial-parameter estimates for ML-built seeds"};
  SG::ReadHandleKey<xAOD::MuonSegmentContainer> m_segmentKey{
      this, "SegmentContainer", "MuonSegmentsFromR4"};
  SG::ReadDecorHandleKey<xAOD::MuonSegmentContainer> m_candidateDecorKey{
      this, "CandidateDecoration", m_segmentKey, "trackCandidateIds",
      "Scheduler dependency on the ML candidate decoration"};
  Gaudi::Property<unsigned int> m_minSegmentsPerCandidate{
      this, "MinSegmentsPerCandidate", 2};
  Gaudi::Property<unsigned int> m_maxSegmentsPerCandidate{
      this, "MaxSegmentsPerCandidate", 0,
      "Discard ML components with more than this many segments; zero disables the cap"};
  Gaudi::Property<unsigned int> m_minLayers{
      this, "MinLayers", 2, "Minimum number of distinct muon layers a seed must span"};
  Gaudi::Property<double> m_minCosConsistency{
      this, "MinCosConsistency", 0.819,  // cos(35 deg), matching SegmentEdgeClassifierTool's default MaxDeltaThetaDeg
      "Minimum cos(angle) between a segment's direction and its seed anchor's "
      "direction for the segment to be included in the seed"};
};

}  // namespace MuonR4

#endif
