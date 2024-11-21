/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRACKRECONSTRUCTION_DETAIL_MEASUREMENTCALIBRATORBASE_H
#define ACTSTRACKRECONSTRUCTION_DETAIL_MEASUREMENTCALIBRATORBASE_H

#include "xAODMeasurementBase/MeasurementDefs.h"
#include "Acts/EventData/Types.hpp"
#include "Acts/Surfaces/SurfaceBounds.hpp"
#include "Acts/EventData/MultiTrajectory.hpp"
#include "Acts/EventData/TrackStateProxy.hpp"
#include "Acts/Utilities/CalibrationContext.hpp"

#include <array>

namespace ActsTrk::detail {
  
  class MeasurementCalibratorBase {
  public:
    MeasurementCalibratorBase() = default;
    
    template <typename state_t>
    inline void setProjector(xAOD::UncalibMeasType measType,
                             Acts::SurfaceBounds::BoundsType boundType,
                             state_t &trackState ) const;
    
    template <std::size_t Dim,
	      typename pos_t,
	      typename cov_t,
	      typename state_t>
    inline void setState(xAOD::UncalibMeasType measType,
			 const pos_t& locpos,
			 const cov_t& cov,
			 Acts::SurfaceBounds::BoundsType boundType,
			 state_t &trackState) const;
    
    template <class measurement_t, typename trajectory_t>
    inline void setStateFromMeasurement(const measurement_t &measurement,
					Acts::SurfaceBounds::BoundsType bound_type,
					typename Acts::MultiTrajectory<trajectory_t>::TrackStateProxy &trackState) const;
    
  protected:
    constexpr static std::array<Acts::BoundSubspaceIndices, 2> s_stripSubspaceIndices = {
      Acts::BoundSubspaceIndices{Acts::eBoundLoc0}, // normal strip: x -> l0
      Acts::BoundSubspaceIndices{Acts::eBoundLoc1} // annulus strip: y -> l0
    };
    
    constexpr static Acts::BoundSubspaceIndices s_pixelSubspaceIndices = {
      Acts::eBoundLoc0, Acts::eBoundLoc1
    };
   constexpr static Acts::BoundSubspaceIndices s_hgtdSubspaceIndices = {
      Acts::eBoundLoc0, Acts::eBoundLoc1, Acts::eBoundTime
    };
  };
  
} // namespace ActsTrk::detail

#include "src/detail/MeasurementCalibratorBase.icc"

#endif
