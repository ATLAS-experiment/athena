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

#include "Acts/Surfaces/detail/LineHelper.hpp"
#include "Acts/Utilities/MathHelpers.hpp"
#include "Acts/Definitions/Units.hpp"

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
    


    const Identifier& identify(const UncalibratedMeasurement* meas) {
        static const Identifier& dummyId{};
        if (meas->numDimensions() == 0) {
            return static_cast<const CombinedMuonStrip*>(meas)->primaryStrip()->identify();
        }
        const auto* muon = dynamic_cast<const MuonMeasurement*>(meas);
        return  muon ? muon->identify() : dummyId;
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
        return positionAndCovariance(combinedPrd->primaryStrip(), 
                                     combinedPrd->secondaryStrip());
    }
    
    std::pair<Amg::Vector2D, AmgSymMatrix(2)> positionAndCovariance(const MuonMeasurement* oneDimMeas) {
        /** @brief dummy value to assign to the non-sensitive part of the covariance */
        using namespace Acts::UnitLiterals;
        constexpr double covStrip = Acts::square(1._km);
        Amg::Vector2D pos{Amg::Vector2D::Zero()};
        AmgSymMatrix(2) cov{covStrip * AmgSymMatrix(2)::Identity()};
        /// These conditions should be trivially fullfilled but it's worth
        /// to keep a check for the debug builds
        assert(oneDimMeas != nullptr);
        assert(oneDimMeas->numDimensions() == 1);

        switch (oneDimMeas->type()) {
            using enum UncalibMeasType;
            case MMClusterType: {
                const auto* clust = static_cast<const MMCluster*>(oneDimMeas);
                pos = clust->localMeasurementPos().block<2,1>(0,0);
                cov(0,0) = clust->localCovariance<1>()[0];
                break;
            } case sTgcStripType: {
                const auto* clust = static_cast<const sTgcMeasurement*>(oneDimMeas);
                const unsigned idx = clust->channelType() == sTgcMeasurement::sTgcChannelTypes::Wire;
                pos = clust->localMeasurementPos().block<2,1>(0,0);
                cov(idx, idx) = clust->localCovariance<1>()(0,0);
                break; 
            } case TgcStripType: {
                const auto* stripMeas = static_cast<const TgcStrip*>(oneDimMeas);
                const unsigned idx = stripMeas->measuresPhi();
                pos = stripMeas->localMeasurementPos().block<2,1>(0,0);
                cov(idx, idx) = stripMeas->localCovariance<1>()(0,0);
                if (idx == 1) {
                    const auto& radialDesign = stripMeas->readoutElement()->stripLayout(stripMeas->layerHash());
                    const auto& sensorPlane = stripMeas->readoutElement()->sensorLayout(stripMeas->layerHash());
                    const Amg::Vector3D phiDir = sensorPlane->to3D(radialDesign.stripDir(stripMeas->channelNumber()),true);
                    const Amg::Vector3D etaDir = sensorPlane->to3D(radialDesign.stripNormal(stripMeas->channelNumber()),true);

                    AmgSymMatrix(2) trf{AmgSymMatrix(2)::Zero()};
                    trf.col(1) = etaDir.block<2,1>(0,0);
                    trf.col(0) = phiDir.block<2,1>(0,0);
                    trf = trf.inverse();
                    cov = trf.transpose() * cov * trf;
                }
                break;
            } case RpcStripType: {
                const auto* clust = static_cast<const RpcMeasurement*>(oneDimMeas);
                const unsigned idx = clust->measuresPhi();
                pos = clust->localMeasurementPos().block<2,1>(0,0);
                cov(idx, idx) = clust->localCovariance<1>()(0,0);
                break;
            } case MdtDriftCircleType: {
                const auto* dc = static_cast<const xAOD::MdtDriftCircle*>(oneDimMeas);
                pos[0] = dc->driftRadius();
                cov(0,0) = dc->driftRadiusCov();
                break;
            } default:
                THROW_EXCEPTION("Unsupported measurement");
        }
        return std::make_pair(pos, cov);
    }

    std::pair<Amg::Vector2D, AmgSymMatrix(2)> 
        positionAndCovariance(const MuonMeasurement* etaStrip,
                              const MuonMeasurement* phiStrip) {

        /// These conditions should be trivially fullfilled
        assert(etaStrip != nullptr);
        assert(phiStrip != nullptr);
        assert(etaStrip->identifierHash() == phiStrip->identifierHash());
        assert(etaStrip->type() == phiStrip->type());
        assert(etaStrip->layerHash() == phiStrip->layerHash());

        const Muon::IMuonIdHelperSvc* idHelperSvc = etaStrip->readoutElement()->idHelperSvc();
        Amg::Vector2D cmbPos{Amg::Vector2D::Zero()};
        AmgSymMatrix(2) cmbCov{AmgSymMatrix(2)::Identity()};
        /// Catch the geniue 2D measurements (Pads, 2D RPC)
        if (etaStrip == phiStrip && etaStrip->numDimensions() == 2) {
            return std::make_pair(xAOD::toEigen(etaStrip->localPosition<2>()),
                                  xAOD::toEigen(etaStrip->localCovariance<2>()));
        }
        switch(etaStrip->type()) {
            using enum UncalibMeasType;
            case RpcStripType: {
                cmbPos[0] = etaStrip->localPosition<1>()[0];
                cmbPos[1] = phiStrip->localPosition<1>()[0];
                cmbCov (0, 0) = etaStrip->localCovariance<1>()(0,0);
                cmbCov (1, 1) = phiStrip->localCovariance<1>()(0,0);
                break;
            } case TgcStripType: {
                const auto* wireMeas = static_cast<const TgcStrip*>(etaStrip);
                const auto* stripMeas = static_cast<const TgcStrip*>(phiStrip);
            
                const auto& radialDesign = stripMeas->readoutElement()->stripLayout(stripMeas->layerHash());
                const auto& wireDesign = wireMeas->readoutElement()->wireGangLayout(wireMeas->layerHash());
                const auto& sensorPlane = wireMeas->readoutElement()->sensorLayout(stripMeas->layerHash());
                const Amg::Vector3D phiDir = sensorPlane->to3D(radialDesign.stripDir(stripMeas->channelNumber()),true);
                const Amg::Vector3D etaDir = sensorPlane->to3D(wireDesign.stripDir(), false);
                // Calculate the combined strip position
                using namespace Acts::detail::LineHelper;
                const Acts::Intersection3D stripIsect = lineIntersect<3>(stripMeas->localMeasurementPos(), phiDir,
                                                                         wireMeas->localMeasurementPos(), etaDir);
            
                cmbPos = stripIsect.position().block<2,1>(0,0);
                
                const double dirDots = phiDir.dot(etaDir);
                /// Apply the stereo transform to the covariance
                AmgSymMatrix(2) stereoTrf{AmgSymMatrix(2)::Identity()};
                const double invDist = 1. / (1. - Acts::square(dirDots));
                stereoTrf(0, 0) = stereoTrf(1, 1) = invDist;
                stereoTrf(0, 1) = stereoTrf(1, 0) = -dirDots * invDist;

                
                AmgSymMatrix(2) basisTrf{AmgSymMatrix(2)::Identity()};
                basisTrf.row(0) = etaDir.block<2,1>(0,0);
                basisTrf.row(1) = phiDir.block<2,1>(0,0);

                stereoTrf = (stereoTrf * basisTrf).inverse();



                cmbCov(1, 1) = wireMeas->localCovariance<1>()(0,0);
                cmbCov(0, 0) = stripMeas->localCovariance<1>()(0,0);
                cmbCov = stereoTrf * cmbCov * stereoTrf.transpose();
                break;
            } case sTgcStripType: {
                // combined sTGC Space points can be strip/wire, strip/pad or pad/wire combinations.
                const auto* primMeas = static_cast<const sTgcMeasurement*>(etaStrip);
                const auto* secMeas = static_cast<const sTgcMeasurement*>(phiStrip);
                if(primMeas->channelType() == sTgcIdHelper::sTgcChannelTypes::Strip) {
                    cmbPos[0] = primMeas->localPosition<1>()[0];
                    cmbCov(0,0) = primMeas->localCovariance<1>()(0,0);
                } else if (primMeas->channelType() == sTgcIdHelper::sTgcChannelTypes::Pad) {
                    cmbPos[0] = primMeas->localPosition<2>()[0];
                    cmbCov(0,0) = primMeas->localCovariance<2>()(0,0);
                } else {
                    THROW_EXCEPTION("Unexpected secondary measurement type for combined sTGC space point "
                                    <<idHelperSvc->toString(etaStrip->identify())
                                    << "secondary measurement " << idHelperSvc->toString(phiStrip->identify()));
                }
                if(secMeas->channelType() == sTgcIdHelper::sTgcChannelTypes::Wire){
                    cmbPos[1] = secMeas->localPosition<1>()[0];
                    cmbCov(1,1) = secMeas->localCovariance<1>()(0,0);
                } else if (secMeas->channelType() == sTgcIdHelper::sTgcChannelTypes::Pad){
                    cmbPos[1] = secMeas->localPosition<2>()[1];
                    cmbCov(1,1) = secMeas->localCovariance<2>()(1,1);
                } else {
                    THROW_EXCEPTION("Unexpected secondary measurement type for combined sTGC space point "
                                    <<idHelperSvc->toString(etaStrip->identify())
                                    << "secondary measurement " << idHelperSvc->toString(secMeas->identify()));
                }
                break;
            } default:{
                THROW_EXCEPTION("Unexpected measurement "<<idHelperSvc->toString(etaStrip->identify()));
                break;
            }
        }
        return std::make_pair(std::move(cmbPos), std::move(cmbCov));
    }
}
