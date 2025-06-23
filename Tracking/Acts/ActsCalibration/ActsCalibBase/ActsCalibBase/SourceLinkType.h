/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSCALIBBASE_SOURCELINKTYPE_H
#define ACTSCALIBBASE_SOURCELINKTYPE_H

namespace ActsTrk::detail{
    /** @brief Enumeration to distinguish between the ATLAS EDM -> Acts::SourceLink variants */
    enum class SourceLinkType {
        TrkMeasurement, /// Calibrated Trk::MeasurementBase objects
        TrkPrepRawData, /// UnCalibrated Trk::PrepRawData objects
        xAODUnCalibMeas, /// Uncalbirated xAOD::UnCalibratedMEeasurement objects
        nTypes /// Number of source link types
    };
}
#endif