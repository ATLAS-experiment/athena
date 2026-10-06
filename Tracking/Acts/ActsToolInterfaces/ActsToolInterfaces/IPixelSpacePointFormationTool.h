/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTOOLINTERFACES_IPIXELSPACEPOINTFORMATIONTOOL_H
#define ACTSTOOLINTERFACES_IPIXELSPACEPOINTFORMATIONTOOL_H

// Athena
#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/EventContext.h"

#include "ActsGeometryInterfaces/GeometryDefs.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "xAODInDetMeasurement/PixelCluster.h"
#include "xAODInDetMeasurement/SpacePointContainer.h"
#include "xAODInDetMeasurement/PixelClusterContainer.h"

#include "Acts/Geometry/GeometryContext.hpp"

namespace ActsTrk {

    /// @class IPixelSpacePointFormationTool
    /// Base class for pixel space point formation tool

    class IPixelSpacePointFormationTool : virtual public IAlgTool {
    public:
        DeclareInterfaceID(IPixelSpacePointFormationTool, 1, 0);

        /// @name Production of space points
        //@{
	/// @param gctx only valid if usesGeometryContext() returns true
	virtual StatusCode producePixelSpacePoint(const EventContext& ctx,
						  const Acts::GeometryContext& gctx,
						  const xAOD::PixelCluster& cluster,
						  xAOD::SpacePoint& sp,
						  const InDetDD::SiDetectorElement& element) const = 0;
        //@}

        /// Whether the caller has to provide a valid geometry context, and hence
        /// declare a dependency on the aligned geometry
        virtual bool usesGeometryContext() const { return false; }

    };

} // ACTSTOOLINTERFACES_IPIXELSPACEPOINTFORMATIONTOOL_H

#endif


