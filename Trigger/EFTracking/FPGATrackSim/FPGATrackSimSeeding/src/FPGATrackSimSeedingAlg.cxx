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


        std::multimap<xAOD::DetectorIDHashType, const xAOD::SpacePoint*> spacePointMap;
        // Populate the multimap with Pixel cluster hashID as key.
        // Only space points with one measurement are considered (i.e. pixels)
        for (const xAOD::SpacePoint* spacePoint : *spacePointsHandle) {
            if (!spacePoint->measurements().empty()) {
            spacePointMap.emplace(spacePoint->measurements().at(0)->identifierHash(), spacePoint);
            }
        }

        // loop over the tracks and make seeds based on the hits in FPGATrackSimTracks
        for (const auto& track : *tracksHandle) {
            std::vector<const FPGATrackSimHit*> hitsToStoreInSeed;
            std::vector<const xAOD::SpacePoint*> spacePointsToStoreInSeed;
            for (const FPGATrackSimHit& hit : track.getFPGATrackSimHits()) {
                if (hit.isReal() && hit.isPixel()) {
                    ATH_MSG_DEBUG("Hit coordinates in module " << hit.getIdentifierHash() << ": (" << hit.getPhiCoord() << ", " << hit.getEtaCoord() << ")");
                    // find in the multimap the SP that matches this globalPosition
                    auto range = spacePointMap.equal_range(hit.getIdentifierHash());
                    for (auto it = range.first; it != range.second; ++it) {
                        constexpr float kEpsilon = std::numeric_limits<float>::epsilon();
                        if (std::abs(hit.getPhiCoord() - it->second->measurements().at(0)->localPosition<2>()[0]) < kEpsilon &&
                            std::abs(hit.getEtaCoord() - it->second->measurements().at(0)->localPosition<2>()[1]) < kEpsilon) {
                            spacePointsToStoreInSeed.push_back(it->second);
                            if (spacePointsToStoreInSeed.size() == m_maxSpacePointsPerSeed) break; // stop if max reached
                        }
                        else
                        {
                            // printout SP coordinates to see why the above check fails
                            ATH_MSG_DEBUG("SpacePoint coordinates: (" << it->second->measurements().at(0)->localPosition<2>()[0] << ", " << it->second->measurements().at(0)->localPosition<2>()[1] << ")");
                        }
                    }
                    if (spacePointsToStoreInSeed.size() == m_maxSpacePointsPerSeed) break; // stop in case we reach the maximum number of space points allowed
                }
            }
            // construct seed based on the space points stored in the vector
            if (spacePointsToStoreInSeed.size() >= m_minSpacePointsPerSeed) { // check that seeds contains at least the minimum number of desired space points
                std::unique_ptr<ActsTrk::ActsSeed<xAOD::SpacePoint>> seed = std::make_unique<ActsTrk::ActsSeed<xAOD::SpacePoint>>(spacePointsToStoreInSeed);
                seed->setVertexZ(track.getZ0());
                if (track.getChi2ndof() != 0.0) {
                    seed->setQuality(1.0 / track.getChi2ndof()); // TODO: validate if this is correct. (technically the larger the value, the better the quality of the seed)
                } else {
                    seed->setQuality(0.0); // or another default value? TODO: check if that's fine
                }
                seeds->push_back(std::move(seed));
            }
        }

        ATH_MSG_DEBUG("Recorded " << seeds->size() << " seeds");
        return StatusCode::SUCCESS;
    }
} // namespace FPGATrackSim
