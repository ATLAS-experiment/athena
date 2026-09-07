// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#include "TracccTritonRunner.hpp"

#include "TracccTritonInitializer.hpp"

#include <chrono>
#include <cstring>
#include <iostream>
#include <stdexcept>

#include <cuda_runtime.h>

// Gaudi / Athena.
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/IAlgManager.h"
#include "GaudiKernel/IAlgorithm.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/SmartIF.h"
#include "StoreGate/StoreGateSvc.h"

// traccc / vecmem EDM
#include "ActsGPUEvent/TracccSiliconCellCollection.h"
#include "ActsGPUEvent/TracccSeedCollection.h"
#include "ActsGPUEvent/TracccSpacepointCollection.h"

#include "vecmem/memory/cuda/device_memory_resource.hpp"
#include "vecmem/memory/host_memory_resource.hpp"
#include "vecmem/utils/cuda/async_copy.hpp"

namespace triton { namespace backend { namespace traccc {

namespace {
// Algorithm names must match the ones the bootstrap jobOptions creates (see
// TracccTritonInitializer.cpp and python/TracccTritonBackendConfig.py).
constexpr const char* kClusterizationAlg = "DeviceClusterizationAlg";
constexpr const char* kSPFormationAlg = "DeviceSPFormationAlg";
constexpr const char* kTripletSeedingAlg = "DeviceTripletSeedingAlg";
}  // namespace

struct StreamGuard {
    cudaStream_t stream = nullptr;
    StreamGuard() { cudaStreamCreate(&stream); }
    ~StreamGuard() {
        if (stream) cudaStreamDestroy(stream);
    }
    StreamGuard(const StreamGuard&) = delete;
    StreamGuard& operator=(const StreamGuard&) = delete;
    void synchronize() const {
        if (stream) cudaStreamSynchronize(stream);
    }
};

struct TracccTritonRunner::Impl {
    TracccTritonInitializer& initializer;

    // Self-contained device resources for the H2D of the incoming cells and
    // the D2H of the outgoing seeds. The device algorithms use their own
    // memory resources (from their provider tools) internally; the cell
    // buffer only has to live in device memory, so it need not share them.
    vecmem::host_memory_resource host_mr;
    vecmem::cuda::device_memory_resource device_mr;
    StreamGuard stream;
    // async_copy is bound to our own stream so that wait()/synchronize()
    // only ever touch this runner's work.
    vecmem::cuda::async_copy copy;

    ServiceHandle<StoreGateSvc> eventStore;

    SmartIF<IAlgorithm> clusterization;
    SmartIF<IAlgorithm> spFormation;
    SmartIF<IAlgorithm> seeding;

    explicit Impl(TracccTritonInitializer& init)
        : initializer(init)
        , host_mr()
        , device_mr(init.deviceId())
        , stream()
        , copy(stream.stream)
        , eventStore("StoreGateSvc", "TracccTritonRunner") {}
};

TracccTritonRunner::TracccTritonRunner(TracccTritonInitializer& initializer)
    : m_impl(std::make_unique<Impl>(initializer)) {

    if (!initializer.isReady()) {
        throw std::runtime_error(
            "TracccTritonRunner: initializer is not ready");
    }

    if (cudaSetDevice(initializer.deviceId()) != cudaSuccess) {
        throw std::runtime_error(
            "TracccTritonRunner: cudaSetDevice(" +
            std::to_string(initializer.deviceId()) + ") failed");
    }

    if (m_impl->eventStore.retrieve().isFailure() || !m_impl->eventStore) {
        throw std::runtime_error(
            "TracccTritonRunner: could not retrieve StoreGateSvc");
    }

    SmartIF<ISvcLocator> svcLocator(&initializer.serviceLocator());
    IAlgManager* algMgr = svcLocator.as<IAlgManager>();
    if (!algMgr) {
        throw std::runtime_error(
            "TracccTritonRunner: no IAlgManager in the embedded kernel");
    }
    m_impl->clusterization =
        algMgr->algorithm(std::string(kClusterizationAlg), /*createIf*/ false);
    m_impl->spFormation =
        algMgr->algorithm(std::string(kSPFormationAlg), /*createIf*/ false);
    m_impl->seeding =
        algMgr->algorithm(std::string(kTripletSeedingAlg), /*createIf*/ false);

    if (!m_impl->clusterization || !m_impl->spFormation || !m_impl->seeding) {
        throw std::runtime_error(
            "TracccTritonRunner: could not retrieve one of the device "
            "algorithms");
    }
}

TracccTritonRunner::~TracccTritonRunner() = default;

TracccTritonRunner::Output
TracccTritonRunner::run(const uint8_t* buffer,
                        std::size_t byteSize,
                        bool printStats) {
    const auto& keys = m_impl->initializer.config();
    Output output;

    auto t0 = std::chrono::high_resolution_clock::now();

    // Deserialize the raw CELLS tensor into a host cell collection
    ::traccc::edm::silicon_cell_collection::host cells(m_impl->host_mr);
    {
        if (buffer == nullptr || byteSize < sizeof(std::uint64_t)) {
            throw std::runtime_error(
                "CELLS buffer is too small to contain the cell count header");
        }
        std::uint64_t numCells = 0;
        std::memcpy(&numCells, buffer, sizeof(std::uint64_t));

        const std::size_t expected =
            sizeof(std::uint64_t) + static_cast<std::size_t>(numCells) * 20u;
        if (byteSize != expected) {
            throw std::runtime_error(
                "CELLS buffer size mismatch: expected " +
                std::to_string(expected) + " bytes for N=" +
                std::to_string(numCells) + ", got " + std::to_string(byteSize));
        }

        static_assert(sizeof(::traccc::channel_id) == 4u,
                      "CELLS wire format assumes 4-byte channel_id");
        static_assert(sizeof(float) == 4u,
                      "CELLS wire format assumes 4-byte float");
        static_assert(sizeof(unsigned int) == 4u,
                      "CELLS wire format assumes 4-byte module_index");

        const std::size_t n = static_cast<std::size_t>(numCells);
        if (n != 0u) {
            const std::size_t block = n * 4u;
            const uint8_t* base = buffer + sizeof(std::uint64_t);
            cells.resize(n);
            std::memcpy(cells.channel0().data(), base + 0u * block, block);
            std::memcpy(cells.channel1().data(), base + 1u * block, block);
            std::memcpy(cells.activation().data(), base + 2u * block, block);
            std::memcpy(cells.time().data(), base + 3u * block, block);
            std::memcpy(cells.module_index().data(), base + 4u * block, block);
        }
    }

    auto t1 = std::chrono::high_resolution_clock::now();

    // Copy cells to the device and record them in StoreGate
    if (m_impl->eventStore->clearStore().isFailure()) {
        throw std::runtime_error("TracccTritonRunner: clearStore() failed");
    }

    EventContext ctx;
    {
        auto deviceCells =
            std::make_unique<::traccc::edm::silicon_cell_collection::buffer>(
                static_cast<unsigned int>(cells.size()), m_impl->device_mr);
        m_impl->copy.setup(*deviceCells)->ignore();
        m_impl->copy(vecmem::get_data(cells), *deviceCells)->wait();

        if (m_impl->eventStore
                ->record(std::move(deviceCells), keys.cellsKey)
                .isFailure()) {
            throw std::runtime_error(
                "TracccTritonRunner: failed to record cells under '" +
                keys.cellsKey + "'");
        }
    }

    auto t2 = std::chrono::high_resolution_clock::now();

    // Run the device chain: clusterization -> SP -> seeding
    for (IAlgorithm* alg :
         {m_impl->clusterization.get(), m_impl->spFormation.get(),
          m_impl->seeding.get()}) {
        if (alg->sysExecute(ctx).isFailure()) {
            throw std::runtime_error(
                "TracccTritonRunner: algorithm '" + alg->name() +
                "' failed during execute()");
        }
    }
    m_impl->stream.synchronize();

    auto t3 = std::chrono::high_resolution_clock::now();

    // Copy the seeds and the spacepoints they point at back to the host
    const ::traccc::edm::seed_collection::buffer* seedsDevice = nullptr;
    if (m_impl->eventStore
                ->retrieve(seedsDevice, keys.seedsKey)
                .isFailure() ||
        seedsDevice == nullptr) {
        throw std::runtime_error(
            "TracccTritonRunner: could not retrieve seeds under '" +
            keys.seedsKey + "'");
    }

    const std::size_t nSeeds = m_impl->copy.get_size(*seedsDevice);

    ::traccc::edm::seed_collection::buffer seedsHostBuffer(
        static_cast<unsigned int>(nSeeds), m_impl->host_mr);
    m_impl->copy.setup(seedsHostBuffer)->ignore();
    m_impl->copy(*seedsDevice, seedsHostBuffer)->wait();
    m_impl->stream.synchronize();

    ::traccc::edm::seed_collection::const_device seedsHost(seedsHostBuffer);
    output.nSeeds = seedsHost.size();

    const ::traccc::edm::spacepoint_collection::buffer* spacepointsDevice =
        nullptr;
    if (m_impl->eventStore
                ->retrieve(spacepointsDevice, keys.spacepointsKey)
                .isFailure() ||
        spacepointsDevice == nullptr) {
        throw std::runtime_error(
            "TracccTritonRunner: could not retrieve spacepoints under '" +
            keys.spacepointsKey + "'");
    }

    ::traccc::edm::spacepoint_collection::buffer spacepointsHostBuffer(
        static_cast<unsigned int>(m_impl->copy.get_size(*spacepointsDevice)),
        m_impl->host_mr);
    m_impl->copy.setup(spacepointsHostBuffer)->ignore();
    m_impl->copy(*spacepointsDevice, spacepointsHostBuffer)->wait();
    m_impl->stream.synchronize();

    ::traccc::edm::spacepoint_collection::const_device spacepointsHost(
        spacepointsHostBuffer);

    // Pack the output tensor
    const std::size_t n = seedsHost.size();
    output.buffer.assign(
        sizeof(std::uint64_t) + n * 9u * sizeof(float), uint8_t{0});

    const std::uint64_t nSeeds64 = static_cast<std::uint64_t>(n);
    std::memcpy(output.buffer.data(), &nSeeds64, sizeof(std::uint64_t));

    if (n != 0u) {
        auto* columns = reinterpret_cast<float*>(
            output.buffer.data() + sizeof(std::uint64_t));
        for (std::size_t i = 0; i < n; ++i) {
            const auto& seed = seedsHost.at(i);
            const auto& bottom = spacepointsHost.at(seed.bottom_index());
            const auto& middle = spacepointsHost.at(seed.middle_index());
            const auto& top = spacepointsHost.at(seed.top_index());

            columns[0 * n + i] = bottom.x();
            columns[1 * n + i] = bottom.y();
            columns[2 * n + i] = bottom.z();
            columns[3 * n + i] = middle.x();
            columns[4 * n + i] = middle.y();
            columns[5 * n + i] = middle.z();
            columns[6 * n + i] = top.x();
            columns[7 * n + i] = top.y();
            columns[8 * n + i] = top.z();
        }
    }

    if (printStats) {
        auto ms = [](auto a, auto b) {
            return std::chrono::duration_cast<std::chrono::milliseconds>(b - a)
                .count();
        };
        std::cout << "[TIMING] Cell deserialization : " << ms(t0, t1) << " ms\n"
                  << "[TIMING] Cell H2D + record    : " << ms(t1, t2) << " ms\n"
                  << "[TIMING] Device chain (3 alg) : " << ms(t2, t3) << " ms\n"
                  << "[TIMING] Seeds: " << output.nSeeds << std::endl;
    }

    return output;
}

}}}  // namespace triton::backend::traccc
