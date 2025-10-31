/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
   */

#include "EFTrackingFPGAPipeline/F150EDMConversionAlg.h"
#include "AthenaKernel/Chrono.h"
#include "EFTrackingFPGAUtility/FPGADataFormatUtilities.h"

namespace EFTrackingFPGAIntegration
{
    StatusCode F150EDMConversionAlg::initialize()
    {

        ATH_CHECK(m_FPGATrackOutput.initialize());
        ATH_CHECK(m_spacePointContainerKey.initialize());
        ATH_CHECK(m_seedKey.initialize());

        return StatusCode::SUCCESS;
    }

    StatusCode F150EDMConversionAlg::execute(const EventContext &ctx) const
    {
        ATH_MSG_DEBUG("Executing F150EDMConversionAlg");

        auto trackOutput = SG::get(m_FPGATrackOutput, ctx);

        // Get the space points
        SG::ReadHandle<xAOD::SpacePointContainer> spacePointsHandle{m_spacePointContainerKey, ctx};
        ATH_CHECK(spacePointsHandle.isValid());

        SG::WriteHandle<ActsTrk::SeedContainer> seedHandle{m_seedKey, ctx};
        ATH_CHECK(seedHandle.record(std::make_unique<ActsTrk::SeedContainer>()));
        ActsTrk::SeedContainer* seeds = seedHandle.ptr();

        std::multimap<xAOD::DetectorIDHashType, Acts::SpacePointIndex2> spacePointMap;
        seeds->spacePoints().reserve(spacePointsHandle->size());
        // Populate the multimap with Pixel cluster hashID as key.
        // Only space points with one measurement are considered (i.e. pixels)
        for (const xAOD::SpacePoint* spacePoint : *spacePointsHandle) {
            if (!spacePoint->measurements().empty()) {
                seeds->spacePoints().push_back(spacePoint);
                spacePointMap.emplace(spacePoint->measurements().at(0)->identifierHash(), seeds->spacePoints().size()-1ul);
            }
        }

        seeds->reserve(trackOutput->size());
        // first three works are header, so skip them for now
        for(unsigned int i = 3; i < trackOutput->size(); i++)
        {
            // Check if the flags track header 
            if(FPGADataFormatUtilities::get_bitfields_GTRACK_HDR_w1(trackOutput->at(i)).flag == FPGADataFormatUtilities::GTRACK_HDR_FLAG)
            {
                auto gtrack_w1 = FPGADataFormatUtilities::get_bitfields_GTRACK_HDR_w1(trackOutput->at(i));
                auto gtrack_w2 = FPGADataFormatUtilities::get_bitfields_GTRACK_HDR_w2(trackOutput->at(++i));
                auto gtrack_w3 = FPGADataFormatUtilities::get_bitfields_GTRACK_HDR_w3(trackOutput->at(++i));
            
                std::vector<Acts::SpacePointIndex2> spacePointsToStoreInSeed;
                //Look for GHITz, till we have a last hit
                bool isLast = false;
                unsigned int hitsInTrack = 0;
                while(!isLast)
                {
                    hitsInTrack++;
                    auto Ghit_w1 = FPGADataFormatUtilities::get_bitfields_GHITZ_w1(trackOutput->at(++i));
                    // decode information from the hit
                    float r = Ghit_w1.rad/FPGADataFormatUtilities::GHITZ_W1_RAD_mf;
                    float phi =  Ghit_w1.phi/FPGADataFormatUtilities::GHITZ_W1_PHI_mf;
                    float x = r * std::cos(phi);
                    float y = r * std::sin(phi);
                    float z = Ghit_w1.z/FPGADataFormatUtilities::GHITZ_W1_Z_mf;
                    isLast = Ghit_w1.last;

                    auto Ghit_w2 = FPGADataFormatUtilities::get_bitfields_GHITZ_w2(trackOutput->at(++i));
                    auto identifierHashW2 = Ghit_w2.cluster1;
                    
                    // find in the multimap the SP that matches this globalPosition
                    auto range = spacePointMap.equal_range(identifierHashW2);
                    for (auto it = range.first; it != range.second; ++it) {
                        constexpr float kEpsilon = 0.1;
                        const xAOD::SpacePoint* spacePoint = seeds->spacePoints().at(it->second);
                        if (std::abs(x - spacePoint->x()) < kEpsilon &&
                            std::abs(y - spacePoint->y()) < kEpsilon&&
                            std::abs(z - spacePoint->z()) < kEpsilon) {
                            spacePointsToStoreInSeed.push_back(it->second);
                            if (spacePointsToStoreInSeed.size() == m_maxSpacePointsPerSeed) break; // stop if max reached
                        }
                    }
                    if (spacePointsToStoreInSeed.size() == m_maxSpacePointsPerSeed) break; // stop in case we reach the maximum number of space points allowed
                }

                if(hitsInTrack != spacePointsToStoreInSeed.size())
                {
                    ATH_MSG_WARNING("Track does not have the same number of hits in the output as the seed container hitsInTrack: "<<hitsInTrack<<" spacePointsToStoreInSeed: "<<spacePointsToStoreInSeed.size());
                }
                // construct seed based on the space points stored in the vector
                if (spacePointsToStoreInSeed.size() >= m_minSpacePointsPerSeed) { // check that seeds contains at least the minimum number of desired space points
                    auto seed = seeds->push_back(spacePointsToStoreInSeed);
                    seed.vertexZ() = gtrack_w2.z0/FPGADataFormatUtilities::GTRACK_HDR_W2_Z0_mf;
                    auto chiSquare = gtrack_w2.score/FPGADataFormatUtilities::GTRACK_HDR_W2_SCORE_mf;
                    if (chiSquare != 0.0) {
                        seed.quality() = 1.0 / chiSquare;
                    } else {
                        seed.quality() = 0.0;
                    }
                }

            }

        }

        ATH_MSG_DEBUG("Recorded " << seeds->size() << " seeds");

        return StatusCode::SUCCESS;
    }

} // namespace EFTrackingFPGAIntegration
