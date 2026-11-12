/*
Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "TracccSeedDeviceConverterAlg.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

#include <vecmem/containers/data/vector_view.hpp>
#include <vecmem/containers/data/vector_buffer.hpp>

#include <array>

namespace ActsTrk {

StatusCode TracccSeedDeviceConverterAlg::initialize()
{
    ATH_MSG_DEBUG("Initializing.");

    ATH_CHECK(m_hostMR.retrieve());
    ATH_CHECK(m_mrTool.retrieve());
    ATH_CHECK(m_copiesTool.retrieve());
    ATH_CHECK(m_streamTool.retrieve());

    ATH_CHECK(m_inputSPKey.initialize());
    ATH_CHECK(m_inputSeedsKey.initialize());
    ATH_CHECK(m_inputSPDeviceKey.initialize());

    ATH_CHECK(m_chronoSvc.retrieve());

    // only one method must be used so the scheduler will wait for the one being used
    // hopefully not too many double negatives...
    const bool externalMap = !m_inputMeasToPixelKey.key().empty();
    ATH_CHECK(m_inputMeasToPixelKey.initialize(externalMap));
    ATH_CHECK(m_inputMeasKey.initialize(!externalMap));
    ATH_MSG_INFO("Measurement -> pixel SP map: "
                 << (externalMap ? "read from " + m_inputMeasToPixelKey.key()
                                 : "built on device from " + m_inputMeasKey.key()));

    ATH_CHECK(m_outputSeedsKey.initialize());

    ATH_MSG_DEBUG("Successfully initialized");
    return StatusCode::SUCCESS;
}

StatusCode TracccSeedDeviceConverterAlg::execute(const EventContext& ctx) const
{
    // =========================================================================
    // ---- Inputs
    // =========================================================================
    auto device_copy = m_copiesTool->deviceCopy(ctx);
    cudaStream_t cuda_stream = static_cast<cudaStream_t>(m_streamTool->stream(ctx));

    auto seeds = SG::makeHandle(m_inputSeedsKey, ctx);
    ATH_CHECK(seeds.isValid());
    // Host xAOD pixel spacepoints (made by TracccMeasurementConverterAlg)
    auto spacepoints = SG::makeHandle(m_inputSPKey, ctx);
    ATH_CHECK(spacepoints.isValid());
    // Device traccc spacepoints (point into the device measurement collection)
    auto traccc_spacepoints = SG::makeHandle(m_inputSPDeviceKey, ctx);
    ATH_CHECK(traccc_spacepoints.isValid());

    const size_type n_seeds = device_copy->get_size(*seeds);

    // ---- Output (recorded even when empty)
    auto seedHandle = SG::makeHandle(m_outputSeedsKey, ctx);
    ATH_CHECK(seedHandle.record(std::make_unique<ActsTrk::SeedContainer>()));
    ActsTrk::SeedContainer* seedsOut = seedHandle.ptr();

    m_nSP += spacepoints->size();
    m_nSeeds += n_seeds;
    if (n_seeds == 0) {
        ATH_MSG_DEBUG("No seeds to convert");
        return StatusCode::SUCCESS;
    }

    // =========================================================================
    // ---- Device step
    // =========================================================================

    // Map traccc measurement -> host pixel SP index (i <-> i: the n-th pixel
    // measurement is the n-th host pixel spacepoint). Either provided by an
    // upstream converter, or built here on device from the measurements.
    std::unique_ptr<ActsTrk::TracccMeasToPixelIndex> own_map{nullptr};
    const ActsTrk::TracccMeasToPixelIndex* meas_to_sp_device = nullptr;
    if (!m_inputMeasToPixelKey.key().empty()) {
        auto mapHandle = SG::makeHandle(m_inputMeasToPixelKey, ctx);
        ATH_CHECK(mapHandle.isValid());
        meas_to_sp_device = mapHandle.cptr();
    } else {
        auto measurements = SG::makeHandle(m_inputMeasKey, ctx);
        ATH_CHECK(measurements.isValid());
        Athena::Chrono chrono("SeedDeviceConv:buildMap", m_chronoSvc.get());
        ATH_CHECK(buildMeasToPixelIndexOnGPU(cuda_stream, m_mrTool->mainMR(), *device_copy,
            device_copy->get_size(*measurements), *measurements, own_map));
        meas_to_sp_device = own_map.get();
    }

    std::unique_ptr<DeviceSeedContainer::buffer> device_seeds{nullptr};
    {
        Athena::Chrono chrono("SeedDeviceConv:kernel", m_chronoSvc.get());
        ATH_CHECK(convertSeedsOnGPU(cuda_stream, m_mrTool->mainMR(), *device_copy,
            n_seeds, *seeds, *traccc_spacepoints, *meas_to_sp_device, device_seeds));
    }

    // =========================================================================
    // ---- Bulk copy: the whole converted SoA buffer in one D2H transfer
    // =========================================================================
    DeviceSeedContainer::buffer host_seeds_buffer(n_seeds, m_hostMR->mr());
    {
        Athena::Chrono chrono("SeedDeviceConv:copyD2H", m_chronoSvc.get());
        device_copy->setup(host_seeds_buffer)->ignore();
        (*device_copy)(*device_seeds, host_seeds_buffer)->wait();
    }
    const DeviceSeedContainer::const_device host_seeds(host_seeds_buffer);

    // =========================================================================
    // ---- Build the host seeds
    // =========================================================================
    {
        Athena::Chrono chrono("SeedDeviceConv:hostLoop", m_chronoSvc.get());
        seedsOut->reserve(n_seeds);
        const std::size_t n_host_sps = spacepoints->size();

        for (size_type s = 0; s < n_seeds; ++s) {
            const auto seed = host_seeds.at(s);

            // Host SP indices -> pointers; any failed lookup fails the event,
            // like the host converter
            const auto& sp_idx = seed.sp_indices();
            std::array<const xAOD::SpacePoint*, kDeviceSeedNSp> sps{};
            for (unsigned int k = 0; k < kDeviceSeedNSp; ++k) {
                if (sp_idx[k] == kDeviceSeedInvalidIndex || sp_idx[k] >= n_host_sps) {
                    ATH_MSG_ERROR("Seed " << s << ": no host spacepoint for SP " << k
                                << " (index " << sp_idx[k] << ")");
                    return StatusCode::FAILURE;
                }
                sps[k] = spacepoints->at(sp_idx[k]);
            }
            seedsOut->push_back(sps, seed.quality(), 0.f);
        }
    }

    ATH_MSG_DEBUG("Wrote " << seedsOut->size() << " seeds to " << m_outputSeedsKey.key());
    return StatusCode::SUCCESS;
}

StatusCode TracccSeedDeviceConverterAlg::finalize()
{
    ATH_MSG_DEBUG("Finalizing.");
    ATH_MSG_DEBUG("Received " << m_nSP << " spacepoints and " << m_nSeeds << " seeds");
    ATH_MSG_DEBUG("Successfully finalized");
    return StatusCode::SUCCESS;
}

} // namespace ActsTrk