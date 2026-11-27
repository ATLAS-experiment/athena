/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONTRACKFINDINGTOOLS_MSTRACKSEEDERTOOL_H
#define MUONTRACKFINDINGTOOLS_MSTRACKSEEDERTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"

#include "GaudiKernel/SystemOfUnits.h"
#include "Acts/Utilities/KDTree.hpp"

#include "ActsGeometry/ContextUtility.h"

#include "xAODMuon/MuonSegmentContainer.h"
#include "MuonTrackEvent/MsTrackSeed.h"
#include "MomentumEstimator.h"
#include "MuonReadoutGeometryR4/MuonDetectorManager.h"
#include "MuonRecToolInterfacesR4/ISegmentSelectionTool.h"
#include "MuonRecToolInterfacesR4/ITrackSeedingDiagnosticsTool.h"
#include "ActsGeometry/ATLASMagneticFieldWrapper.h"
#include "ActsGeometryInterfaces/ITrackingGeometrySvc.h"

#include "xAODMeasurementBase/UncalibratedMeasurementContainer.h"

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
            /** @brief Abrivate the segment vector */
            using SegVec_t = std::vector<const xAOD::MuonSegment*>;
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

            /** @copydoc ITrackSeedingDiagnosticsTool::wthinBounds */
            virtual bool withinBounds(const Amg::Vector2D& projPos,
                                      const Location loc) const override final;
            /** @copydoc ITrackSeedingDiagnosticsTool::expressOnCylinder */
            virtual Amg::Vector2D expressOnCylinder(const Acts::GeometryContext& tgContext,
                                                    const xAOD::MuonSegment& segment,
                                                    const Location loc,
                                                    const ExpandedSector sector) const override final;

            /** @brief Estimate the charge over momentum of a muon track candidate from the 
             *         contained segments. The position and direction of the segments are projected onto a 
             *         given phi plane, defined by the segments with phi information or the sector plane.  
             *         The muon trajectory is approximated as 2D trajectory within this plane to avoid side 
             *         effects from (non)-present phi measurements.
             *  @param tgContext: The geometry context to align the segment w.r.t sector
             *  @param seed: Reference to the seed of interest. */
            virtual double estimateQoverP(const EventContext& ctx,
                                           const MsTrackSeed& seed) const override final;
 
        private:
            /** @brief Wire direction of the segment: the x axis of the chamber volume of its first precision hit that
             *         is not an outlier, else (truth segments without measurements) the x axis of the sector envelope */
            Acts::Vector3 wireDirection(const Acts::GeometryContext& tgContext,
                                        const xAOD::MuonSegment& segment) const;

            /** @brief Construct a complete search tree from a MuonSegment container
             *  @param segments: Reference to the segment container to construct. */
            SearchTree_t constructTree(const Acts::GeometryContext& tgContext,
                                       const xAOD::MuonSegmentContainer& segments) const;
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
             *  @param ctx: EventContext (magnetic field for the ordering by fit quality)
             *  @param unresolved: Input MsTrackSeedContainer with duplicates */
            MsTrackSeedContainer resolveOverlaps(const EventContext& ctx, MsTrackSeedContainer&& unresolved) const;
            /** @brief  */
            MsTrackSeedContainer splitSeeds(MsTrackSeedContainer&& unsplitted) const;
            /** @brief Checks whether the seed has collected two segments on the same
             *         stations. Such that it needs to be split into multiple seeds 
             *  @param seed: The reference to the seed to check */
            bool splitSeed(const MsTrackSeed& seed) const;
            /** @brief Counts the number of shared precision hits betwen two segments
             *         Two segment share the same hit if the identifier of the precision
             *         hit is the same       
             *  @param a: Reference to the first segment in the comparison
             *  @param b: Reference to the second segment in the comparison */
            std::uint8_t countShared(const xAOD::MuonSegment& a, 
                                    const xAOD::MuonSegment& b) const;
            /** @brief Counts the number of stations sharing each at least
             *         one segment between two track seed.
             *  @param a: Reference to the first track seed
             *  @param b: Reference to the second track seed */
            std::uint8_t countShared(const MsTrackSeed& a,
                                     const MsTrackSeed& b) const;
            /** @brief Helper function to retrieve the beamspot */
            std::optional<MomentumEstimator::BeamSpot> retrieveBS(const EventContext& ctx,
                                                                  const Acts::GeometryContext& tgContext) const;
            
            /** @brief The radius of he barrel cylinder */
            DoubleProperty m_barrelRadius{this, "BarrelRadius", 7.*Gaudi::Units::m};
            /** @brief The maximum length of the barrel cylinder, if 
             *         not capped by the placement of the endcap discs */
            DoubleProperty m_barrelLength{this, "BarrelLength", 25.*Gaudi::Units::m};
            /** @brief Position of the endcap discs */
            DoubleProperty m_endcapDiscZ{this, "EndcapDiscZ", 15.*Gaudi::Units::m};
            /** @brief Radius of the endcap discs */
            DoubleProperty m_endcapDiscRadius{this, "EndcapRadius", 13.*Gaudi::Units::m};
            /** @brief Maximum separation of point on the cylinder to be picked up
              *         onto a seed */
            DoubleProperty m_seedHalfLength{this, "SeedHalfLength", 100.*Gaudi::Units::cm};
            /** @brief Momentum resolution in the barrel */
            DoubleProperty m_barrelMomentumRes{this, "BarrelMomentumResolution", 0.05};
            /** @brief Momentum resolution in the endcap */
            DoubleProperty m_endcapMomentumRes{this, "EndcapMomentumResolution", 0.05};
            /** @brief Consider two track seeds sharing a segment if their
             *         number of precision hits exceeds the threshold */
            UnsignedIntegerProperty m_nHitsShareSeg{this, "numHitsSegmentSharing", 3};
            /** @brief Consider two track seeds to be equivalent if they share segments on
             *         in x stations. If one candidate has only 2 stations, the used value is 1. */
            UnsignedIntegerProperty m_nSegShareSeed{this, "numSegmentsSeedSharing", 2};
            /************************* Momentum Estimator ************************** */
            /** @copydoc MomentumEstimator::Config::nFieldSteps  */
            UnsignedIntegerProperty m_nFieldSteps{this, "qpNFieldSteps", 30};
            /** @copydoc MomentumEstimator::Config::refitBendingPlane  */
            BooleanProperty m_refitBendingPlane{this, "qpRefitBendingPlane", false};
            /** @copydoc MomentumEstimator::Config::skipNswInBendingPlane */
            BooleanProperty m_skipNswInBendingPlane{this,"qpSkipNswInBendingPlane", false};
            /** @copydoc MomentumEstimator::Config::minBendingPower */
            DoubleProperty m_minBendingPower{this, "qpMinBendingPower", 0.1 * Gaudi::Units::tesla * Gaudi::Units::m};
            /** @copydoc MomentumEstimator::Config::outlierChi2 */
            DoubleProperty m_outlierChi2{this, "qpOutlierChi2", 10.};
            /** @copydoc MomentumEstimator::Config::maxDroppedSegments  */
            UnsignedIntegerProperty m_maxDroppedSegments{this, "qpMaxDroppedSegments", 3};
            /** @copydoc MomentumEstimator::Config::planeShiftSigmaB */
            DoubleProperty m_planeShiftSigmaB{this, "qpPlaneShiftSigmaBarrel", 40. * Gaudi::Units::mm};
            /** @copydoc MomentumEstimator::Config::planeShiftSigmaE */
            DoubleProperty m_planeShiftSigmaE{this, "qpPlaneShiftSigmaEndcap", 7. * Gaudi::Units::mm};
            /** @copydoc MomentumEstimator::Config::planeRotSigmaB */
            DoubleProperty m_planeRotSigmaB{this, "qpPlaneRotSigmaBarrel", 25. * Gaudi::Units::mrad};
            /** @copydoc MomentumEstimator::Config::planeRotSigmaE */
            DoubleProperty m_planeRotSigmaE{this, "qpPlaneRotSigmaEndcap", 5. * Gaudi::Units::mrad};
            /** @brief The beam spot measurement key, used in the momentum fit recovery of the inner segments. Empty to do not use it. */
            SG::ReadHandleKey<xAOD::UncalibratedMeasurementContainer> m_beamSpotKey{this, "qpBeamSpotKey", ""};
            /** @brief Pointer to the magnetic field provider */
            std::shared_ptr<const Acts::MagneticFieldProvider> m_bField{std::make_shared<ATLASMagneticFieldWrapper>()};
            /** @brief Momentum Estimator */
            std::unique_ptr<const MomentumEstimator> m_qOverPest{nullptr};

            /** @brief Pointer to the segement selection tool which compares
             *         two segments for their compatibilitiy */
            ToolHandle<ISegmentSelectionTool> m_segSelector{this, "SegmentSelectionTool" , "" };
            /** @brief Tracking geometry service */
            ServiceHandle<ActsTrk::ITrackingGeometrySvc> m_trackingGeometrySvc{this, "TrackingGeometrySvc", "ActsTrackingGeometrySvc"};
            /** @brief Utility to fetch the geometry, magnetic field and calibration context in the event */
            ActsTrk::ContextUtility m_ctxProvider{this};
            /** @brief Declare the data dependency on the standard Mdt+Rpc+Tgc segment container
             *         & on the NSW segment container */
            SG::ReadHandleKey<xAOD::MuonSegmentContainer> m_segmentKey{this, "SegmentContainer", "MsSegments" };
            /** @brief Pointer to the muon detector manager */
            const MuonGMR4::MuonDetectorManager* m_detMgr{nullptr};
    };
}


#endif
