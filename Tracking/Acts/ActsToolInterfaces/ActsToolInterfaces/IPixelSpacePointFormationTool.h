/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTOOLINTERFACES_IPIXELSPACEPOINTFORMATIONTOOL_H
#define ACTSTOOLINTERFACES_IPIXELSPACEPOINTFORMATIONTOOL_H

// Athena
#include "GaudiKernel/IAlgTool.h"

#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "xAODInDetMeasurement/PixelCluster.h"
#include "xAODInDetMeasurement/SpacePointContainer.h"

#include "xAODInDetMeasurement/PixelClusterAuxDataCache.h"

namespace ActsTrk {

    /// @class IPixelSpacePointFormationTool
    /// Base class for pixel space point formation tool

    class IPixelSpacePointFormationTool : virtual public IAlgTool {
    public:
        DeclareInterfaceID(IPixelSpacePointFormationTool, 1, 0);
        using PixelCluster_t = traits::ElementProxies<PixelClusterAuxDataCache<Utils::AccessPolicy::Const> >::ClusterProxy<Utils::AccessPolicy::Const>;

        /// @name Production of space points
        //@{
	virtual StatusCode producePixelSpacePoint(const PixelCluster_t& cluster,
						  xAOD::SpacePoint& sp,
						  const InDetDD::SiDetectorElement& element) const = 0;
        //@}

    };

} // ACTSTOOLINTERFACES_IPIXELSPACEPOINTFORMATIONTOOL_H

#endif


