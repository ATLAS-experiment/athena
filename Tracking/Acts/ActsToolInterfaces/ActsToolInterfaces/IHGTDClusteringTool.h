/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTOOLINTERFACES_IHGTDCLUSTERINGTOOL_H
#define ACTSTOOLINTERFACES_IHGTDCLUSTERINGTOOL_H

#include <GaudiKernel/IAlgTool.h>
#include <HGTD_RawData/HGTD_RDO_Container.h>
#include <HGTD_RawData/HGTD_ALTIROC_RDO_Container.h>
#include <xAODInDetMeasurement/HGTDClusterContainer.h>
#include "xAODInDetMeasurement/HGTDClusterAuxContainer.h"

namespace ActsTrk {

class IHGTDClusteringTool : virtual public IAlgTool {
public:
    DeclareInterfaceID(IHGTDClusteringTool, 1, 0);

    using RDOContainer = HGTD_RDO_Container;
    using RawDataCollection = RDOContainer::base_value_type;
    using ClusterContainer = xAOD::HGTDClusterContainer;
    using ClusterAuxContainer = xAOD::HGTDClusterAuxContainer;

    virtual StatusCode
    clusterize(const EventContext& ctx,
	       const RawDataCollection& RDOs,
	       ClusterContainer& container) const = 0;
    
    virtual StatusCode
    clusterize(const EventContext& ctx,
        const HGTD_ALTIROC_RDO_Collection& RDOs,
        ClusterContainer& container) const = 0;
};

}

#endif

