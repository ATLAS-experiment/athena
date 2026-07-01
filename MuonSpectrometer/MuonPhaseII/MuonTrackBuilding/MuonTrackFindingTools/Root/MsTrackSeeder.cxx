/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonTrackFindingTools/MsTrackSeeder.h"
#include "MuonTrackEvent/TrackingHelpers.h"
#include "MuonTruthHelpers/MuonSimHitHelpers.h"

#include "Acts/Utilities/Helpers.hpp"
#include "Acts/Definitions/Tolerance.hpp"
#include "Acts/Definitions/Units.hpp"

#include "FourMomUtils/P4Helpers.h"
namespace {
    using namespace Acts::UnitLiterals;
    /** @brief Check if the charges of two PtimesQ estimates agree */
    constexpr bool chargeAgree(double PtimesQ1, double PtimesQ2) {
        return PtimesQ1 * PtimesQ2 > 0;
    };
    /** @brief Calculate the momentum deviation between two PtimesQ estimates. The function is symmetric. */
    double momentumDev(double PtimesQ1, double PtimesQ2) {
        const double denom {std::max(std::abs(PtimesQ1) + std::abs(PtimesQ2), Acts::s_epsilon)};
        return std::abs(PtimesQ1 - PtimesQ2) / denom;
    };
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
       
        /** Initialize the field extraction steps */
        const double stepSize {1. / static_cast<double>(m_cfg.nFieldSteps)};
        for (std::size_t i = 0; i < m_cfg.nFieldSteps; ++i) {
            m_fieldExtpSteps.insert((static_cast<double>(i) + 0.5) * stepSize);
        }
    }
    Amg::Vector3D MsTrackSeeder::segPosOntoPhiPlane(const Amg::Vector3D& planeNorm,
                                                    const int Sector,
                                                    const Amg::Vector3D& posToProject) {
        // Find the sensor direction
        Amg::Vector3D projDir {
            ExpandedSector{static_cast<unsigned>(Sector), 
                           ExpandedSector::SectorProjector::center}.normalDir()};
        return Acts::PlanarHelper::intersectPlane(
            posToProject, projDir, planeNorm, Amg::Vector3D::Zero()).position();                                
    }
    Amg::Vector3D MsTrackSeeder::segDirOntoPhiPlane(const Amg::Vector3D& planeNorm,
                                                    const Amg::Vector3D& dirToProject) {
        return (dirToProject - dirToProject.dot(planeNorm) * planeNorm).unit();                                                
    }
    Amg::Vector2D MsTrackSeeder::expressOnCylinder(const xAOD::MuonSegment& segment,
                                                   const Location loc,
                                                   const ExpandedSector sector) const {
        /// extrapolated position
        const Amg::Vector3D pos{segPosOntoPhiPlane(
            sector.normalDir(), segment.sector(), segment.position())};
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
    double MsTrackSeeder::estimateQtimesP(const AtlasFieldCacheCondObj& magField,
                                          const Amg::Vector3D& planeNorm,
                                          const PosMomPair_t& p1, 
                                          const PosMomPair_t& p2,
                                          const PosMomPair_t& p3) const {
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
        
        MagField::AtlasFieldCache fieldCache{};
        magField.getInitializedCache(fieldCache);

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
                // Compute the charge agreement score & momentum deviation
                double w {std::min(est1.weight, est2.weight)};
                double chargeScore {chargeAgree(est1.PtimesQ, est2.PtimesQ) ? 1. : -1.};
                double pDevPenalty {momentumDev(est1.PtimesQ, est2.PtimesQ)};
                // Update the scores
                est1.score += w * (chargeScore - pDevPenalty);
                est2.score += w * (chargeScore - pDevPenalty);
            }
        }
        if (msgLvl(MSG::VERBOSE)) {
            std::vector<std::string> names {"Pair01", "Pair12", "Pair02Seg", "Pair02Pos"};
            for (const auto& [i, est] : Acts::enumerate(estimates)) {
                ATH_MSG_VERBOSE(__func__<<"() Estimate "<<names[i]<<": PtimesQ: "<<est.PtimesQ*1e-3
                    <<", weight: "<<est.weight<<", score: "<<est.score);
            }
        }
        // Find the best charge estimate and estimate the final momentum as a weighted average of the 
        // estimates that agree with the best charge
        const Estimate& bestEstimate {*std::ranges::max_element(estimates, 
            std::ranges::less{}, &Estimate::score)};
        const double charge {std::copysign(1., bestEstimate.PtimesQ)};

        double totalSum {0.}, totalWeight {0.};
        for (const Estimate& est : estimates) {
            if (chargeAgree(est.PtimesQ, charge)) {
                totalSum += est.PtimesQ * est.weight;
                totalWeight += est.weight;
            }
        }
        assert(totalWeight > Acts::s_epsilon);
        return totalSum / totalWeight;
    }
    double MsTrackSeeder::estimateQtimesP(const AtlasFieldCacheCondObj& magField,
                                          const Amg::Vector3D& planeNorm,
                                          const PosMomPair_t& p1,
                                          const PosMomPair_t& p2) const {
        MagField::AtlasFieldCache fieldCache{};
        magField.getInitializedCache(fieldCache);

        return getPtimesQ(forceIntegration(p1, p2, planeNorm, fieldCache),
                          p2.second - p1.second);
    }
    Amg::Vector3D MsTrackSeeder::forceIntegration(const PosMomPair_t& point1,
                                                  const PosMomPair_t& point2,
                                                  const Amg::Vector3D& planeNorm,
                                                  MagField::AtlasFieldCache& fieldCache) const {
        const auto& [pos1, dir1] = point1;
        const auto& [pos2, dir2] = point2;

        Amg::Vector3D locField{Amg::Vector3D::Zero()};
        Amg::Vector3D accumForce{Amg::Vector3D::Zero()};
        for (double fieldStep : m_fieldExtpSteps) {
            const Amg::Vector3D extPos {(1. - fieldStep) * pos1 + fieldStep * pos2};
            const Amg::Vector3D extDir {((1. - fieldStep) * dir1 + fieldStep * dir2).unit()};
                    
            fieldCache.getField(extPos.data(), locField.data());
            const Amg::Vector3D locForce {locField.dot(planeNorm) * extDir.cross(planeNorm)};
            accumForce += locForce;

            ATH_MSG_VERBOSE(__func__<<"() step: "<<fieldStep
                <<", pos: "<<Amg::toString(extPos)<<", dir: "<<Amg::toString(extDir)
                <<" --> local |B|: "<<locField.mag()*1e3 << " [T]"<<", |Bnorm|: "<<locField.dot(planeNorm)*1e3
                <<" [T], local |v x Bnorm|: "<<locForce.mag()*1e3<<" [T].");
        }
        const double dS {(pos2 - pos1).mag() / static_cast<double>(m_fieldExtpSteps.size())};
        return accumForce * dS;
    }
    double MsTrackSeeder::getPtimesQ(const Amg::Vector3D& forceIntegral, 
                                     const Amg::Vector3D& deltaDir) const {
        const double PtimesQ {0.3 * Gaudi::Units::GeV * forceIntegral.mag2() / deltaDir.dot(forceIntegral)};

        ATH_MSG_VERBOSE("estimateQtimesP() force integral: "<<forceIntegral.mag()<<" [T*m], deltaDir: "
            <<deltaDir.mag()<<", cos: "<<deltaDir.dot(forceIntegral)/ (deltaDir.mag() * forceIntegral.mag())
            <<", PtimesQ: "<<PtimesQ/Gaudi::Units::GeV <<" [GeV].");
        return PtimesQ;                                    
    }
    double MsTrackSeeder::estimateQtimesP(const AtlasFieldCacheCondObj& magField,
                                          const MsTrackSeed& seed) const {
        using namespace Muon::MuonStationIndex;
        MagField::AtlasFieldCache fieldCache{};
        magField.getInitializedCache(fieldCache); 

        /** Calculate the averaged phi from the segments */
        double deltaPhiAcc {0.};
        std::optional<double> centralPhi {};
        unsigned nSegsWithPhi{0};
        for (const xAOD::MuonSegment* segment : seed.segments()) {
            if (segment->nPhiLayers() > 0) {
                if (!centralPhi) centralPhi = segment->position().phi();
                deltaPhiAcc += P4Helpers::deltaPhi(*centralPhi, segment->position().phi());
                ++nSegsWithPhi;
            }
        }
        const double circPhi {nSegsWithPhi > 0 
            ? P4Helpers::deltaPhi(*centralPhi + deltaPhiAcc / nSegsWithPhi, 0.) 
            : seed.sector().phi()};

        std::array<const xAOD::MuonSegment*, 3> segmentsToUse{};
        // Try first to find segments in the inner, middle and outer layers. If both a barrel
        // and endcap segments are present in the same layer, the barrel segment is preferred.
        for (const xAOD::MuonSegment* segment : seed.segments()) {
            ChIndex chIndex {segment->chamberIndex()};
            switch(toLayerIndex(chIndex)) {
                case LayerIndex::Inner:
                    if (!segmentsToUse[0] || isBarrel(chIndex)) {
                        segmentsToUse[0] = segment;
                    }
                    break;
                case LayerIndex::Middle:
                    if (!segmentsToUse[1] || isBarrel(chIndex)) {
                        segmentsToUse[1] = segment;
                    }
                    break;
                case LayerIndex::Outer:
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

        /** If less than 3 segments are found check whether the track crosses the BEE or EE chamber and use that segment as the third one. */
        if (nSegments < 3) {
            auto missingSeg = std::ranges::find(segmentsToUse, nullptr);
            for (const xAOD::MuonSegment* segment : seed.segments()) {
                LayerIndex layIndex {toLayerIndex(segment->chamberIndex())};
                if (layIndex != LayerIndex::Extended && layIndex != LayerIndex::BarrelExtended) {
                    continue;
                }
                assert(missingSeg != segmentsToUse.end());
                *missingSeg = segment;
                nSegments++;
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
        auto point = [&planeNorm](const xAOD::MuonSegment* seg) {
            return std::make_pair(segPosOntoPhiPlane(planeNorm, seg->sector(), seg->position()),
                                  segDirOntoPhiPlane(planeNorm, seg->direction()));
        };
        return nSegments == 3 
            ? estimateQtimesP(magField, planeNorm, point(segmentsToUse[0]), point(segmentsToUse[1]), point(segmentsToUse[2]))
            : estimateQtimesP(magField, planeNorm, point(segmentsToUse[0]), point(segmentsToUse[1]));
    }
    void MsTrackSeeder::appendSegment(const xAOD::MuonSegment* segment,
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
            const Amg::Vector2D refPoint{expressOnCylinder(*segment, loc, projSector)};
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
    SearchTree_t MsTrackSeeder::constructTree(const xAOD::MuonSegmentContainer& segments) const{
        TreeRawVec_t rawData{};
        rawData.reserve(3*segments.size());
        for (const xAOD::MuonSegment* segment : segments){
            appendSegment(segment, Location::Barrel, rawData);
            appendSegment(segment, Location::Endcap, rawData);
        }
        ATH_MSG_VERBOSE("Create a new tree with "<<rawData.size()<<" entries. ");
        return SearchTree_t{std::move(rawData)};
    }
    std::unique_ptr<MsTrackSeedContainer> MsTrackSeeder::findTrackSeeds(const EventContext& ctx,
                                                                        const xAOD::MuonSegmentContainer& segments) const {
        SearchTree_t orderedSegs{constructTree(segments)};
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
            std::optional<LayerIndex> layerIndex{std::nullopt};
            bool foundSingleStationSeed{true};

            for(const xAOD::MuonSegment* seg : newSeed.segments()) {
                if (!layerIndex) {
                    layerIndex = toLayerIndex(seg->chamberIndex());
                    ATH_MSG_DEBUG("First segment is in layer "<<*layerIndex);
                } else if ( (*layerIndex) != toLayerIndex(seg->chamberIndex())) {
                    ATH_MSG_DEBUG("Found segment in layer "<<toLayerIndex(seg->chamberIndex())<<" which is different from the first segment in layer "<<*layerIndex);
                    foundSingleStationSeed = false;
                    break;
                } 
            }
            if(foundSingleStationSeed) {
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
