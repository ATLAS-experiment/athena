/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file  Trigger/EFTracking/TracccTritonBackend/src/TracccTritonRunner.cxx
 * @author Miles Cochran-Branson
 * @date September 2026
 * @brief Call the device reconstruction algs. once per-event in the Triton backend
 */

#include "TracccTritonRunner.h"

#include "TracccTritonInitializer.h"

#include <chrono>
#include <cstring>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

#include <cuda_runtime.h>

// Gaudi / Athena.
#include "AthenaKernel/ExtendedEventContext.h"
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/ThreadLocalContext.h"
#include "GaudiKernel/IAlgExecStateSvc.h"
#include "GaudiKernel/IAlgManager.h"
#include "GaudiKernel/IAlgorithm.h"
#include "GaudiKernel/IHiveWhiteBoard.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/SmartIF.h"
#include "StoreGate/StoreGateSvc.h"

// traccc / vecmem EDM
#include "ActsGPUEvent/TracccSiliconCellCollection.h"

#include "vecmem/memory/cuda/device_memory_resource.hpp"
#include "vecmem/memory/host_memory_resource.hpp"
#include "vecmem/utils/cuda/async_copy.hpp"

namespace triton { namespace backend { namespace traccc {

struct StreamGuard {
    cudaStream_t stream = nullptr;
    StreamGuard() { cudaStreamCreate(&stream); }
    ~StreamGuard() {
        if (stream) cudaStreamDestroy(stream);
    }
    StreamGuard(const StreamGuard&) = delete;
    StreamGuard& operator=(const StreamGuard&) = delete;
    void synchronize() {
        if (stream) cudaStreamSynchronize(stream);
    }
};

struct TracccTritonRunner::Impl {

    TracccTritonInitializer& initializer;

    vecmem::host_memory_resource host_mr;
    vecmem::cuda::device_memory_resource device_mr;
    StreamGuard stream;
    vecmem::cuda::async_copy copy;

    ServiceHandle<StoreGateSvc> eventStore;
    SmartIF<IHiveWhiteBoard> whiteboard;

    const std::size_t slot;
    std::size_t eventCounter = 0;

    /// The AthSequencer configured by TracccTritonDeviceRecoCfg
    SmartIF<IAlgorithm> chain;
    SmartIF<IAlgExecStateSvc> algExecStates;

    Impl(TracccTritonInitializer& init, std::size_t theSlot)
        : initializer(init)
        , host_mr()
        , device_mr(init.deviceId())
        , stream()
        , copy(stream.stream)
        , eventStore("StoreGateSvc", "TracccTritonRunner")
        , slot(theSlot) {}
};

TracccTritonRunner::TracccTritonRunner(TracccTritonInitializer& initializer,
                                       std::size_t slot)
    : m_impl(std::make_unique<Impl>(initializer, slot)) {

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

    // plumbing to allow multi-model-inst. per GPU
    m_impl->whiteboard =
        svcLocator->service<IHiveWhiteBoard>("EventDataSvc", /*createIf*/ false);
    if (!m_impl->whiteboard) {
        throw std::runtime_error(
            "TracccTritonRunner: EventDataSvc is not an IHiveWhiteBoard");
    }
    if (slot >= m_impl->whiteboard->getNumberOfStores()) {
        throw std::runtime_error(
            "TracccTritonRunner: slot " + std::to_string(slot) +
            " is out of range, the whiteboard has only " +
            std::to_string(m_impl->whiteboard->getNumberOfStores()) +
            " store(s)");
    }

    IAlgManager* algMgr = svcLocator.as<IAlgManager>();
    if (!algMgr) {
        throw std::runtime_error(
            "TracccTritonRunner: no IAlgManager in the embedded application");
    }
    m_impl->chain = algMgr->algorithm(initializer.config().sequenceName,
                                      /*createIf*/ false);
    if (!m_impl->chain) {
        throw std::runtime_error(
            "TracccTritonRunner: could not retrieve the device chain '" +
            initializer.config().sequenceName +
            "' from the embedded Athena application");
    }

    // AthSequencer::execute() skips itself if its AlgExecState for this
    // context is already Done, so the states have to be reset once per
    // request the way AthenaEventLoopMgr does between events
    m_impl->algExecStates = svcLocator->service<IAlgExecStateSvc>(
        "AlgExecStateSvc", /*createIf*/ false);
    if (!m_impl->algExecStates) {
        throw std::runtime_error(
            "TracccTritonRunner: could not retrieve AlgExecStateSvc");
    }
}

TracccTritonRunner::~TracccTritonRunner() = default;

TracccTritonRunner::Output TracccTritonRunner::run(const uint8_t* buffer,
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

    if (m_impl->whiteboard->selectStore(m_impl->slot).isFailure()) {
        throw std::runtime_error(
            "TracccTritonRunner: could not select store for slot " +
            std::to_string(m_impl->slot));
    }

    // Copy cells to the device and record them in StoreGate
    if (m_impl->eventStore->clearStore().isFailure()) {
        throw std::runtime_error("TracccTritonRunner: clearStore() failed");
    }

    EventContext ctx(m_impl->eventCounter++, m_impl->slot);
    ctx.setExtension(
        Atlas::ExtendedEventContext(m_impl->eventStore->hiveProxyDict()));
    Gaudi::Hive::setCurrentContext(ctx);
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

    // Run the device chain as the python configuration declared it.
    // Clear the previous request's execution states first, otherwise the
    // sequencer sees itself as Done and silently runs nothing.
    m_impl->algExecStates->reset(ctx);

    if (m_impl->chain->sysExecute(ctx).isFailure()) {
        std::ostringstream states;
        m_impl->algExecStates->dump(states, ctx);
        throw std::runtime_error(
            "TracccTritonRunner: device chain '" + m_impl->chain->name() +
            "' failed during execute(). Algorithm states:\n" + states.str());
    }
    m_impl->stream.synchronize();

    auto t3 = std::chrono::high_resolution_clock::now();

    // Copy the fitted tracks and the measurements their states point at back to the host
    const traccc_track_container::buffer* tracksDevice = nullptr;
    if (m_impl->eventStore->retrieve(tracksDevice, keys.tracksKey).isFailure() ||
        tracksDevice == nullptr) {
        throw std::runtime_error(
            "TracccTritonRunner: could not retrieve tracks under '" +
            keys.tracksKey + "'");
    }

    output.tracks.tracks =
        m_impl->copy.to(tracksDevice->tracks, m_impl->host_mr, nullptr,
                        vecmem::copy::type::device_to_host);
    output.tracks.states =
        m_impl->copy.to(tracksDevice->states, m_impl->host_mr, nullptr,
                        vecmem::copy::type::device_to_host);

    const ::traccc::edm::measurement_collection::buffer* measurementsDevice =
        nullptr;
    if (m_impl->eventStore
                ->retrieve(measurementsDevice, keys.measurementsKey)
                .isFailure() ||
        measurementsDevice == nullptr) {
        throw std::runtime_error(
            "TracccTritonRunner: could not retrieve measurements under '" +
            keys.measurementsKey + "'");
    }

    output.measurements =
        m_impl->copy.to(*measurementsDevice, m_impl->host_mr, nullptr,
                        vecmem::copy::type::device_to_host);
    m_impl->stream.synchronize();

    output.nTracks = output.tracksAndStates().tracks.size();

    if (printStats) {
        auto ms = [](auto a, auto b) {
            return std::chrono::duration_cast<std::chrono::milliseconds>(b - a)
                .count();
        };
        
        const std::string tag =
            "[TIMING][slot " + std::to_string(m_impl->slot) + "] ";
        std::cout << tag << "Cell deserialization : " << ms(t0, t1) << " ms\n"
                  << tag << "Cell H2D + record    : " << ms(t1, t2) << " ms\n"
                  << tag << "Device chain         : " << ms(t2, t3) << " ms\n"
                  << tag << "Tracks: " << output.nTracks << std::endl;
    }

    return output;
}

}}}  // namespace triton::backend::traccc
