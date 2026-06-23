/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONTRACKFINDINGTOOLS_MSTRACKSEEDER_H
#define MUONTRACKFINDINGTOOLS_MSTRACKSEEDER_H

#include "AthenaBaseComps/AthMessaging.h"

#include "GaudiKernel/SystemOfUnits.h"
#include "Acts/Utilities/KDTree.hpp"


#include "xAODMuon/MuonSegmentContainer.h"
#include "MuonTrackEvent/MsTrackSeed.h"
#include "MuonReadoutGeometryR4/MuonDetectorManager.h"
#include "MuonRecToolInterfacesR4/ISegmentSelectionTool.h"
#include "ActsGeometry/ATLASMagneticFieldWrapper.h"


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
            /** @brief Configuration object */
            struct Config{
                /** @brief The radius of the barrel cylinder to seed */
                double barrelRadius{7.*Gaudi::Units::m};
                /** @brief The maximum length of the barrel cylinder, if 
                 *         not capped by the placement of the endcap discs */
                double barrelLength{25.*Gaudi::Units::m};
                /** @brief Position of the endcap discs */
                double endcapDiscZ{15.*Gaudi::Units::m};
                /** @brief Radius of the endcap discs */
                double endcapDiscRadius{13.*Gaudi::Units::m};
                /** @brief Maximum separation of point on the cylinder to be picked up
                 *         onto a seed */
                double seedHalfLength{25.*Gaudi::Units::cm};
                /** @brief Detector manager to fetch the sector enevelope transforms */
                const MuonGMR4::MuonDetectorManager* detMgr{};
                /** @brief Pointer to the segement selection tool which compares
                 *         two segments for their compatibilitiy */
                const ISegmentSelectionTool* selector{nullptr};
                /** @brief Steps between two segments to integrate the magnetic field */
                std::set<double> fieldExtpSteps{0.,0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,1};
            };
            /** @brief Definition of the search tree class */
            using SearchTree_t = Acts::KDTree<3, const xAOD::MuonSegment*, double, std::array, 6>;
            /** @brief Enum toggling whether the segment is in the endcap or barrel */
            using Location = MsTrackSeed::Location;
            /** @brief Recycle the expanded sector */
            using SectorProjector = ExpandedSector::SectorProjector;
            using VecOpt_t = std::optional<Amg::Vector3D>;
            /** @brief Abrivation of the seed coordinates */
            enum class SeedCoords : std::uint8_t{
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
             *  @param gctx: Geometry context to fetch the transforms of the associated 
             *               sector envelope
             *  @param segments: Reference to the segment container to construct. */
            SearchTree_t constructTree(const ActsTrk::GeometryContext& gctx,
                                       const xAOD::MuonSegmentContainer& segments) const;
            /** @brief Expresses the segment on the cylinder surface. 
             *  @param gctx: Geometry context to fetch the transforms of the associated 
             *               sector envelope
             *  @param segment: Reference to the segment of consideration
             *  @param loc: Surface location: [barrel/endcap]
             *  @param expandedSector: The expanded sector number taking the
             *         overlap regions between large and small sectors into 
             *         account [0;32] */
            Amg::Vector2D expressOnCylinder(const ActsTrk::GeometryContext& gctx, 
                                            const xAOD::MuonSegment& segment,
                                            const Location loc,
                                            const ExpandedSector sector) const;
            /** @brief Projects the passed segment onto the plane with global phi = x
             *         The local coordinate system is arranged such that the x-axis
             *         is co-linear to the phi direction. The segment is moved is moved
             *         along that line until the radial line with phi =x is crossed
             *  @param gctx: Geometry context to retrieve the local -> global transform 
             *               of the sector
             *  @param segment: The segment to be moved 
             *  @param projectedPhi: The value that the projected segment shall have afterwards */
            Amg::Vector3D projectOntoPhiPlane(const ActsTrk::GeometryContext& gctx, 
                                              const xAOD::MuonSegment& segment,
                                              const double projectPhi) const;
            /** @brief Calculates the precision deflection angle between two segments
             *         on a muon track. 
             *  @param segment1: The inner segment 
             *  @param segment2: The outer segment
             *  @param sector: The expanded sector in which the segments are expressed */
            double estimateTwoStationP(const xAOD::MuonSegment& segment1,
                                   const xAOD::MuonSegment& segment2,
                                   const AtlasFieldCacheCondObj& magField) const;
            /** @brief Estimate the q /p of the seed candidate from the contained segments. A circle from 
             *         the inner, middle & outer segment points is constructed. To avoid side effects from
             *         (non)-present phi measurements, the segments are expressed on the sector planes
             *  @param gctx: Geometry context to fetch the transforms of the associated sector envelope
             *  @param magFiel: Reference to the magnetic field holder
             *  @param seed: Reference to the seed of interest. */
            double estimateQtimesP(const ActsTrk::GeometryContext& gctx,
                                   const AtlasFieldCacheCondObj& magField,
                                   const MsTrackSeed& seed) const;
            /** @brief Returns whether the expression on the cylinder is within the surface bounds
             *  @param projPos: Projected position on the cylinder
             *  @param loc: Surface location: [barrel/endcap] */
            bool withinBounds(const Amg::Vector2D& projPos,
                              const Location loc) const;
            /** @brief Constructs the MS track seeds from the segment container
             *  @param ctx: EventContext to access conditions / event data
             *  @param segments: Refrence to the overall event's segment container  */
            std::unique_ptr<MsTrackSeedContainer> findTrackSeeds(const EventContext& ctx,
                                                                 const ActsTrk::GeometryContext& gctx,
                                                                 const xAOD::MuonSegmentContainer& segments) const;
            /** @brief Returns the spectrometer envelope associated to the segment
             *         (Coord system where the parameter are expressed)
             *  @param segment: Reference to the segment of interest */
            const MuonGMR4::SpectrometerSector* envelope(const xAOD::MuonSegment& segment) const;
        private:
            using PosMomPair_t = std::pair<Amg::Vector3D, Amg::Vector3D>;
            std::pair<double, unsigned> averageBField(const PosMomPair_t& start,
                                                      const PosMomPair_t& end,
                                                      const AtlasFieldCacheCondObj& magField,
                                                      const bool skipFirst) const;

            /** @brief Calculates the radius of the bending circle from three points using the 
             *         sagitta. If one point is not defined, the origin is inserted instead.
             *         If two or more points are not set, a nullopt is returned */
            std::optional<double> calculateRadius(VecOpt_t&& pI, VecOpt_t&& pM, VecOpt_t&& pO,
                                                  const Amg::Vector3D& planeNorm) const;

            /** @brief Abbrivation of the KDTree raw data vector */
            using TreeRawVec_t = SearchTree_t::vector_t;
            /** @brief Append the to the raw data container. If the projection onto 
             *         the barrel cylinder / endcap discs exceeds the bounds, the segment is not added.
             *         Segments in sector 1/16 are mirrored into sector 0/17 to complete the search range
             *  @param gctx: Geometry context to fetch the transforms of the associated sector envelope
             *  @param segment: Pointer to the segment to add
             *  @param loc: Switch whether the segment shall be projected onto barrel/endcap
             *  @param outContainer: Raw KDTree data vector where the segment is appended */
            void appendSegment(const ActsTrk::GeometryContext& gctx,
                               const xAOD::MuonSegment* segment,
                               const Location loc,
                               TreeRawVec_t& outContainer) const;
            /** @brief Removes exact duplciates or partial subsets of the MsTrackSeeds
             *  @param unresolved: Input MsTrackSeedContainer with duplicates */
            std::unique_ptr<MsTrackSeedContainer> resolveOverlaps(MsTrackSeedContainer&& unresolved) const;

            Config m_cfg{};
    };
}


#endif
