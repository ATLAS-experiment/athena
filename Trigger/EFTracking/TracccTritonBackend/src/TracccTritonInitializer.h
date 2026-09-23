/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file  Trigger/EFTracking/TracccTritonBackend/src/TracccTritonInitializer.h
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
/// @brief Owns the embedded Athena application that backs the Traccc Triton
///        backend, plus the process-wide device infrastructure it brings up.
///
/// The device reconstruction algorithms are ordinary Gaudi
/// @c AthReentrantAlgorithm components: they read/write StoreGate and depend
/// on provider tools and DetectorStore services. They therefore cannot be
/// constructed as plain C++ objects; an initialized Athena application
/// has to exist.
///
/// The python configuration schedules them inside a single @c AthSequencer
/// named @c Config::sequenceName, which is what @c TracccTritonRunner
/// executes; the composition and ordering of the chain therefore live
/// entirely in @c TracccTritonDeviceRecoCfg.
class TracccTritonInitializer {
public:
    /// @brief Construction parameters, normally read from the model config,
    ///        plus the StoreGate keys @c initialize() fills in from the
    ///        python configuration.
    struct Config {
        /// CUDA device id this instance is pinned to.
        int deviceId = 0;
        /// Number of event-store slots to create, one per Triton model instance
        std::size_t nSlots = 1;
        /// Name of the @c AthSequencer holding the per-request device chain.
        std::string sequenceName = "TracccDeviceRecoSeq";
        /// @name StoreGate keys, filled in by @c initialize().
        /// intermediate keys of the algorithm are not needed
        std::string cellsKey;
        std::string measurementsKey;
        std::string tracksKey;
        std::string geoIdMappingKey;
        /// @}
    };

    /// Get the process-wide singleton.
    static TracccTritonInitializer& instance();

    /// Boot the embedded Athena application and resolve all cached handles.
    /// @throws std::runtime_error on any failure to bring it up.
    void initialize(const Config& config);

    /// Tear the application down. Safe to call when not initialized.
    void finalize();

    /// Whether @c initialize has completed successfully.
    bool isReady() const;

    /// The construction parameters the application was configured with.
    const Config& config() const;

    /// The CUDA device id this instance is pinned to.
    int deviceId() const;

    /// The service locator of the embedded Athena application (never null
    /// once ready). Used by @c TracccTritonRunner to look up the device chain.
    ISvcLocator& serviceLocator() const;

    const ActsTrk::GeometryIdMapping& geometryIdMapping() const;

    /// Claim the next free event-store slot for a Triton model instance.
    /// @throws std::runtime_error if more slots are claimed than the
    ///         application was configured with.
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
