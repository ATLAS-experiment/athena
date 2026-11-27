/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "MsTrackSeederTool.h"

#include "MuonTrackEvent/TrackingHelpers.h"

#include "Acts/Utilities/Helpers.hpp"
#include "Acts/Definitions/Tolerance.hpp"
#include "Acts/Geometry/TrackingGeometry.hpp"
#include "ActsInterop/Logger.h"

#include "Acts/Surfaces/LineBounds.hpp"
#include "Acts/Surfaces/RectangleBounds.hpp"
#include "Acts/Surfaces/DiamondBounds.hpp"
#include "Acts/Surfaces/TrapezoidBounds.hpp"
#include "Acts/Surfaces/PlaneSurface.hpp"

#include "xAODMuonPrepData/UtilFunctions.h"
#include "xAODAuxiliaryMeasurement/AuxiliaryMeasurement.h"
#include "GaudiKernel/PhysicalConstants.h"
#include <algorithm>
#include <limits>

namespace {
    /** @brief Check if a segment is an NSW segment. */
    bool isNswSegment(const xAOD::MuonSegment& seg) {
        using namespace Muon::MuonStationIndex;
        return seg.technology() == TechnologyIndex::STGC ||
               seg.technology() == TechnologyIndex::MM ||
               toStationIndex(seg.chamberIndex()) == StIndex::EE;
    }
    /** @brief Convert rad to deg. */
    double inDeg(double angle) {
        return angle / Gaudi::Units::deg;
    }
    /** @brief Convert Tesla * length from Athena to Acts units */
    constexpr double fieldLengthToActs {(Acts::UnitConstants::T * Acts::UnitConstants::m) /
            (Gaudi::Units::tesla * Gaudi::Units::m)};
}

namespace MuonR4{
    using SearchTree_t = MsTrackSeederTool::SearchTree_t;
    using SegVec_t = MsTrackSeederTool::SegVec_t;

    StatusCode MsTrackSeederTool::initialize() {
        ATH_CHECK(m_ctxProvider.initialize());
        ATH_CHECK(m_segSelector.retrieve());
        ATH_CHECK(m_trackingGeometrySvc.retrieve());
        ATH_CHECK(m_segmentKey.initialize(!m_segmentKey.empty()));
        ATH_CHECK(detStore()->retrieve(m_detMgr));

        ATH_CHECK(m_beamSpotKey.initialize(!m_beamSpotKey.empty()));
        MomentumEstimator::Config qOverPcfg{};
        qOverPcfg.nFieldSteps = m_nFieldSteps;
        qOverPcfg.refitBendingPlane = m_refitBendingPlane;
        qOverPcfg.skipNswInBendingPlane = m_skipNswInBendingPlane;
        qOverPcfg.minBendingPower = fieldLengthToActs * m_minBendingPower;
        qOverPcfg.outlierChi2 = m_outlierChi2;
        qOverPcfg.maxDroppedSegments = m_maxDroppedSegments;
        qOverPcfg.planeShiftSigmaB = ActsTrk::lengthToActs(m_planeShiftSigmaB);
        qOverPcfg.planeShiftSigmaE = ActsTrk::lengthToActs(m_planeShiftSigmaE);
        qOverPcfg.planeRotSigmaB = m_planeRotSigmaB;
        qOverPcfg.planeRotSigmaE = m_planeRotSigmaE;
        qOverPcfg.wireDir = [this](const Acts::GeometryContext& tgContext, 
                                   const xAOD::MuonSegment& segment) -> Acts::Vector3 {
            return wireDirection(tgContext, segment);
        };
        qOverPcfg.sectFrame = [this](const Acts::GeometryContext& tgContext,
                                     const xAOD::MuonSegment& segment) -> const Acts::Transform3& {
            return envelope(segment)->localToGlobalTransform(tgContext);
        };
        qOverPcfg.bField = m_bField;
        m_qOverPest = std::make_unique<const MomentumEstimator>(std::move(qOverPcfg), 
                                                                makeActsAthenaLogger(this, name()));
        return StatusCode::SUCCESS;
    }

    Acts::Result<Acts::BoundTrackParameters> 
        MsTrackSeederTool::estimateStartParameters(const EventContext& ctx,
                                                   const MsTrackSeed& seed) const {
            const Acts::GeometryContext tgContext = m_ctxProvider.getGeometryContext(ctx);

            const xAOD::MuonSegment* refSeg{nullptr};
            Acts::BoundMatrix cov{Acts::BoundMatrix::Zero()};
            for (const xAOD::MuonSegment* segment : seed.segments()) {    
                /** Ususally we would like to take the first segment with a sufficient amount of phi hits 
                 *  to set the initial position and direction of the track fit. However in some cases,
                 *  the segment from the NSW has a missreconstructed phi direction which causes the track fit to loose 
                 *  all BW and OW hits in the first iteration. Therefore if the first segment is a NSW segment, we first try 
                 *  to use a non-NSW segments with enough phi hits. If we don't find any segment with enough phi hits 
                 *  we will use the NSW segment as reference as long as it passes the seeding quality criteria.  */
                if (!refSeg && !isNswSegment(*segment) &&  
                    m_segSelector->passSeedingQuality(ctx, *segment)) {
                    refSeg = segment;
                    ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Set reference segment to "<<printSegment(*segment));
                }
                Acts::BoundTrackParameters boundPars = SegmentFit::boundSegmentPars(tgContext, *m_detMgr, *segment);
                if (!boundPars.covariance()) {
                    continue;
                }
                for (int i =0 ; i < cov.cols(); ++i) {
                    cov(i,i) += (*boundPars.covariance())(i,i);
                }

            }
            //if we did not find a reference segment let's try the NSW one before we give up on the track
            if(!refSeg){
                for (const xAOD::MuonSegment* segment : seed.segments()) {
                    if (isNswSegment(*segment) && 
                        m_segSelector->passSeedingQuality(ctx, *segment)) {
                        refSeg = segment;
                        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - NSW is the best what we have apparently....");
                        break;
                    }
                }
            }
            if (!refSeg) {
                ATH_MSG_WARNING(__func__<<"() "<<__LINE__
                    <<" - No reference segment passing the seeding quality was found.");
                return Acts::Result<Acts::BoundTrackParameters>::failure(std::make_error_code(std::errc::invalid_argument));
            }
            Amg::Vector3D seedPos{atFirstSurface(tgContext, *refSeg)};
            Amg::Vector3D seedDir{refSeg->direction()};
            ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Initial seed Pos: "<<Amg::toString(seedPos)
                <<" theta/phi: "<<inDeg(seedPos.theta())<<" / "<<inDeg(seedPos.phi())
                <<", dir: "<<Amg::toString(seedDir) 
                <<" theta/phi: "<<inDeg(seedDir.theta())<<" / "<<inDeg(seedDir.phi()));
            
            const xAOD::MuonSegment* frontSegment = seed.segments().front();
    
            const xAOD::UncalibratedMeasurement* firstMeas {firstMeasurement(*frontSegment)};
            const Acts::Surface& firstSurf = xAOD::muonSurface(firstMeas);
            const Acts::GeometryIdentifier volId = volumeId(firstSurf);
      
            // Find the first measurement
            const Acts::TrackingVolume* volume{MuonGMR4::highestAlignable(m_trackingGeometrySvc->trackingGeometry()->findVolume(volId))};
                       
            if (!volume) {
                ATH_MSG_WARNING(__func__<<"() "<<__LINE__
                                <<" - Failed to find tracking volume for seed measurement "<<volId);
                return Acts::Result<Acts::BoundTrackParameters>::failure(std::make_error_code(std::errc::invalid_argument));
            }
            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__
                            <<" - Bounding volume "<<volume->volumeName()
                            <<", trf: "<<Amg::toString(volume->localToGlobalTransform(tgContext))
                            <<", bounds: "<<volume->volumeBounds());
            /** The middle or outer segment provide the phi information. Not so easy becasue we want to
                Take the y0 & precision direction from the inner segment but the phi & x0 from a straight
                line extrapolation onto the plane */
            if (frontSegment != refSeg) {
                const Amg::Vector3D frontSegPos = atFirstSurface(tgContext, *frontSegment);
                const Acts::Transform3 toFirstTrf = firstSurf.localToGlobalTransform(tgContext).inverse();
                const Amg::Vector3D locFrontSegPos = toFirstTrf * frontSegPos;
                if (!volume->inside(tgContext, frontSegPos)) {
                    ATH_MSG_WARNING(__func__<<"() "<<__LINE__<<" - Segment "<<printSegment(*frontSegment)
                                    <<" not inside mother volume: "<<volume->volumeName()<<", "
                                    <<Amg::toString(volume->globalToLocalTransform(tgContext)*frontSegPos)
                                    <<", bounds: "<<volume->volumeBounds()<<", "
                                    <<SegmentFit::localSegmentPars(*frontSegment));
                }
                /** Update the local seed direction */
                {
                    const Acts::Transform3& toLoc{volume->globalToLocalTransform(tgContext)};
                    const Amg::Vector3D locSeedDir = toLoc.linear() * seedDir;
                    const Amg::Vector3D locFrontDir = toLoc.linear() * frontSegment->direction();
                    seedDir = volume->localToGlobalTransform(tgContext).linear() *
                              Acts::makeDirectionFromAxisTangents(houghTanAlpha(locSeedDir), 
                                                                  houghTanBeta(locFrontDir));
                    ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Updated seed direction theta/phi: "
                        <<inDeg(seedDir.theta())<<" / "<<inDeg(seedDir.phi()));
                }
                /** Extrapolate the seed segment onto the inner plane. We want to take the precision 
                    intercept from the inner segment and the non-precision intercept from the extrapolated
                    segment */
                const Acts::MultiIntersection firstIsect = firstSurf.intersect(tgContext, seedPos, seedDir,
                                                                               Acts::BoundaryTolerance::Infinite());
                const Amg::Vector3D locSeedAtFirst = toFirstTrf * firstIsect.at(0).position();
                if (firstSurf.type() == Acts::Surface::SurfaceType::Straw) {
                    const auto& bounds = static_cast<const Acts::LineBounds&>(firstSurf.bounds());
                    using enum Acts::LineBounds::BoundValues;
                    /** We want the drift radius coordinate from the segment and the coordinate along
                        the tube form the back extrapolated segment */ 
                    const Amg::Vector3D locStartPos{locFrontSegPos.x(), locFrontSegPos.y(),
                                                    std::clamp(locSeedAtFirst.z(), -bounds.get(eHalfLengthZ), bounds.get(eHalfLengthZ))};
                    ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - The first surface is a straw "
                                   <<bounds<<", track seed @first: "<<Amg::toString(locSeedAtFirst)<<" vs. segment @first: "
                                   <<Amg::toString(locFrontSegPos)<<", updated local start pos: "<<Amg::toString(locStartPos));
                    seedPos = firstSurf.localToGlobalTransform(tgContext) * locStartPos;
                } else if (firstSurf.type() == Acts::Surface::SurfaceType::Plane) {
                    if (isNswSegment(*frontSegment)) {
                        seedPos = frontSegPos;
                    } else {
                        /** We want the precision coordinate from the segment and the coordinate along the
                         *  strip from the back extrapolated segment */ 
                        Acts::Vector2 locStartPos2D{Acts::Vector2::Zero()};
                        const auto& bounds = firstSurf.bounds();
                        switch (bounds.type()) {
                            case Acts::SurfaceBounds::BoundsType::eRectangle:
                            case Acts::SurfaceBounds::BoundsType::eTrapezoid:
                            case Acts::SurfaceBounds::BoundsType::eDiamond:
                                /** Precision coordinate along local x, strip direction along local y */
                                locStartPos2D = Acts::Vector2{locFrontSegPos.x(), locSeedAtFirst.y()};
                                break;
                            default:
                                ATH_MSG_WARNING(__func__<<"() "<<__LINE__<<" - Unexpected surface bounds type "
                                                        <<firstSurf.bounds().type());
                                return Acts::Result<Acts::BoundTrackParameters>::failure(std::make_error_code(std::errc::invalid_argument));
                        }
                        /** The bounds are expressed in the same local frame as the surface transform */
                        if (!bounds.inside(locStartPos2D)) {
                            locStartPos2D = bounds.closestPoint(locStartPos2D, Acts::SquareMatrix2::Identity());
                        }
                        const Amg::Vector3D locStartPos {locStartPos2D.x(),locStartPos2D.y(), locFrontSegPos.z()};

                        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - The first surface is a plane with bounds "
                                        <<firstSurf.bounds() <<", track seed @first: "<<Amg::toString(locSeedAtFirst)<<" vs. segment @first: "
                                        <<Amg::toString(locFrontSegPos)<<", updated local start pos: "<<Amg::toString(locStartPos));
                        seedPos = firstSurf.localToGlobalTransform(tgContext) * locStartPos;
                    }
                } else {
                    ATH_MSG_WARNING(__func__<<"() "<<__LINE__<<" - Unexpected surface type "<<firstSurf.type());
                    return Acts::Result<Acts::BoundTrackParameters>::failure(std::make_error_code(std::errc::invalid_argument));
                }
                ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Updated seed position: "<<Amg::toString(seedPos)
                    <<" theta/phi: "<<inDeg(seedPos.theta())<<" / "<<inDeg(seedPos.phi()));
            }


            auto boundSurf = MuonGMR4::bottomBoundary(*volume);
            if (!boundSurf) {
                ATH_MSG_WARNING(__func__<<"() "<<__LINE__<<" - Failed to find boundary surface for tracking volume");
                 return Acts::Result<Acts::BoundTrackParameters>::failure(std::make_error_code(std::errc::invalid_argument));
            }
            std::shared_ptr<const Acts::Surface> targetSurf{};
            /** @brief Utility lambda to find the corrct boundary surface on which the start parameters 
             *         shall be created. The start parameters are linearly extrapolated onto the plane surface.
             *         The boundary surface is acceptable if it's in front of the seed position and within
             *         the surface bounds.  */
            auto propagateToBoundary = [&](const Acts::Surface& volBoundary) -> Acts::Result<Amg::Vector3D> {

                const Acts::Transform3& trf{volBoundary.localToGlobalTransform(tgContext)};
                using namespace Acts::PlanarHelper;
                auto pIsect = intersectPlane(seedPos, seedDir, trf.linear().col(Amg::z), trf.translation());
                /// The extrapolation needs to go backwards and stay within the surface boundaries
                if (pIsect.pathLength() > Acts::s_epsilon || !pIsect.isValid()) {
                    ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Intersection @"<<Amg::toString(pIsect.position())
                                    <<" is forward "<<pIsect.pathLength()<<" or invalid "<<(!pIsect.isValid())
                                    <<" within volume "<<volume->inside(tgContext, pIsect.position()));
                    return Acts::Result<Amg::Vector3D>::failure(std::make_error_code(std::errc::invalid_argument));
                }
                Acts::Result<Amg::Vector2D> locPos = volBoundary.globalToLocal(tgContext, pIsect.position(), 
                                                                               Amg::Vector3D::Zero());
                if (!locPos.ok()){
                    ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Intersection is not on surface "<<
                                    Amg::toString(trf.inverse()*pIsect.position()));
                    return Acts::Result<Amg::Vector3D>::failure(std::make_error_code(std::errc::invalid_argument));
                }
                if (!volBoundary.insideBounds(*locPos)) {
                    ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Intersection is outside the boundaries: "<<
                                    Amg::toString(*locPos)<<", bounds: "<<volBoundary.bounds());
                    return Acts::Result<Amg::Vector3D>::failure(std::make_error_code(std::errc::invalid_argument));
                }
                targetSurf = volBoundary.getSharedPtr();
                return Acts::Result<Amg::Vector3D>::success(pIsect.position());
            };
            /** Attempt first the propagation towards the bottom boundary surface */
            auto pIsect = propagateToBoundary(*boundSurf);
            /** If that fails and the volume is alignable, try all the 
             *  portal surfaces. Volume portals are not sorted in order but 
             *  the placements are. */
            if (!pIsect.ok() && volume->isAlignable()) {
                const Acts::VolumePlacementBase* placement = volume->volumePlacement();
                for (std::size_t portal = 0; !pIsect.ok()  && portal< placement->nPortalPlacements(); ++portal) {
                   pIsect = propagateToBoundary(placement->portalPlacement(portal)->surface());
                }
            }
            if (!pIsect.ok()) {
                ATH_MSG_WARNING(__func__<<"() "<<__LINE__<<" - Cannot create valid start parameters from seed "<<seed<<".");
                return Acts::Result<Acts::BoundTrackParameters>::failure(std::make_error_code(std::errc::invalid_argument));
            }
            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Extrapolated seed position: "<<Amg::toString(*pIsect)
                            <<" eta/phi: "<<pIsect->eta()<<" / "<<(inDeg(pIsect->phi())));
 
            const Acts::Vector4 fourPos = ActsTrk::convertPosToActs(*pIsect, (*pIsect).mag() / Gaudi::Units::c_light);
            /** Calculate the initial q / p estimator */
            const double momRes {seed.location() == Location::Barrel ? m_barrelMomentumRes : m_endcapMomentumRes};
            const double qOverP = estimateQoverP(ctx, seed)/ ActsTrk::energyToActs(1.);
            cov (Acts::eBoundQOverP, Acts::eBoundQOverP) = Acts::square(momRes * qOverP);
            
            return Acts::BoundTrackParameters::create(tgContext, targetSurf, 
                                                      fourPos, seedDir, qOverP, cov, 
                                                      Acts::ParticleHypothesis::muon());
    }
    Acts::Vector3 MsTrackSeederTool::wireDirection(const Acts::GeometryContext& tgContext,
                                                   const xAOD::MuonSegment& segment) const {
        const std::size_t nMeas = nMeasurements(segment);
        for (std::size_t meas = 0; meas < nMeas; ++ meas) {
            if (isOutlierMeasurement(segment, meas)) {
                continue;
            }
            const xAOD::UncalibratedMeasurement* measPtr = getMeasurement(segment, meas);
            if (xAOD::isPrecisionHit(measPtr)) {
                const Acts::TrackingVolume* volume = m_trackingGeometrySvc->trackingGeometry()->findVolume(volumeId(xAOD::muonSurface(measPtr)));
                if (!volume) {
                    ATH_MSG_WARNING(__func__<<"() "<<__LINE__<<" - Failed to find tracking volume for measurement "
                                    <<volumeId(xAOD::muonSurface(measPtr)));
                    continue;
                }
                return volume->localToGlobalTransform(tgContext).linear().col(Amg::x);
            }
        }
        /** Fall back function for the truth segment test */
        return envelope(segment)->surface().localToGlobalTransform(tgContext).linear().col(Amg::x);
    }
    Amg::Vector2D MsTrackSeederTool::expressOnCylinder(const Acts::GeometryContext& tgContext,
                                                       const xAOD::MuonSegment& segment,
                                                       const Location loc,
                                                       const ExpandedSector sector) const {
        /// extrapolated position
        const Amg::Vector3D pos{Acts::PlanarHelper::intersectPlane(segment.position(), 
                                                                     wireDirection(tgContext, segment), 
                                                                     sector.normalDir(), 0).position()};
        const Amg::Vector3D dir{segment.direction()};
 
        const Amg::Vector2D projPos{pos.perp(), pos.z()};
        const Amg::Vector2D projDir{dir.perp(), dir.z()};

        ATH_MSG_VERBOSE( "segment position:" << Amg::toString(segment.position())
                      << ", direction: " << Amg::toString(segment.direction()) );
        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Express segment in @"<<Amg::toString(pos)
                        <<", direction: "<<Amg::toString(dir)<< " sector projector: " << sector 
                        << " location: " << Acts::toUnderlying(loc));
        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Projected position onto sector: "<<Amg::toString(projPos)
                        <<", projected direction: "<<Amg::toString(projDir));
 
        double lambda{0.};
        if (Location::Barrel == loc) {
            lambda = Amg::intersect<2>(projPos, projDir, Amg::Vector2D::UnitX(), 
                                        m_barrelRadius).value_or(10. * Gaudi::Units::km);
            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Intersect with barrel at radius: "<<m_barrelRadius<<" --> "<<Amg::toString(projPos + lambda * projDir));
        } else {
            lambda = Amg::intersect<2>(projPos, projDir, Amg::Vector2D::UnitY(), 
                                       Acts::copySign(1.*m_endcapDiscZ, projPos[1])).value_or(10. * Gaudi::Units::km);
            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Intersect with endcap at z: "<<Acts::copySign(1.*m_endcapDiscZ, projPos[1])
            <<" --> "<<Amg::toString(projPos + lambda * projDir));
        }
        return projPos + lambda * projDir;  
    }
    bool MsTrackSeederTool::withinBounds(const Amg::Vector2D& projPos,
                                         const Location loc) const {
        using enum Location;
        if (loc == Barrel && std::abs(projPos[1]) > std::min(m_endcapDiscZ, m_barrelLength)) {
            ATH_MSG_VERBOSE(__func__<<"()  "<<__LINE__<<" - Position "<<Amg::toString(projPos)<<
                            " exceeds cylinder boundaries ("<<(1.*m_barrelRadius)<<", "
                            <<std::min(m_endcapDiscZ, m_barrelLength)<<")");
            return false;
        } else if (loc == Endcap && (0 > projPos[0] || projPos[0] > m_endcapDiscRadius)) {
            ATH_MSG_VERBOSE(__func__<<"()  "<<__LINE__<<" - Position "<<Amg::toString(projPos)<<
            " exceeds endcap boundaries ("<<(1.*m_endcapDiscRadius)<<", "<<Acts::copySign(1.*m_endcapDiscZ, projPos[1])<<")");
            return false;
        }
        return true;
    }
    void MsTrackSeederTool::appendSegment(const Acts::GeometryContext& tgContext,
                                          const xAOD::MuonSegment* segment,
                                          const Location loc,
                                          TreeRawVec_t& outContainer) const {
        
        const unsigned segSector = segment->sector();
        for (const auto proj : {SectorProjector::leftOverlap, 
                                SectorProjector::center, 
                                SectorProjector::rightOverlap}) {
            /// Check whether the segment belongs to the left or right sector as well
            const ExpandedSector projSector{segSector, proj};
            /// Segments with phi measurements go in the center of their own sector, and in the overlap with a
            /// neighbouring sector if their phi is inside that sector (in the overlap or in the neighbour's center)
            if (segment->nPhiLayers() > 0 && proj != SectorProjector::center) {
                const ExpandedSector phiSector{segment->position().phi()};
                const unsigned neighbour = projSector.msSector() == segSector ? projSector.adjacentMsSector()
                                                                              : projSector.msSector();
                if (phiSector != projSector && phiSector != ExpandedSector{neighbour, SectorProjector::center}) {
                    ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Segment @"<<Amg::toString(segment->position())
                        <<" is not in sector "<<projSector);
                    continue;
                }
            }
            const Amg::Vector2D refPoint{expressOnCylinder(tgContext, *segment, loc, projSector)};
            if (!withinBounds(refPoint, loc)) {
                 continue;
            }
            using enum SeedCoords;
            std::array<double, 3> coords{Acts::filledArray<double, 3>(0.)};
            /** Use the sector expanded sector coordinate */
            coords[Acts::toUnderlying(eSector)] = projSector.sector();
            /** Enumeration to indicate whether the segment is expressed on the negative endcap (-1),
             *  the barrel (0) or the positive endcap */
            coords[Acts::toUnderlying(eDetSection)] =  Acts::copySign(Acts::toUnderlying(loc), refPoint[1]);
            /** Coordinate on the cylinder */
            coords[Acts::toUnderlying(ePosOnCylinder)] = refPoint[Location::Barrel == loc];
            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Add segment "<<printSegment(*segment)
                            <<" with "<<coords<<" to the search tree");
            outContainer.emplace_back(std::move(coords), segment);
        }
    }
    SearchTree_t MsTrackSeederTool::constructTree(const Acts::GeometryContext& tgContext,
                                                  const xAOD::MuonSegmentContainer& segments) const{
        TreeRawVec_t rawData{};
        rawData.reserve(3*segments.size());
        for (const xAOD::MuonSegment* segment : segments){
            appendSegment(tgContext, segment, Location::Barrel, rawData);
            appendSegment(tgContext, segment, Location::Endcap, rawData);
        }
        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Create a new tree with "<<rawData.size()<<" entries. ");
        return SearchTree_t{std::move(rawData)};
    }
    StatusCode MsTrackSeederTool::findTrackSeeds(const EventContext& ctx,
                                                 std::vector<MsTrackSeed>& outputSeeds) const {
        
        const xAOD::MuonSegmentContainer* segments{nullptr};
        ATH_CHECK(SG::get(segments, m_segmentKey , ctx));
        const Acts::GeometryContext tgContext = m_ctxProvider.getGeometryContext(ctx);
        SearchTree_t orderedSegs{constructTree(tgContext, *segments)};
        MsTrackSeedContainer trackSeeds{};
        using enum SeedCoords;
        for (const auto& [coords, seedingSeg] : orderedSegs) {
            /** Bad segment not suitable for track seeding or the segment coordinates are
             *  just mirrored at the overlap between sector 1 -> 16 */
             if (!m_segSelector->passSeedingQuality(ctx, *seedingSeg)) {
                ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Segment "<<printSegment(*seedingSeg)
                                               <<" does not pass the seeding quality.");
                continue;
            }
            /** Define the search range. */    
            SearchTree_t::range_t selectRange{};
            /** Ensure that only endcap / barrel seeds are considered. 
             *  The values are integers -> add tiny margin */
            selectRange[Acts::toUnderlying(eDetSection)].shrink(coords[Acts::toUnderlying(eDetSection)] - 0.1, 
                                                                coords[Acts::toUnderlying(eDetSection)] + 0.1);
            /** Move 25 cm along the projected plane */
            selectRange[Acts::toUnderlying(ePosOnCylinder)].shrink(coords[Acts::toUnderlying(ePosOnCylinder)] - m_seedHalfLength, 
                                                                   coords[Acts::toUnderlying(ePosOnCylinder)] + m_seedHalfLength);
            /** Include the neighbouring sectors */
            selectRange[Acts::toUnderlying(eSector)].shrink(coords[Acts::toUnderlying(eSector)] -0.25, 
                                                            coords[Acts::toUnderlying(eSector)] +0.25);
            
            MsTrackSeed newSeed{static_cast<Location>(std::abs(coords[Acts::toUnderlying(eDetSection)])),
                                ExpandedSector{static_cast<std::int8_t>(coords[Acts::toUnderlying(eSector)])}};
            /** Append the seeding candidate segment -> used to sort the other segments in seed */
            newSeed.addSegment(seedingSeg);
            /** Using the cube above, let the tree search for all compatible segments */
            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Search for compatible segments to "<<printSegment(*seedingSeg)<<".");
            orderedSegs.rangeSearchMapDiscard(selectRange, [&](
                    const SearchTree_t::coordinate_t& /*coords*/,
                    const xAOD::MuonSegment* extendWithMe) {
                        /** Ensure that the sector overlap and momentum vectors are compatible with a MS trajectory */
                        if (!m_segSelector->compatibleForTrack(ctx, *seedingSeg, *extendWithMe)) {
                            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Segment "<<printSegment(*extendWithMe)<<" is not compatible.");
                            return;
                        }
                        newSeed.addSegment(extendWithMe);
            });
            /** No compatible segment within the second station was found */
            if (newSeed.stations().size() < 2) {
                ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Reject seed with segments only "<<newSeed.stations().size()
                                        <<" stations");
                continue;
            }
            /** Calculate the seed's position */
            const double r = newSeed.location() == Location::Barrel ? 1.*m_barrelRadius 
                                                                    : coords[Acts::toUnderlying(ePosOnCylinder)];
            const double z = newSeed.location() == Location::Barrel ? coords[Acts::toUnderlying(ePosOnCylinder)] 
                                                                    : coords[Acts::toUnderlying(eDetSection)]* m_endcapDiscZ;

            Amg::Vector3D pos = r * newSeed.sector().radialDir()
                              + z * Amg::Vector3D::UnitZ();
            /** The seed position is mainly for visualization */
            newSeed.setPosition(std::move(pos));
            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Add new seed "<<newSeed);
            trackSeeds.emplace_back(std::move(newSeed));
        }
        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Found in total "<<trackSeeds.size()<<" before overlap removal");
        // outputSeeds
        trackSeeds = resolveOverlaps(ctx, std::move(trackSeeds));
        std::ranges::move(trackSeeds, std::back_inserter(outputSeeds));
        return StatusCode::SUCCESS;
    }

    bool MsTrackSeederTool::splitSeed(const MsTrackSeed& seedCand) const {
        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Check whether \n"<<seedCand
                               <<"\n has two segments on the same station");
        const auto seedSegments = seedCand.segments();
        /// there exists exactly one segment on each station
        if (seedSegments.size() == seedCand.stations().size()) {
            return false;
        }
        for (std::size_t seg = 1 ; seg < seedSegments.size(); ++seg) {
            for (std::size_t seg1 = 0 ; seg1 < seg; ++seg1) {
                if (seedSegments[seg]->chamberIndex() == seedSegments[seg1]->chamberIndex()) {
                    ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Found two segments in the same station on the seed \n"
                        <<" --- "<<printSegment(*seedSegments[seg])<<"\n"
                        <<" --- "<<printSegment(*seedSegments[seg1]));
                    return true;
                }
            }
        }
        return false;
    }
   
    MsTrackSeedContainer MsTrackSeederTool::splitSeeds(MsTrackSeedContainer&& unsplitted) const {
        MsTrackSeedContainer seedsToSplit{};
        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Select the seeds amongst "
                                <<unsplitted.size()<<" seed candidates which can be split");
        /// Copy all splitable seed candidates to a second vector
        std::ranges::copy_if(unsplitted, std::back_inserter(seedsToSplit), 
                             [this](const MsTrackSeed& seedCand){
                                 return splitSeed(seedCand);
                             });
        // No splittable candidte found
        if (seedsToSplit.empty()) {
            return unsplitted;
        }
        // Remove first the seed candidates to be split
        auto [begin , end] = std::ranges::remove_if(unsplitted, [this](const MsTrackSeed& seedCand){
            return splitSeed(seedCand);
        });
        unsplitted.erase(begin, end);
        using namespace Muon::MuonStationIndex;
        /// Sort the segments per station
        std::array<SegVec_t, Acts::toUnderlying(ChIndex::ChIndexMax)> segsPerStation{};
        std::vector<SegVec_t> splitSeedSegs{};
        for (const MsTrackSeed& splitMe : seedsToSplit) {
            for (const xAOD::MuonSegment* seg : splitMe.segments()) {
                segsPerStation[Acts::toUnderlying(seg->chamberIndex())].push_back(seg);
            }
            for (SegVec_t& vec : segsPerStation) {
                if (vec.empty()) {
                    continue;
                }
                /// Start the chain by splitting the content
                /// into single element vectors 
                if (splitSeedSegs.empty()) {
                    for (const xAOD::MuonSegment* seg : vec) {
                        splitSeedSegs.emplace_back(std::vector{seg});
                    }
                } 
                /// Stations with just one segment are quite easy.
                /// Add the element to all the splitSeedSegs vectors
                else if (vec.size() == 1) {
                   for (SegVec_t& appendMe : splitSeedSegs) {
                        appendMe.push_back(vec.front());
                   }
                } else {
                   /// Sort the segment by precision hits
                   std::ranges::sort(vec, [](const xAOD::MuonSegment* a,
                                             const xAOD::MuonSegment* b){
                       if (a->nPrecisionHits() != b->nPrecisionHits()) {
                           return a->nPrecisionHits() > b->nPrecisionHits();
                       }
                       return a->nPhiLayers() > b->nPhiLayers();
                   });
                   /// Copy the existing unique segments over
                   std::vector<SegVec_t> expandMe{std::move(splitSeedSegs)};
                   splitSeedSegs.reserve(expandMe.size() * vec.size());
                   for (const xAOD::MuonSegment* seg : vec) {
                        for (const SegVec_t& expand : expandMe) {
                            splitSeedSegs.emplace_back(expand).push_back(seg);
                        }
                   }
                }
                // Do not to forget to clear at the end
                vec.clear();
            }
            /// Create the new seeed
            ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Split the "
                <<splitMe<<"\n into "<<splitSeedSegs.size()<<" seeds.");
            for (SegVec_t splitted :  splitSeedSegs){
                unsplitted.push_back(MsTrackSeed{splitMe.location(), splitMe.sector()});
                for (const xAOD::MuonSegment* seg: splitted) {
                    unsplitted.back().addSegment(seg);
                }
                ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Created new "<<unsplitted.back());
            }
            splitSeedSegs.clear();
        }
        return unsplitted;
    }

    std::uint8_t MsTrackSeederTool::countShared(const MsTrackSeed& seedA,
                                                const MsTrackSeed& seedB) const {
        /// Count how many segments are shared amongst
        using namespace Muon::MuonStationIndex;
        std::array<std::uint8_t, Acts::toUnderlying(StIndex::StIndexMax)> sharedCounter{};

        for (std::size_t a{0}; a < seedA.segments().size(); ++a){
            const xAOD::MuonSegment* segA = seedA.segments()[a];
            std::uint8_t& shared = sharedCounter[Acts::toUnderlying(toStationIndex(segA->chamberIndex()))];

            for(std::size_t b{0}; shared == 0 && b< seedB.segments().size(); ++b){
                const xAOD::MuonSegment* segB = seedB.segments()[b];               
                if (countShared(*segA, *segB) )
                shared += (countShared(*segA, *segB) >= m_nHitsShareSeg);
            }
        }
        std::uint8_t shared{0};
        for (std::uint8_t n : sharedCounter) {
            shared+=n;
        }
        return shared;
    }

    inline std::uint8_t 
        MsTrackSeederTool::countShared(const xAOD::MuonSegment& a, 
                                       const xAOD::MuonSegment& b) const {
        /// Segment are identical 
        if (a.index() == b.index()) {
            return a.nPrecisionHits();
        }
        if (a.chamberIndex() != b.chamberIndex() || a.sector() != b.sector() ||
            a.etaIndex() != b.etaIndex()) {
            return 0u;
        }
        if (a.nPrecisionHits() > b.nPrecisionHits()) {
            return countShared(b, a);
        }
        unsigned shared{0};
        for (std::size_t hitA =0; hitA < nMeasurements(a); ++hitA) {
            if (isOutlierMeasurement(a, hitA)) {
                continue;
            }
            /// Just count the precision hits on track
            const xAOD::UncalibratedMeasurement* measA = getMeasurement(a, hitA);
            if (!xAOD::isPrecisionHit(measA)) {
                continue;
            }
            for (std::size_t hitB = 0 ; hitB < nMeasurements(b); ++hitB) {
                if (getMeasurement(b, hitB) == measA) {
                    ++shared;
                    break;
                }
            }
        }
        return shared;
    }

    MsTrackSeedContainer
        MsTrackSeederTool::resolveOverlaps(const EventContext& ctx, MsTrackSeedContainer&& unresolved) const {
        /** Resort the seeds starting from the ones with the most segments to the lowest  */
        std::ranges::sort(unresolved, [](const MsTrackSeed& a, const MsTrackSeed&b) {
            return a.segments().size() > b.segments().size();
        });
        auto [begin, end] = std::ranges::unique(unresolved, 
            [](const MsTrackSeed& a, const MsTrackSeed& b) {
            const auto segsA = a.segments();
            const auto segsB = b.segments();
            if (segsA.size() == segsB.size()) {
                for (std::size_t s = 0 ; s < segsA.size(); ++s) {
                    if (segsA[s] != segsB[s]) {
                        return false;
                    }
                }
                return true;
            } else if (segsA.size() < segsB.size()) {
                return std::ranges::all_of(segsA, [&segsB](const xAOD::MuonSegment* seg) {
                    return Acts::rangeContainsValue(segsB, seg); 
                });
            }
            return std::ranges::all_of(segsB, [&segsA]( const xAOD::MuonSegment* seg) {
                return Acts::rangeContainsValue(segsA, seg);
            });
        });
        unresolved.erase(begin, end);
        MsTrackSeedContainer outputSeeds{splitSeeds(std::move(unresolved))};
        const Acts::GeometryContext& tgContext {m_ctxProvider.getGeometryContext(ctx)};
        const Acts::MagneticFieldContext& mfContext {m_ctxProvider.getMagneticFieldContext(ctx)};
        const std::optional<MomentumEstimator::BeamSpot> bs {retrieveBS(ctx, tgContext)};
        std::shared_ptr<std::uint8_t> overlapMarker{};
        /** For each seed, we store: std::nullopt if non-overlapping, +chi2/ndf for a  
         *  good fit, -chi2/ndf for a bad one (-max if it failed)  */
        std::vector<std::pair<std::optional<double>, std::size_t>> order{};
        for (std::size_t s = 0; s < outputSeeds.size(); ++s) {
            order.emplace_back(std::nullopt, s);
        }
        for (auto [s1, seed1] : Acts::enumerate(outputSeeds)) {
            /// We create an overlap marker
            for (std::size_t s2 = s1 +1 ; s2 < outputSeeds.size(); ++s2) {
                MsTrackSeed& seed2 = outputSeeds.at(s2);
                const unsigned shortestSeed = std::min(seed1.stations().size(), seed2.stations().size());
                if (countShared(seed1, seed2) >= 
                        (shortestSeed > 2u ? m_nSegShareSeed.value() : 1u)) {
                    if(!overlapMarker) {
                        overlapMarker = std::make_shared<std::uint8_t>(0);
                        seed1.prepareOverlap(overlapMarker);
                    }
                    seed2.prepareOverlap(overlapMarker);
                    order[s1].first = order[s2].first = 0.;
                }
            }
            overlapMarker.reset();
            if (order[s1].first) {
                const auto fit {m_qOverPest->fitQoverP(tgContext, mfContext, seed1, /*withRecovery=*/false, bs)};
                order[s1].first = fit ? m_qOverPest->isGoodFit(fit) ? fit->redChi2() : -fit->redChi2()
                                      : -std::numeric_limits<double>::max();                      
            }
            
        }
        /** Order the seeds by the plain momentum fit (most stations, then lowest chi2/ndf) */
        std::ranges::stable_sort(order, [&](const auto& a, const auto& b) {
            if (!a.first || !b.first) {
                return b.first.has_value();
            }
            const MsTrackSeed& seedA {outputSeeds[a.second]};
            const MsTrackSeed& seedB {outputSeeds[b.second]};
            if (seedA.stations().size() != seedB.stations().size()) {
                return seedA.stations().size() > seedB.stations().size();
            }
            const bool goodFitA {*a.first >= 0.};
            const bool goodFitB {*b.first >= 0.};
            if (goodFitA != goodFitB) {
                return goodFitA;
            }
            if (seedA.segments().size() != seedB.segments().size()) {
                return seedA.segments().size() > seedB.segments().size();
            }
            return std::abs(*a.first) < std::abs(*b.first);
        });
        MsTrackSeedContainer ordered{};
        for (const auto& [_, s] : order) {
            ordered.push_back(std::move(outputSeeds[s]));
        }
        return ordered;
    } 
    double MsTrackSeederTool::estimateQoverP(const EventContext& ctx,
                                             const MsTrackSeed& seed) const {
        const Acts::GeometryContext& tgContext {m_ctxProvider.getGeometryContext(ctx)};
        const Acts::MagneticFieldContext& mfContext {m_ctxProvider.getMagneticFieldContext(ctx)};
        const auto fitResult = m_qOverPest->fitQoverP(tgContext, mfContext,
                                                      seed, /*withRecovery=*/true,
                                                      retrieveBS(ctx, tgContext));
        if (!fitResult) {
            ATH_MSG_WARNING(__func__<<"() "<<__LINE__<<" no q/p fit for seed "<<seed
                <<" (no bending field along the seed and no beam spot)");
            return 1. / (5 * Gaudi::Units::TeV);     
        }
        return fitResult->qOverP  / ActsTrk::energyToAthena(1.);
    }

    const MuonGMR4::SpectrometerSector* 
        MsTrackSeederTool::envelope(const xAOD::MuonSegment& segment) const{
        return m_detMgr->getSectorEnvelope(segment.chamberIndex(), 
                                           segment.sector(), 
                                           segment.etaIndex());
    }

    std::optional<MomentumEstimator::BeamSpot> 
        MsTrackSeederTool::retrieveBS(const EventContext& ctx,
                                      const Acts::GeometryContext& tgContext) const {
        if (m_beamSpotKey.empty()) {
            return std::nullopt;
        }
        const xAOD::UncalibratedMeasurementContainer* beamSpotCont {nullptr};
        if (SG::get(beamSpotCont, m_beamSpotKey, ctx).isSuccess() 
                && beamSpotCont && !beamSpotCont->empty()) {

            const auto* bsMeas = static_cast<const xAOD::AuxiliaryMeasurement*>(beamSpotCont->front());
            MomentumEstimator::BeamSpot bs{};
            bs.position = bsMeas->surface()->center(tgContext);
            bs.sigmaZ= std::sqrt(bsMeas->localCovariance<2>()(1, 1));
            return bs;
        } else {
            ATH_MSG_WARNING(__func__<<"() "<<__LINE__<<" - No beam spot measurement");
            return std::nullopt;
        }                                
    }
}