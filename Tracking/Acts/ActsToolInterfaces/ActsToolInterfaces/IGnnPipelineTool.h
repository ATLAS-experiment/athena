/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTOOLINTERFACES_IGNNPIPELINETOOL_H
#define ACTSTOOLINTERFACES_IGNNPIPELINETOOL_H

// Athena
#include "GaudiKernel/IAlgTool.h"

// ACTS EDM
#include "ActsEvent/SeedContainer.h"
#include "xAODInDetMeasurement/SpacePointContainer.h"

// Others
#include <vector>

namespace ActsTrk {

/// Interface for the GNN seeding pipeline tool.
class IGnnPipelineTool : virtual public IAlgTool {
 public:
  DeclareInterfaceID(IGnnPipelineTool, 1, 0);

  virtual ~IGnnPipelineTool() = default;

  /** @brief Build seeds from the input space points using the GNN pipeline.
   *         The pipeline performs graph construction, edge classification and
   *         track building to group space points into seed candidates.
   *  @param spacePointCollections: Input space point containers to seed from
   *  @param seeds: Output container the produced seeds are appended to. The
   *         space points of each seed are ordered by increasing radius.
   *  @return: StatusCode::SUCCESS on success, StatusCode::FAILURE otherwise */
  virtual StatusCode buildSeed(
      const std::vector<const xAOD::SpacePointContainer*>&
          spacePointCollections,
      ActsTrk::SeedContainer& seeds) const = 0;
};

}  // namespace ActsTrk

#endif
