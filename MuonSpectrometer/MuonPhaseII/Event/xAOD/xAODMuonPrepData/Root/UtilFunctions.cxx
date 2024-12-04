/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "xAODMuonPrepData/UtilFunctions.h"
#include "GeoModelKernel/throwExcept.h"

#include "MuonReadoutGeometryR4/MdtReadoutElement.h"
#include "MuonReadoutGeometryR4/RpcReadoutElement.h"
#include "MuonReadoutGeometryR4/TgcReadoutElement.h"
#include "MuonReadoutGeometryR4/MmReadoutElement.h"
#include "MuonReadoutGeometryR4/sTgcReadoutElement.h"

#include "xAODMuonPrepData/MdtDriftCircle.h"
#include "xAODMuonPrepData/MdtTwinDriftCircle.h"
#include "xAODMuonPrepData/RpcStrip.h"
#include "xAODMuonPrepData/RpcStrip2D.h"
#include "xAODMuonPrepData/RpcMeasurement.h"
#include "xAODMuonPrepData/TgcStrip.h"
#include "xAODMuonPrepData/MMCluster.h"
#include "xAODMuonPrepData/sTgcMeasurement.h"
#include "xAODMuonPrepData/versions/AccessorMacros.h"
#include "MuonReadoutGeometryR4/SpectrometerSector.h"
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
        return reEle->msSector()->globalToLocalTrans(gctx) * reEle->localToGlobalTrans(gctx, hash);
    }
}

namespace xAOD{
    const MuonGMR4::MuonReadoutElement* readoutElement(const UncalibratedMeasurement* meas){
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
                THROW_EXCEPTION("Unsupported measurement given "<<typeid(*meas).name());
        }
        THROW_EXCEPTION("Something went wrong with measurement "<<typeid(*meas).name());
        return nullptr;
    }
    const Identifier& identify(const UncalibratedMeasurement* meas) {
        static const Identifier defId{};
        if (!meas) {
            return defId;
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
        return defId;
    }
    Amg::Vector3D positionInChamber(const ActsGeometryContext& gctx,
                                    const UncalibratedMeasurement* meas){
        if (!meas) return Amg::Vector3D::Zero();
        switch (meas->type()) {
            case UncalibMeasType::MdtDriftCircleType: {
                const MdtDriftCircle* dc = static_cast<const MdtDriftCircle*>(meas);
                return toChamberTransform(gctx, dc) * dc->localCirclePosition();
            } case UncalibMeasType::RpcStripType: {
                const RpcMeasurement* strip = static_cast<const RpcMeasurement*>(meas);
                return toChamberTransform(gctx, strip) * strip->localMeasurementPos();
            } case UncalibMeasType::TgcStripType: {
                const TgcStrip* strip = static_cast<const TgcStrip*>(meas);
                return toChamberTransform(gctx, strip) *(strip->localPosition<1>()[Trk::locX] * Amg::Vector3D::UnitX());
            } case UncalibMeasType::MMClusterType: {
                const MMCluster* clust = static_cast<const MMCluster*>(meas);
                return toChamberTransform(gctx, clust) *(clust->localPosition<1>()[Trk::locX] * Amg::Vector3D::UnitX());
            } case UncalibMeasType::sTgcStripType: {
                const sTgcMeasurement* sTgc = static_cast<const sTgcMeasurement*>(meas);
                if (sTgc->channelType() == sTgcIdHelper::sTgcChannelTypes::Strip ||
                    sTgc->channelType() == sTgcIdHelper::sTgcChannelTypes::Wire) {
                    return toChamberTransform(gctx, sTgc) * (sTgc->localPosition<1>()[Trk::locX] * Amg::Vector3D::UnitX());
                }
                Amg::Vector3D locPos{Amg::Vector3D::Zero()};
                locPos.block<2,1>(0,0) = toEigen(sTgc->localPosition<2>());
                return toChamberTransform(gctx, sTgc) * locPos;
            } default: {
                THROW_EXCEPTION("Measurement "<<typeid(*meas).name()<<" is not supported");
            }
        }
        THROW_EXCEPTION("Something did not went right with "<<typeid(*meas).name());
        return Amg::Vector3D::Zero();
    }
    Amg::Vector3D channelDirInChamber(const ActsGeometryContext& gctx,
                                      const UncalibratedMeasurement* meas) {        
        if (!meas) return Amg::Vector3D::Zero();
        switch (meas->type()) {
            case UncalibMeasType::MdtDriftCircleType:{
                const MdtDriftCircle* dc = static_cast<const MdtDriftCircle*>(meas);
                return toChamberTransform(gctx,dc).linear() * Amg::Vector3D::UnitZ();
            } case UncalibMeasType::RpcStripType: {
                const RpcMeasurement* strip = static_cast<const RpcMeasurement*>(meas);
                return toChamberTransform(gctx, strip).linear() * Amg::Vector3D::UnitY();
            } case UncalibMeasType::TgcStripType: {
                const TgcStrip* strip = static_cast<const TgcStrip*>(meas);            
                const Amg::Transform3D trf = toChamberTransform(gctx, strip);
                Amg::Vector3D dir{Amg::Vector3D::UnitY()};
                if (strip->measuresPhi()) {
                    dir.block<2,1>(0,0) = strip->readoutElement()->stripLayout(strip->gasGap()).stripDir(strip->channelNumber());
                } 
                return trf.linear() *dir;
            } case UncalibMeasType::MMClusterType: {
                const MMCluster* clust = static_cast<const MMCluster*>(meas);
                return toChamberTransform(gctx,  clust).linear() * Amg::Vector3D::UnitY();
            }  case UncalibMeasType::sTgcStripType: {
                const sTgcMeasurement* sTgc = static_cast<const sTgcMeasurement*>(meas);
                return toChamberTransform(gctx,  sTgc).linear() * Amg::Vector3D::UnitY();
            } default: {
                THROW_EXCEPTION("Measurement "<<typeid(*meas).name()<<" is not supported");
            }
        }
        THROW_EXCEPTION("Something did not went right with "<<typeid(*meas).name());
        return Amg::Vector3D::Zero();        
    }
    Amg::Vector3D channelNormalInChamber(const ActsGeometryContext& gctx,
                                         const UncalibratedMeasurement* meas) {
        
        switch(meas->type()) {
            case UncalibMeasType::MdtDriftCircleType: {
                const MdtDriftCircle* dc = static_cast<const MdtDriftCircle*>(meas);
                return toChamberTransform(gctx,dc).linear() * Amg::Vector3D::UnitY();
            } case UncalibMeasType::RpcStripType: {
                const RpcMeasurement* strip = static_cast<const RpcMeasurement*>(meas);
                return toChamberTransform(gctx, strip).linear() * Amg::Vector3D::UnitX();
            } case UncalibMeasType::TgcStripType: {
                const TgcStrip* strip = static_cast<const TgcStrip*>(meas);            
                const Amg::Transform3D trf = toChamberTransform(gctx, strip);
                Amg::Vector3D dir{Amg::Vector3D::UnitX()};
                if (strip->measuresPhi()) {
                    dir.block<2,1>(0,0) = strip->readoutElement()->stripLayout(strip->gasGap()).stripNormal(strip->channelNumber());
                } 
                return trf.linear() *dir;
            } case UncalibMeasType::MMClusterType: {
                const MMCluster* clust = static_cast<const MMCluster*>(meas);
                return toChamberTransform(gctx,  clust).linear() * Amg::Vector3D::UnitX();
            } case UncalibMeasType::sTgcStripType: {
                const sTgcMeasurement* sTgc = static_cast<const sTgcMeasurement*>(meas);
                return toChamberTransform(gctx,  sTgc).linear() * Amg::Vector3D::UnitX();
            } default:
                THROW_EXCEPTION("Measurement "<<typeid(*meas).name()<<" is not supported");
        }
        THROW_EXCEPTION("Something did not went right with "<<typeid(*meas).name()); 
        return Amg::Vector3D::Zero();  
    }
}
