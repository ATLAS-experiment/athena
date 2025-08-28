/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONSTATIONINTESECTCOND_MUONINTERSECTGEOMETRY_H
#define MUONSTATIONINTESECTCOND_MUONINTERSECTGEOMETRY_H

#include "GeoPrimitives/GeoPrimitives.h"
#include "MuonStationIntersectCond/MuonStationIntersect.h"
namespace MuonGM {
    class MuonDetectorManager;
}  // namespace MuonGM
namespace Muon {

    class MuonIntersectGeometry {
    public:
        virtual ~MuonIntersectGeometry() = default;
        virtual MuonStationIntersect intersection(const MuonGM::MuonDetectorManager* detMgr,
                                                  const Amg::Vector3D& pos, const Amg::Vector3D& dir) const = 0;
    };

}  // namespace Muon

#endif
