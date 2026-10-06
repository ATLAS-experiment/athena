/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "MsTrackSeederTool.h"

#include "MuonTrackEvent/TrackingHelpers.h"

#include "Acts/Utilities/Helpers.hpp"
#include "Acts/Definitions/Tolerance.hpp"
#include "Acts/Definitions/Units.hpp"
#include "Acts/Geometry/TrackingGeometry.hpp"

#include "Acts/Surfaces/LineBounds.hpp"
#include "Acts/Surfaces/RectangleBounds.hpp"
#include "Acts/Surfaces/DiamondBounds.hpp"
#include "Acts/Surfaces/TrapezoidBounds.hpp"
#include "Acts/Surfaces/PlaneSurface.hpp"

#include "xAODMuonPrepData/UtilFunctions.h"
#include "FourMomUtils/P4Helpers.h"
#include "CxxUtils/trapping_fp.h"
#include "GaudiKernel/PhysicalConstants.h"

namespace {
    using namespace Acts::UnitLiterals;
    /** @brief Check if the charges of two PtimesQ estimates agree */
    constexpr bool chargeAgree(double PtimesQ1, double PtimesQ2) {
        return PtimesQ1 * PtimesQ2 > 0;
    };
    /** @brief Calculate the momentum deviation between two PtimesQ estimates. The function is symmetric. */
    inline double momentumDev(double PtimesQ1, double PtimesQ2) {
        const double denom {std::max(std::abs(PtimesQ1) + std::abs(PtimesQ2), Acts::s_epsilon)};
        return std::abs(PtimesQ1 - PtimesQ2) / denom;
    };
    /** @brief Calculate the reduced chi-squared of a segment. */
    float reducedChi2(const xAOD::MuonSegment& seg) {
        // Tell clang to optimize assuming that FP operations may trap.
        CXXUTILS_TRAPPING_FP;
        return seg.chiSquared() / std::max(1.f, seg.numberDoF());
    }
    /** @brief Print brief segment information. */
    std::string print(const xAOD::MuonSegment& seg) {
        return std::format("{:}, nPrecHits: {:}, nPhiHits: {:}", MuonR4::printID(seg),
                           seg.nPrecisionHits(), seg.nPhiLayers());
    }
    /** @brief Check if a segment is an NSW segment. */
    bool isNswSegment(const xAOD::MuonSegment& seg) {
        using namespace Muon::MuonStationIndex;
        return seg.technology() == TechnologyIndex::STGC ||
               seg.technology() == TechnologyIndex::MM ||
               toStationIndex(seg.chamberIndex()) == StIndex::EE;
    }

    /// A seed whose segments can't fill at least two slots crashes it. Check the topology before calling.
    bool canEstimateQtimesP(const MuonR4::MsTrackSeed& seed) {
        using namespace Muon::MuonStationIndex;
        bool hasInner{false}, hasMiddle{false}, hasOuter{false};
        unsigned int nExtended{0};
        for (const xAOD::MuonSegment* seg : seed.segments()) {
            switch (toLayerIndex(seg->chamberIndex())) {
                case LayerIndex::Inner:  hasInner = true; break;
                case LayerIndex::Middle: hasMiddle = true; break;
                case LayerIndex::Outer:  hasOuter = true; break;
                case LayerIndex::Extended:
                case LayerIndex::BarrelExtended: ++nExtended; break;
                default: break;
            }
        }
        const unsigned int nIMO = hasInner + hasMiddle + hasOuter;
        return nIMO + nExtended >= 2u;
    }

    /** @brief Convert rad to deg. */
    double inDeg(double angle) {
        return angle / Gaudi::Units::deg;
    }
}

namespace MuonR4{
    using SearchTree_t = MsTrackSeederTool::SearchTree_t;

    StatusCode MsTrackSeederTool::initialize() {
        ATH_CHECK(m_ctxProvider.initialize());
        ATH_CHECK(m_segSelector.retrieve());
        ATH_CHECK(m_trackingGeometrySvc.retrieve());
        ATH_CHECK(m_segmentKey.initialize(!m_segmentKey.empty()));
        ATH_CHECK(detStore()->retrieve(m_detMgr));

        if (m_nFieldSteps == 0) {
            ATH_MSG_ERROR("The number of field steps must not be zero "<<m_nFieldSteps);
            return StatusCode::FAILURE;
        }
        /** Initialize the field extraction steps */
        const double stepSize {1. / m_nFieldSteps};
        for (std::size_t i = 0; i < m_nFieldSteps; ++i) {
            m_fieldExtpSteps.push_back((static_cast<double>(i) + 0.5) * stepSize);
        }
        return StatusCode::SUCCESS;
    }

    Acts::Result<Acts::BoundTrackParameters> 
        MsTrackSeederTool::estimateStartParameters(const EventContext& ctx,
                                                   const MsTrackSeed& seed) const {
            const Acts::GeometryContext tgContext = m_ctxProvider.getGeometryContext(ctx);
            const Acts::MagneticFieldContext mfContext = m_ctxProvider.getMagneticFieldContext(ctx);
            MagField::AtlasFieldCache magField{};
            mfContext.get<const AtlasFieldCacheCondObj*>()->getInitializedCache(magField);

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
                    ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Set reference segment to "<<::print(*segment));
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
                ATH_MSG_DEBUG(__func__<<"() "<<__LINE__
                                <<" - No reference segment passing seeding quality was found.");
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
                    ATH_MSG_WARNING(__func__<<"() "<<__LINE__<<" - Segment "<<::print(*frontSegment)
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
                        Acts::Vector2 locStartPos2D {locFrontSegPos.x(), locSeedAtFirst.y()};
                        const auto& bounds = firstSurf.bounds();
                        switch (bounds.type()) {
                            case Acts::SurfaceBounds::BoundsType::eRectangle:
                                if (!bounds.inside(locStartPos2D)) {
                                    locStartPos2D = bounds.closestPoint(locStartPos2D, Acts::SquareMatrix2::Identity());
                                }
                                break;
                            case Acts::SurfaceBounds::BoundsType::eTrapezoid:
                            case Acts::SurfaceBounds::BoundsType::eDiamond:
                                /** Trapezoids and diamonds have their x and y axis swapped */
                                std::swap(locStartPos2D.x(), locStartPos2D.y());
                                if (!bounds.inside(locStartPos2D)) {
                                    locStartPos2D = bounds.closestPoint(locStartPos2D, Acts::SquareMatrix2::Identity());
                                }
                                std::swap(locStartPos2D.x(), locStartPos2D.y());
                                break;
                            default:
                                ATH_MSG_WARNING(__func__<<"() "<<__LINE__<<" - Unexpected surface bounds type "
                                                        <<firstSurf.bounds().type());
                                return Acts::Result<Acts::BoundTrackParameters>::failure(std::make_error_code(std::errc::invalid_argument));
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
                ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" Cannot create valid start parameters from seed "<<seed<<".");
                return Acts::Result<Acts::BoundTrackParameters>::failure(std::make_error_code(std::errc::invalid_argument));
            }
            if (!canEstimateQtimesP(seed)) {
                ATH_MSG_WARNING(__func__<<"() "<<__LINE__<<" Cannot estimate q*p from seed "<<seed
                                <<" - insufficient inner/middle/outer layer coverage.");
                return Acts::Result<Acts::BoundTrackParameters>::failure(std::make_error_code(std::errc::invalid_argument));
            }
            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Extrapolated seed position: "<<Amg::toString(*pIsect)
                            <<" eta/phi: "<<pIsect->eta()<<" / "<<(inDeg(pIsect->phi())));
 
            const Acts::Vector4 fourPos = ActsTrk::convertPosToActs(*pIsect, (*pIsect).mag() / Gaudi::Units::c_light);
            /** Calculate the initial q / p estimator */
            const double momRes {seed.location() == Location::Barrel ? m_barrelMomentumRes : m_endcapMomentumRes};
            const double qOverP = 1./ ActsTrk::energyToActs(estimateQtimesP(tgContext, seed, magField));
            cov (Acts::eBoundQOverP, Acts::eBoundQOverP) = Acts::square(momRes * qOverP);
            
            return Acts::BoundTrackParameters::create(tgContext, targetSurf, 
                                                      fourPos, seedDir, qOverP, cov, 
                                                      Acts::ParticleHypothesis::muon());
    }
    Amg::Vector3D MsTrackSeederTool::segPosOntoPhiPlane(const Acts::GeometryContext& tgContext,
                                                        const Amg::Vector3D& planeNormal,
                                                        const xAOD::MuonSegment& segment) const{
        const std::size_t nMeas = nMeasurements(segment);
        Amg::Vector3D wireDir{Amg::Vector3D::Zero()};

        for (std::size_t meas = 0; meas < nMeas; ++ meas) {
            if (isOutlierMeasurement(segment, meas)) {
                continue;
            }
            const xAOD::UncalibratedMeasurement* measPtr = getMeasurement(segment, meas);
            if (xAOD::isPrecisionHit(measPtr)) {
                wireDir =  m_trackingGeometrySvc->trackingGeometry()->findVolume(volumeId(xAOD::muonSurface(measPtr)))->
                                                  localToGlobalTransform(tgContext).linear().col(Amg::x);
                break;
            }
        }
        /** Fall back function for the truth segment test */
        if (nMeas == 0ul) {
            wireDir = envelope(segment)->surface().localToGlobalTransform(tgContext).linear().col(Amg::x);
        }

        return Acts::PlanarHelper::intersectPlane(segment.position(), wireDir, 
                                                  planeNormal, Amg::Vector3D::Zero()).position();                                
    }
    Amg::Vector2D MsTrackSeederTool::expressOnCylinder(const Acts::GeometryContext& tgContext,
                                                       const xAOD::MuonSegment& segment,
                                                       const Location loc,
                                                       const ExpandedSector sector) const {
        /// extrapolated position
        const Amg::Vector3D pos{segPosOntoPhiPlane(tgContext, sector.normalDir(), segment)};
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
    double MsTrackSeederTool::estimateQtimesP(const Amg::Vector3D& planeNorm,
                                              const PosMomPair_t& p1, 
                                              const PosMomPair_t& p2,
                                              const PosMomPair_t& p3,
                                              MagField::AtlasFieldCache& fieldCache) const {
        // When 3 points are available, we can use each pair of segments to estimate 
        // the momentum and charge, and then combine the estimates. To make the 
        // combination, we define a struct to hold each PtimesQ estimate
        struct Estimate {
            double PtimesQ{0.};
            // Weighting factor based on the magnitude of the integrated force.
            double weight{0.};
            // Scores as a combination of charge agreement and momentum deviation.
            double score{0.};
        };
        
        const Amg::Vector3D force12 {forceIntegration(p1, p2, planeNorm, fieldCache)};
        const Amg::Vector3D force23 {forceIntegration(p2, p3, planeNorm, fieldCache)};
        const Amg::Vector3D force13 {force12 + force23};
        
        std::vector<Estimate> estimates{};
        const double weightNorm {force12.mag() + force23.mag()};
        // Pairwise momentum estimates: 12 and 23
        estimates.emplace_back(getPtimesQ(force12, p2.second - p1.second), force12.mag()/weightNorm, 0.);
        estimates.emplace_back(getPtimesQ(force23, p3.second - p2.second), force23.mag()/weightNorm, 0.);
        // Two estimates for pair 13: using segment directions and using position differences
        const double weight13 {force13.mag()/weightNorm};
        estimates.emplace_back(getPtimesQ(force13, p3.second - p1.second), weight13, 0.);
        const Amg::Vector3D t12 {(p2.first - p1.first).unit()};
        const Amg::Vector3D t23 {(p3.first - p2.first).unit()};
        estimates.emplace_back(getPtimesQ(force13, t23 - t12), weight13, 0.);

        for (std::size_t i {0}; i < estimates.size(); ++i) {
            Estimate& est1 {estimates[i]};
            for (std::size_t j {i+1}; j < estimates.size(); ++j) {
                Estimate& est2 {estimates[j]};
                double agreementScore {(chargeAgree(est1.PtimesQ, est2.PtimesQ) ? 1. : -1.) - 
                                        momentumDev(est1.PtimesQ, est2.PtimesQ)};
                /** Update the scores, encoding how well this estimate agrees 
                 *  with the other estimates, weighted by their reliability. */
                est1.score += est2.weight * agreementScore;
                est2.score += est1.weight * agreementScore;
            }
        }
        /** Shift scores to positive values so that reliability can be
         *  used multiplicatively when selecting the best estimate. */
        const double minScore {std::ranges::min_element(estimates, 
            {}, &Estimate::score)->score};
        for (Estimate& est : estimates) {
            est.score -= minScore;
        }
        if (msgLvl(MSG::VERBOSE)) {
            std::vector<std::string> names {"Pair01", "Pair12", "Pair02Seg", "Pair02Pos"};
            for (const auto [i, est] : Acts::enumerate(estimates)) {
                ATH_MSG_VERBOSE(__func__<<"() Estimate "<<names[i]<<": PtimesQ: "<<est.PtimesQ*1e-3
                    <<", weight: "<<est.weight<<", score: "<<est.score);
            }
        }
        /** Find the best charge estimate and estimate the final momentum as a weighted average 
         *  of the estimates that agree with the best charge */
        const Estimate& bestEstimate {*std::ranges::max_element(estimates, 
            std::ranges::less{}, [](const Estimate& est){
                return est.score * est.weight;})};
        const double charge {std::copysign(1., bestEstimate.PtimesQ)};

        double totalSum {0.}, totalWeight {0.};
        for (const Estimate& est : estimates) {
            /** We exclude the estimation given from Pair02Pos because it is an approximation,
             *  and therefore the estimation is less precise. */
            if (&est == &estimates.back()) {
                continue;
            }
            if (chargeAgree(est.PtimesQ, charge)) {
                totalSum += est.PtimesQ * est.weight;
                totalWeight += est.weight;
            }
        }
        assert(totalWeight > Acts::s_epsilon);
        return totalSum / totalWeight;
    }
    double MsTrackSeederTool::estimateQtimesP(const Amg::Vector3D& planeNorm,
                                              const PosMomPair_t& p1,
                                              const PosMomPair_t& p2,
                                              MagField::AtlasFieldCache& fieldCache) const {
  
        return getPtimesQ(forceIntegration(p1, p2, planeNorm, fieldCache),
                          p2.second - p1.second);
    }
    Amg::Vector3D MsTrackSeederTool::forceIntegration(const PosMomPair_t& point1,
                                                      const PosMomPair_t& point2,
                                                      const Amg::Vector3D& planeNorm,
                                                      MagField::AtlasFieldCache& fieldCache) const {
        const auto& [pos1, dir1] = point1;
        const auto& [pos2, dir2] = point2;
        ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Integrate field from "<<Amg::toString(pos1)
            <<" to "<<Amg::toString(pos2)<<", direction changing from "
            <<inDeg(dir1.theta())<<" / "<<inDeg(dir1.phi())<<" to "
            <<inDeg(dir2.theta())<<" / "<<inDeg(dir2.phi()));

        Amg::Vector3D locField{Amg::Vector3D::Zero()};
        Amg::Vector3D accumForce{Amg::Vector3D::Zero()};
        for (double fieldStep : m_fieldExtpSteps) {
            const Amg::Vector3D extPos {(1. - fieldStep) * pos1 + fieldStep * pos2};
            const Amg::Vector3D extDir {((1. - fieldStep) * dir1 + fieldStep * dir2).unit()};
                    
            fieldCache.getField(extPos.data(), locField.data());
            const Amg::Vector3D locForce {locField.dot(planeNorm) * extDir.cross(planeNorm)};
            accumForce += locForce;

            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - step: "<<fieldStep
                <<", pos: "<<Amg::toString(extPos)<<", dir: "<<inDeg(extDir.theta())
                <<" / "<<inDeg(extDir.phi())<<" --> local |B|: "<<locField.mag()*1e3
                <<" [T]"<<", |Bnorm|: "<<locField.dot(planeNorm)*Gaudi::Units::GeV
                <<" [T], local |v x Bnorm|: "<<locForce.mag()*Gaudi::Units::GeV<<" [T].");
        }
        const double dS {(pos2 - pos1).mag() / static_cast<double>(m_fieldExtpSteps.size())};
        ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Integrated force: "<<Amg::toString(accumForce)<<", dS: "<<dS);
        return accumForce * dS;
    }
    double MsTrackSeederTool::getPtimesQ(const Amg::Vector3D& forceIntegral, 
                                     const Amg::Vector3D& deltaDir) const {
        const double PtimesQ {0.3 * Gaudi::Units::GeV * forceIntegral.mag2() / deltaDir.dot(forceIntegral)};

        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - estimateQtimesP() force integral: "<<forceIntegral.mag()<<" [T*m], deltaDir: "
            <<deltaDir.mag()<<", cos: "<<deltaDir.dot(forceIntegral)/ (deltaDir.mag() * forceIntegral.mag())
            <<", PtimesQ: "<<PtimesQ/Gaudi::Units::GeV <<" [GeV].");
        return PtimesQ;                                    
    }
    double MsTrackSeederTool::estimateQtimesP(const Acts::GeometryContext& tgContext,
                                              const MsTrackSeed& seed,
                                              MagField::AtlasFieldCache& magField) const {
        using namespace Muon::MuonStationIndex;

        // Guard canEstimateQtimesP here, not only at call sites.
        if (!canEstimateQtimesP(seed)) {
            ATH_MSG_WARNING(__func__<<"() "<<__LINE__<<" Cannot estimate q*p from seed "<<seed
                            <<" - insufficient inner/middle/outer layer coverage.");
            return 0.;
        }

        /** Calculate the averaged phi from the segments */
        double deltaPhiAcc {0.};
        std::optional<double> centralPhi {};
        unsigned nSegsWithPhi{0};
        for (const xAOD::MuonSegment* segment : seed.segments()) {
            if (segment->nPhiLayers() > 0) {
                const double segPhi {segment->position().phi()};
                if (!centralPhi) centralPhi = segPhi;
                deltaPhiAcc += P4Helpers::deltaPhi(*centralPhi, segPhi);
                ++nSegsWithPhi;
            }
        }
        const double circPhi {nSegsWithPhi > 0 
            ? P4Helpers::deltaPhi(*centralPhi + deltaPhiAcc / nSegsWithPhi, 0.) 
            : seed.sector().phi()};
        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Average segment phi: "
            <<inDeg(circPhi)<<" [deg], nSegsWithPhi: "<<nSegsWithPhi);

        std::array<const xAOD::MuonSegment*, 3> segmentsToUse{};
        // Try first to find segments in the inner, middle and outer layers. If both a barrel
        // and endcap segments are present in the same layer, the barrel segment is preferred.
        for (const xAOD::MuonSegment* segment : seed.segments()) {
            ChIndex chIndex {segment->chamberIndex()};
            switch(toLayerIndex(chIndex)) {
                using enum LayerIndex;
                case Inner:
                    if (!segmentsToUse[0] || isBarrel(chIndex)) {
                        segmentsToUse[0] = segment;
                    }
                    break;
                case Middle:
                    if (!segmentsToUse[1] || isBarrel(chIndex)) {
                        segmentsToUse[1] = segment;
                    }
                    break;
                case Outer:
                    if (!segmentsToUse[2] || isBarrel(chIndex)) {
                        segmentsToUse[2] = segment;
                    }
                    break;
                default:
                    break;
            }
        }
        unsigned nSegments = std::ranges::count_if(segmentsToUse, 
            [](const xAOD::MuonSegment* seg) { return seg != nullptr; });

        /** If less than 3 segments are found check whether the track crosses 
         *  the BEE or EE chamber and use that segment as the third one. */
        if (nSegments < 3) {
            auto missingSeg = std::ranges::find(segmentsToUse, nullptr);
            for (const xAOD::MuonSegment* segment : seed.segments()) {
                LayerIndex layIndex {toLayerIndex(segment->chamberIndex())};
                if (layIndex != LayerIndex::Extended && layIndex != LayerIndex::BarrelExtended) {
                    continue;
                }
                assert(missingSeg != segmentsToUse.end());
                *missingSeg = segment;
                ++nSegments;
                if (nSegments == 3) {
                    break;
                }
                missingSeg = std::ranges::find(segmentsToUse, nullptr);
            }
            std::ranges::sort(segmentsToUse, [](const xAOD::MuonSegment* seg1, const xAOD::MuonSegment* seg2) {
                if (!seg1 || !seg2) {
                    return seg1 != nullptr;
                }
                return seg1->position().perp() < seg2->position().perp();
            });
        }
        const Amg::Vector3D planeNorm {Acts::makeDirectionFromPhiTheta(circPhi + 90._degree, 90._degree)};
        auto point = [&](const xAOD::MuonSegment* seg) {
            const Amg::Vector3D projSegPos {segPosOntoPhiPlane(tgContext, planeNorm, *seg)};

            /** Fall back function for the truth segment test */
            if (nMeasurements(*seg) == 0ul) {
                return std::make_pair(projSegPos,
                                      Acts::makeDirectionFromPhiTheta(circPhi, seg->direction().theta()));
            }
            
            const Acts::Surface& firstSurf {xAOD::muonSurface(firstMeasurement(*seg))};
            const Acts::TrackingVolume* volume{
                MuonGMR4::highestAlignable(m_trackingGeometrySvc->trackingGeometry()->findVolume(volumeId(firstSurf)))};
            
            if (!volume) {
                ATH_MSG_WARNING("estimateQtimesP() "<<__LINE__
                    <<" - Failed to find tracking volume for seed measurement "<<volumeId(firstSurf));
                return std::make_pair(projSegPos,
                                      Acts::makeDirectionFromPhiTheta(circPhi, seg->direction().theta()));
            }
            const Acts::Transform3& toLoc {volume->globalToLocalTransform(tgContext)};
            const Amg::Vector3D locSegDir {toLoc.linear() * seg->direction()};
            const Amg::Vector3D locNormal {toLoc.linear() * planeNorm};
            /** Contrain the direction to have the same tangent in the precision
             *  plane and to lay in the bending plane */
            const double tanBeta {houghTanBeta(locSegDir)};
            const double tanAlpha {- (locNormal.y() * tanBeta + locNormal.z()) / locNormal.x()};
            const Amg::Vector3D newDir = volume->localToGlobalTransform(tgContext).linear() *
                Acts::makeDirectionFromAxisTangents(tanAlpha, tanBeta);

            return std::make_pair(projSegPos, newDir);
        };
        return nSegments == 3 
            ? estimateQtimesP(planeNorm, point(segmentsToUse[0]), point(segmentsToUse[1]), point(segmentsToUse[2]), magField)
            : estimateQtimesP(planeNorm, point(segmentsToUse[0]), point(segmentsToUse[1]), magField);
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
            if (segment->nPhiLayers() > 0 &&  projSector != ExpandedSector{segment->position().phi()}) {
                ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Segment @"<<Amg::toString(segment->position())
                    <<" is not in sector "<<projSector);
                continue;
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
            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Add segment "<<::print(*segment)
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
        for (const auto& [coords, seedCandidate] : orderedSegs) {
            /** Bad segment not suitable for track seeding or the segment coordinates are
             *  just mirrored at the overlap between sector 1 -> 16 */
             if (!m_segSelector->passSeedingQuality(ctx, *seedCandidate)){
                ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Segment "<<::print(*seedCandidate)<<" does not pass the seeding quality.");
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
            /** Using the cube above, let the tree search for all compatible segments */
            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Search for compatible segments to "<<::print(*seedCandidate)<<".");
            orderedSegs.rangeSearchMapDiscard(selectRange, [&](
                    const SearchTree_t::coordinate_t& /*coords*/,
                    const xAOD::MuonSegment* extendWithMe) {
                        /** Ensure that the sector overlap and momentum vectors are compatible with a MS trajectory */
                        if (!m_segSelector->compatibleForTrack(ctx, *seedCandidate, *extendWithMe)) {
                            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Segment "<<::print(*extendWithMe)<<" is not compatible.");
                            return;
                        }
                        auto itr = std::ranges::find_if(newSeed.segments(), [extendWithMe](const xAOD::MuonSegment* onSeed){
                            return extendWithMe->chamberIndex() == onSeed->chamberIndex();
                        });
                        if (itr == newSeed.segments().end()){
                            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Add segment "<<::print(*extendWithMe)<<" to seed.");
                            newSeed.addSegment(extendWithMe);
                        }
                        else if (reducedChi2(**itr) > reducedChi2(*extendWithMe) &&
                                     (*itr)->nPhiLayers() <= extendWithMe->nPhiLayers()) {

                             ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Replace segment "<<::print(**itr)<<" with "
                                             <<::print(*extendWithMe)<<" on seed due to better chi2.");
                            newSeed.replaceSegment(*itr, extendWithMe);
                        }
            });
            /** No segments were combined */
            if (newSeed.segments().empty()) {
                continue;
            }

            newSeed.addSegment(seedCandidate);

            // Let's check if we build a single station seed and if yes reject it.
            using namespace Muon::MuonStationIndex;
            if(toLayerIndex(newSeed.segments().front()->chamberIndex()) == toLayerIndex(newSeed.segments().back()->chamberIndex())){
                ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Reject seed with segments in the same station.");
                continue;
            }

            //Check if we have multiple segments from the same station, if so split the seed and create duplicate seeds

            /** Calculate the seed's position */
            const double r = newSeed.location() == Location::Barrel ? 1.*m_barrelRadius 
                                                                    : coords[Acts::toUnderlying(ePosOnCylinder)];
            const double z = newSeed.location() == Location::Barrel ? coords[Acts::toUnderlying(ePosOnCylinder)] 
                                                                    : coords[Acts::toUnderlying(eDetSection)]* m_endcapDiscZ;

            Amg::Vector3D pos = r * newSeed.sector().radialDir()
                              + z * Amg::Vector3D::UnitZ();
            
            newSeed.setPosition(std::move(pos));
            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Add new seed "<<newSeed);
            trackSeeds.emplace_back(std::move(newSeed));
        }
        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Found in total "<<trackSeeds.size()<<" before overlap removal");
        // outputSeeds
        trackSeeds = resolveOverlaps(std::move(trackSeeds));
        outputSeeds.insert(outputSeeds.end(), std::make_move_iterator(trackSeeds.begin()),
                                              std::make_move_iterator(trackSeeds.end()));
        return StatusCode::SUCCESS;
    }
    MsTrackSeedContainer
        MsTrackSeederTool::resolveOverlaps(MsTrackSeedContainer&& unresolved) const {

        /** Resort the seeds starting from the ones with the most segments to the lowest  */
        std::ranges::sort(unresolved, [](const MsTrackSeed& a, const MsTrackSeed&b) {
            return a.segments().size() > b.segments().size();
        });
        MsTrackSeedContainer outputSeeds{};
        outputSeeds.reserve(unresolved.size());
        std::ranges::copy_if(std::move(unresolved), std::back_inserter(outputSeeds),
            [&outputSeeds](const MsTrackSeed& testMe) {
                for (const MsTrackSeed&  good : outputSeeds){
                    if (!testMe.sector().isNeighbour(good.sector())) {
                        continue;
                    }
                    const std::size_t sharedSegs = std::ranges::count_if(testMe.segments(),
                                                                      [&good](const xAOD::MuonSegment* segInTest){
                                                                          return Acts::rangeContainsValue(good.segments(), segInTest);
                                                                      });
                    if (sharedSegs == testMe.segments().size()) {
                        return false;
                    }
                }
                return true;
            });

        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Found in total "<<outputSeeds.size()<<" after overlap removal");
        return outputSeeds;
    } 
    const MuonGMR4::SpectrometerSector* 
        MsTrackSeederTool::envelope(const xAOD::MuonSegment& segment) const{
        return m_detMgr->getSectorEnvelope(segment.chamberIndex(), 
                                           segment.sector(), 
                                           segment.etaIndex());
    }
}
