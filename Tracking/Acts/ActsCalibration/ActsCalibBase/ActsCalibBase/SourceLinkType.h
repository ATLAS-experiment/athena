/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSCALIBBASE_SOURCELINKTYPE_H
#define ACTSCALIBBASE_SOURCELINKTYPE_H


#include "Acts/Utilities/OstreamFormatter.hpp"
namespace ActsTrk::detail{
    /** @brief Enumeration to distinguish between the ATLAS EDM -> Acts::SourceLink variants */
    enum class SourceLinkType {
        TrkMeasurement, /// Calibrated Trk::MeasurementBase objects
        TrkPrepRawData, /// UnCalibrated Trk::PrepRawData objects
        xAODUnCalibMeas, /// Uncalbirated xAOD::UnCalibratedMeasurement objects
        nTypes /// Number of source link types
    };
    inline std::ostream& operator<<(std::ostream& ostr, const SourceLinkType sl) {
        switch (sl) {
            using enum SourceLinkType;
            case TrkMeasurement: 
                ostr<<"TrkMeasurement";
                break;
            case TrkPrepRawData: 
                ostr<<"TrkPrepRawData";
                break;
            case xAODUnCalibMeas:
                ostr<<"xAODUnCalibMeas";
                break;
            case nTypes: 
                ostr<<"nTypes";
                break;
        }
        return ostr;
    }
}
ACTS_OSTREAM_FORMATTER(ActsTrk::detail::SourceLinkType);
#endif