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
#include <iostream>
#include <mutex>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>

#include <cuda_runtime.h>

// Gaudi / Athena.
#include "AthCUDAInterfaces/IStreamSvc.h"
#include "AthenaKernel/ExtendedEventContext.h"
#include "AthenaKernel/IConditionsCleanerSvc.h"
#include "CxxUtils/checker_macros.h"
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/ThreadLocalContext.h"
#include "GaudiKernel/IAlgExecStateSvc.h"
#include "GaudiKernel/IAlgManager.h"
#include "GaudiKernel/IAlgorithm.h"
#include "GaudiKernel/IHiveWhiteBoard.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/Incident.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/SmartIF.h"
#include "StoreGate/StoreGateSvc.h"

// traccc / vecmem EDM
#include "ActsGPUEvent/TracccSiliconCellCollection.h"
#include "TracccTritonClient/TracccEdmSerialization.h"

#include "vecmem/memory/cuda/device_memory_resource.hpp"
#include "vecmem/memory/host_memory_resource.hpp"
#include "vecmem/utils/copy.hpp"
#include "vecmem/utils/cuda/async_copy.hpp"

namespace triton { namespace backend { namespace traccc {

namespace {
    /// Run bookkeeping shared by all runners, since BeginRun is seen
    /// by the whole application. The mutex also serializes the conditions sequence
    struct SharedRunState {
        std::mutex mutex;
        std::optional<EventIDBase::number_type> currentRun;
    };
    SharedRunState sharedRunState ATLAS_THREAD_SAFE;

    /// Device-to-host copy of a collection with jagged vectors
    template <typename DEVICE_BUFFER, typename HOST_CONTAINER>
    void copyJaggedToHost(const vecmem::copy& deviceCopy,
                          const DEVICE_BUFFER& device, HOST_CONTAINER& host,
                          vecmem::memory_resource& host_mr) {
        auto staged = deviceCopy.to(vecmem::get_data(device), host_mr, nullptr,
                                    vecmem::copy::type::device_to_host);
        vecmem::copy{}(vecmem::get_data(staged), host,
                       vecmem::copy::type::host_to_host)->wait();
    }
}  // namespace

struct TracccTritonRunner::Impl {

    TracccTritonInitializer& initializer;

    vecmem::host_memory_resource host_mr;
    vecmem::cuda::device_memory_resource device_mr;
    /// The stream the device chain runs on for this instance owned by the stream service
    cudaStream_t stream = nullptr;
    std::optional<vecmem::cuda::async_copy> copy;

    ServiceHandle<StoreGateSvc> eventStore;
    SmartIF<IHiveWhiteBoard> whiteboard;

    const std::size_t slot;
    std::size_t eventCounter = 0;

    /// The AthSequencer configured by TracccTritonBootstrap.bootstrap()
    SmartIF<IAlgorithm> chain;
    SmartIF<IAlgorithm> condChain;
    SmartIF<IAlgExecStateSvc> algExecStates;
    SmartIF<IIncidentSvc> incidentSvc;
    SmartIF<Athena::IConditionsCleanerSvc> conditionsCleaner;

    Impl(TracccTritonInitializer& init, std::size_t theSlot)
        : initializer(init)
        , host_mr()
        , device_mr(init.deviceId())
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

    // The device chain takes its stream from this service, one per slot
    SmartIF<AthCUDA::IStreamSvc> streamSvc =
        svcLocator->service<AthCUDA::IStreamSvc>(
            initializer.config().streamSvcName, /*createIf*/ false);
    if (!streamSvc) {
        throw std::runtime_error(
            "TracccTritonRunner: could not retrieve the CUDA stream service '" +
            initializer.config().streamSvcName +
            "'; TracccTritonBootstrap.py has to set "
            "flags.CUDA.Stream = CUDAStream.PerEvent");
    }
    m_impl->stream = static_cast<cudaStream_t>(
        streamSvc->stream(EventContext(0, slot)));
    m_impl->copy.emplace(m_impl->stream);

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
    m_impl->condChain = algMgr->algorithm(initializer.config().condSequenceName,
                                          /*createIf*/ false);

    m_impl->incidentSvc =
        svcLocator->service<IIncidentSvc>("IncidentSvc", /*createIf*/ false);
    if (!m_impl->incidentSvc) {
        throw std::runtime_error( 
            "TracccTritonRunner: could not retrieve IncidentSvc");
    }

    // Drops conditions objects no longer needed by any event, as the event
    // loop manager does at the start of each event; without it a long-running server
    // keeps every IOV it has ever seen
    if (m_impl->condChain) {
        m_impl->conditionsCleaner =
            svcLocator->service<Athena::IConditionsCleanerSvc>(
                "Athena::ConditionsCleanerSvc", /*createIf*/ true);
        if (!m_impl->conditionsCleaner) {
            throw std::runtime_error(
                "TracccTritonRunner: could not retrieve "
                "Athena::ConditionsCleanerSvc");
        }
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

void TracccTritonRunner::run(const uint8_t* buffer, std::size_t byteSize,
                             const EventIDBase& eventID,
                             EventIDBase::number_type conditionsRun,
                             bool printStats, Output& output) {
    const auto& keys = m_impl->initializer.config();

    auto t0 = std::chrono::high_resolution_clock::now();

    // Deserialize the CELLS tensor into a host cell collection
    ::traccc::edm::silicon_cell_collection::host cells(m_impl->host_mr);
    if (buffer == nullptr) {
        throw std::runtime_error("TracccTritonRunner: no CELLS buffer");
    }
    TracccTriton::deserialize(buffer, byteSize, cells);

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

    // Conditions handles are resolved against the event's IOV, so the
    // context carries the client event's id and conditions run, as the
    // event loop manager would set them up
    const std::size_t eventNumber = m_impl->eventCounter++;
    EventContext ctx(eventNumber, m_impl->slot);
    EventIDBase ctxEventID(eventID);
    ctxEventID.set_event_number(eventNumber);
    ctx.setEventID(ctxEventID);
    ctx.setExtension(Atlas::ExtendedEventContext(
        m_impl->eventStore->hiveProxyDict(), conditionsRun));
    Gaudi::Hive::setCurrentContext(ctx);
    {
        auto deviceCells =
            std::make_unique<::traccc::edm::silicon_cell_collection::buffer>(
                static_cast<unsigned int>(cells.size()), m_impl->device_mr);
        m_impl->copy->setup(*deviceCells)->ignore();
        (*m_impl->copy)(vecmem::get_data(cells), *deviceCells)->wait();

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

    if (m_impl->condChain) {
        std::lock_guard<std::mutex> lock(sharedRunState.mutex);

        // IOVDbSvc resolves conditions folders at the time it is given on
        // BeginRun (there is no BeginEvent/EventInfo here), so fire it
        // whenever the run changes, as the event loop manager does
        const EventIDBase::number_type run = ctx.eventID().run_number();
        if (sharedRunState.currentRun != run) {
            sharedRunState.currentRun = run;
            m_impl->incidentSvc->fireIncident(
                Incident("TracccTritonRunner", IncidentType::BeginRun, ctx));
        }

        // Drop the conditions objects no event in flight needs any more,
        // as the event loop manager does at the start of each event.
        // Synchronously, since there is no scheduler to run it in a task.
        if (m_impl->conditionsCleaner->event(ctx, /*allowAsync*/ false)
                .isFailure()) {
            throw std::runtime_error(
                "TracccTritonRunner: conditions cleaning failed");
        }

        // Make the conditions for this IOV available; the conditions algs
        // skip themselves once their object is valid for the IOV, so this is
        // cheap for every request after the first one of an IOV
        if (m_impl->condChain->sysExecute(ctx).isFailure()) {
            std::ostringstream states;
            m_impl->algExecStates->dump(states, ctx);
            throw std::runtime_error(
                "TracccTritonRunner: conditions sequence '" +
                m_impl->condChain->name() +
                "' failed during execute(). Algorithm states:\n" + states.str());
        }
    }

    if (m_impl->chain->sysExecute(ctx).isFailure()) {
        std::ostringstream states;
        m_impl->algExecStates->dump(states, ctx);
        throw std::runtime_error(
            "TracccTritonRunner: device chain '" + m_impl->chain->name() +
            "' failed during execute(). Algorithm states:\n" + states.str());
    }
    // The algorithms return with work still queued on the stream
    if (cudaStreamSynchronize(m_impl->stream) != cudaSuccess) {
        throw std::runtime_error(
            "TracccTritonRunner: cudaStreamSynchronize failed after the "
            "device chain");
    }

    auto t3 = std::chrono::high_resolution_clock::now();

    // Copy the fitted tracks and the measurements their states point at back to the host
    const traccc_track_container::buffer* tracksDevice = nullptr;
    if (m_impl->eventStore->retrieve(tracksDevice, keys.tracksKey).isFailure() ||
        tracksDevice == nullptr) {
        throw std::runtime_error(
            "TracccTritonRunner: could not retrieve tracks under '" +
            keys.tracksKey + "'");
    }

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

    const ::traccc::edm::silicon_cluster_collection::buffer* clustersDevice =
        nullptr;
    if (m_impl->eventStore->retrieve(clustersDevice, keys.clustersKey)
                .isFailure() ||
        clustersDevice == nullptr) {
        throw std::runtime_error(
            "TracccTritonRunner: could not retrieve clusters under '" +
            keys.clustersKey + "'");
    }

    const vecmem::copy& copy = *m_impl->copy;
    copyJaggedToHost(copy, tracksDevice->tracks, output.tracks.tracks,
                     m_impl->host_mr);
    copy(tracksDevice->states, output.tracks.states,
         vecmem::copy::type::device_to_host)->wait();
    copy(*measurementsDevice, output.measurements,
         vecmem::copy::type::device_to_host)->wait();
    copyJaggedToHost(copy, *clustersDevice, output.clusters, m_impl->host_mr);

    auto t4 = std::chrono::high_resolution_clock::now();

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
                  << tag << "Output D2H           : " << ms(t3, t4) << " ms\n"
                  << tag << "Tracks: " << output.tracks.tracks.size()
                  << std::endl;
    }
}

}}}  // namespace triton::backend::traccc
