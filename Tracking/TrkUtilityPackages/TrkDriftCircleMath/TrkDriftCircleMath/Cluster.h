/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DCMATH_CLUSTER_H
#define DCMATH_CLUSTER_H

#include <ostream>
#include <vector>

#include "GaudiKernel/MsgStream.h"
#include "TrkDriftCircleMath/ClusterId.h"
#include "TrkDriftCircleMath/LocVec2D.h"


namespace Muon {
    class MuonClusterOnTrack;
}
namespace TrkDriftCircleMath {

    /** class representing a cluster meaurement */
    class Cluster {
    public:
        /** minimal constructor:
            @param pos local position of cluster
            @param err error on cluster position */
        Cluster(const LocVec2D& pos, double err) : m_pos(pos), m_err(err) {}

        /** full constructor:
            @param pos local position of cluster
            @param err error on cluster position
            @param id cluster identifier
            @param rot: Pointer to the ROT out of which the cluster is made
            @param index index of cluster use to link it back to the original RIO_OnTrack */
        Cluster(const LocVec2D& pos, double err, ClusterId id, 
                const Muon::MuonClusterOnTrack* rot,
                unsigned int index) : 
            m_pos(pos), m_err(err), m_id(id), m_rot{rot}, m_idx{index} {}

        ~Cluster() = default;

        /** local position of the cluster */
        const LocVec2D& position() const { return m_pos; }

        /** cluster identifier */
        const ClusterId& id() const { return m_id; }

        /** direct access to local x coordinate of cluster */
        double x() const { return m_pos.x(); }

        /** direct access to y coordinate of cluster */
        double y() const { return m_pos.y(); }

        /** error on cluster position */
        double err() const { return m_err; }

        /** index of cluster in input vector, use to link back the cluster to the MuonClusterOnTrack */
        const Muon::MuonClusterOnTrack* rot() const { return m_rot; }
        /** @brief: Index w.r.t cluster vector*/
        unsigned int index() const { return m_idx; }

    private:
        LocVec2D m_pos{0., 0.};
        double m_err{0.};
        ClusterId m_id{};
        const Muon::MuonClusterOnTrack* m_rot{nullptr};
        unsigned int m_idx{0};
    };

    typedef std::vector<Cluster> CLVec;
    typedef CLVec::iterator CLIt;
    typedef CLVec::const_iterator CLCit;
}  // namespace TrkDriftCircleMath

std::ostream& operator<<(std::ostream& os, const TrkDriftCircleMath::Cluster& dc);

MsgStream& operator<<(MsgStream& os, const TrkDriftCircleMath::Cluster& dc);

#endif
