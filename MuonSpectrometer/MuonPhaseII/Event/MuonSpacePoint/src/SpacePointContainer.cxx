/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonSpacePoint/SpacePointContainer.h"
#include "AthenaKernel/getMessageSvc.h"

namespace MuonR4 {
    bool SpacePointBucket::operator<(const SpacePointBucket& other) const {
        using ChamberSorter = MuonGMR4::MuonDetectorManager::MSEnvelopeSorter;
        static const ChamberSorter sorter{};
        int chambCompare = -sorter(msSector(), other.msSector()) + 
                            sorter(other.msSector(), msSector());
        if (chambCompare) return chambCompare < 0;
        return bucketId() < other.bucketId();
    }
    void SpacePointBucket::setCoveredRange(double min, double max) {
        m_min = min;
        m_max = max;
    }
    void SpacePointBucket::setBucketId(unsigned int id) {
        m_bucketId = id;
    }
    void SpacePointBucket::populateChamberLocations() {
        if (!msSector()){
            MsgStream msg{Athena::getMessageSvc(), "SpacePointBucket"};
            msg<<MSG::ERROR<< "populateChamberLocations() can only be called once we have a valid hit"<<endmsg;
            return; 
        }
        // loop over all chambers in the sector
        Amg::Vector3D minPos{m_min * Amg::Vector3D::UnitY()},
                      maxPos{m_max * Amg::Vector3D::UnitY()};
        for (auto & chamber : msSector()->chamberLocations()){                   
            minPos[Amg::z] = maxPos[Amg::z] = chamber.location().z();
            /// chamber is fully embedded in the bucket
            if ((minPos.y() < chamber.minY() && maxPos.y() > chamber.maxY()) ||
                /// Partial overlap 
                chamber.insideYZ(minPos) || chamber.insideYZ(maxPos)) {
                m_chamberLocs.push_back(chamber);
            }
        }
    }
}