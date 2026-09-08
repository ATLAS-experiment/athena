// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#ifndef TRACCC_TRITON_RUNNER_H
#define TRACCC_TRITON_RUNNER_H

#include <cstddef>
#include <cstdint>
#include <memory>

#include "ActsGPUEvent/TracccMeasurementCollection.h"
#include "ActsGPUEvent/TracccTrackContainer.h"

namespace triton { namespace backend { namespace traccc {

class TracccTritonInitializer;

/// @class TracccTritonRunner
///
/// @brief Drives the device reconstruction chain for a single Triton request.
///
/// Per call to @c run() the following happens:
///
///   1. the raw @c CELLS byte buffer is deserialized into a host
///      @c traccc::edm::silicon_cell_collection;
///   2. it is copied to the device and recorded in StoreGate under the key the
///      clusterization algorithm reads;
///   3. the device algorithms (clusterization -> spacepoint formation ->
///      triplet seeding -> track parameter estimation -> track finding) are
///      executed in order on a fresh @c EventContext;
///   4. the resulting track container (tracks + track states) and the
///      measurement collection its states point at are copied back to the
///      host and handed to the caller, which turns them into the
///      TRK_PARAMS / MEASUREMENTS / COVARIANCES / GEOMETRY_IDS output
///      tensors expected by @c TracccTritonClient.
///
/// The runner holds no GPU state of its own: memory resources, copies and
/// streams all come from the embedded Gaudi kernel owned by
/// @c TracccTritonInitializer.
class TracccTritonRunner {
public:
    /// @brief The host-resident reconstruction result of one request.
    ///
    /// The @c const_device views handed out below alias the buffers held
    /// here, so they must not outlive the @c Output object.
    struct Output {
        /// Fitted tracks and their track states.
        traccc_track_container::buffer tracks;
        /// The measurements the track states reference by index.
        ::traccc::edm::measurement_collection::buffer measurements;

        /// Number of tracks found (before any quality selection).
        std::size_t nTracks = 0;

        traccc_track_container::const_device tracksAndStates() const {
            return traccc_track_container::const_device{
                traccc_track_container::const_view{tracks}};
        }
        ::traccc::edm::measurement_collection::const_device
        measurementCollection() const {
            return ::traccc::edm::measurement_collection::const_device{
                measurements};
        }
    };

    /// Construct a runner bound to an already-initialized kernel.
    /// @throws std::runtime_error if the initializer is not ready.
    explicit TracccTritonRunner(TracccTritonInitializer& initializer);

    ~TracccTritonRunner();

    TracccTritonRunner(const TracccTritonRunner&) = delete;
    TracccTritonRunner& operator=(const TracccTritonRunner&) = delete;

    /// Run the full chain for one request.
    ///
    /// The @p buffer is the raw @c CELLS tensor gathered by
    /// BackendInputCollector. Its byte layout (native endianness, length
    /// 8 + 20*N) is:
    ///   offset 0      : uint64_t N              (cell count)
    ///   then 5 column blocks of N entries each:
    ///     channel0 (u32), channel1 (u32), activation (f32),
    ///     time (f32), module_index (u32)
    /// which mirrors @c TritonTracccTrackMaker::serializeCells on the client.
    ///
    /// @param buffer     pointer to the gathered CELLS byte buffer.
    /// @param byteSize   size of @p buffer in bytes.
    /// @param printStats emit per-stage timing to stdout.
    /// @throws std::runtime_error on any deserialization, algorithm or copy
    ///         failure.
    Output run(const uint8_t* buffer, std::size_t byteSize, bool printStats);

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

}}}  // namespace triton::backend::traccc

#endif  // TRACCC_TRITON_RUNNER_H
