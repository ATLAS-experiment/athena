/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSPATTERNRECOGNITION_GNNSEEDINGTOOL_H
#define ACTSPATTERNRECOGNITION_GNNSEEDINGTOOL_H

// ATHENA
#include "ActsToolInterfaces/IGnnPipelineTool.h"
#include "ActsToolInterfaces/ISeedingTool.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"

// Other
#include <string>
#include <vector>

namespace ActsTrk {

/// Seeding tool that delegates seed finding to the GNN pipeline and
/// exposes the result through the ISeedingTool interface.
/// Each GNN track candidate is passed on as one (long) seed.
class GnnSeedingTool : public extends<AthAlgTool, ActsTrk::ISeedingTool> {
 public:
  GnnSeedingTool(const std::string& type, const std::string& name,
                 const IInterface* parent);

  virtual StatusCode initialize() override;

  virtual StatusCode createSeeds(
      const EventContext& ctx,
      const std::vector<const xAOD::SpacePointContainer*>&
          spacePointCollections,
      const Eigen::Vector3f& beamSpotPos, float bFieldInZ,
      ActsTrk::SeedContainer& seedContainer) const override;

 private:
  ToolHandle<ActsTrk::IGnnPipelineTool> m_gnnPipelineTool{
      this, "GnnPipelineTool", "", "GNN seeding pipeline"};
};

}  // namespace ActsTrk

#endif
