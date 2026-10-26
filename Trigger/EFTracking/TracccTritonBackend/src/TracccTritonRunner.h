/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file  Trigger/EFTracking/TracccTritonBackend/src/TracccTritonRunner.h
 * @author Miles Cochran-Branson
 * @date September 2026
 * @brief Call the device reconstruction algs. once per-event in the Triton backend
 */

#ifndef TRACCC_TRITON_RUNNER_H
#define TRACCC_TRITON_RUNNER_H

#include <cstddef>
#include <cstdint>
#include <memory>

#include "ActsGPUEvent/TracccMeasurementCollection.h"
#include "ActsGPUEvent/TracccSiliconClusterCollection.h"
#include "ActsGPUEvent/TracccTrackContainer.h"

#include "GaudiKernel/EventIDBase.h"
#include "vecmem/memory/host_memory_resource.hpp"

namespace triton { namespace backend { namespace traccc {

class TracccTritonInitializer;

/// @class TracccTritonRunner
///
/// @brief Drives the device reconstruction chain for a single Triton request.

class TracccTritonRunner {
public:
    /// @brief The host-resident reconstruction result of one request: the
    ///        track container, the measurements its states index into, and
    ///        the clusters (cell indices) those measurements were built from.
    ///
    /// Owns the memory resource its containers allocate from, so it is
    /// filled in place by @c run() rather than returned by value.
    struct Output {
        Output() = default;
        Output(const Output&) = delete;
        Output& operator=(const Output&) = delete;

        vecmem::host_memory_resource mr;
        ::traccc::edm::measurement_collection::host measurements{mr};
        ::traccc::edm::silicon_cluster_collection::host clusters{mr};
        traccc_track_container::host tracks{mr};
    };

    /// @c TracccTritonInitializer::acquireSlot().
    /// @throws std::runtime_error if the initializer is not ready or @p slot
    ///         is not one of the slots the application was configured with.
    TracccTritonRunner(TracccTritonInitializer& initializer, std::size_t slot);

    ~TracccTritonRunner();

    TracccTritonRunner(const TracccTritonRunner&) = delete;
    TracccTritonRunner& operator=(const TracccTritonRunner&) = delete;

    /// Run the full chain for one request.
    ///
    /// @param buffer     pointer to the gathered CELLS bytes, a serialized
    ///                   traccc silicon_cell_collection (TracccEdmSerialization.h)
    /// @param byteSize   size of @p buffer in bytes.
    /// @param eventID    the client event's id (run, lumiblock, time stamp),
    ///                   which the conditions are looked up for
    /// @param conditionsRun run number override for the conditions (MC)
    /// @param printStats emit per-stage timing to stdout.
    /// @param output     filled with the result
    /// @throws std::runtime_error on any deserialization, algorithm or copy
    ///         failure.
    void run(const uint8_t* buffer, std::size_t byteSize,
             const EventIDBase& eventID, EventIDBase::number_type conditionsRun,
             bool printStats, Output& output);

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

}}}  // namespace triton::backend::traccc

#endif  // TRACCC_TRITON_RUNNER_H
