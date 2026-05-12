/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRK_DATAPREPARATION_HGTD_TIMEDCLUSTERING_TOOL_H
#define ACTSTRK_DATAPREPARATION_HGTD_TIMEDCLUSTERING_TOOL_H

#include "HgtdClusteringToolBase.h"

#include "Acts/Definitions/Units.hpp"

namespace ActsTrk {
class HgtdAuxDataCache;

template <typename T_RDOCollection>
struct HgtdCollectionAdapter;

class HgtdTimedClusteringTool :
    public HgtdClusteringToolBase {
public:
    using BASE = HgtdClusteringToolBase;
    using BASE::BASE;

    virtual StatusCode initialize() override;

    virtual StatusCode clusterize(const EventContext& /*ctx*/,
                                  const IHGTDClusteringTool::RawDataCollectionVariant& RDOs,
                                  IHGTDClusteringTool::CellContainer &cellContainer) const override;

private:
  Gaudi::Property<double> m_timeTollerance {this, "TimeTollerance", 0.035 * Acts::UnitConstants::ns};
  Gaudi::Property<bool> m_addCorners {this, "AddCorners", true};
};

}

#endif

