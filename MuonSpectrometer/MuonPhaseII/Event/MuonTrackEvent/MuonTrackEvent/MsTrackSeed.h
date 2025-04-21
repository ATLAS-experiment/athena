/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONTRACKEVENT_MSTRACKSEED_H
#define MUONTRACKEVENT_MSTRACKSEED_H

#include <vector>

#include "AthContainers/DataVector.h"
#include "MuonSpacePoint/SpacePointContainer.h"
#include "MuonPatternEvent/Segment.h"
#include "xAODMuon/MuonSegment.h"
/** @brief The MsTrackSeed represents all segments which may be 
 *         compatible with a single track trajectory */
namespace MuonR4{
    class MsTrackSeed {
        public:
            MsTrackSeed() = default;

            /** @brief Returns the vector of associated segments */
            const std::vector<const xAOD::MuonSegment*>& segments() const;
            /** @brief Returns the list of detailed segments */
            const std::vector<const Segment*>& detailedSegments() const;
            /** @brief Append a segment to the seed */
            void addSegment(const xAOD::MuonSegment* seg);
            /** @brief Returns the list of associated buckets */
            const std::unordered_set<const SpacePointBucket*>& buckets() const;
            /** @brief Returns the seed's position */
            const Amg::Vector3D& position() const;
            /** @brief set the seed's position */
            void setPosition(Amg::Vector3D&& pos);
            /** @brief Returns the associated MS sector */
            const MuonGMR4::SpectrometerSector* msSector() const;
        private:
            Amg::Vector3D m_pos{Amg::Vector3D::Zero()};
            std::vector<const xAOD::MuonSegment*> m_segments{};
            std::vector<const Segment*> m_detSegments{};
            std::unordered_set<const SpacePointBucket*> m_buckets{};
    };
    using MsTrackSeedContainer = std::vector<MsTrackSeed>;
}
CLASS_DEF( MuonR4::MsTrackSeedContainer , 1290595104 , 1 )
#endif