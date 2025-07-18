/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "ActsCalibrators/xAODUncalibMeasSurfAcc.h"
#include "ActsCalibrators/xAODUncalibMeasCalibrator.h"
#include "ActsGeometry/DetectorElementToActsGeometryIdMap.h"
#include "ActsGeometry/SurfaceOfMeasurementUtil.h"
#include "xAODAuxillaryMeasurement/AuxillaryMeasurement.h"
#include "xAODMuonPrepData/UtilFunctions.h"

namespace ActsTrk::detail{
    xAODUncalibMeasSurfAcc::xAODUncalibMeasSurfAcc(const ActsTrk::ITrackingGeometryTool* trackGeoTool):
        m_actsTrackingGeometry{trackGeoTool->trackingGeometry().get()},
        m_detectorElementToGeometryIdMap{trackGeoTool->surfaceIdMap()}{}
           
    const Acts::Surface* xAODUncalibMeasSurfAcc::operator()(const Acts::SourceLink& sourceLink) const {
        return get(xAODUncalibMeasCalibrator::unpack(sourceLink));
    }
    const Acts::Surface* xAODUncalibMeasSurfAcc::get(const xAOD::UncalibratedMeasurement* meas) const {
        switch (meas->type()) {
            using enum xAOD::UncalibMeasType;
            /** ID measurements use the tracking geometry surface id look up to fetch the surface */
            case PixelClusterType:
            case StripClusterType:
            case HGTDClusterType:{
                assert(m_detectorElementToGeometryIdMap);
                assert(m_actsTrackingGeometry);
                const auto geoKey = makeDetectorElementKey(meas->type(), meas->identifierHash());
                const auto geoid_iter = m_detectorElementToGeometryIdMap->find(geoKey);
                if (geoid_iter == m_detectorElementToGeometryIdMap->end()) {
                    return nullptr;
                }
                return m_actsTrackingGeometry->findSurface( DetectorElementToActsGeometryIdMap::getValue(*geoid_iter));
            }
            /** Muon measurements have a direct link to the readout geometry -> surface */
            case MdtDriftCircleType:
            case RpcStripType:
            case TgcStripType:
            case sTgcStripType:
            case MMClusterType:{
                return &xAOD::muonSurface(meas);
                break;
            }case Other: {
                const auto* pMeas = static_cast<const xAOD::AuxillaryMeasurement*>(meas);
                return pMeas->surface().get();
            }

            default:
                break;
              
        }
#ifndef NDEBUG
        throw std::domain_error("xAODUncalibMeasSurfAcc() - Cannot decode surface type");
#endif        
        return nullptr;
    }
}