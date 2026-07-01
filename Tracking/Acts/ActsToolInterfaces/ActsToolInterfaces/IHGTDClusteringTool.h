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

    struct Cluster {
        std::vector<Identifier::value_type> ids;
        std::vector<int> tots;
        std::vector<double> times;
    };
    using ClusterCollection = std::vector<Cluster>;

    virtual StatusCode
    clusterize(const EventContext& ctx,
	       const RawDataCollection& RDOs,
               std::vector<ClusterCollection>& collection) const = 0;

    virtual StatusCode
    clusterize(const EventContext& ctx,
        const HGTD_ALTIROC_RDO_Collection& RDOs,
        std::vector<ClusterCollection>& collection) const = 0;

    virtual std::any
    createEventDataCache(xAOD::HGTDClusterContainer& cont,
                         std::size_t nClusterRDOs) const = 0;

    virtual StatusCode
    makeClusters(const EventContext& ctx,
                 const ClusterCollection& cluster,
                 xAOD::HGTDClusterContainer& container,
                 size_t& icluster,
                 std::any& cache) const = 0;
};

}

#endif

