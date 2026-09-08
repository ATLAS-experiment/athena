/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file  Trigger/EFTracking/TracccTritonBackend/src/TracccTritonInitializer.hpp
 * @author Miles Cochran-Branson
 * @date September 2026
 * @brief Initialization of GPU tracking algs. for use in Triton as-a-Service implementation
 */

#ifndef TRACCC_TRITON_INITIALIZER_H
#define TRACCC_TRITON_INITIALIZER_H

#include <cstddef>
#include <memory>
#include <string>

#include "ActsGPUEvent/GeometryIdMapping.h"

class ISvcLocator;

namespace triton { namespace backend { namespace traccc {

/// @class TracccTritonInitializer
///
/// @brief Owns the embedded Gaudi/Athena kernel that backs the Traccc Triton
///        backend, plus the process-wide device infrastructure it brings up.
///
/// The device reconstruction algorithms
/// (@c ActsTrk::DeviceClusterizationAlg, @c ActsTrk::DeviceSPFormationAlg,
/// @c ActsTrk::DeviceTripletSeedingAlg, @c ActsTrk::DeviceTrkParamEstimationAlg
/// and @c ActsTrk::DeviceTrackFindingAlg) are ordinary Gaudi
/// @c AthReentrantAlgorithm components: they read/write StoreGate and depend
/// on provider tools and DetectorStore services. They therefore cannot be
/// constructed as plain C++ objects; a Gaudi kernel has to be running.
class TracccTritonInitializer {
public:
    /// @brief Construction parameters, normally read from the model config.
    struct Config {
        /// CUDA device id this instance is pinned to.
        int deviceId = 0;
        /// Number of event-store slots to create, one per Triton model instance
        std::size_t nSlots = 1;
        /// StoreGate keys shared with @c TracccTritonRunner. These must match
        /// the values used in @c TracccTritonDeviceRecoCfg.
        std::string cellsKey = "TracccTritonCells";
        std::string measurementsKey = "TracccTritonMeasurements";
        std::string spacepointsKey = "TracccTritonSpacepoints";
        std::string seedsKey = "TracccTritonSeeds";
        std::string trkParamsKey = "TracccTritonTrackParameters";
        std::string tracksKey = "TracccTritonTracks";
        std::string geoIdMappingKey = "TracccGeometryIdMapping";
    };

    /// Get the process-wide singleton.
    static TracccTritonInitializer& instance();

    /// Boot the Gaudi kernel and resolve all cached handles.
    /// @throws std::runtime_error on any failure to bring the kernel up.
    void initialize(const Config& config);

    /// Tear the kernel down. Safe to call when not initialized.
    void finalize();

    /// Whether @c initialize has completed successfully.
    bool isReady() const;

    /// The construction parameters the kernel was brought up with.
    const Config& config() const;

    /// The CUDA device id this instance is pinned to.
    int deviceId() const;

    /// The service locator of the embedded Gaudi kernel (never null once
    /// ready). Used by @c TracccTritonRunner to look up the device algorithms.
    ISvcLocator& serviceLocator() const;

    const ActsTrk::GeometryIdMapping& geometryIdMapping() const;

    /// Claim the next free event-store slot for a Triton model instance.
    /// @throws std::runtime_error if more slots are claimed than the kernel
    ///         was brought up with.
    std::size_t acquireSlot();

    ~TracccTritonInitializer();

    TracccTritonInitializer(const TracccTritonInitializer&) = delete;
    TracccTritonInitializer& operator=(const TracccTritonInitializer&) = delete;

private:
    TracccTritonInitializer();

    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

}}}  // namespace triton::backend::traccc

#endif  // TRACCC_TRITON_INITIALIZER_H
