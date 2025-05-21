/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSCALIBRATION_DETAIL_MEASUREMENTCALIBRATORBASE_H
#define ACTSCALIBRATION_DETAIL_MEASUREMENTCALIBRATORBASE_H

#include "GeoPrimitives/GeoPrimitives.h"

#include "Acts/EventData/Types.hpp"
#include "Acts/Surfaces/SurfaceBounds.hpp"
#include "Acts/EventData/MultiTrajectory.hpp"
#include "Acts/EventData/TrackStateProxy.hpp"
#include "Acts/Utilities/CalibrationContext.hpp"

#include "ActsCalibration/SourceLinkType.h"

#include <array>

namespace ActsTrk::detail {
  /** @brief Base class providing the boiler code to fill the Acts multi trajectory track states. 
   *         The states are filled with the n-dimensional local position & covariance, the source link
   *         to the ATLAS measurement and finally the projector is configured. A simple enum represents
   *         all tracking measurement configurations exisiting in ATLAS. */
  class MeasurementCalibratorBase {
  public:
    MeasurementCalibratorBase() = default;
    /** @brief Enum encoding the possible projectors used in ATLAS. Their integer representations 
     *         correspond to the element index in the s_boundSpaceIndices member */
    enum class ProjectorType{
        e1DimNoTime = 0,      /// Project out solely the locX (Applies to Itk strips, Rpc, Tgc, sTgc, Mm)
        e1DimRotNoTime = 1,   /// Project out solely the locY - Complementary projector if the strip plane is rotated
                              ///                              (Applies to Itk endcap strips, Rpc, Tgc, sTgc)
        e2DimNoTime = 2,      /// Project out the two spatial coordinates - (Applies to ITk pixel, BI-Rpc, sTgc pad)
        e1DimWithTime = 3,    /// Project out the locX & time coordinate - (Applies to Rpc, Tgc, Mm, sTgc)  
        e1DimRotWithTime = 4, /// Project out the locY & time coordinate - (Applies to Rpc, Tgc, sTgc)
        e2DimWithTime = 5,    /// Project out the two spatial coordinate & time - (Applies to HGTD)
    };

    /** @brief Abbrivation of the track state proxy type */
    template<typename trajectory_t>
    using TrackState_t = typename Acts::MultiTrajectory<trajectory_t>::TrackStateProxy;
    /** @brief Abbrivation of the const track state proxy type */
    template<typename trajectory_t>
    using ConstTrackState_t = typename Acts::MultiTrajectory<trajectory_t>::ConstTrackStateProxy;
    
    /** @brief Copy the local position & covariance into the Acts track state proxy.
     *  @tparam Dim: Dimension of the measurement
     *  @tparam trajectory_t: Data type of the track state proxy backend
     *  @tparam pos_t: Data type of the [Dim x 1] position vector
     *  @tparam cov_t: Data type of the [Dim x Dim] covariance matrix
     *  @param projector: Projector configuration of the measurement
     *  @param locpos: Calibrated local postion
     *  @param cov: Calibrated local covariance
     *  @param link: Source link to associate with the state
     *  @param trackState: Refrence to the track state proxy to write.  */
    template <std::size_t Dim, typename trajectory_t, 
              typename pos_t, typename cov_t>
      void setState(const ProjectorType projector,
                    const pos_t& locpos,
                    const cov_t& cov,
                    Acts::SourceLink link,
                    TrackState_t<trajectory_t>& trackState) const;
  private:
    /** @brief Array to map the Projector types to the bound index configurations  used
     *         by the ATLAS detector measurements */
    constexpr static std::array<Acts::BoundSubspaceIndices, 6> s_boundSpaceIndices{
        Acts::BoundSubspaceIndices{Acts::eBoundLoc0}, // One dimenion without time
        Acts::BoundSubspaceIndices{Acts::eBoundLoc1}, // Complementary one dimension without time
        Acts::BoundSubspaceIndices{Acts::eBoundLoc0, Acts::eBoundLoc1}, /// Two dimensions without time
        Acts::BoundSubspaceIndices{Acts::eBoundLoc0, Acts::eBoundTime}, // One dimension with time
        Acts::BoundSubspaceIndices{Acts::eBoundLoc1, Acts::eBoundTime}, // Complementary one dimension with time
        Acts::BoundSubspaceIndices{Acts::eBoundLoc0, Acts::eBoundLoc1, Acts::eBoundTime} /// Two dimensions with time
    };
  };
  
} // namespace ActsTrk::detail

#include "ActsCalibration/MeasurementCalibratorBase.icc"

#endif
