/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_CLUSTER_CLUSTER_DEF_H
#define COLUMNAR_CLUSTER_CLUSTER_DEF_H

#include <ColumnarCore/ContainerId.h>
#include <xAODCaloEvent/CaloClusterContainer.h>
#include <xAODCaloEvent/CaloCluster.h>

namespace columnar
{
  template<> struct ContainerIdTraits<ContainerId::cluster> final
  {
    static constexpr bool isDefined = true;
    static constexpr bool isMutable = false;
    static constexpr bool perEventRange = true;
    static constexpr bool perEventId = false;

    /// the xAOD type to use with ObjectId
    using xAODObjectIdType = const xAOD::CaloCluster;

    /// the xAOD type to use with ObjectRange
    using xAODObjectRangeType = const xAOD::CaloClusterContainer;

    /// the xAOD type to use with ElementLink
    using xAODElementLinkType = xAOD::CaloClusterContainer;
  };
}

#endif
