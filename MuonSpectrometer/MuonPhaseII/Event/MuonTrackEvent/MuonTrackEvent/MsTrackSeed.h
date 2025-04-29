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
/** @brief MsTrackSeed represents the collection of segments that may be compatible with
 *         a muon track trajectory hypothesis. To construct a seed, the segments are projected
 *         onto a cylinder which is roughly intersecting the middle stations of the spectrometer.  */
namespace MuonR4{

    class MsTrackSeed {
        public:
            /** @brief Enum defining whether the seed is made in the endcap / barrel */
            enum class Location: int8_t{
                Undefined =-1,
                Barrel,
                Endcap
            };
            /** @brief Constructor with location defintion */
            MsTrackSeed(const Location loc);
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
            /** @brief Returns the location of the seed */
            Location location() const;
            /** @brief Equality operator */
            bool operator==(const MsTrackSeed& other) const;
            /** @brief Returns if all segments of this seed are also in the seed as well */
            bool operator<(const MsTrackSeed& other) const;
        private:
            /** @brief Returns whether two spectrometer sectors may be compatbile 
             *  @param secA: First sector to compare
             *  @param secB: Second sector to compare */
            static bool compatibleSectors(const MuonGMR4::SpectrometerSector* secA,
                                          const MuonGMR4::SpectrometerSector* secB);

            Location m_loc{Location::Undefined};
            Amg::Vector3D m_pos{Amg::Vector3D::Zero()};
            std::vector<const xAOD::MuonSegment*> m_segments{};
            std::vector<const Segment*> m_detSegments{};
            std::unordered_set<const SpacePointBucket*> m_buckets{};
    };
    using MsTrackSeedContainer = std::vector<MsTrackSeed>;
    std::ostream& operator<<(std::ostream& ostr, const MuonR4::MsTrackSeed& seed);

}
CLASS_DEF( MuonR4::MsTrackSeedContainer , 1290595104 , 1 )
#endif