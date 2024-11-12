/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRACKRECONSTRUCTION_DETAIL_TRKMEASUREMENTCALIBRATOR_H
#define ACTSTRACKRECONSTRUCTION_DETAIL_TRKMEASUREMENTCALIBRATOR_H

#include "src/detail/MeasurementCalibratorBase.h"

#include "Acts/EventData/Types.hpp"
#include "Acts/EventData/SourceLink.hpp"
#include "TrkMeasurementBase/MeasurementBase.h"
#include "xAODMeasurementBase/MeasurementDefs.h"
#include "xAODMeasurementBase/MeasurementDefs.h"
#include "TrkEventPrimitives/LocalParameters.h"
#include "ActsEventCnv/IActsToTrkConverterTool.h"

namespace ActsTrk::detail {

  class TrkMeasurementCalibrator
    : public MeasurementCalibratorBase {
  private:

    // internal class definition
    class MeasurementAdapter {
    public:
      MeasurementAdapter(const Trk::MeasurementBase &measurement);

      xAOD::UncalibMeasType type() const;

      template <std::size_t DIM>
      inline const Trk::LocalParameters& localPosition() const;

      template <std::size_t DIM>
      inline const Amg::MatrixX &localCovariance() const;
      
   private:
      const Trk::MeasurementBase *m_measurement {nullptr};
   };

    // class definition
  public:
    TrkMeasurementCalibrator(const ActsTrk::IActsToTrkConverterTool &converter_tool);
    
    template <typename trajectory_t>
    void calibrate([[maybe_unused]] const Acts::GeometryContext &gctx,
                   [[maybe_unused]] const Acts::CalibrationContext & cctx,
                   const Acts::SourceLink& sl,
                   typename Acts::MultiTrajectory<trajectory_t>::TrackStateProxy trackState) const;

  private:
    const ActsTrk::IActsToTrkConverterTool *m_converterTool {nullptr};
  };
  
}

#include "src/detail/TrkMeasurementCalibrator.icc"

#endif
