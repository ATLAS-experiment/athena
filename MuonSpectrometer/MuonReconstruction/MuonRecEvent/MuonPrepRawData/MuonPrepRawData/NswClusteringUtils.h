/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONPREPRAWDATA_NSWCLUSTERING_UTILS_H
#define MUONPREPRAWDATA_NSWCLUSTERING_UTILS_H

#include "MuonPrepRawData/MuonCluster.h"
#include "TrkSurfaces/Surface.h"

/** 
 * Small helper functions to define a not neccessarily right-handed coordinate system which is
 * convinient for the estimate of the uncertainties during the NSW cluster formation.
 * 
*/
namespace Muon {
    namespace NswClustering {
        /// Rotates a direction vector into a local frame:
        ///    x-axis : Parallell to the radial direction of the detector centre
        ///    y-axis : Pependicular to the x-axis in the transverse plane
        ///    z-axis : Points to the big wheel
        inline Amg::Vector3D toLocal(const Amg::Transform3D& toLocalTrans, const Amg::Vector3D& dir) {
            Amg::Rotation3D rotMat{toLocalTrans.linear()};
            const Amg::Vector3D rotDir = rotMat * dir;
            return Amg::Vector3D{rotDir.x(), rotDir.y(), std::abs(rotDir.z())};
        }

        inline Amg::Vector3D toLocal(const Trk::Surface& surf, const Amg::Vector3D& dir){
            Amg::Rotation3D rotMat{surf.transform().inverse().linear()};
            const Amg::Vector3D rotDir = rotMat * dir;
            return Amg::Vector3D{rotDir.x(), rotDir.y(), std::abs(rotDir.z())};
        }
        inline Amg::Vector3D toLocal(const MuonCluster& prd,const Amg::Vector3D& dir){
            return toLocal(prd.detectorElement()->surface(prd.identify()), dir);
        }
        inline Amg::Vector3D toLocal(const MuonCluster& prd) {
            return toLocal(prd, prd.globalPosition().unit());
        }
    }
}
#endif