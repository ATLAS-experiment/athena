/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
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
#include "xAODMuonPrepData/sTgcStripCluster.h"
#include "xAODMuonPrepData/sTgcPadHit.h"
#include "xAODMuonPrepData/sTgcWireHit.h"
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
        if (!meas) {
            return nullptr;
        }
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
    ::Muon::MuonStationIndex::TechnologyIndex toTechnologyIndex(const UncalibMeasType aodType){
        using enum ::Muon::MuonStationIndex::TechnologyIndex;
        switch (aodType){
            case UncalibMeasType::MdtDriftCircleType:
                return MDT;
            case UncalibMeasType::RpcStripType:
                return RPC;
            case UncalibMeasType::TgcStripType:
                return TGC;
            case UncalibMeasType::MMClusterType:
                return MM;
            case UncalibMeasType::sTgcStripType:
                return STGC;
            default:
                return TechnologyUnknown;
        }
    }
    
    std::pair<Amg::Vector2D, AmgSymMatrix(2)> 
        positionAndCovariance(const CombinedMuonStrip* combinedPrd) {

        const Muon::IMuonIdHelperSvc* idHelperSvc = muonReadoutElement(combinedPrd)->idHelperSvc();
        Amg::Vector2D cmbPos {Amg::Vector2D::Zero()};
        AmgSymMatrix(2) cmbCov{AmgSymMatrix(2)::Identity()};

        switch(combinedPrd->type()) {
            using enum UncalibMeasType;
            case RpcStripType: {
                cmbPos[0] = combinedPrd->primaryStrip()->localPosition<1>()[0];
                cmbPos[1] = combinedPrd->secondaryStrip()->localPosition<1>()[0];
                cmbCov (0, 0) = combinedPrd->primaryStrip()->localCovariance<1>()(0,0);
                cmbCov (1, 1) = combinedPrd->secondaryStrip()->localCovariance<1>()(0,0);
                break;
            } case TgcStripType: {
                const auto* wireMeas = static_cast<const TgcStrip*>(combinedPrd->primaryStrip());
                const auto* stripMeas = static_cast<const TgcStrip*>(combinedPrd->secondaryStrip());
            
                const auto& radialDesign = stripMeas->readoutElement()->stripLayout(stripMeas->layerHash());
                const auto& wireDesign = wireMeas->readoutElement()->wireGangLayout(wireMeas->layerHash());            
            
                const double dirDots = radialDesign.stripDir(stripMeas->channelNumber()).dot(wireDesign.stripNormal());
                /// Apply the stereo transform to the covariance
                AmgSymMatrix(2) stereoTrf{AmgSymMatrix(2)::Identity()};
                const double invDist = 1. / (1. - Acts::square(dirDots));
                stereoTrf(0, 0) = stereoTrf(1, 1) = invDist;
                stereoTrf(0, 1) = stereoTrf(1, 0) = -dirDots * invDist;

                cmbPos = stereoTrf* Amg::Vector2D{
                    combinedPrd->primaryStrip()->localPosition<1>()[0],
                    combinedPrd->secondaryStrip()->localPosition<1>()[0]};
                cmbCov (0, 0) = wireMeas->localCovariance<1>()(0,0);
                cmbCov (1, 1) = stripMeas->localCovariance<1>()(0,0);
                break;
            } case sTgcStripType: {
                // combined sTGC Space points can be strip/wire, strip/pad or pad/wire combinations.
                const auto* primMeas = static_cast<const sTgcMeasurement*>(combinedPrd->primaryStrip());
                const auto* secMeas = static_cast<const sTgcMeasurement*>(combinedPrd->secondaryStrip());
                if(primMeas->channelType() == sTgcIdHelper::sTgcChannelTypes::Strip) {
                    cmbPos[0] = primMeas->localPosition<1>()[0];
                    cmbCov(0,0) = primMeas->localCovariance<1>()(0,0);
                } else if (primMeas->channelType() == sTgcIdHelper::sTgcChannelTypes::Pad) {
                    cmbPos[0] = primMeas->localPosition<2>()[0];
                    cmbCov(0,0) = primMeas->localCovariance<2>()(0,0);
                } else {
                    THROW_EXCEPTION("Unexpected secondary measurement type for combined sTGC space point "
                                    <<idHelperSvc->toString(identify(combinedPrd))
                                    << "secondary measurement " << idHelperSvc->toString(identify(secMeas)));
                }
                if(secMeas->channelType() == sTgcIdHelper::sTgcChannelTypes::Wire){
                    cmbPos[1] = secMeas->localPosition<1>()[0];
                    cmbCov(1,1) = secMeas->localCovariance<1>()(0,0);
                } else if (secMeas->channelType() == sTgcIdHelper::sTgcChannelTypes::Pad){
                    cmbPos[1] = secMeas->localPosition<2>()[1];
                    cmbCov(1,1) = secMeas->localCovariance<2>()(1,1);
                } else {
                    THROW_EXCEPTION("Unexpected secondary measurement type for combined sTGC space point "
                                    <<idHelperSvc->toString(identify(combinedPrd))
                                    << "secondary measurement " << idHelperSvc->toString(identify(secMeas)));
                }
                break;
            } default:{
                THROW_EXCEPTION("Unexpected measurement "<<idHelperSvc->toString(identify(combinedPrd)));
                break;
            }
        }
        return std::make_pair(std::move(cmbPos), std::move(cmbCov));
    }
}
