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
                /** @brief number of steps between two segments to integrate the magnetic field */
                unsigned nFieldSteps{10};
                /** @brief Detector manager to fetch the sector enevelope transforms */
                const MuonGMR4::MuonDetectorManager* detMgr{};
                /** @brief Pointer to the segement selection tool which compares
                 *         two segments for their compatibilitiy */
                const ISegmentSelectionTool* selector{nullptr};
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
             *  @param segments: Reference to the segment container to construct. */
            SearchTree_t constructTree(const xAOD::MuonSegmentContainer& segments) const;
            /** @brief Expresses the segment on the cylinder surface. 
             *  @param segment: Reference to the segment of consideration
             *  @param loc: Surface location: [barrel/endcap]
             *  @param expandedSector: The expanded sector number taking the
             *         overlap regions between large and small sectors into 
             *         account [0;32] */
            Amg::Vector2D expressOnCylinder(const xAOD::MuonSegment& segment,
                                            const Location loc,
                                            const ExpandedSector sector) const;
            /** @brief Projects the segment position onto the plane with global phi = x
             *         The local coordinate system is arranged such that the x-axis
             *         is co-linear to the phi direction. The segment is moved along
             *         the MDT's wire direction in that sector.
             *  @param planeNorm: Normal of the phi plane onto which the segment is projected
             *  @param Sector: Sector of the segment to be projected, needed to find the wire direction 
             *  @param posToProject: Position to project */
            static Amg::Vector3D segPosOntoPhiPlane(const Amg::Vector3D& planeNorm,
                                                    const int Sector,
                                                    const Amg::Vector3D& posToProject);
            /** @brief Projects the segment direction onto the plane with global phi = x by
             *         removing the component orthogonal to the plane.
             *  @param planeNorm: Normal of the phi plane onto which the segment is projected
             *  @param dirToProject: Direction to project */
            static Amg::Vector3D segDirOntoPhiPlane(const Amg::Vector3D& planeNorm,
                                                    const Amg::Vector3D& dirToProject);
            using PosMomPair_t = std::pair<Amg::Vector3D, Amg::Vector3D>;
            /** @brief Estimate the charge times momentum of a muon candidate when three points 
             *         are available. The given position and direction of each point need to be projected onto the given 
             *         phi plane, and the muon trajectory is approximated as 2D trajectory within this plane.
             *  @param magField: Magnetic field
             *  @param planeNorm: Normal of the bending plane containing the muon trajectory in this simplified approach
             *  @param seg1: First segment
             *  @param seg2: Second segment
             *  @param seg3: Third segment
             *  @return: Estimated Q*P value */
            double estimateQtimesP(const AtlasFieldCacheCondObj& magField,
                                   const Amg::Vector3D& planeNorm,
                                   const PosMomPair_t& p1, 
                                   const PosMomPair_t& p2,
                                   const PosMomPair_t& p3) const;
            /** @brief Estimate the charge times momentum of a muon candidate when two points are 
             *         available. The position and direction of each point need to be projected onto the given phi plane,
             *         and the muon trajectory is approximated as 2D trajectory within this plane.
             *  @param magField: Magnetic field
             *  @param planeNorm: Normal of the bending plane containing the muon trajectory in this simplified approach
             *  @param seg1: First segment
             *  @param seg2: Second segment
             *  @return: Estimated Q*P value */
            double estimateQtimesP(const AtlasFieldCacheCondObj& magField,
                                   const Amg::Vector3D& planeNorm,
                                   const PosMomPair_t& p1, 
                                   const PosMomPair_t& p2) const;
            /** @brief Estimate the charge times momentum of a muon track candidate from the 
             *         contained segments. The position and direction of the segments are projected onto a 
             *         given phi plane, defined by the segments with phi information or the sector plane.  
             *         The muon trajectory is approximated as 2D trajectory within this plane to avoid side 
             *         effects from (non)-present phi measurements.
             *  @param magFiel: Reference to the magnetic field holder
             *  @param seed: Reference to the seed of interest. */
            double estimateQtimesP(const AtlasFieldCacheCondObj& magField,
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
                                                                 const xAOD::MuonSegmentContainer& segments) const;
        private:
            /** @brief Compute the charge times momentum from the integral of lorentz force and the 
             *         total change in direction
             *  @param forceIntegral: Cumulative lorentz force
             *  @param deltaDir: Change in direction
             *  @return: Estimated Q*P value */
            double getPtimesQ(const Amg::Vector3D& forceIntegral, 
                              const Amg::Vector3D& deltaDir) const;
            /** @brief Compute the integral of magnetic force (v x B ) dS along a trajectory, given the initial 
             *         and final positions and directions of the trajectory. The trajectory is approximated as 
             *         a straight line between the two positions, and the magnetic field is evaluated at several 
             *         points along this line.
             *  @param point1: Initial position and direction
             *  @param point2: Final position and direction
             *  @param planeNorm: Normal vector of the bending plane used for the momentum estimation, used to 
             *         extract the orthogonal component to the field
             *  @param fieldCache: Magnetic field cache
             *  @return: Integral of magnetic force */
            Amg::Vector3D forceIntegration(const PosMomPair_t& point1,
                                           const PosMomPair_t& point2,
                                           const Amg::Vector3D& planeNorm,
                                           MagField::AtlasFieldCache& fieldCache) const;
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
            
            std::set<double> m_fieldExtpSteps{};
            Config m_cfg{};
    };
}


#endif
