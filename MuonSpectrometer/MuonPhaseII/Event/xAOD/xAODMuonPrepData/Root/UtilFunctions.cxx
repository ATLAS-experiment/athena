/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "xAODMuonPrepData/UtilFunctions.h"
#include "GeoModelHelpers/throwExcept.h"
#include "xAODMuonPrepData/MdtDriftCircle.h"
#include "xAODMuonPrepData/MdtTwinDriftCircle.h"
#include "xAODMuonPrepData/RpcStrip.h"
#include "xAODMuonPrepData/RpcStrip2D.h"
#include "xAODMuonPrepData/RpcMeasurement.h"
#include "xAODMuonPrepData/TgcStrip.h"
#include "xAODMuonPrepData/MMCluster.h"
#include "xAODMuonPrepData/sTgcMeasurement.h"
#include "xAODMuonPrepData/versions/AccessorMacros.h"
#include "MuonReadoutGeometryR4/MuonChamber.h"
#include "TrkEventPrimitives/ParamDefs.h"

namespace {
        template<class MeasType> Amg::Transform3D toChamberTransform(const ActsGeometryContext& gctx,
                                                                     const MeasType* unCalibMeas) {
        
        IdentifierHash hash{};
        if constexpr(std::is_same_v<MeasType, xAOD::MdtDriftCircle> ||
                     std::is_same_v<MeasType, xAOD::MdtTwinDriftCircle>) {
            hash = unCalibMeas->measurementHash();
        } else {
            hash = unCalibMeas->layerHash();
        }
        const MuonGMR4::MuonReadoutElement* reEle{unCalibMeas->readoutElement()};
        return reEle->getChamber()->globalToLocalTrans(gctx) * reEle->localToGlobalTrans(gctx, hash);
    }
}

namespace xAOD{
    const MuonGMR4::MuonReadoutElement* readoutElement(const UncalibratedMeasurement* meas){
        if (!meas) return nullptr;
        if (meas->type() == UncalibMeasType::MdtDriftCircleType){
            return static_cast<const MdtDriftCircle*>(meas)->readoutElement();
        } else if (meas->type() == UncalibMeasType::RpcStripType) {
            return static_cast<const RpcMeasurement*>(meas)->readoutElement();
        } else if (meas->type() == UncalibMeasType::TgcStripType) {
            return static_cast<const TgcStrip*>(meas)->readoutElement();
        } else if (meas->type() == UncalibMeasType::MMClusterType) {
            return static_cast<const MMCluster*>(meas)->readoutElement();
        } else if (meas->type() == UncalibMeasType::sTgcStripType) {
            return static_cast<const sTgcMeasurement*>(meas)->readoutElement();
        }
        THROW_EXCEPTION("Unsupported measurement given "<<typeid(*meas).name());
        return nullptr;
    }
    Identifier identify(const UncalibratedMeasurement* meas) {
        if (!meas) return Identifier{};
        if (meas->type() == UncalibMeasType::MdtDriftCircleType){
            return static_cast<const MdtDriftCircle*>(meas)->identify();
        } else if (meas->type() == UncalibMeasType::RpcStripType) {
            return static_cast<const RpcMeasurement*>(meas)->identify();
        } else if (meas->type() == UncalibMeasType::TgcStripType) {
            return static_cast<const TgcStrip*>(meas)->identify();
        } else if (meas->type() == UncalibMeasType::MMClusterType) {
            return static_cast<const MMCluster*>(meas)->identify();
        } else if (meas->type() == UncalibMeasType::sTgcStripType) {
            return static_cast<const sTgcMeasurement*>(meas)->identify();
        }
        THROW_EXCEPTION("Unsupported measurement given "<<typeid(*meas).name());
        return Identifier{};
    }
    Amg::Vector3D positionInChamber(const ActsGeometryContext& gctx,
                                    const UncalibratedMeasurement* meas){
        if (!meas) return Amg::Vector3D::Zero();

        if (meas->type() == UncalibMeasType::MdtDriftCircleType) {
            const MdtDriftCircle* dc = static_cast<const MdtDriftCircle*>(meas);
            return toChamberTransform(gctx, dc) * dc->localCirclePosition();
        } else if (meas->type() == UncalibMeasType::RpcStripType) {
            const RpcMeasurement* strip = static_cast<const RpcMeasurement*>(meas);
            toChamberTransform(gctx, strip) * strip->localMeasurementPos();
        } else if (meas->type() == UncalibMeasType::TgcStripType) {
            const TgcStrip* strip = static_cast<const TgcStrip*>(meas);
            return toChamberTransform(gctx, strip) *(strip->localPosition<1>()[Trk::locX] * Amg::Vector3D::UnitX());
        } else if (meas->type() == UncalibMeasType::MMClusterType) {
            const MMCluster* clust = static_cast<const MMCluster*>(meas);
            return toChamberTransform(gctx, clust) *(clust->localPosition<1>()[Trk::locX] * Amg::Vector3D::UnitX());
        } else if (meas->type() == UncalibMeasType::sTgcStripType) {
            const sTgcMeasurement* sTgc = static_cast<const sTgcMeasurement*>(meas);
            if (sTgc->channelType() == sTgcIdHelper::sTgcChannelTypes::Strip ||
                sTgc->channelType() == sTgcIdHelper::sTgcChannelTypes::Wire) {
                return toChamberTransform(gctx, sTgc) * (sTgc->localPosition<1>()[Trk::locX] * Amg::Vector3D::UnitX());
            }
            Amg::Vector3D locPos{Amg::Vector3D::Zero()};
            locPos.block<2,1>(0,0) = toEigen(sTgc->localPosition<2>());
            return toChamberTransform(gctx, sTgc) * locPos;
            
        } else {
            THROW_EXCEPTION("Measurement "<<typeid(*meas).name()<<" is not supported");
        }
        return Amg::Vector3D::Zero();
    }
    Amg::Vector3D channelDirInChamber(const ActsGeometryContext& gctx,
                                      const UncalibratedMeasurement* meas) {        
        if (!meas) return Amg::Vector3D::Zero();
        if (meas->type() == UncalibMeasType::MdtDriftCircleType) {
            const MdtDriftCircle* dc = static_cast<const MdtDriftCircle*>(meas);
            return toChamberTransform(gctx,dc).linear() * Amg::Vector3D::UnitZ();
        } else if (meas->type() == UncalibMeasType::RpcStripType) {
            const RpcMeasurement* strip = static_cast<const RpcMeasurement*>(meas);
            return toChamberTransform(gctx, strip).linear() * Amg::Vector3D::UnitY();
        } else if (meas->type() == UncalibMeasType::TgcStripType) {
            const TgcStrip* strip = static_cast<const TgcStrip*>(meas);            
            const Amg::Transform3D trf = toChamberTransform(gctx, strip);
            Amg::Vector3D dir{Amg::Vector3D::UnitY()};
            if (strip->measuresPhi()) {
                dir.block<2,1>(0,0) = strip->readoutElement()->stripLayout(strip->gasGap()).stripDir(strip->channelNumber());
            } 
            return trf.linear() *dir;
        } else if (meas->type() == UncalibMeasType::MMClusterType) {
            const MMCluster* clust = static_cast<const MMCluster*>(meas);
            return toChamberTransform(gctx,  clust).linear() * Amg::Vector3D::UnitY();
        } else if (meas->type() == UncalibMeasType::sTgcStripType) {
            const sTgcMeasurement* sTgc = static_cast<const sTgcMeasurement*>(meas);
            return toChamberTransform(gctx,  sTgc).linear() * Amg::Vector3D::UnitY();
        }
        THROW_EXCEPTION("Measurement "<<typeid(*meas).name()<<" is not supported");
        return Amg::Vector3D::Zero();        
    }
    Amg::Vector3D channelNormalInChamber(const ActsGeometryContext& gctx,
                                         const UncalibratedMeasurement* meas) {
        
        if (meas->type() == UncalibMeasType::MdtDriftCircleType) {
            const MdtDriftCircle* dc = static_cast<const MdtDriftCircle*>(meas);
            return toChamberTransform(gctx,dc).linear() * Amg::Vector3D::UnitY();
        } else if (meas->type() == UncalibMeasType::RpcStripType) {
            const RpcMeasurement* strip = static_cast<const RpcMeasurement*>(meas);
            return toChamberTransform(gctx, strip).linear() * Amg::Vector3D::UnitX();
        } else if (meas->type() == UncalibMeasType::TgcStripType) {
            const TgcStrip* strip = static_cast<const TgcStrip*>(meas);            
            const Amg::Transform3D trf = toChamberTransform(gctx, strip);
            Amg::Vector3D dir{Amg::Vector3D::UnitX()};
            if (strip->measuresPhi()) {
                dir.block<2,1>(0,0) = strip->readoutElement()->stripLayout(strip->gasGap()).stripNormal(strip->channelNumber());
            } 
            return trf.linear() *dir;
        } else if (meas->type() == UncalibMeasType::MMClusterType) {
            const MMCluster* clust = static_cast<const MMCluster*>(meas);
            return toChamberTransform(gctx,  clust).linear() * Amg::Vector3D::UnitX();
        } else if (meas->type() == UncalibMeasType::sTgcStripType) {
            const sTgcMeasurement* sTgc = static_cast<const sTgcMeasurement*>(meas);
            return toChamberTransform(gctx,  sTgc).linear() * Amg::Vector3D::UnitX();
        }
        THROW_EXCEPTION("Measurement "<<typeid(*meas).name()<<" is not supported");
        return Amg::Vector3D::Zero();  
    }
}
