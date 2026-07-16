/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "ActsEvent/EnumConversion.h"

#include <format>

namespace ActsTrk {

    DetectorType toDetType(const xAOD::UncalibMeasType mType) {
        switch (mType) {
            using enum xAOD::UncalibMeasType;
            case PixelClusterType: return DetectorType::Pixel;
            case StripClusterType: return DetectorType::Sct;
            case MdtDriftCircleType: return DetectorType::Mdt;
            case RpcStripType: return DetectorType::Rpc;
            case TgcStripType: return DetectorType::Tgc;
            case MMClusterType: return DetectorType::Mm;
            case sTgcStripType: return DetectorType::sTgc;
            case HGTDClusterType: return DetectorType::Hgtd;
            case nTypes:
            case Other: return DetectorType::UnDefined;
        }
        return DetectorType::UnDefined;
    }
    xAOD::UncalibMeasType toMeasType(const DetectorType dType) {
        switch (dType) {
            using enum DetectorType;
            case Pixel: return xAOD::UncalibMeasType::PixelClusterType;  
            case Sct: return xAOD::UncalibMeasType::StripClusterType;  
            case Mdt: return xAOD::UncalibMeasType::MdtDriftCircleType;
            case Rpc: return xAOD::UncalibMeasType::RpcStripType;      
            case Tgc: return xAOD::UncalibMeasType::TgcStripType;      
            case Mm: return xAOD::UncalibMeasType::MMClusterType;     
            case sTgc: return xAOD::UncalibMeasType::sTgcStripType;     
            case Hgtd: return xAOD::UncalibMeasType::HGTDClusterType;

            default:
                throw std::runtime_error(std::format("toMeasType() - Unsupported {} type parsed", dType));
        }
        return xAOD::UncalibMeasType::nTypes;
    }
    Trk::TrackInfo::TrackFitter toTrkFitterType(const xAOD::TrackFitter fType) {
        switch (fType) {
            using enum xAOD::TrackFitter;
            case Unknown: return Trk::TrackInfo::TrackFitter::Unknown;
            case iPatTrackFitter: return Trk::TrackInfo::TrackFitter::iPatTrackFitter;
            case xKalman: return Trk::TrackInfo::TrackFitter::xKalman;
            case GaussianSumFilter: return Trk::TrackInfo::TrackFitter::GaussianSumFilter;
            case GlobalChi2Fitter: return Trk::TrackInfo::TrackFitter::GlobalChi2Fitter;
            case DistributedKalmanFilter: return Trk::TrackInfo::TrackFitter::DistributedKalmanFilter;
            case DeterministicAnnealingFilter: return Trk::TrackInfo::TrackFitter::DeterministicAnnealingFilter;
            case KalmanFitter: return Trk::TrackInfo::TrackFitter::KalmanFitter;
            case KalmanDNAFitter: return Trk::TrackInfo::TrackFitter::KalmanDNAFitter;
            case MuonboyFitter: return Trk::TrackInfo::TrackFitter::MuonboyFitter;

            default: 
                throw std::runtime_error("toTrkFitterType() - Unsupported type parsed");
        }
    }

}