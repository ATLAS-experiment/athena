/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSCALIBRATION_DETAIL_TRKMEASUREMENTCALIBRATOR_H
#define ACTSCALIBRATION_DETAIL_TRKMEASUREMENTCALIBRATOR_H

#include "ActsCalibBase/MeasurementCalibratorBase.h"
//
#include "Acts/EventData/Types.hpp"
#include "Acts/EventData/SourceLink.hpp"
#include "TrkEventPrimitives/LocalParameters.h"
#include "TrkMeasurementBase/MeasurementBase.h"

namespace ActsTrk::detail {
  /** @brief Calibrator class that links the legacy Trk::MeasurementBase objects
   *         with the Acts MultiTrajectory track state proxies without applying any
   *         further calibrating the measurement  */
  class TrkMeasurementCalibrator: public MeasurementCalibratorBase {
    
    public:
      /** @brief Unpacks the Acts::SourceLink to a Trk measurement */
      static const Trk::MeasurementBase* unpack(const Acts::SourceLink& sl);
      /** @brief Default constructor */
      TrkMeasurementCalibrator() = default;
      /** @brief Calibrator delegate implementation to calibrate the ActsTrk fit from 
       *         Trk::MeasurementBase objects
       *  @tparam trajectory_t: Tepmlate parameter of the underlying MultTrajectory container backend
       *  @param gctx: Geometry context to access the alignment of the surface
       *  @param cctx: Calibration context to access the calibration constants from the conditions store
       *  @param sl: Reference to the packed SourceLink
       *  @param trackState: Reference to the multi trajectory track state to fill */
      template <Acts::TrackStateProxyConcept proxy_t>
      void calibrate(const Acts::GeometryContext &gctx,
                     const Acts::CalibrationContext & cctx,
                     const Acts::SourceLink& sl,
                     proxy_t trackState) const;
  };
}

#include "ActsCalibrators/TrkMeasurementCalibrator.icc"

#endif
