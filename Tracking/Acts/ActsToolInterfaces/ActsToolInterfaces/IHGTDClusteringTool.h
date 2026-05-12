/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTOOLINTERFACES_IHGTDCLUSTERINGTOOL_H
#define ACTSTOOLINTERFACES_IHGTDCLUSTERINGTOOL_H

#include <HGTD_RawData/HGTD_RDO_Container.h>
#include <HGTD_RawData/HGTD_ALTIROC_RDO_Container.h>
#include <xAODInDetMeasurement/HGTDClusterContainer.h>
#include "xAODInDetMeasurement/HGTDClusterAuxContainer.h"
#include "ICellClusteringToolBase.h"

#include <variant>

namespace ActsTrk {

// forward declaration (defintion  Tracking/Acts/ActsDataPreparation/src/details/CellContainer.h)
template <typename coordinates_t, std::size_t NDIM, std::unsigned_integral index_t>
struct CellContainer;

class IHGTDClusteringTool : virtual public IAlgTool {
public:
    DeclareInterfaceID(IHGTDClusteringTool, 1, 0);

    using RDOContainer = HGTD_RDO_Container;
    using RDOContainerVariant = std::variant<const RDOContainer *, const HGTD_ALTIROC_RDO_Container* >;
    using RawDataCollection = RDOContainer::base_value_type;
    using RawDataCollectionVariant = std::variant<const RawDataCollection *,const HGTD_ALTIROC_RDO_Collection * >;
   
    using ClusterContainer = xAOD::HGTDClusterContainer;
    using ClusterAuxContainer = xAOD::HGTDClusterAuxContainer;

    using CellContainer = ActsTrk::CellContainer<std::int8_t,3,std::uint16_t>;

    virtual std::pair<unsigned int, unsigned int>
    countCells(const RDOContainerVariant &RDOs,
               const std::vector<IdentifierHash> &listOfIds) const =0;

    virtual StatusCode
    clusterize(const EventContext& ctx,
	       const RawDataCollectionVariant& RDOs,
               CellContainer& cellContainer) const = 0;

    virtual std::any
    createEventDataCache(xAOD::HGTDClusterContainer& cont,
                         std::size_t nClusterRDOs) const = 0;

    virtual StatusCode
    makeClusters(const EventContext& ctx,
                 const RDOContainerVariant &rdoContainer,
                 const CellContainer& cellContainer,
                 unsigned int module_i,
                 unsigned int icluster,
                 xAOD::HGTDClusterContainer& container,
                 std::any& cache) const =0;
};

}

#endif

