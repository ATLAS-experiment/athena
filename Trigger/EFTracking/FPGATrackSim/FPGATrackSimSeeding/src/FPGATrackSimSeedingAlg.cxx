// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#include "src/FPGATrackSimSeedingAlg.h"
#include <array>
#include <map>
#include "xAODInDetMeasurement/SpacePoint.h"

namespace FPGATrackSim {

    FPGATrackSim::FPGATrackSimSeedingAlg::FPGATrackSimSeedingAlg(const std::string& name, ISvcLocator* pSvcLocator) : AthReentrantAlgorithm(name, pSvcLocator) {
}
    StatusCode FPGATrackSimSeedingAlg::initialize() {
        ATH_CHECK(m_FPGATrackCollectionKey.initialize());
        ATH_CHECK(m_pixelClusterContainerKey.initialize());
        ATH_CHECK(m_spacePointContainerKey.initialize());
        ATH_CHECK(m_seedKey.initialize());

        return StatusCode::SUCCESS;
    }

    StatusCode FPGATrackSimSeedingAlg::execute(const EventContext& ctx) const {

        SG::ReadHandle<FPGATrackSimTrackCollection> tracksHandle{m_FPGATrackCollectionKey, ctx};
        SG::ReadHandle<xAOD::PixelClusterContainer> pixelClustersHandle{m_pixelClusterContainerKey, ctx};
        SG::ReadHandle<xAOD::SpacePointContainer> spacePointsHandle{m_spacePointContainerKey, ctx};
    
        ATH_CHECK(tracksHandle.isValid());
        ATH_CHECK(pixelClustersHandle.isValid());
        ATH_CHECK(spacePointsHandle.isValid());

        SG::WriteHandle<ActsTrk::SeedContainer> seedHandle{m_seedKey, ctx};
        ATH_CHECK(seedHandle.record(std::make_unique<ActsTrk::SeedContainer>()));
        ActsTrk::SeedContainer* seeds = seedHandle.ptr();

        // Use a map since identifiers are unique
        std::map<Identifier::value_type, const xAOD::SpacePoint*> spacePointMap;
        
        // Populate the map with pixel cluster identifier (rdoID) as key.
        // Only space points with one measurement are considered (i.e. pixels). In case strips are needed the code should not be based on the one-measurement-per-space-point assumption.
        for (const xAOD::SpacePoint* spacePoint : *spacePointsHandle) {
            if (!spacePoint->measurements().empty()) {
                const auto identifier = spacePoint->measurements().at(0)->identifier();
                
                // Check for duplicates (shouldn't happen if all works as expected)
                auto [it, inserted] = spacePointMap.emplace(identifier, spacePoint);
                if (!inserted) {
                    ATH_MSG_ERROR("Duplicate identifier 0x" << std::hex << identifier << std::dec 
                                  << " found for space point. Keeping first occurrence.");
                }
            }
        }

        seeds->reserve(tracksHandle->size());
        // loop over the tracks and make seeds based on the hits in FPGATrackSimTracks
        for (const auto& track : *tracksHandle) {
            std::vector<const xAOD::SpacePoint*> spacePointsToStoreInSeed;
            for (const FPGATrackSimHit& hit : track.getFPGATrackSimHits()) {
                if (hit.isReal() && hit.isPixel()) {
                    ATH_MSG_DEBUG("Hit coordinates in module " << hit.getRdoIdentifier() 
                                  << ": (" << hit.getPhiCoord() << ", " << hit.getEtaCoord() << ")");
                    
                    // map lookup
                    auto it = spacePointMap.find(hit.getRdoIdentifier());
                    if (it != spacePointMap.end()) {
                        spacePointsToStoreInSeed.push_back(it->second);
                    } else {
                        ATH_MSG_ERROR("No SP found for hit identifier 0x"
                                      << std::hex << hit.getRdoIdentifier() << std::dec);
                    }
                    
                    // stop in case we reach the maximum number of space points allowed
                    if (spacePointsToStoreInSeed.size() == m_maxSpacePointsPerSeed) break; 
                }
            }
            
            // Construct seed based on the space points stored in the vector
            if (spacePointsToStoreInSeed.size() >= m_minSpacePointsPerSeed) { // check that seeds contains at least the minimum number of desired space points
                // TODO: validate if this is correct. (technically the larger the value, the better the quality of the seed)
                // or another default value? TODO: check if that's fine
                const float quality = track.getChi2ndof() != 0.0 ? 1.0 / track.getChi2ndof() : 0.0;

                seeds->push_back(spacePointsToStoreInSeed, quality, track.getZ0());
            }
        }

        ATH_MSG_DEBUG("Recorded " << seeds->size() << " seeds");
        return StatusCode::SUCCESS;
    }
} // namespace FPGATrackSim
