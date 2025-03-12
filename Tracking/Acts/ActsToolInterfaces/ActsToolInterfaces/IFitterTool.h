/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTOOLINTERFACES_IFITTERTOOL_H
#define ACTSTOOLINTERFACES_IFITTERTOOL_H

#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/EventContext.h"

#include "ActsGeometry/ATLASSourceLink.h"
#include "ActsGeometry/DetectorElementToActsGeometryIdMap.h"

#include "ActsEvent/TrackContainer.h"
#include "ActsEvent/Seed.h"

#include "Acts/EventData/TrackParameters.hpp"
#include "Acts/Geometry/GeometryContext.hpp"
#include "Acts/MagneticField/MagneticFieldContext.hpp"
#include "Acts/Utilities/CalibrationContext.hpp"
#include "xAODMeasurementBase/UncalibratedMeasurement.h"
#include "TrkTrack/Track.h"
namespace ActsTrk {

  class IFitterTool : virtual public IAlgTool {
  public:
    DeclareInterfaceID(IFitterTool, 1, 0);

    virtual    
      std::unique_ptr< ActsTrk::MutableTrackContainer >
      fit(const EventContext& ctx,
	  const ActsTrk::Seed &seed,
	  const Acts::BoundTrackParameters& initialParams,
	  const Acts::GeometryContext& tgContext,
	  const Acts::MagneticFieldContext& mfContext,
	  const Acts::CalibrationContext& calContext,
	  const DetectorElementToActsGeometryIdMap &detector_element_to_geoid) const = 0;

    /// @brief development interface for EF tracking usage. 
    virtual    
      std::unique_ptr< ActsTrk::MutableTrackContainer >
      fit(const EventContext& ctx,
	    const std::vector<ActsTrk::ATLASUncalibSourceLink> & clusterList,
      const Acts::BoundTrackParameters& initialParams,
      const Acts::GeometryContext& tgContext,
      const Acts::MagneticFieldContext& mfContext,
      const Acts::CalibrationContext& calContext,
      const DetectorElementToActsGeometryIdMap &detector_element_to_geoid,
      const Acts::Surface* targetSurface = nullptr) const = 0;

    virtual
     StatusCode fit(const EventContext& ctx,
		    const ActsTrk::TrackContainer::ConstTrackProxy& track,          
		    ActsTrk::MutableTrackContainer& trackContainer) const = 0;
    
  };
  

}

#endif
