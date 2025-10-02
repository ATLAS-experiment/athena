/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONTRACKFINDINGTOOLS_MSTRACKSEEDER_H
#define MUONTRACKFINDINGTOOLS_MSTRACKSEEDER_H

#include "AthenaBaseComps/AthMessaging.h"

#include "GaudiKernel/SystemOfUnits.h"
#include "Acts/Utilities/KDTree.hpp"
#include "xAODMuon/MuonSegmentContainer.h"
#include "MuonTrackEvent/MsTrackSeed.h"
#include "MuonRecToolInterfacesR4/ISegmentSelectionTool.h"


namespace MuonR4{
    /** @brief Helper class to group muon sgements that may belong to a muon trajectory. 
     *         The reconstructed muon segments are projected onto the surface of a cylinder crossing 
     *         roughly the middle stations of the MS. They are then appended to a 3-dimensional search
     *         tree using the extrapolated coordinate on the cylnder, the cylinder surface index and 
     *         the segment's associated sector number.
     * 
     *          
     * */
    class MsTrackSeeder: public AthMessaging {
        public:

            struct Config{
                /** @brief The radius of the barrel cylinder to seed */
                double barrelRadius{7.*Gaudi::Units::m};
                /** @brief The maximum length of the barrel cylinder, if 
                 *         not capped by the placement of the endcap discs */
                double barrelLength{25.*Gaudi::Units::m};
                /** @brief Position of the endcap discs */
                double endcapDiscZ{15.*Gaudi::Units::m};
                /** @brief Radius of the endcap discs */
                double endcapDiscRadius{12.*Gaudi::Units::m};
                /** @brief Maximum separation of point on the cylinder to be picked up
                 *         onto a seed */
                double seedHalfLength{25.*Gaudi::Units::cm};
                /** @brief Pointer to the segement selection tool which compares
                 *         two segments for their compatibilitiy */
                const ISegmentSelectionTool* selector{nullptr};
            };
            /** @brief Definition of the search tree class */
            using SearchTree_t = Acts::KDTree<3, const xAOD::MuonSegment*, double, std::array, 6>;
            /** @brief Enum toggling whether the segment is in the endcap or barrel */
            using Location = MsTrackSeed::Location;
            /** @brief Abrivation of the seed coordinates */
            enum SeedCoords{
                /** Encode the seed location (-1,1 -> endcaps, 0 -> barrel  */
                eDetSection,
                /** Sector of the associated spectrometer sector */
                eSector,
                /** Extrapolation position along the cylinder surface */
                ePosOnCylinder
            };

            /** @brief Standard constructor
             *  @param msgName: Name of the seeder's msgStream
             *  @param cfg: Configured cylinder dimensions, cuts & selection tool*/
            MsTrackSeeder(const std::string& msgName, Config&& cfg);   
            /** @brief Construct a complete search tree from a MuonSegment container
             *  @param segments: Reference to the segment container to construct. */
            SearchTree_t constructTree(const xAOD::MuonSegmentContainer& segments) const;
            /** @brief Expresses the segment on the cylinder surface. 
             *  @param segment: Reference to the segment of consideration
             *  @param loc: Surface location: [barrel/endcap] */
            Amg::Vector2D expressOnCylinder(const xAOD::MuonSegment& segment,
                                            const Location loc) const;
            
            /** @brief Returns whether the expression on the cylinder is within the surface bounds
             *  @param projPos: Projected position on the cylinder
             *  @param loc: Surface location: [barrel/endcap] */
            bool withinBounds(const Amg::Vector2D& projPos,
                              const Location loc) const;
            /** @brief Constructs the MS track seeds from the segment container
             *  @param ctx: EventContext to access conditions / event data
             *  @param segments: Refrence to the overall event's segment container  */
            std::unique_ptr<MsTrackSeedContainer> findTrackSeeds(const EventContext& ctx,
                                                                 const xAOD::MuonSegmentContainer& segments) const;
        private:
            /** @brief Abbrivation of the KDTree raw data vector */
            using TreeRawVec_t = SearchTree_t::vector_t;
            /** @brief Append the to the raw data container. If the projection onto 
             *         the barrel cylinder / endcap discs exceeds the bounds, the segment is not added.
             *         Segments in sector 1/16 are mirrored into sector 0/17 to complete the search range
             *  @param segment: Pointer to the segment to add
             *  @param loc: Switch whether the segment shall be projected onto barrel/endcap
             *  @param outContainer: Raw KDTree data vector where the segment is appended */
            void appendSegment(const xAOD::MuonSegment* segment,
                               const Location loc,
                               TreeRawVec_t& outContainer) const;
            /** @brief Removes exact duplciates or partial subsets of the MsTrackSeeds
             *  @param unresolved: Input MsTrackSeedContainer with duplicates */
            std::unique_ptr<MsTrackSeedContainer> resolveOverlaps(MsTrackSeedContainer&& unresolved) const;

            Config m_cfg{};
    };
}


#endif
