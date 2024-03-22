/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "xAODMuonPrepData/UtilFunctions.h"

#include "xAODMuonPrepData/MdtDriftCircle.h"
#include "xAODMuonPrepData/RpcStrip.h"
#include "xAODMuonPrepData/TgcStrip.h"
#include "xAODMuonPrepData/MMCluster.h"
#include "xAODMuonPrepData/sTgcMeasurement.h"
#include "xAODMuonPrepData/versions/AccessorMacros.h"
#include "MuonReadoutGeometryR4/MuonChamber.h"

namespace {
        template<class MeasType> Amg::Transform3D toChamberTransform(const ActsGeometryContext& gctx,
                                                                     const MeasType* unCalibMeas) {
        
        IdentifierHash hash{};
        if constexpr(std::is_same<MeasType, xAOD::MdtDriftCircle>::value) {
            hash = unCalibMeas->measurementHash();
        } else {
            hash = unCalibMeas->layerHash();
        }
        return unCalibMeas->readoutElement()->getChamber()->globalToLocalTrans(gctx) * 
               unCalibMeas->readoutElement()->localToGlobalTrans(gctx, hash);
    }
}

namespace xAOD{
    const MuonGMR4::MuonReadoutElement* readoutElement(const UncalibratedMeasurement* meas){
        if (!meas) return nullptr;
        if (meas->type() == xAOD::UncalibMeasType::MdtDriftCircleType){
            return static_cast<const xAOD::MdtDriftCircle*>(meas)->readoutElement();
        } else if (meas->type() == xAOD::UncalibMeasType::RpcStripType) {
            return static_cast<const xAOD::RpcStrip*>(meas)->readoutElement();
        } else if (meas->type() == xAOD::UncalibMeasType::TgcStripType) {
            return static_cast<const xAOD::TgcStrip*>(meas)->readoutElement();
        } else if (meas->type() == xAOD::UncalibMeasType::MMClusterType) {
            return static_cast<const xAOD::MMCluster*>(meas)->readoutElement();
        } else if (meas->type() == xAOD::UncalibMeasType::sTgcStripType) {
            return static_cast<const xAOD::sTgcMeasurement*>(meas)->readoutElement();
        }
        THROW_EXCEPT("Unsupported measurement given "<<typeid(*meas).name());
        return nullptr;
    }
    

    Amg::Vector3D positionInChamber(const ActsGeometryContext& gctx,
                                    const xAOD::UncalibratedMeasurement* meas){
        if (!meas) return Amg::Vector3D::Zero();

        if (meas->type() == xAOD::UncalibMeasType::MdtDriftCircleType) {
            const xAOD::MdtDriftCircle* dc = static_cast<const xAOD::MdtDriftCircle*>(meas);
            return toChamberTransform(gctx, dc).translation();
        } else if (meas->type() == xAOD::UncalibMeasType::RpcStripType) {
            const xAOD::RpcStrip* strip = static_cast<const xAOD::RpcStrip*>(meas);
            return toChamberTransform(gctx, strip) *(strip->localPosition<1>()[0] * Amg::Vector3D::UnitX());
        } else if (meas->type() == xAOD::UncalibMeasType::TgcStripType) {
            const xAOD::TgcStrip* strip = static_cast<const xAOD::TgcStrip*>(meas);
            return toChamberTransform(gctx, strip) *(strip->localPosition<1>()[0] * Amg::Vector3D::UnitX());
        } else {
            THROW_EXCEPT("Measurement "<<typeid(*meas).name()<<" is not supported");
        }
        return Amg::Vector3D::Zero();
    }
    Amg::Vector3D channelDirInChamber(const ActsGeometryContext& gctx,
                                    const xAOD::UncalibratedMeasurement* meas) {        
        if (!meas) return Amg::Vector3D::Zero();
        if (meas->type() == xAOD::UncalibMeasType::MdtDriftCircleType) {
            const xAOD::MdtDriftCircle* dc = static_cast<const xAOD::MdtDriftCircle*>(meas);
            return toChamberTransform(gctx,dc).linear() * Amg::Vector3D::UnitZ();
        } else if (meas->type() == xAOD::UncalibMeasType::RpcStripType) {
            const xAOD::RpcStrip* strip = static_cast<const xAOD::RpcStrip*>(meas);
            return toChamberTransform(gctx, strip).linear() * Amg::Vector3D::UnitY();
        } else if (meas->type() == xAOD::UncalibMeasType::TgcStripType) {
            const xAOD::TgcStrip* strip = static_cast<const xAOD::TgcStrip*>(meas);            
            const Amg::Transform3D trf = toChamberTransform(gctx, strip);
            Amg::Vector3D dir{Amg::Vector3D::UnitY()};
            if (strip->measuresPhi()) {
                dir.block<2,1>(0,0) = strip->readoutElement()->stripLayout(strip->gasGap()).stripDir(strip->channelNumber());
            } 
            return trf.linear() *dir;
        }
        THROW_EXCEPT("Measurement "<<typeid(*meas).name()<<" is not supported");
        return Amg::Vector3D::Zero();        
    }
    Amg::Vector3D channelNormalInChamber(const ActsGeometryContext& gctx,
                                         const UncalibratedMeasurement* meas) {
        
        if (meas->type() == xAOD::UncalibMeasType::MdtDriftCircleType) {
            const xAOD::MdtDriftCircle* dc = static_cast<const xAOD::MdtDriftCircle*>(meas);
            return toChamberTransform(gctx,dc).linear() * Amg::Vector3D::UnitY();
        } else if (meas->type() == xAOD::UncalibMeasType::RpcStripType) {
            const xAOD::RpcStrip* strip = static_cast<const xAOD::RpcStrip*>(meas);
            return toChamberTransform(gctx, strip).linear() * Amg::Vector3D::UnitX();
        } else if (meas->type() == xAOD::UncalibMeasType::TgcStripType) {
            const xAOD::TgcStrip* strip = static_cast<const xAOD::TgcStrip*>(meas);            
            const Amg::Transform3D trf = toChamberTransform(gctx, strip);
            Amg::Vector3D dir{Amg::Vector3D::UnitX()};
            if (strip->measuresPhi()) {
                dir.block<2,1>(0,0) = strip->readoutElement()->stripLayout(strip->gasGap()).stripNormal(strip->channelNumber());
            } 
            return trf.linear() *dir;
        }
        THROW_EXCEPT("Measurement "<<typeid(*meas).name()<<" is not supported");
        return Amg::Vector3D::Zero();  
    }
}
