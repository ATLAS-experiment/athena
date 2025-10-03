/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "xAODMuonPrepData/UtilFunctions.h"
#include "GeoModelKernel/throwExcept.h"

#include "GeoPrimitives/GeoPrimitives.h"
#include "MuonReadoutGeometryR4/MdtReadoutElement.h"
#include "MuonReadoutGeometryR4/RpcReadoutElement.h"
#include "MuonReadoutGeometryR4/TgcReadoutElement.h"
#include "MuonReadoutGeometryR4/MmReadoutElement.h"
#include "MuonReadoutGeometryR4/sTgcReadoutElement.h"
#include "MuonReadoutGeometryR4/SpectrometerSector.h"

#include "xAODMeasurementBase/MeasurementDefs.h"
#include "xAODMuonPrepData/MdtDriftCircle.h"
#include "xAODMuonPrepData/MdtTwinDriftCircle.h"
#include "xAODMuonPrepData/RpcStrip.h"
#include "xAODMuonPrepData/RpcStrip2D.h"
#include "xAODMuonPrepData/RpcMeasurement.h"
#include "xAODMuonPrepData/TgcStrip.h"
#include "xAODMuonPrepData/MMCluster.h"
#include "xAODMuonPrepData/sTgcMeasurement.h"
#include "TrkEventPrimitives/ParamDefs.h"

namespace {
    template <class MeasType> const Acts::Surface& fetchSurface(const xAOD::UncalibratedMeasurement* meas) {
        auto castedM = static_cast<const MeasType*>(meas);
        IdentifierHash hash{};
        if constexpr(std::is_same_v<xAOD::MdtDriftCircle, MeasType>) {
            hash = castedM->measurementHash();
        } else {
            hash = castedM->layerHash();
        }
        return castedM->readoutElement()->surface(hash);
    }
}

namespace xAOD{
    const MuonGMR4::MuonReadoutElement* muonReadoutElement(const UncalibratedMeasurement* meas){
         if (!meas) return nullptr;
        switch (meas->type()) {
            case UncalibMeasType::MdtDriftCircleType:{
                return static_cast<const MdtDriftCircle*>(meas)->readoutElement();
            } case UncalibMeasType::RpcStripType: {
                return static_cast<const RpcMeasurement*>(meas)->readoutElement();
            } case UncalibMeasType::TgcStripType:{
                return static_cast<const TgcStrip*>(meas)->readoutElement();
            } case UncalibMeasType::sTgcStripType:{
                return static_cast<const sTgcMeasurement*>(meas)->readoutElement();
            } case UncalibMeasType::MMClusterType:{
                return static_cast<const MMCluster*>(meas)->readoutElement();
            } default:
#ifndef NDEBUG
                THROW_EXCEPTION("Unsupported measurement given "<<typeid(*meas).name());
#endif
                break;
        }
        return nullptr;
    }
    const Acts::Surface& muonSurface(const xAOD::UncalibratedMeasurement* meas) {
        if (!meas) {
            THROW_EXCEPTION("No measurement passed");
        }
        switch (meas->type()) {
            case UncalibMeasType::MdtDriftCircleType:{
                return fetchSurface<MdtDriftCircle>(meas);
            } case UncalibMeasType::RpcStripType: {
                return fetchSurface<RpcMeasurement>(meas);
            } case UncalibMeasType::TgcStripType:{
                return fetchSurface<TgcStrip>(meas);
            } case UncalibMeasType::sTgcStripType:{
                return fetchSurface<sTgcMeasurement>(meas);
            } case UncalibMeasType::MMClusterType:{
                return fetchSurface<MMCluster>(meas);
            } default:
                THROW_EXCEPTION("Unsupported measurement given "<<typeid(*meas).name());
                break;
        }
    }

    const Identifier& identify(const UncalibratedMeasurement* meas) {
        static const Identifier detId{};
        if (!meas) {
            return detId;
        }
        switch (meas->type()) {
            case UncalibMeasType::MdtDriftCircleType :{
                return static_cast<const MdtDriftCircle*>(meas)->identify();
            } case UncalibMeasType::RpcStripType: {
                return static_cast<const RpcMeasurement*>(meas)->identify();
            } case UncalibMeasType::TgcStripType: {
                return static_cast<const TgcStrip*>(meas)->identify();
            } case UncalibMeasType::MMClusterType: {
                return static_cast<const MMCluster*>(meas)->identify();
            } case UncalibMeasType::sTgcStripType: {
                return static_cast<const sTgcMeasurement*>(meas)->identify();
            } default: {
#ifndef NDEBUG
                THROW_EXCEPTION("Unsupported measurement given "<<typeid(*meas).name());
#endif
                break;
            }
        }
        return detId;
    }
}
