/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonTrackFindingTools/MsTrackSeeder.h"
#include "MuonPatternHelpers/MatrixUtils.h"
#include "MuonTrackEvent/TrackingHelpers.h"
#include "MuonDetDescrUtils/MuonSectorMapping.h"
#include "MuonTruthHelpers/MuonSimHitHelpers.h"

#include "Acts/Utilities/Helpers.hpp"
#include "Acts/Surfaces/detail/LineHelper.hpp"
#include "Acts/Definitions/Units.hpp"
#include "GaudiKernel/PhysicalConstants.h"
namespace {
    using namespace Acts::UnitLiterals;
    constexpr double inDeg(const double a) {
        return a / 1._degree;
    }
    /** @brief Ensure that the parsed sector number is following the MS sector schema
     *         0 is mapped to 16 and 17 is mapped to 1.
     *  @param sector: Calculated sector number */
    constexpr int ringSector(const int sector) {
        constexpr int nSectors = Muon::MuonStationIndex::numberOfSectors();
        return sector == 0 ?  nSectors : (sector > nSectors ? 1 : sector); 
    }
    /** @brief Maps the sector 33 -> 0 to close the extended MS symmetry ring  */
    constexpr int ringOverlap(const int sector) {
        constexpr int nSectors = 2*Muon::MuonStationIndex::numberOfSectors();
        return  sector > nSectors ? 1 : sector;
    }
    /** @brief Average position */
    void average(const Amg::Vector3D& segPos, std::optional<Amg::Vector3D>& posSlot) {
        if (!posSlot) {
            posSlot = segPos;
        } else {
            posSlot = 0.5 * ((*posSlot) + segPos); 
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
    static const Muon::MuonSectorMapping sectorMap{};
}

namespace MuonR4{
    using SearchTree_t = MsTrackSeeder::SearchTree_t;
    using SectorProjector = MsTrackSeeder::SectorProjector;
    std::string to_string(const SectorProjector proj){
        using enum SectorProjector;
        switch (proj) {
            case leftOverlap: 
                return "overlap with left sector";
            case center:
                return "sector center";
            case rightOverlap:
                return "overlap with right sector";
            default:
                return "";
        }
    }

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


    double MsTrackSeeder::projectedPhi(const int sector,
                                       const SectorProjector proj) {
        return sectorMap.sectorOverlapPhi(sector, ringSector(sector + Acts::toUnderlying(proj)));
    }
    inline const MuonGMR4::SpectrometerSector* 
        MsTrackSeeder::envelope(const xAOD::MuonSegment& segment) const{
        return m_cfg.detMgr->getSectorEnvelope(segment.chamberIndex(), 
                                               segment.sector(), 
                                               segment.etaIndex());
    }
    MsTrackSeeder::SectorProjector 
        MsTrackSeeder::projectorFromSeed(const xAOD::MuonSegment& seg,
                                         const MsTrackSeed& refSeed) {
        
        int doubSector = 2 * seg.sector();
        constexpr int nSectors = 2*Muon::MuonStationIndex::numberOfSectors();
        if (refSeed.sector() == nSectors && doubSector ==2) {
            return SectorProjector::leftOverlap;
        } else if (refSeed.sector() == 1 && doubSector == nSectors) {
            return SectorProjector::rightOverlap;
        }
        return static_cast<SectorProjector>(refSeed.sector() - doubSector );
    }
    Amg::Vector3D MsTrackSeeder::projectOntoSector(const ActsTrk::GeometryContext& gctx, 
                                                   const xAOD::MuonSegment& segment,
                                                   const MsTrackSeed& seed) const {
       return projectOntoSector(gctx, segment, projectorFromSeed(segment, seed));
    }
    Amg::Vector3D MsTrackSeeder::projectOntoSector(const ActsTrk::GeometryContext& gctx, 
                                                   const xAOD::MuonSegment& segment,
                                                   const SectorProjector proj) const {
        /// Fetch the phi onto which we want to project the segment without leaving the wire.
        const double sectorPhi{projectedPhi(segment.sector(), proj)};
        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Project onto "<<to_string(proj)<<" in "
                        <<envelope(segment)->identString());
        return projectOntoPhiPlane(gctx, segment, sectorPhi);
    }
    Amg::Vector3D MsTrackSeeder::projectOntoPhiPlane(const ActsTrk::GeometryContext& gctx, 
                                                     const xAOD::MuonSegment& segment,
                                                     const double projectPhi) const {
        using enum SectorProjector;
        const Amg::Vector3D segPos3D{segment.position()};
        /// Recall that the sector coordinate system is defined such that the x-axis 
        /// is aligned with the nominal wire direction
        const Amg::Vector3D dirAlongTube{envelope(segment)->localToGlobalTrans(gctx).linear().col(0)};
        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Project onto phi: "<<inDeg(projectPhi));
        const Amg::Vector3D radialDir = Amg::getRotateZ3D(projectPhi) * Amg::Vector3D::UnitX();
        using namespace Acts::detail::LineHelper;
        /// Calculate the proper intersection point
        const auto isect = lineIntersect<3>(segPos3D.z()*Amg::Vector3D::UnitZ(), radialDir,
                                            segPos3D, dirAlongTube);
        using namespace Muon::MuonStationIndex;
        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Projected "<<printID(segment) 
            <<" segment in @"<<Amg::toString(segPos3D)<<" + "
            <<Amg::toString(segment.direction())<<", chi2: "<<(segment.chiSquared() / std::max(1.f, segment.numberDoF()))
            <<", nDoF: "<<segment.numberDoF()<<" --> "<<Amg::toString(isect.position()));
        return isect.position();
    }
    Amg::Vector2D MsTrackSeeder::expressOnCylinder(const ActsTrk::GeometryContext& gctx,
                                                   const xAOD::MuonSegment& segment,
                                                   const Location loc,
                                                   const SectorProjector proj) const {
        /// extrapolated position
        const Amg::Vector3D pos{projectOntoSector(gctx, segment, proj)};
        const Amg::Vector3D dir{segment.direction()};
 
        const Amg::Vector2D projPos{pos.perp(), pos.z()};
        const Amg::Vector2D projDir{dir.perp(), dir.z()};
 
        double lambda{0.};
        if (Location::Barrel == loc) {
            lambda = Amg::intersect<2>(projPos, projDir, Amg::Vector2D::UnitX(), 
                                        m_cfg.barrelRadius).value_or(10. * Gaudi::Units::km);
        } else {
            lambda = Amg::intersect<2>(projPos, projDir, Amg::Vector2D::UnitY(), 
                                       sign(projPos[1])* m_cfg.endcapDiscZ).value_or(10. * Gaudi::Units::km);
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
            " exceeds endcap boundaries ("<<m_cfg.endcapDiscRadius<<", "<<(sign(projPos[1])*m_cfg.endcapDiscZ)<<")");
            return false;
        }
        return true;
    }
    std::optional<double> MsTrackSeeder::calculateRadius(VecOpt_t&& pI, VecOpt_t&& pM, VecOpt_t&& pO,
                                                         const Amg::Vector3D& planeNorm) const {
        if (!pI || !pM || !pO) {
            return std::nullopt;
        } 
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
                circPhi += std::atan2(segment->y(), segment->x());
                ++nSegsWithPhi;
            }
        }
        if (!nSegsWithPhi){
            circPhi = projectedPhi(seed.segments()[0]->sector(),
                                    projectorFromSeed(*seed.segments()[0], seed));
        } else {
            circPhi /=nSegsWithPhi;
        }
        auto layerPos{Acts::filledArray<VecOpt_t, 6>(std::nullopt)};
        double avgBField{0.}, avgTheta{0.};
        const xAOD::TruthParticle* truthMuon{nullptr};
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
                if (!truthMuon) {
                    truthMuon = getTruthMatchedParticle(segment);
                }
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

                for (double fieldStep : m_cfg.fieldExtpSteps) {
                    /* The */
                    if (fieldStep == 0. && s > 0) {
                        continue;
                    }
                    const Amg::Vector3D extPos =  (1. -fieldStep) * projPos + fieldStep * nextPos;
                    const Amg::Vector3D extDir = ((1. -fieldStep) * projDir + fieldStep * nextDir).unit();
                    // const Amg::Vector3D loc
                    fieldCache.getField(extPos.data(), locField.data());
                    const double localB = extDir.cross(locField).mag();

                    ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Segment "<<s<<", step: "<<fieldStep
                                <<", position: "<<Amg::toString(extPos)<<", dir: "<<Amg::toString(extDir)
                                <<" --> localB: "<<(localB * 1000.)<<" T."
                                <<", B: "<<Amg::toString(locField* 1000.)
                                <<", PxB:"<<Amg::toString(extDir.cross(locField).unit())
                                <<", planeNorm: "<<Amg::toString(planeNorm));
                    avgBField += localB;
                    ++nBFieldPoints;
                }
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
            return 5.*Gaudi::Units::TeV;
        } 
        const double r = 0.5* (barrelR.value_or(*endcapR) +
                               endcapR.value_or(*barrelR));
        ///
        const double P = 0.3* Gaudi::Units::GeV* avgBField * r / std::abs(std::sin(avgTheta)); 

        ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Estimated radius "<<r / Gaudi::Units::m<<" [m] --> P: "<<
                        (P / Gaudi::Units::GeV)<<" [GeV]");        

        if (truthMuon && sign(P) !=  truthMuon->charge() && truthMuon->abseta() < 2.5 && 
                (truthMuon->abseta() < 1.3 || truthMuon->abseta() > 1.4) ) {
            ATH_MSG_WARNING("Invalid charge, pT: "<<(truthMuon->pt() / Gaudi::Units::GeV)<<" [GeV], eta: "
                    <<truthMuon->eta()<<", phi: "<<(truthMuon->phi() / 1._degree)<<", q: "<<truthMuon->charge());
        }
        return P;
    }
    void MsTrackSeeder::appendSegment(const ActsTrk::GeometryContext& gctx,
                                      const xAOD::MuonSegment* segment,
                                      const Location loc,
                                      TreeRawVec_t& outContainer) const {
        
        const int segSector = segment->sector();
        for (const auto proj : {SectorProjector::leftOverlap, SectorProjector::center, SectorProjector::rightOverlap}) {
            /// Check whether the segment belongs to the left or right sector as well
            const int projSector = ringSector(segSector + Acts::toUnderlying(proj));
            if (segment->nPhiLayers() > 0 && proj != SectorProjector::center && 
                !sectorMap.insideSector(projSector, segment->position().phi())) {
                ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Segment @"<<Amg::toString(segment->position())
                    <<" is not in sector "<<projSector<<" which is "<<to_string(proj) <<" to "<<segment->sector());
                continue;
            }
            const Amg::Vector2D refPoint{expressOnCylinder(gctx, *segment, loc, proj)};
            if (!withinBounds(refPoint, loc)) {
                 continue;
            }
            using enum SeedCoords;
            std::array<double, 3> coords{};
            /** Blow-up the number of sectors by a factor of 2. The even numbers represent the 
             *  segments expressed @ the sector centre. The odd numbers represent the overlap region
             *  between two adjacent sectors. For sector 16, the right overlap region is mapped to 1 */
            const int treeSector = 2*segSector + Acts::toUnderlying(proj);
            coords[Acts::toUnderlying(eSector)] = ringOverlap(treeSector); 
            /** Enumeration to indicate whether the segment is expressed on the negative endcap (-1),
             *  the barrel (0) or the positive endcap */
            coords[Acts::toUnderlying(eDetSection)] =  Acts::toUnderlying(loc) * sign(refPoint[1]);
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
                                static_cast<int>(coords[Acts::toUnderlying(eSector)])};
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
                        } else if (reducedChi2(**itr) > reducedChi2(*extendWithMe) &&
                                     (*itr)->nPhiLayers() <= extendWithMe->nPhiLayers()) {
                            newSeed.replaceSegment(*itr, extendWithMe);
                        }
            });
            /** No segments were combined */
            if (newSeed.segments().empty()) {
                continue;
            }
            newSeed.addSegment(seedCandidate);
            /** Calculate the seed's position */
            const double r = newSeed.location() == Location::Barrel ? m_cfg.barrelRadius 
                                                                    : coords[Acts::toUnderlying(ePosOnCylinder)];
            const double z = newSeed.location() == Location::Barrel ? coords[Acts::toUnderlying(ePosOnCylinder)] 
                                                                    : coords[Acts::toUnderlying(eDetSection)]* m_cfg.endcapDiscZ;
            const int secCoord = coords[Acts::toUnderlying(eSector)];

            const double phi = sectorMap.sectorOverlapPhi( (secCoord - secCoord % 2) / 2, (secCoord + secCoord % 2) / 2 );
            Amg::Vector3D pos = r * Acts::makeDirectionFromPhiTheta(phi, 90._degree)
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
            [this, &outputSeeds](const MsTrackSeed& testMe) {
                MsTrackSeedContainer::iterator test_itr = 
                    std::ranges::find_if(*outputSeeds, [&testMe](const MsTrackSeed& good) {
                        if (ringOverlap(good.sector() - testMe.sector()) > 1) {
                            return false;
                        }
                        return std::ranges::find_if(testMe.segments(), 
                            [&good](const xAOD::MuonSegment* segInTest) {
                            return std::ranges::find(good.segments(), segInTest) != good.segments().end();
                        }) != testMe.segments().end();
                    });
                /** There is no segment which shares at least one segment with this candidate */
                if (test_itr == outputSeeds->end()) {
                    ATH_MSG_VERBOSE("Add new seed "<<testMe);
                    return true;
                }
                /// Take the longer seed
                if ( (*test_itr).segments().size() < testMe.segments().size()){
                    (*test_itr) = testMe;
                }
                return false;
            });
        return outputSeeds;
    } 
}