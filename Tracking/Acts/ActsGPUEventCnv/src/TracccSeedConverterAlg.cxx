/*
Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "TracccSeedConverterAlg.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

#include <stdexcept>

namespace ActsTrk {

StatusCode TracccSeedConverterAlg::initialize()
{
    ATH_MSG_DEBUG("Initializing.");

    ATH_CHECK(m_hostMR.retrieve());
    ATH_CHECK(m_copy.retrieve());

    ATH_CHECK(m_inputSPKey.initialize());
    ATH_CHECK(m_inputSeedsKey.initialize());
    ATH_CHECK(m_inputSPDeviceKey.initialize());
    ATH_CHECK(m_inputMeasToPixelSPKey.initialize());

    ATH_CHECK(m_outputSeedsKey.initialize());

    ATH_MSG_DEBUG("Successfully initialized");
    return StatusCode::SUCCESS;
}


StatusCode TracccSeedConverterAlg::execute(const EventContext& ctx) const
{
    // ---- Retrieve HOST RESIDENT spacepoints (always needed) ----
    // These are made during the TracccMeasurementConverterAlg therefore the pixel meas index in PixelCluster container
    // is the same as the pixel spacepoint index in the SpacePoint container
    auto spacepoints = SG::makeHandle(m_inputSPKey, ctx);
    ATH_CHECK(spacepoints.isValid());

    // ---- Retrieve mapping from traccc measurement index to pixel spacepoint index (always needed) ----
    auto measMap = SG::makeHandle(m_inputMeasToPixelSPKey, ctx);
    ATH_CHECK(measMap.isValid());

    // ---- Retrieve DEVICE resident traccc spacepoints ----
    // these are made on the GPU, therefore the spacepoint points to measurement index
    // in the traccc measurement collection on the device, 
    // (reminder: pixel and strip measurements are in the same collection on the device)
    // which is the NOT the same index in the PixelCluster container 
    
    auto traccc_spacepoints = SG::makeHandle(m_inputSPDeviceKey, ctx);
    ATH_CHECK(traccc_spacepoints.isValid());

    auto copy = m_copy->copy(ctx);

    traccc::edm::spacepoint_collection::buffer traccc_spacepoints_buffer{
        copy->get_size(*traccc_spacepoints), m_hostMR->mr()};
    copy->setup(traccc_spacepoints_buffer)->ignore();
    (*copy)(*traccc_spacepoints, traccc_spacepoints_buffer)->wait();

    traccc::edm::spacepoint_collection::const_device traccc_sp(
        traccc_spacepoints_buffer);

    // ---- Retrieve DEVICE resident traccc seeds ----
    auto seeds = SG::makeHandle(m_inputSeedsKey, ctx);
    ATH_CHECK(seeds.isValid());

    traccc::edm::seed_collection::buffer traccc_seeds_buffer{
        copy->get_size(*seeds), m_hostMR->mr()};
    copy->setup(traccc_seeds_buffer)->ignore();
    (*copy)(*seeds, traccc_seeds_buffer)->wait();

    traccc::edm::seed_collection::const_device traccc_seeds(
        traccc_seeds_buffer);

    ATH_MSG_DEBUG("Read " << traccc_seeds.size() << " seeds and " << spacepoints->size() << " spacepoints from device and host, respectively.");
    m_nSP += spacepoints->size();
    m_nSeeds += traccc_seeds.size();

    // -- Write HOST resident ACTS seed container ----
    SG::WriteHandle<ActsTrk::SeedContainer> seedHandle =
      SG::makeHandle(m_outputSeedsKey, ctx);
    
    ATH_CHECK(seedHandle.record(std::make_unique<ActsTrk::SeedContainer>()));
    ActsTrk::SeedContainer* seedPtrs = seedHandle.ptr();    

    for (size_t st = 0; st < traccc_seeds.size(); ++st) {
        
        const auto& seed = traccc_seeds.at(st);
        std::vector<unsigned int> sp_traccc_index{
            seed.bottom_index(), seed.middle_index(), seed.top_index()}; 

        std::vector<unsigned int> sp_host_index{
            std::numeric_limits<unsigned int>::max(),
            std::numeric_limits<unsigned int>::max(),
            std::numeric_limits<unsigned int>::max()};
        
        ATH_MSG_VERBOSE("Traccc seed " << st << " with spacepoints: ");
        for (int sp = 0; sp < 3; sp++) {
            const auto& sp_traccc = traccc_sp.at(sp_traccc_index[sp]);
            const unsigned int measIdx = sp_traccc.measurement_index_1();
            const unsigned int hostIdx = (*measMap)[measIdx];
            ATH_CHECK(hostIdx != std::numeric_limits<unsigned int>::max());

            sp_host_index[sp] = hostIdx;

            const auto& sp_host = spacepoints->at(hostIdx);
            ATH_MSG_VERBOSE(sp_traccc.x() << "," << sp_traccc.y()
                                            << "," << sp_traccc.z());
            ATH_MSG_VERBOSE(sp_host->globalPosition().x() << "," << sp_host->globalPosition().y()
                                            << "," << sp_host->globalPosition().z());                                                                    
        }

        seedPtrs->push_back(
            std::array{spacepoints->at(sp_host_index[0]),
                        spacepoints->at(sp_host_index[1]),
                        spacepoints->at(sp_host_index[2])},
            0.f, 0.f);
    }

    ATH_MSG_DEBUG(" Seed Container " << m_outputSeedsKey.key()
                                     << " created with " << seedPtrs->size() << " seeds");

    return StatusCode::SUCCESS;
}

StatusCode TracccSeedConverterAlg::finalize()
{
    ATH_MSG_DEBUG("Finalizing.");

    ATH_MSG_DEBUG("Received " << m_nSP << " spacepoints and " << m_nSeeds << " seeds.");

    ATH_MSG_DEBUG("Successfully finalized");
    return StatusCode::SUCCESS;
}

} // namespace ActsTrk
