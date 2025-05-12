/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonTrackFindingTools/MsTrackSeeder.h"
#include "MuonPatternHelpers/MatrixUtils.h"
#include "MuonTrackEvent/TrackingHelpers.h"

namespace MuonR4{
    using SearchTree_t = MsTrackSeeder::SearchTree_t;

    MsTrackSeeder::MsTrackSeeder(const std::string& msgName, Config&& cfg):
        AthMessaging{msgName},
        m_cfg{std::move(cfg)}{}

    Amg::Vector2D MsTrackSeeder::expressOnCylinder(const xAOD::MuonSegment& segment,
                                                   const Location loc) const {
        const Amg::Vector3D pos{segment.position()};
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
            ATH_MSG_VERBOSE(__func__<<"()  "<<__LINE__<<" Position "<<Amg::toString(projPos)<<
                            "exceeds cylinder boundaries ("<<m_cfg.barrelRadius<<", "
                            <<std::min(m_cfg.endcapDiscZ, m_cfg.barrelLength)<<")");
            return false;
        } else if (loc == Endcap && (0 > projPos[0] || projPos[0] > m_cfg.endcapDiscRadius)) {
            ATH_MSG_VERBOSE(__func__<<"()  "<<__LINE__<<" Position "<<Amg::toString(projPos)<<
            "exceeds endcap boundaries ("<<m_cfg.endcapDiscRadius<<", "<<(sign(projPos[1])*m_cfg.endcapDiscZ)<<")");
            return false;
        }
        return true;
    }
    void MsTrackSeeder::appendSegment(const xAOD::MuonSegment* segment,
                                      const Location loc,
                                      TreeRawVec_t& outContainer) const {
        
        const Amg::Vector2D refPoint{expressOnCylinder(*segment, loc)};
        ATH_MSG_VERBOSE(__func__<<"()  "<<__LINE__<<" - Try to add new segment: "
                    <<detailedSegment(*segment)->msSector()->identString()<<" "
                    <<Amg::toString(segment->position())<<" + "<<Amg::toString(segment->direction())
                    <<" --> "<<Amg::toString(refPoint));
        if (!withinBounds(refPoint, loc)) {
            return;
        }
        std::array<double, 3> coords{};
        coords[eDetSection] = sign(refPoint[1]) * static_cast<int>(loc);
        const int sector = detailedSegment(*segment)->msSector()->sector();
        coords[eSector] = sector;
        coords[ePosOnCylinder] = refPoint[Location::Barrel == loc];
        outContainer.emplace_back(std::move(coords), segment);
        /** Extend the segment's at the sector numbering discontinuity */
        if (sector == 1 || sector== 16) {
            outContainer.emplace_back(outContainer.back()).first[eSector] = (sector == 1 ? 17 : 0);
        }
    }
    SearchTree_t MsTrackSeeder::constructTree(const xAOD::MuonSegmentContainer& segments) const{
        TreeRawVec_t rawData{};
        rawData.reserve(2*segments.size());
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

        for (const auto& [coords, seedCandidate] : orderedSegs) {
            /** Bad segment not suitable for track seeding or the segment coordinates are
             *  just mirrored at the overlap between sector 1 -> 16 */
            const Segment* recoCandidate = detailedSegment(*seedCandidate);
             if (coords[eSector] == 0 || coords[eSector] == 17 || 
                !m_cfg.selector->passSeedingQuality(ctx, *recoCandidate)){
                continue;
            }
            /** Define the search range. */    
            SearchTree_t::range_t selectRange{};
            /** Ensure that only endcap / barrel seeds are considered. 
             *  The values are integers -> add tiny margin */
            selectRange[eDetSection].shrink(coords[eDetSection] - 0.1, coords[eDetSection] + 0.1);
            /** Move 25 cm along the projected plane */
            selectRange[ePosOnCylinder].shrink(coords[ePosOnCylinder] - m_cfg.seedHalfLength, 
                                               coords[ePosOnCylinder] + m_cfg.seedHalfLength);
            /** Include the neighbouring sectors */
            selectRange[eSector].shrink(coords[eSector]-1.25, coords[eSector] + 1.25);
            
            MsTrackSeed newSeed{static_cast<Location>(std::abs(coords[eDetSection]))};
            /** Using the cube above, let the tree search for all compatible segments */
            
            orderedSegs.rangeSearchMapDiscard(selectRange, [this, &ctx, &newSeed, &recoCandidate](
                    const SearchTree_t::coordinate_t& /*coords*/,
                    const xAOD::MuonSegment* extendWithMe) {
                        /** Ensure that the sector overlap and momentum vectors are compatible with a MS trajectory */
                        const Segment* extendCandidate = detailedSegment(*extendWithMe);
                        if (m_cfg.selector->compatibleForTrack(ctx, *recoCandidate, *extendCandidate)) {
                            newSeed.addSegment(extendWithMe);
                        }
            });
            /** No segments were combined */
            if (newSeed.segments().empty()) {
                continue;
            }
            newSeed.addSegment(seedCandidate);
            /** Calculate the seed's position */
            const double r = newSeed.location() == Location::Barrel ? m_cfg.barrelRadius : coords[ePosOnCylinder];
            const double z = newSeed.location() == Location::Barrel ? coords[ePosOnCylinder] 
                                                 : coords[eDetSection]* m_cfg.endcapDiscZ;
            Amg::Vector3D pos = r * Amg::dirFromAngles(seedCandidate->position().phi(), 90. * Gaudi::Units::deg)
                              + z * Amg::Vector3D::UnitZ();
            
            newSeed.setPosition(std::move(pos));
            trackSeeds.emplace_back(std::move(newSeed));
        }
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
                /** Only seeds which are not subsets of others are kept*/
                const bool add = std::ranges::none_of(*outputSeeds, [&testMe](const MsTrackSeed& goodSeed){
                                    return testMe < goodSeed;
                                });
                if (add) {
                    ATH_MSG_VERBOSE("Add new seed "<<std::endl<<testMe);
                }
                return add;
            });
        return outputSeeds;
    } 
}