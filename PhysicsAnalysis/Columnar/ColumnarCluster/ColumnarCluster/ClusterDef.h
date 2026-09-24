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
  struct ClusterDef : RegularContainerId<xAOD::CaloCluster,xAOD::CaloClusterContainer>
  {
    /// a unique internal identifier for this container
    static constexpr std::string_view idName = "cluster";
  };

  template<ColumnarMode CM>
  using ClusterId = ObjectId<ClusterDef, CM>;
  template<ColumnarMode CM>
  using OptClusterId = OptObjectId<ClusterDef, CM>;
  template<typename CT,ColumnarMode CM> using ClusterAccessor  = AccessorTemplate<ClusterDef,CT,ColumnAccessMode::input,CM>;
  template<typename CT,ColumnarMode CM> using ClusterDecorator = AccessorTemplate<ClusterDef,CT,ColumnAccessMode::output,CM>;
}

#endif
