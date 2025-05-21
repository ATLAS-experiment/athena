/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "ActsCalibration/xAODUncalibMeasSurfAcc.h"
#include "ActsCalibration/xAODUncalibMeasCalibrator.h"

#include "ActsGeometry/SurfaceOfMeasurementUtil.h"
#include "xAODMuonPrepData/UtilFunctions.h"

namespace ActsTrk::detail{
    xAODUncalibMeasSurfAcc::xAODUncalibMeasSurfAcc(const Acts::TrackingGeometry* trackGeom,
                                                   const DetectorElementToActsGeometryIdMap* assocMap):
        m_actsTrackingGeometry{trackGeom},
        m_detectorElementToGeometryIdMap{assocMap} {}
           
    const Acts::Surface* xAODUncalibMeasSurfAcc::operator()(const Acts::SourceLink& sourceLink) const {
        return get(xAODUncalibMeasCalibrator::unpack(sourceLink));
    }
    const Acts::Surface* xAODUncalibMeasSurfAcc::get(const xAOD::UncalibratedMeasurement* meas) const {
        switch (meas->type()) {
            using enum xAOD::UncalibMeasType;
            /** ID measurements use the tracking geometry surface id look up to fetch the surface */
            case PixelClusterType:
            case StripClusterType:
            case HGTDClusterType:
                return getSurfaceOfMeasurement(*m_actsTrackingGeometry,*m_detectorElementToGeometryIdMap, *meas);
            /** Muon measurements have a direct link to the readout geometry -> surface */
            case MdtDriftCircleType:
            case RpcStripType:
            case TgcStripType:
            case sTgcStripType:
            case MMClusterType:
                return &xAOD::muonSurface(meas);
            default:
                break;
              
        }
#ifndef NDEBUG
        throw std::domain_error("xAODUncalibMeasSurfAcc() - Cannot decode surface type");
#endif        
        return nullptr;
    }
}