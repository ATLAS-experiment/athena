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
#include "ActsGPUEvent/TracccTrackContainer.h"

namespace triton { namespace backend { namespace traccc {

class TracccTritonInitializer;

/// @class TracccTritonRunner
///
/// @brief Drives the device reconstruction chain for a single Triton request.

class TracccTritonRunner {
public:
    /// @brief The host-resident reconstruction result of one request.
    ///
    /// The @c const_device views handed out below alias the buffers held
    /// here, so they must not outlive the @c Output object.
    struct Output {

        traccc_track_container::buffer tracks;
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

    /// @c TracccTritonInitializer::acquireSlot().
    /// @throws std::runtime_error if the initializer is not ready or @p slot
    ///         is not one of the slots the application was configured with.
    TracccTritonRunner(TracccTritonInitializer& initializer, std::size_t slot);

    ~TracccTritonRunner();

    TracccTritonRunner(const TracccTritonRunner&) = delete;
    TracccTritonRunner& operator=(const TracccTritonRunner&) = delete;

    /// Run the full chain for one request.
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
