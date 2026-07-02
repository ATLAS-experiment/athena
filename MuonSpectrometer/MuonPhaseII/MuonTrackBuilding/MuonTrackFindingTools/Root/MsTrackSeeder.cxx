/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonTrackFindingTools/MsTrackSeeder.h"
#include "MuonTrackEvent/TrackingHelpers.h"
#include "MuonTrackEvent/Circle.h"
#include "MuonTruthHelpers/MuonSimHitHelpers.h"

#include "Acts/Utilities/Helpers.hpp"
#include "Acts/Definitions/Tolerance.hpp"
#include "Acts/Surfaces/detail/LineHelper.hpp"
#include "Acts/Definitions/Units.hpp"

#include "GaudiKernel/PhysicalConstants.h"
namespace {
    constexpr double straightlineMom = 5.*Gaudi::Units::TeV;

    using namespace Acts::UnitLiterals;
    constexpr double inDeg(const double a) {
        return a / 1._degree;
    }
    /** @brief Average position */
    void average(const Amg::Vector3D& segPos, std::optional<Amg::Vector3D>& posSlot) {
        if (!posSlot) {
            posSlot = segPos;
        } else {
            *posSlot = 0.5 * (*posSlot + segPos);
        }
    }
    /** @brief return the segment theta */
    inline Amg::Vector3D dirForBField(const xAOD::MuonSegment& seg,
                                      const double circPhi) {
        //if (seg.nPhiLayers() > 0) {
        //    return seg.direction();
        //}
        const double theta = std::atan2(Acts::fastHypot(seg.px(), seg.py()), seg.pz());
        return Acts::makeDirectionFromPhiTheta(circPhi, theta);
    }
    std::string print(const Amg::Vector3D& v){
        return std::format("r: {:.2f}, z: {:.2f}, phi: {:.2f}, theta: {:.2f}",
                            v.perp(), v.z(), inDeg((v.phi())), inDeg(v.theta()));
    }

    float reducedChi2(const xAOD::MuonSegment& seg) {
        return seg.chiSquared() / std::max(1.f, seg.numberDoF());
    }
    std::string print(const xAOD::MuonSegment& seg) {
        return std::format("{:}, nPrecHits: {:}, nPhiHits: {:}", MuonR4::printID(seg),
                           seg.nPrecisionHits(), seg.nPhiLayers());
    }
}

namespace MuonR4{
    using SearchTree_t = MsTrackSeeder::SearchTree_t;
    MsTrackSeeder::MsTrackSeeder(const std::string& msgName, Config&& cfg):
        AthMessaging{msgName},
        m_cfg{std::move(cfg)}{
        auto& v{m_cfg.fieldExtpSteps};
        v.insert(0.); v.insert(1.);
        if (std::ranges::any_of(v, [](const double x){
            return x < 0. || x > 1.;
        })) {
            THROW_EXCEPTION("Found invalid extrapolation steps.");
        }
    }
    const MuonGMR4::SpectrometerSector* 
        MsTrackSeeder::envelope(const xAOD::MuonSegment& segment) const{
        return m_cfg.detMgr->getSectorEnvelope(segment.chamberIndex(), 
                                               segment.sector(), 
                                               segment.etaIndex());
    }
    Amg::Vector3D MsTrackSeeder::projectOntoPhiPlane(const ActsTrk::GeometryContext& gctx, 
                                                     const xAOD::MuonSegment& segment,
                                                     const double projectPhi) const {
        using enum SectorProjector;
        const Amg::Vector3D segPos3D{segment.position()};
        /// Recall that the sector coordinate system is defined such that the x-axis 
        /// is aligned with the nominal wire direction
        const Amg::Vector3D dirAlongTube{envelope(segment)->localToGlobalTransform(gctx).linear().col(0)};
        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Project onto phi: "<<inDeg(projectPhi));
        const Amg::Vector3D radialDir = Acts::makeDirectionFromPhiTheta(projectPhi, 90._degree);
        using namespace Acts::detail::LineHelper;
        /// Calculate the proper intersection point
        const auto isect = lineIntersect<3>(segPos3D.z()*Amg::Vector3D::UnitZ(), radialDir,
                                            segPos3D, dirAlongTube);
        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Projected "<<printID(segment) 
            <<" segment in @"<<Amg::toString(segPos3D)<<" + "
            <<Amg::toString(segment.direction())<<", chi2: "<<(segment.chiSquared() / std::max(1.f, segment.numberDoF()))
            <<", nDoF: "<<segment.numberDoF()<<" --> "<<Amg::toString(isect.position()));
        return isect.position();
    }
    Amg::Vector2D MsTrackSeeder::expressOnCylinder(const ActsTrk::GeometryContext& gctx,
                                                   const xAOD::MuonSegment& segment,
                                                   const Location loc,
                                                   const ExpandedSector sector) const {
        /// extrapolated position
        const Amg::Vector3D pos{projectOntoPhiPlane(gctx, segment, sector.phi())};
        const Amg::Vector3D dir{segment.direction()};
 
        const Amg::Vector2D projPos{pos.perp(), pos.z()};
        const Amg::Vector2D projDir{dir.perp(), dir.z()};

        ATH_MSG_VERBOSE( "segment position:" << Amg::toString(segment.position())<< ", direction: " << Amg::toString(segment.direction()) );
        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Express segment in @"<<Amg::toString(pos)
                        <<", direction: "<<Amg::toString(dir)<< " sector projector: " << sector 
                        << " location: " << Acts::toUnderlying(loc));
        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Projected position onto sector: "<<Amg::toString(projPos)
                        <<", projected direction: "<<Amg::toString(projDir));
 
        double lambda{0.};
        if (Location::Barrel == loc) {
            lambda = Amg::intersect<2>(projPos, projDir, Amg::Vector2D::UnitX(), 
                                        m_cfg.barrelRadius).value_or(10. * Gaudi::Units::km);
                                        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Intersect with barrel at radius: "<<m_cfg.barrelRadius<<" --> "<<Amg::toString(projPos + lambda * projDir));
        } else {
            lambda = Amg::intersect<2>(projPos, projDir, Amg::Vector2D::UnitY(), 
                                       Acts::copySign(m_cfg.endcapDiscZ, projPos[1])).value_or(10. * Gaudi::Units::km);
                                       ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Intersect with endcap at z: "<<Acts::copySign(m_cfg.endcapDiscZ, projPos[1])<<" --> "<<Amg::toString(projPos + lambda * projDir));
        }
        return projPos + lambda * projDir;  
    }
    bool MsTrackSeeder::withinBounds(const Amg::Vector2D& projPos,
                                     const Location loc) const {
        using enum Location;
        if (loc == Barrel && std::abs(projPos[1]) > std::min(m_cfg.endcapDiscZ, m_cfg.barrelLength)) {
            ATH_MSG_VERBOSE(__func__<<"()  "<<__LINE__<<" - Position "<<Amg::toString(projPos)<<
                            " exceeds cylinder boundaries ("<<m_cfg.barrelRadius<<", "
                            <<std::min(m_cfg.endcapDiscZ, m_cfg.barrelLength)<<")");
            return false;
        } else if (loc == Endcap && (0 > projPos[0] || projPos[0] > m_cfg.endcapDiscRadius)) {
            ATH_MSG_VERBOSE(__func__<<"()  "<<__LINE__<<" - Position "<<Amg::toString(projPos)<<
            " exceeds endcap boundaries ("<<m_cfg.endcapDiscRadius<<", "<<Acts::copySign(m_cfg.endcapDiscZ, projPos[1])<<")");
            return false;
        }
        return true;
    }
    std::optional<double> MsTrackSeeder::calculateRadius(VecOpt_t&& pI, VecOpt_t&& pM, VecOpt_t&& pO,
                                                         const Amg::Vector3D& planeNorm) const {
        if (!pI || !pM || !pO) {
            return std::nullopt;
        }
        // Circle momCirc{*pI, *pM, *pO};
        // return momCirc.radius()* std::copysign(1., planeNorm.dot(momCirc.normal()));

        const Amg::Vector3D leverL = (*pO) - (*pI);

        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Construct sagitta from "
                <<"\n --- Inner:  "<<print(*pI) <<"\n --- Middle: "<<print(*pM)
                <<"\n --- Outer:  "<<print(*pO));
        if (std::abs(planeNorm.dot(leverL)) > Acts::s_onSurfaceTolerance || 
            std::abs(planeNorm.dot( (*pM) - (*pI))) > Acts::s_onSurfaceTolerance) {
            THROW_EXCEPTION("The lever arm is in the bending plane: "<<Amg::toString(planeNorm)
                        <<", "<<Amg::toString(leverL.unit())<<", "<<Amg::toString( ((*pM) - (*pI)).unit()));
        }
        const Amg::Vector3D sagittaDir = leverL.cross(planeNorm).unit();
        std::optional<double> sagitta = Amg::intersect<3>(*pI, leverL.unit(), *pM, sagittaDir);
        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Estimated sagitta: "<<(sagitta ? 
                     std::to_string(sagitta.value_or(0.)) : "---")<<", lever arm: "
                    <<(leverL.mag() / Gaudi::Units::m)<<" [m]"  );
        if (!sagitta) {
            return std::nullopt;
        }
        /// Derive the radius from the two equations. The line connecting pI and pO divides up
        /// the radius in the sagitta and the complementary section H
        ///  R = S + H
        /// Also H is the catheter of the triangle <R, L / 2, H> 
        ///         R^{2} = L^{2} / 4 + H^{2}
        /// --->    R^{2} = L^{2} / 4 + (R-S)^{2} 
        /// --->    2*S*R = L^{2} / 4 + S^{2} (Approximating S^{2} = 0)
        /// --->    R = L^{2} / (8 * s)
        return leverL.dot(leverL) / (8. * (*sagitta));
    }

    double MsTrackSeeder::estimateTwoStationP(const xAOD::MuonSegment& segment1,
                                              const xAOD::MuonSegment& segment2,
                                              const AtlasFieldCacheCondObj& magField) const {
        
        MagField::AtlasFieldCache fieldCache{};
        magField.getInitializedCache(fieldCache); 

        Amg::Vector3D pos1 = segment1.position();
        Amg::Vector3D pos2 = segment2.position();
        const double phi = segment1.nPhiLayers() > segment2.nPhiLayers() ? 
                           pos1.phi() : pos2.phi();
        const Amg::Vector3D norm = Acts::makeDirectionFromPhiTheta(phi + 90._degree, 90._degree);
        auto projector = [&norm](const Amg::Vector3D& d) -> Amg::Vector3D {
            return d - d.dot(norm) * norm;
        };

        pos1 = projector(pos1);
        pos2 = projector(pos2);

        const Amg::Vector3D e1 = (pos2-pos1).unit();
        const Amg::Vector3D e2 = norm.cross(e1);

        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - "
                     <<Amg::toString(e1)<<" |e1|="<<e1.mag()<<", "<<Amg::toString(e2)
                     <<" |e2|="<<e2.mag()<<", "<<Amg::toString(norm));
        Amg::Vector3D dir1 = projector(segment1.direction());
        Amg::Vector3D dir2 = projector(segment2.direction());

        Amg::Vector3D locField{Amg::Vector3D::Zero()};
        double avgForce{0.};
        for (double fieldStep : m_cfg.fieldExtpSteps) {
            const Amg::Vector3D extPos =  (1.-fieldStep) * pos1 + fieldStep * pos2;
            const Amg::Vector3D extDir =  ((1.-fieldStep) * dir1 + fieldStep * dir2).unit();
            fieldCache.getField(extPos.data(), locField.data());
            const Amg::Vector3D force = locField.cross(extDir);
           
            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - sample point: "<<Amg::toString(extPos)
                <<", sample dir: "<<Amg::toString(extDir)<<" --> B-field: "
            <<Amg::toString(locField * 1.e3)<<", direction: "<<
             std::copysign(1., force.dot(e2))<<", force: "<<force.mag()*1.e3);
            avgForce += force.mag();
        }
        avgForce /= m_cfg.fieldExtpSteps.size();
        const double alpha = std::copysign(Amg::angle(dir1, dir2), (dir2-dir1).dot(e2));
        const double P = avgForce * (pos1-pos2).mag() / 
                        (std::abs(std::sin(pos1.theta())) * 
                        (std::abs(alpha) > Acts::s_epsilon ? alpha : Acts::s_epsilon) );
        
        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Deflection angle between\n"
            <<" *** "<<print(segment1)<<"\n"<<" *** "<<print(segment2)<<"\n"
            <<" is "<<inDeg(alpha)<<" degree. Field projection: "<<avgForce*1.E3
            <<", momentum: "<<P<<"\n"
            <<", angle 1: "<<inDeg(Amg::angle(pos1, dir1))
            <<", angle 2: "<<inDeg(Amg::angle(pos2, dir2)));
        return P;
    }
    inline std::pair<double, unsigned> MsTrackSeeder::averageBField(const PosMomPair_t& start,
                                                                    const PosMomPair_t& end,
                                                                    const AtlasFieldCacheCondObj& magField,
                                                                    const bool skipFirst) const {
        MagField::AtlasFieldCache fieldCache{};
        magField.getInitializedCache(fieldCache); 

        double avgBField{0.};
        unsigned nBFieldPoints{0};
        Amg::Vector3D locField{Amg::Vector3D::Zero()};
        for (double fieldStep : m_cfg.fieldExtpSteps) {
            /// The first field step may be skipped if it's already accumulated in a 
            /// previous averaging where the start of this call is the end 
            /// of the previous call
            if (fieldStep == 0. && skipFirst) {
                continue;
            }
            const Amg::Vector3D extPos =  (1. -fieldStep) * start.first + fieldStep * end.first;
            const Amg::Vector3D extDir = ((1. -fieldStep) * start.second + fieldStep * end.second).unit();
                  
            fieldCache.getField(extPos.data(), locField.data());
            const double localB = extDir.cross(locField).mag();

            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - step: "<<fieldStep
                <<", position: "<<Amg::toString(extPos)<<", dir: "<<Amg::toString(extDir)
                <<" --> localB: "<<(localB * 1000.)<<" T."
                <<", B: "<<Amg::toString(locField* 1000.)
                <<", PxB:"<<Amg::toString(extDir.cross(locField).unit()));
                avgBField += localB;
                ++nBFieldPoints;
        }
        return std::make_pair(avgBField, nBFieldPoints);
    }
    double MsTrackSeeder::estimateQtimesP(const ActsTrk::GeometryContext& gctx,
                                          const AtlasFieldCacheCondObj& magField,
                                          const MsTrackSeed& seed) const {
        
        MagField::AtlasFieldCache fieldCache{};
        magField.getInitializedCache(fieldCache); 
        
        /** Calculate the averaged phi from the segments */
        double circPhi{0.};
        unsigned nSegsWithPhi{0};
        for (const xAOD::MuonSegment* segment : seed.segments()) {
            if (segment->nPhiLayers() > 0) {
                circPhi += segment->position().phi();
                ++nSegsWithPhi;
            }
        }
        if (!nSegsWithPhi){
            circPhi = seed.sector().phi();
        } else {
            circPhi /=nSegsWithPhi;
        }
        auto layerPos{Acts::filledArray<VecOpt_t, 6>(std::nullopt)};
        double avgBField{0.}, avgTheta{0.};

        const Amg::Vector3D planeNorm = Acts::makeDirectionFromPhiTheta(circPhi+ 90._degree, 90._degree);

        {
            Amg::Vector3D projPos = projectOntoPhiPlane(gctx, *seed.segments()[0], circPhi);
            Amg::Vector3D projDir = dirForBField(*seed.segments()[0], circPhi); 
            Amg::Vector3D locField{Amg::Vector3D::Zero()};
            unsigned nBFieldPoints{0};
            const unsigned nSeedSeg = seed.segments().size();
            using namespace Muon::MuonStationIndex;
            for (unsigned int s = 0; s < nSeedSeg; ++s) {
                avgTheta += projDir.theta();
                /** Calculate the sagitta points */
                const xAOD::MuonSegment& segment{*seed.segments()[s]};
                switch (toStationIndex(segment.chamberIndex())) {
                    using enum StIndex;
                    case BI:
                    case BE:{
                        average(projPos, layerPos[0]);
                        break;
                    } case BM: {
                        average(projPos, layerPos[1]);
                        break;
                    } case BO: {
                        average(projPos, layerPos[2]);
                        break;
                    } case EI:
                      case EE: {
                        average(projPos, layerPos[3]);
                        break;
                    } case EM : {
                        average(projPos, layerPos[4]);
                        break;
                    } case EO : {
                        average(projPos, layerPos[5]);
                        break;
                    } default: {
                        break;
                    }
                }
                if (s + 1 == nSeedSeg) {
                    break;
                }
                /** Sample the magnetic field */
                Amg::Vector3D nextDir = dirForBField(*seed.segments()[s+1], circPhi);
                Amg::Vector3D nextPos = projectOntoPhiPlane(gctx, *seed.segments()[s+1], circPhi);
                const auto [bFieldSeg, nPointsSeg] =  averageBField(std::make_pair(projPos, projDir),
                                                                    std::make_pair(nextPos, nextDir),
                                                                    magField, s>0);
                
                avgBField += bFieldSeg;
                nBFieldPoints += nPointsSeg;
                projPos = std::move(nextPos);
                projDir = std::move(nextDir);
            }
            avgBField /= std::max(nBFieldPoints, 1u);
            ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - averaged field: "<<(avgBField * 1000.)
                        <<" T, number of points: "<<nBFieldPoints<<".");
        }
        avgTheta /= seed.segments().size();
        /// Calculate the radius from the barrel segments
        std::optional<double> barrelR = calculateRadius(std::move(layerPos[0]), 
                                                        std::move(layerPos[1]), 
                                                        std::move(layerPos[2]), planeNorm);
        /// Calculate the radius from the endcap segments
        std::optional<double> endcapR = calculateRadius(std::move(layerPos[3]), 
                                                        std::move(layerPos[4]), 
                                                        std::move(layerPos[5]), planeNorm);
        /// If no radius could be calculated return the straight line estimator
        if (!barrelR && !endcapR) {
            /** Fall back via the angular estimator */
            const xAOD::MuonSegment* frontSeg = seed.segments().front();
            const xAOD::MuonSegment* backSeg = seed.segments().back();
            using namespace Muon::MuonStationIndex;
            /// Magnetic field is too weak for the EM / EO case -> return straight line
            if (toStationIndex(frontSeg->chamberIndex()) == StIndex::EM && 
                toStationIndex(backSeg->chamberIndex()) == StIndex::EO) {
                return straightlineMom;
            }
            /// Calculate the momentum via the deflection angle
            const Amg::Vector3D innerDir = dirForBField(*frontSeg, circPhi);
            const Amg::Vector3D innerPos = projectOntoPhiPlane(gctx, *frontSeg, circPhi);

            const Amg::Vector3D outerDir = dirForBField(*backSeg, circPhi);
            const Amg::Vector3D outerPos = projectOntoPhiPlane(gctx, *backSeg, circPhi);

            const double alpha = outerDir.theta() - innerDir.theta();
            const double P = avgBField * (outerPos-innerPos).mag() / (
                (std::abs(alpha) > Acts::s_epsilon ? alpha : Acts::s_epsilon) * 
                std::abs(std::sin(avgTheta)));
            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Angular difference between "
                <<printID(*frontSeg)<<" & "<<printID(*backSeg)<<" is "<<inDeg(alpha)
                <<" -> estimated momentum: "<<P);
            return P;
        }
        const double r = 0.5* ((barrelR ? *barrelR : *endcapR) +
                               (endcapR ? *endcapR : *barrelR));
        ///
        const double P = 0.3* Gaudi::Units::GeV* avgBField * r / std::abs(std::sin(avgTheta)); 

        ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Estimated radius "<<r / Gaudi::Units::m<<" [m] --> P: "<<
                        (P / Gaudi::Units::GeV)<<" [GeV]");        

        return P;
    }
    void MsTrackSeeder::appendSegment(const ActsTrk::GeometryContext& gctx,
                                      const xAOD::MuonSegment* segment,
                                      const Location loc,
                                      TreeRawVec_t& outContainer) const {
        
        const unsigned segSector = segment->sector();
        for (const auto proj : {SectorProjector::leftOverlap, SectorProjector::center, SectorProjector::rightOverlap}) {
            /// Check whether the segment belongs to the left or right sector as well
            const ExpandedSector projSector{segSector, proj};
            if (segment->nPhiLayers() > 0 &&  projSector != ExpandedSector{segment->position().phi()}) {
                ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Segment @"<<Amg::toString(segment->position())
                    <<" is not in sector "<<projSector);
                continue;
            }
            const Amg::Vector2D refPoint{expressOnCylinder(gctx, *segment, loc, projSector)};
            if (!withinBounds(refPoint, loc)) {
                 continue;
            }
            using enum SeedCoords;
            std::array<double, 3> coords{};
            /** Blow-up the number of sectors by a factor of 2. The even numbers represent the 
             *  segments expressed @ the sector centre. The odd numbers represent the overlap region
             *  between two adjacent sectors. For sector 16, the right overlap region is mapped to 1 */
            coords[Acts::toUnderlying(eSector)] = projSector.sector();
            /** Enumeration to indicate whether the segment is expressed on the negative endcap (-1),
             *  the barrel (0) or the positive endcap */
            coords[Acts::toUnderlying(eDetSection)] =  Acts::copySign(Acts::toUnderlying(loc), refPoint[1]);
            /** Coordinate on the cylinder */
            coords[Acts::toUnderlying(ePosOnCylinder)] = refPoint[Location::Barrel == loc];
            ATH_MSG_VERBOSE("Add segment "<<print(*segment)<<", seed quality: "
            <<m_cfg.selector->passSeedingQuality(Gaudi::Hive::currentContext(), *detailedSegment(*segment))
            <<" with "<<coords<<" to the search tree");
            outContainer.emplace_back(std::move(coords), segment);
        }
    }
    SearchTree_t MsTrackSeeder::constructTree(const ActsTrk::GeometryContext& gctx,
                                              const xAOD::MuonSegmentContainer& segments) const{
        TreeRawVec_t rawData{};
        rawData.reserve(3*segments.size());
        for (const xAOD::MuonSegment* segment : segments){
            appendSegment(gctx, segment, Location::Barrel, rawData);
            appendSegment(gctx, segment, Location::Endcap, rawData);
        }
        ATH_MSG_VERBOSE("Create a new tree with "<<rawData.size()<<" entries. ");
        return SearchTree_t{std::move(rawData)};
    }
    std::unique_ptr<MsTrackSeedContainer> MsTrackSeeder::findTrackSeeds(const EventContext& ctx,
                                                                        const ActsTrk::GeometryContext& gctx,
                                                                        const xAOD::MuonSegmentContainer& segments) const {
        SearchTree_t orderedSegs{constructTree(gctx, segments)};
        MsTrackSeedContainer trackSeeds{};
        using enum SeedCoords;
        for (const auto& [coords, seedCandidate] : orderedSegs) {
            /** Bad segment not suitable for track seeding or the segment coordinates are
             *  just mirrored at the overlap between sector 1 -> 16 */
            const Segment* recoSeedCandidate = detailedSegment(*seedCandidate);
             if (!m_cfg.selector->passSeedingQuality(ctx, *recoSeedCandidate)){
                ATH_MSG_VERBOSE("Segment "<<print(*seedCandidate)<<" does not pass the seeding quality.");
                continue;
            }
            /** Define the search range. */    
            SearchTree_t::range_t selectRange{};
            /** Ensure that only endcap / barrel seeds are considered. 
             *  The values are integers -> add tiny margin */
            selectRange[Acts::toUnderlying(eDetSection)].shrink(coords[Acts::toUnderlying(eDetSection)] - 0.1, 
                                                                coords[Acts::toUnderlying(eDetSection)] + 0.1);
            /** Move 25 cm along the projected plane */
            selectRange[Acts::toUnderlying(ePosOnCylinder)].shrink(coords[Acts::toUnderlying(ePosOnCylinder)] - m_cfg.seedHalfLength, 
                                                                   coords[Acts::toUnderlying(ePosOnCylinder)] + m_cfg.seedHalfLength);
            /** Include the neighbouring sectors */
            selectRange[Acts::toUnderlying(eSector)].shrink(coords[Acts::toUnderlying(eSector)] -0.25, 
                                                            coords[Acts::toUnderlying(eSector)] +0.25);
            
            MsTrackSeed newSeed{static_cast<Location>(std::abs(coords[Acts::toUnderlying(eDetSection)])),
                                ExpandedSector{static_cast<std::int8_t>(coords[Acts::toUnderlying(eSector)])}};
            /** Using the cube above, let the tree search for all compatible segments */
            ATH_MSG_VERBOSE("Search for compatible segments to "<<print(*seedCandidate)<<".");
            orderedSegs.rangeSearchMapDiscard(selectRange, [&](
                    const SearchTree_t::coordinate_t& /*coords*/,
                    const xAOD::MuonSegment* extendWithMe) {
                        /** Ensure that the sector overlap and momentum vectors are compatible with a MS trajectory */
                        const Segment* extendCandidate = detailedSegment(*extendWithMe);
                        if (!m_cfg.selector->compatibleForTrack(ctx, *recoSeedCandidate, *extendCandidate)) {
                            ATH_MSG_VERBOSE("Segment "<<print(*extendWithMe)<<" is not compatible.");
                            return;
                        }
                        auto itr = std::ranges::find_if(newSeed.segments(), [extendWithMe](const xAOD::MuonSegment* onSeed){
                            return extendWithMe->chamberIndex() == onSeed->chamberIndex();
                        });
                        if (itr == newSeed.segments().end()){
                            ATH_MSG_VERBOSE("Add segment "<<print(*extendWithMe)<<" to seed.");
                            newSeed.addSegment(extendWithMe);
                        }
                        else if (reducedChi2(**itr) > reducedChi2(*extendWithMe) &&
                                     (*itr)->nPhiLayers() <= extendWithMe->nPhiLayers()) {

                             ATH_MSG_VERBOSE("Replace segment "<<print(**itr)<<" with "<<print(*extendWithMe)
                                             <<" on seed due to better chi2.");
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
                ATH_MSG_VERBOSE("Reject seed with segments in the same station.");
                continue;
            }

            //Check if we have multiple segments from the same station, if so split the seed and create duplicate seeds

            /** Calculate the seed's position */
            const double r = newSeed.location() == Location::Barrel ? m_cfg.barrelRadius 
                                                                    : coords[Acts::toUnderlying(ePosOnCylinder)];
            const double z = newSeed.location() == Location::Barrel ? coords[Acts::toUnderlying(ePosOnCylinder)] 
                                                                    : coords[Acts::toUnderlying(eDetSection)]* m_cfg.endcapDiscZ;

            Amg::Vector3D pos = r * newSeed.sector().radialDir()
                              + z * Amg::Vector3D::UnitZ();
            
            newSeed.setPosition(std::move(pos));
            ATH_MSG_VERBOSE("Add new seed "<<newSeed);
            trackSeeds.emplace_back(std::move(newSeed));
        }
        ATH_MSG_VERBOSE("Found in total "<<trackSeeds.size()<<" before overlap removal");
        return resolveOverlaps(std::move(trackSeeds));
    }
    std::unique_ptr<MsTrackSeedContainer> 
        MsTrackSeeder::resolveOverlaps(MsTrackSeedContainer&& unresolved) const {

        /** Resort the seeds starting from the ones with the most segments to the lowest  */
        std::ranges::sort(unresolved, [](const MsTrackSeed& a, const MsTrackSeed&b) {
            return a.segments().size() > b.segments().size();
        });
        auto outputSeeds = std::make_unique<MsTrackSeedContainer>();
        outputSeeds->reserve(unresolved.size());
        std::ranges::copy_if(std::move(unresolved), std::back_inserter(*outputSeeds),
            [&outputSeeds](const MsTrackSeed& testMe) {
                for (const MsTrackSeed&  good : *outputSeeds){
                    if (!testMe.sector().isNeighbour(good.sector())) {
                        continue;
                    }
                    const std::size_t sharedSegs = std::ranges::count_if(testMe.segments(),
                                                                      [&good](const xAOD::MuonSegment* segInTest){
                                                                          return std::ranges::find(good.segments(), segInTest) != 
                                                                                 good.segments().end();
                                                                      });
                    if (sharedSegs == testMe.segments().size()) {
                        return false;
                    }
                }
                return true;
            });

        ATH_MSG_VERBOSE("Found in total "<<outputSeeds->size()<<" after overlap removal");
        return outputSeeds;
    } 
}
