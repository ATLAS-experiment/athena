/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRK_DATAPREPARATION_HGTD_CLUSTERING_TOOL_H
#define ACTSTRK_DATAPREPARATION_HGTD_CLUSTERING_TOOL_H

#include "HgtdClusteringToolBase.h"

namespace ActsTrk {

class HgtdClusteringTool :
    public HgtdClusteringToolBase {
public:
    using BASE = HgtdClusteringToolBase;
    using BASE::BASE;

    virtual StatusCode clusterize(const EventContext& /*ctx*/,
                                  const IHGTDClusteringTool::RawDataCollectionVariant& RDOs,
                                  IHGTDClusteringTool::CellContainer &cellContainer) const override;
private:
    BooleanProperty m_sortByLocalx{this, "SortByLocalX", false, "Sort cluster by local-x"};
};
}
#endif

