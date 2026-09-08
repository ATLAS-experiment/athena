/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
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
            /** @brief Constructor with location defintion
             *  @param loc: Localtion definition whether the seed is constructed 
             *              on the barrel or on the endcap surface
             *  @param sector: In which tree sector is the seed constructed:
             *                    sector: 2*MS-sector +- Overlap */
            MsTrackSeed(const Location loc,
                        const ExpandedSector sector);
            /** @brief Returns the vector of associated segments */
            std::span<const xAOD::MuonSegment* const> segments() const;
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
        private:
            /** @brief Location variable */
            Location m_loc{Location::Undefined};
            ExpandedSector m_sector{static_cast<std::int8_t>(0)};
            Amg::Vector3D m_pos{Amg::Vector3D::Zero()};
            std::vector<const xAOD::MuonSegment*> m_segments{};          
    };
    using MsTrackSeedContainer = std::vector<MsTrackSeed>;
    std::ostream& operator<<(std::ostream& ostr, const MuonR4::MsTrackSeed& seed);

}
CLASS_DEF( MuonR4::MsTrackSeedContainer , 1290595104 , 1 )
ACTS_OSTREAM_FORMATTER (MuonR4::MsTrackSeed::Location);
#endif