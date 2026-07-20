/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONTRACKFINDINGTOOLS_MSTRACKSEEDERTOOL_H
#define MUONTRACKFINDINGTOOLS_MSTRACKSEEDERTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"

#include "GaudiKernel/SystemOfUnits.h"
#include "Acts/Utilities/KDTree.hpp"


#include "xAODMuon/MuonSegmentContainer.h"
#include "MuonTrackEvent/MsTrackSeed.h"
#include "MuonReadoutGeometryR4/MuonDetectorManager.h"
#include "MuonRecToolInterfacesR4/ISegmentSelectionTool.h"
#include "MuonRecToolInterfacesR4/ITrackSeedingDiagnosticsTool.h"
#include "ActsGeometryInterfaces/ITrackingGeometryTool.h"
#include "ActsGeometryInterfaces/IExtrapolationTool.h"
#include "MagFieldConditions/AtlasFieldCacheCondObj.h"

#include <span>

namespace MuonR4{
    /** @brief Helper class to group muon sgements that may belong to a muon trajectory. 
     *         The reconstructed muon segments are projected onto the surface of a cylinder crossing 
     *         roughly the middle stations of the MS. They are then appended to a 3-dimensional search
     *         tree using the extrapolated coordinate on the cylnder, the cylinder surface index and 
     *         the segment's associated sector number.
     * 
     *          
     * */
    class MsTrackSeederTool: public extends<AthAlgTool, ITrackSeedingDiagnosticsTool> {
        public:

            /** @brief Definition of the search tree class */
            using SearchTree_t = Acts::KDTree<3, const xAOD::MuonSegment*, double, std::array, 6>;
            /** @brief Enum toggling whether the segment is in the endcap or barrel */
            using Location = MsTrackSeed::Location;
            /** @brief Recycle the expanded sector */
            using SectorProjector = ExpandedSector::SectorProjector;
            /** @brief Abrivation of the seed coordinates */
            enum class SeedCoords : std::uint8_t{
                /** Encode the seed location (-1,1 -> endcaps, 0 -> barrel  */
                eDetSection,
                /** Sector of the associated spectrometer sector */
                eSector,
                /** Extrapolation position along the cylinder surface */
                ePosOnCylinder
            };
            /** @brief Abbrivation of the KDTree raw data vector */
            using TreeRawVec_t = SearchTree_t::vector_t;

            /** @brief Copy the constructor from the base class */
            using base_class::base_class;
            /**  @copydoc AthAlgTool::initialize  */
            virtual StatusCode initialize() override final;

            /**  @copydoc ITrackSeedingTool::findTrackSeeds  */
            virtual StatusCode findTrackSeeds(const EventContext& ctx,
                                              std::vector<MsTrackSeed>& outSeeds) const override final;

            /**  @copydoc ITrackSeedingTool::estimateStartParameters  */
            virtual Acts::Result<Acts::BoundTrackParameters> 
                estimateStartParameters(const EventContext& ctx,
                                        const MsTrackSeed& seed) const override final;

            /**  @copydoc ITrackSeedingTool::estimateQtimesP  */
            virtual double estimateQtimesP(const EventContext& ctx,
                                           const Amg::Vector3D& planeNorm,
                                           std::span<const PosMomPair_t> circlePoints) const override final;

            /** @copydoc ITrackSeedingDiagnosticsTool::wthinBounds */
            virtual bool withinBounds(const Amg::Vector2D& projPos,
                                      const Location loc) const override final;
            /** @copydoc ITrackSeedingDiagnosticsTool::expressOnCylinder */
            virtual Amg::Vector2D expressOnCylinder(const Acts::GeometryContext& tgContext,
                                                    const xAOD::MuonSegment& segment,
                                                    const Location loc,
                                                    const ExpandedSector sector) const override final;

            /** @brief Estimate the charge times momentum of a muon track candidate from the 
             *         contained segments. The position and direction of the segments are projected onto a 
             *         given phi plane, defined by the segments with phi information or the sector plane.  
             *         The muon trajectory is approximated as 2D trajectory within this plane to avoid side 
             *         effects from (non)-present phi measurements.
             *  @param tgContext: The geometry context to align the segment w.r.t sector
             *  @param seed: Reference to the seed of interest.
             *  @param fieldCache: The initialized magnetic field map*/
            virtual double estimateQtimesP(const Acts::GeometryContext& tgContext,
                                           const MsTrackSeed& seed,
                                           MagField::AtlasFieldCache& fieldCache) const override final;
 
        private:
            
            /** @brief Projects the segment position onto the plane with global phi = x
             *         The local coordinate system is arranged such that the x-axis
             *         is co-linear to the phi direction. The segment is moved along
             *         the MDT's wire direction in that sector.
             *  @param planeNorm: Normal of the phi plane onto which the segment is projected
             *  @param sector: Sector of the segment to be projected, needed to find the wire direction 
             *  @param posToProject: Position to project */
             Amg::Vector3D segPosOntoPhiPlane(const Acts::GeometryContext& tgContext,
                                              const Amg::Vector3D& planeNormal,
                                              const xAOD::MuonSegment& segment) const;

            /** @brief Construct a complete search tree from a MuonSegment container
             *  @param segments: Reference to the segment container to construct. */
            SearchTree_t constructTree(const Acts::GeometryContext& tgContext,
                                       const xAOD::MuonSegmentContainer& segments) const;

            /** @brief Estimate the charge times momentum of a muon candidate when three points 
             *         are available. The given position and direction of each point need to be projected onto the given 
             *         phi plane, and the muon trajectory is approximated as 2D trajectory within this plane.
             *  @param planeNorm: Normal of the bending plane containing the muon trajectory in this simplified approach
             *  @param p1: First segment
             *  @param p2: Second segment
             *  @param p3: Third segment
             *  @param fieldCache: The initialized magnetic field map
             *  @return: Estimated Q*P value */
            double estimateQtimesP(const Amg::Vector3D& planeNorm,
                                   const PosMomPair_t& p1, 
                                   const PosMomPair_t& p2,
                                   const PosMomPair_t& p3,
                                   MagField::AtlasFieldCache& fieldCache) const;
            /** @brief Estimate the charge times momentum of a muon candidate when two points are 
             *         available. The position and direction of each point need to be projected onto the given phi plane,
             *         and the muon trajectory is approximated as 2D trajectory within this plane.
             *  @param planeNorm: Normal of the bending plane containing the muon trajectory in this simplified approach
             *  @param seg1: First segment
             *  @param seg2: Second segment
             *  @param fieldCache: The initialized magnetic field map
             *  @return: Estimated Q*P value */
            double estimateQtimesP(const Amg::Vector3D& planeNorm,
                                   const PosMomPair_t& p1, 
                                   const PosMomPair_t& p2,
                                   MagField::AtlasFieldCache& fieldCache) const;
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
            /** @brief Append the segment to the raw data container. If the projection onto 
             *         the barrel cylinder / endcap discs exceeds the bounds, the segment is not added.
             *         Segments at the interval boundaries of the expanded sector are mirred into the
             *         bin next, but outside the interval.
             *  @param tgContext: Geometry context to find the proper wire direction.
             *  @param segment: Pointer to the segment to add
             *  @param loc: Switch whether the segment shall be projected onto barrel/endcap
             *  @param outContainer: Raw KDTree data vector where the segment is appended */
            void appendSegment(const Acts::GeometryContext& tgContext,
                               const xAOD::MuonSegment* segment,
                               const Location loc,
                               TreeRawVec_t& outContainer) const;
             /** @brief Returns the spectrometer envelope associated to the segment
             *         (Coord system where the parameter are expressed)
             *  @param segment: Reference to the segment of interest */
            const MuonGMR4::SpectrometerSector* envelope(const xAOD::MuonSegment& segment) const;
            /** @brief Removes exact duplciates or partial subsets of the MsTrackSeeds
             *  @param unresolved: Input MsTrackSeedContainer with duplicates */
            MsTrackSeedContainer resolveOverlaps(MsTrackSeedContainer&& unresolved) const;
            
            /** @brief The list of field steps in the force field integration */
            std::vector<double> m_fieldExtpSteps{};
            /** @brief The radius of he barrel cylinder */
            Gaudi::Property<double> m_barrelRadius{this, "BarrelRadius", 7.*Gaudi::Units::m};
            /** @brief The maximum length of the barrel cylinder, if 
             *         not capped by the placement of the endcap discs */
            Gaudi::Property<double> m_barrelLength{this, "BarrelLength", 25.*Gaudi::Units::m};
            /** @brief Position of the endcap discs */
            Gaudi::Property<double> m_endcapDiscZ{this, "EndcapDiscZ", 15.*Gaudi::Units::m};
            /** @brief Radius of the endcap discs */
            Gaudi::Property<double> m_endcapDiscRadius{this, "EndcapRadius", 13.*Gaudi::Units::m};
            /** @brief Maximum separation of point on the cylinder to be picked up
              *         onto a seed */
            Gaudi::Property<double> m_seedHalfLength{this, "SeedHalfLength", 25.*Gaudi::Units::cm};
            /** @brief number of steps between two segments to integrate the magnetic field */
            Gaudi::Property<unsigned> m_nFieldSteps{this, "nFieldSteps", 10};
            /** @brief Pointer to the segement selection tool which compares
             *         two segments for their compatibilitiy */
            ToolHandle<ISegmentSelectionTool> m_segSelector{this, "SegmentSelectionTool" , "" };
            /** @brief Tracking geometry tool */
            PublicToolHandle<ActsTrk::ITrackingGeometryTool> m_trackingGeometryTool{this, "TrackingGeometryTool", ""};
            /** @brief Track extrapolation tool */
            ToolHandle<ActsTrk::IExtrapolationTool> m_extrapolationTool{this, "ExtrapolationTool" ,"" };
            /** @brief Declare the data dependency on the standard Mdt+Rpc+Tgc segment container
             *         & on the NSW segment container */
            SG::ReadHandleKey<xAOD::MuonSegmentContainer> m_segmentKey{this, "SegmentContainer", "MuonSegmentsFromR4" };
            /** @brief  */
            const MuonGMR4::MuonDetectorManager* m_detMgr{nullptr};
    };
}


#endif
