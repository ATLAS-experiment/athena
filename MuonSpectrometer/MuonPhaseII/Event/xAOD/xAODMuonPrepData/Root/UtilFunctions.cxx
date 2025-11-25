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
#include "xAODMuonPrepData/CombinedMuonStrip.h" 


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
        /// Composite space point EDM
        if (meas->numDimensions() == 0u) {
           const auto* comp = static_cast<const CombinedMuonStrip*>(meas);
           return muonReadoutElement(comp->primaryStrip());
        }
        switch (meas->type()) {
            using enum UncalibMeasType;
            case MdtDriftCircleType: {
                return static_cast<const MdtDriftCircle*>(meas)->readoutElement();
            } case RpcStripType: {
                return static_cast<const RpcMeasurement*>(meas)->readoutElement();
            } case TgcStripType: {
                return static_cast<const TgcStrip*>(meas)->readoutElement();
            } case sTgcStripType: {
                return static_cast<const sTgcMeasurement*>(meas)->readoutElement();
            } case MMClusterType: {
                return static_cast<const MMCluster*>(meas)->readoutElement();
            } default:
#ifndef NDEBUG
                THROW_EXCEPTION("Unsupported measurement given "<<typeid(*meas).name());
#endif
                break;
        }
        return nullptr;
    }
    const Acts::Surface& muonSurface(const UncalibratedMeasurement* meas) {
        if (!meas) {
            THROW_EXCEPTION("No measurement passed");
        }
        /// Composite space point EDM
        if (meas->numDimensions() == 0u) {
           const auto* comp = static_cast<const CombinedMuonStrip*>(meas);
           return muonSurface(comp->primaryStrip());
        }
        switch (meas->type()) {
            using enum UncalibMeasType;
            case MdtDriftCircleType: {
                return fetchSurface<MdtDriftCircle>(meas);
            } case RpcStripType: {
                return fetchSurface<RpcMeasurement>(meas);
            } case TgcStripType:{
                return fetchSurface<TgcStrip>(meas);
            } case sTgcStripType: {
                return fetchSurface<sTgcMeasurement>(meas);
            } case MMClusterType:{
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
        /// Composite space point EDM
        if (meas->numDimensions() == 0u) {
           const auto* comp = static_cast<const CombinedMuonStrip*>(meas);
           return identify(comp->primaryStrip());
        }
        switch (meas->type()) {
            using enum UncalibMeasType;
            case MdtDriftCircleType: {
                return static_cast<const MdtDriftCircle*>(meas)->identify();
            } case RpcStripType: {
                return static_cast<const RpcMeasurement*>(meas)->identify();
            } case TgcStripType: {
                return static_cast<const TgcStrip*>(meas)->identify();
            } case MMClusterType: {
                return static_cast<const MMCluster*>(meas)->identify();
            } case sTgcStripType: {
                return static_cast<const sTgcMeasurement*>(meas)->identify();
            } case Other: {
                return detId;
            } default: {
                THROW_EXCEPTION("Unsupported measurement given "<<typeid(*meas).name());
                break;
            }
        }
        return detId;
    }
    IdentifierHash layerHash(const UncalibratedMeasurement* meas) {
        if (!meas) {
            return IdentifierHash{};
        }
                /// Composite space point EDM
        if (meas->numDimensions() == 0u) {
           const auto* comp = static_cast<const CombinedMuonStrip*>(meas);
           return layerHash(comp->primaryStrip());
        }
        switch (meas->type()) {
            using enum UncalibMeasType;
            case MdtDriftCircleType: {
                return static_cast<const MdtDriftCircle*>(meas)->measurementHash();
            } case RpcStripType: {
                return static_cast<const RpcMeasurement*>(meas)->layerHash();
            } case TgcStripType: {
                return static_cast<const TgcStrip*>(meas)->layerHash();
            } case MMClusterType: {
                return static_cast<const MMCluster*>(meas)->layerHash();
            } case sTgcStripType: {
                return static_cast<const sTgcMeasurement*>(meas)->layerHash();
            } case Other: {
                break;
            } default: {
                THROW_EXCEPTION("Unsupported measurement given "<<typeid(*meas).name());
                break;
            }
        }
        return IdentifierHash{};
    }
}
