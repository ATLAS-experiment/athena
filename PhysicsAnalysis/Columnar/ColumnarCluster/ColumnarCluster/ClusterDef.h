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
  namespace ContainerId
  {
    struct cluster : regularCIBase<xAOD::CaloCluster,xAOD::CaloClusterContainer>
    {
      /// a unique internal identifier for this container
      static constexpr std::string_view idName = "cluster";
    };
  }

  using ClusterId = ObjectId<ContainerId::cluster>;
  using OptClusterId = OptObjectId<ContainerId::cluster>;
  template<typename CT,typename CM=ColumnarModeDefault> using ClusterAccessor  = AccessorTemplate<ContainerId::cluster,CT,ColumnAccessMode::input,CM>;
  template<typename CT,typename CM=ColumnarModeDefault> using ClusterDecorator = AccessorTemplate<ContainerId::cluster,CT,ColumnAccessMode::output,CM>;
}

#endif
