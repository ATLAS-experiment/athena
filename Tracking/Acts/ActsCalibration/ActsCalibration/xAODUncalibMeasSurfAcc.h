/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSCALIBRATION_DETAIL_XAODUNCALIBMEASSURFACC_H
#define ACTSCALIBRATION_DETAIL_XAODUNCALIBMEASSURFACC_H

#include "Acts/EventData/SourceLink.hpp"
#include "Acts/Geometry/TrackingGeometry.hpp"

#include "xAODMeasurementBase/UncalibratedMeasurement.h"
#include "ActsGeometryInterfaces/IActsTrackingGeometryTool.h"

namespace ActsTrk {
    class DetectorElementToActsGeometryIdMap;
}
namespace ActsTrk::detail{
    /** @brief Helper class to access the Acts::surface associated with an Uncalibrated xAOD measurement.
     *         In the ID domain, the Acts identifier of the uncalibrated measurement is looked-up in the
     *         DetectorElementToActsGeometryIdMap and then the tracking geometry is used to return the 
     *         corresponding surface. In the muon domain, all measurements have a link to the associated
     *         readout element and the surface look-up is performed via a helper funciton. */
    class xAODUncalibMeasSurfAcc {
        public:
            /** @brief Empty default constructor -> conversion will crash for ID measurements */
            xAODUncalibMeasSurfAcc() = default;
            /** @brief Constructor taking the pointer to the Tracking geometry tool. 
             *         The tracking geometry and the surface association map are directly retrieved
             *         from the tool. 
             *  @param trackGeoTool: Pointer to the tracking geometry tool*/
            xAODUncalibMeasSurfAcc(const IActsTrackingGeometryTool* trackGeoTool);
            /** @brief Operator called by the Acts API to fetch the surface. */
            const Acts::Surface* operator()(const Acts::SourceLink& sourceLink) const;
            /** @brief Operator */
            const Acts::Surface* get(const xAOD::UncalibratedMeasurement* meas) const;
        private:
            const Acts::TrackingGeometry *m_actsTrackingGeometry{nullptr};
            const DetectorElementToActsGeometryIdMap *m_detectorElementToGeometryIdMap{nullptr};

    };
}

#endif