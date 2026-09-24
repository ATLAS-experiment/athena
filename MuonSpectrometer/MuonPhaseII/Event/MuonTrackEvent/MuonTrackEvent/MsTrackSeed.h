/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONTRACKEVENT_MSTRACKSEED_H
#define MUONTRACKEVENT_MSTRACKSEED_H

#include <vector>

#include "AthContainers/DataVector.h"
#include "MuonSpacePoint/SpacePointContainer.h"
#include "MuonTrackEvent/ExpandedSector.h"
#include "MuonPatternEvent/Segment.h"

#include "xAODMuon/MuonSegment.h"

#include "Acts/Utilities/OstreamFormatter.hpp"
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
            static std::string toString(const Location loc);
            friend std::ostream& operator<<(std::ostream& ostr, const Location loc) {
                return ostr<<toString(loc);
            }
            friend std::ostream& operator<<(std::ostream& ostr, const MuonR4::MsTrackSeed& seed) {
                 return seed.print(ostr);
            }
            /** @brief Constructor with location defintion
             *  @param loc: Localtion definition whether the seed is constructed 
             *              on the barrel or on the endcap surface
             *  @param sector: In which tree sector is the seed constructed:
             *                    sector: 2*MS-sector +- Overlap */
            explicit MsTrackSeed(const Location loc,
                                 const ExpandedSector sector);
            /** @brief Returns the vector of associated segments */
            std::span<const xAOD::MuonSegment* const> segments() const;
            /** @brief The station indices of the associated sements */
            std::span<const Muon::MuonStationIndex::StIndex> stations() const;
            /** @brief Returns the list of associated buckets */
            std::vector<const SpacePointBucket*> buckets() const;
            /** @brief Append a segment to the seed */
            void addSegment(const xAOD::MuonSegment* seg);
            /** @brief Replaces an already added segment in the seed with a better suited one
             *  @param exist: Pointer to the segment that is already part of the seed 
             *                (Exception is thrown if not)
             *  @param updated: Pointer to the segment with which the segment is replaced with */
            void replaceSegment(const xAOD::MuonSegment* exist,
                                const xAOD::MuonSegment* updated);

            /** @brief Returns the seed's position */
            const Amg::Vector3D& position() const;
            /** @brief set the seed's position */
            void setPosition(Amg::Vector3D&& pos);
            /** @brief Returns the location of the seed */
            Location location() const;
            /** @brief Returns the seed's sector*/
            ExpandedSector sector() const { return m_sector; }
            /** @brief Returns the number of stations crossed by the seed */
            std::size_t nStations() const;
            /** @brief Returns the number of measurements on the seed */
            std::size_t nMeasurements() const;
            /** @brief Prepare the overlap marker */
            void prepareOverlap(std::shared_ptr<std::uint8_t> marker);
            /** @brief Trigger the overlap marker. Shared seeds are not passed */
            void triggerOverlapMarker();
            /** @brief Returns whether the seed has been marked for overlap */
            bool hasOverlap() const;
        private:
            std::ostream& print(std::ostream& ostr) const;
            /** @brief Return the position of the reference segment if exists */
            float pathLength(const xAOD::MuonSegment& segment) const;
            /** @brief Location variable */
            Location m_loc{Location::Undefined};
            /** @brief The expanded sector of the seed (centre/left/right) + sector */
            ExpandedSector m_sector{static_cast<std::int8_t>(0)};
            /** @brief Posiion of the seed in the seeding space */
            Amg::Vector3D m_pos{Amg::Vector3D::Zero()};
            /** @brief List of associated segments */
            std::vector<const xAOD::MuonSegment*> m_segments{};
            /** @brief Number of measurements in the seed*/
            std::size_t m_nMeasurements{0ul};
            /** @brief Number of stations */
            std::vector<Muon::MuonStationIndex::StIndex> m_stations{};
            /** @brief The overlap marker */
            std::vector<std::shared_ptr<std::uint8_t>> m_overlapMarker{};
    };
    using MsTrackSeedContainer = std::vector<MsTrackSeed>;
}
CLASS_DEF( MuonR4::MsTrackSeedContainer , 1290595104 , 1 )
ACTS_OSTREAM_FORMATTER (MuonR4::MsTrackSeed::Location);
#endif