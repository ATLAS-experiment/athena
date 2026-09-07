// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#ifndef TRACCC_TRITON_INITIALIZER_H
#define TRACCC_TRITON_INITIALIZER_H

#include <memory>
#include <string>
#include <unordered_map>

#include "Identifier/Identifier.h"

class ISvcLocator;

namespace ActsTrk {
class IDeviceDetectorDescriptionProviderSvc;
}  // namespace ActsTrk

namespace triton { namespace backend { namespace traccc {

/// @class TracccTritonInitializer
///
/// @brief Owns the embedded Gaudi/Athena kernel that backs the Traccc Triton
///        backend, plus the process-wide device infrastructure it brings up.
///
/// The device reconstruction algorithms
/// (@c ActsTrk::DeviceClusterizationAlg, @c ActsTrk::DeviceSPFormationAlg and
/// @c ActsTrk::DeviceTripletSeedingAlg) are ordinary Gaudi
/// @c AthReentrantAlgorithm components: they read/write StoreGate and depend
/// on provider tools and DetectorStore services. They therefore cannot be
/// constructed as plain C++ objects; a Gaudi kernel has to be running.
///
/// This class boots that kernel exactly once per process (Gaudi's @c ToolSvc,
/// @c DetectorStore and the two device-description provider services are all
/// singletons, so a second kernel in the same process is not possible). The
/// Triton model must consequently be configured with
/// @c instance_group { count: 1 }.
///
/// Bringing the kernel up requires resolving the same @c ComponentAccumulator
/// wiring (~20 Gaudi properties across the device algorithms, their provider
/// tools and the memory-resource/copy/stream tools underneath) that
/// @c TracccTritonDeviceRecoCfg produces for a normal athena job. Rather than
/// re-deriving that wiring as literal C++ property-setting calls -- which
/// would silently drift from the real Athena defaults -- this class embeds a
/// Python interpreter and calls @c TracccTritonBootstrap.bootstrap()
/// (python/TracccTritonBootstrap.py), which builds the real
/// ComponentAccumulator and drives it up through
/// @c ApplicationMgr.initialize() (mirroring @c ComponentAccumulator.run(),
/// stopping short of the event-loop-only @c start()/run()/stop() calls --
/// see @c AtlasTest/TestTools/src/initGaudi.cxx for the equivalent minimal
/// C++ bootstrap, which likewise never calls @c start() before algorithms
/// have @c sysExecute() called on them directly). Because the
/// @c ApplicationMgr is a process singleton, this class then retrieves the
/// very same kernel instance Python just configured via
/// @c Gaudi::createApplicationMgr() on the C++ side.
///
/// Everything heavy (Gaudi, Python, vecmem, traccc, detray) is hidden behind
/// a pimpl, so this header stays cheap to include from the backend
/// translation unit.
class TracccTritonInitializer {
public:
    /// @brief Construction parameters, normally read from the model config.
    struct Config {
        /// CUDA device id this instance is pinned to.
        int deviceId = 0;
        /// StoreGate keys shared with @c TracccTritonRunner. These must match
        /// the values used in @c TracccTritonDeviceRecoCfg.
        std::string cellsKey = "TracccTritonCells";
        std::string measurementsKey = "TracccTritonMeasurements";
        std::string spacepointsKey = "TracccTritonSpacepoints";
        std::string seedsKey = "TracccTritonSeeds";
    };

    /// Get the process-wide singleton.
    static TracccTritonInitializer& instance();

    /// Boot the Gaudi kernel and resolve all cached handles. Idempotent and
    /// thread-safe: subsequent calls with the same config are no-ops.
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

    /// The cached detray -> Athena identifier map, owned by the
    /// @c IDeviceDetectorDescriptionProviderSvc brought up during init.
    const std::unordered_map<uint64_t, Identifier>& detrayToAthenaMap() const;

    /// The detector description provider service (never null once ready).
    const ActsTrk::IDeviceDetectorDescriptionProviderSvc& detectorService() const;

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
