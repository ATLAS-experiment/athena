/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "src/detail/TrkMeasurementCalibrator.h"

namespace ActsTrk::detail {

  // internal class implementation
  TrkMeasurementCalibrator::MeasurementAdapter::MeasurementAdapter(const Trk::MeasurementBase &measurement)
    : m_measurement(&measurement)
  {}

  xAOD::UncalibMeasType TrkMeasurementCalibrator::MeasurementAdapter::type() const
  {
    switch (m_measurement->localParameters().dimension()) {
    case 1: return xAOD::UncalibMeasType::StripClusterType;
    case 2:  return xAOD::UncalibMeasType::PixelClusterType;
    case 3:  return xAOD::UncalibMeasType::HGTDClusterType;
    default: return xAOD::UncalibMeasType::Other;
    }
  }
  
  // class implementation
  TrkMeasurementCalibrator::TrkMeasurementCalibrator(const ActsTrk::IActsToTrkConverterTool &converter_tool)
    : m_converterTool(&converter_tool)
  {}

} // namespace ActsTrk::detail
