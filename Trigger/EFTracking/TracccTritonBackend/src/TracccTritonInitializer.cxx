/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file  Trigger/EFTracking/TracccTritonBackend/src/TracccTritonInitializer.cxx
 * @author Miles Cochran-Branson
 * @date September 2026
 * @brief Initialization of GPU tracking algs. for use in Triton as-a-Service implementation
 */

#include "Python.h"

#include "TracccTritonInitializer.h"

#include <dlfcn.h>

#include <atomic>
#include <cstddef>
#include <mutex>
#include <stdexcept>
#include <string>

// Gaudi kernel embedding.
#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/IAppMgrUI.h"
#include "GaudiKernel/IAlgManager.h"
#include "GaudiKernel/IAlgorithm.h"
#include "GaudiKernel/IService.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/StateMachine.h"

#include "ActsGPUEvent/GeometryIdMapping.h"
#include "StoreGate/StoreGateSvc.h"

namespace triton { namespace backend { namespace traccc {

// TracccTritonBootstrap.bootstrap() brings up the Gaudi kernel
// with the device chain configured by TracccTritonDeviceRecoCfg, under these
// fixed algorithm instance names, so the Runner can look them up afterward.
namespace {
    constexpr const char* kClusterizationAlg = "DeviceClusterizationAlg";
    constexpr const char* kSPFormationAlg = "DeviceSPFormationAlg";
    constexpr const char* kTripletSeedingAlg = "DeviceTripletSeedingAlg";
    constexpr const char* kTrkParamEstimationAlg = "DeviceTrkParamEstimationAlg";
    constexpr const char* kTrackFindingAlg = "DeviceTrackFindingAlg";

    void promoteSelfToGlobal() {
        Dl_info info{};
        if (!dladdr(reinterpret_cast<void*>(&promoteSelfToGlobal), &info) ||
            !info.dli_fname) {
            throw std::runtime_error(
                "TracccTritonInitializer: dladdr() could not determine this "
                "library's own path");
        }
        if (!dlopen(info.dli_fname, RTLD_NOW | RTLD_GLOBAL | RTLD_NOLOAD)) {
            throw std::runtime_error(
                std::string("TracccTritonInitializer: failed to promote '") +
                info.dli_fname + "' to RTLD_GLOBAL: " + dlerror());
        }
    }
}  // namespace

struct TracccTritonInitializer::Impl {
    Config config;
    SmartIF<IAppMgrUI> app;
    SmartIF<ISvcLocator> svcLocator;
    const ActsTrk::GeometryIdMapping* geoIdMapping = nullptr;
    bool ready = false;
    /// Next event-store slot to hand out; see acquireSlot().
    std::atomic<std::size_t> nextSlot{0};
    PyThreadState* pyMainThreadState = nullptr;

    /// Look up one of the device algorithms by the name it was created under.
    SmartIF<IAlgorithm> algorithm(const std::string& name) const {
        IAlgManager* algMgr = svcLocator.as<IAlgManager>();
        if (!algMgr) {
            throw std::runtime_error(
                "TracccTritonInitializer: no IAlgManager available");
        }
        SmartIF<IAlgorithm> alg = algMgr->algorithm(name, /*createIf*/ false);
        if (!alg) {
            throw std::runtime_error(
                "TracccTritonInitializer: could not retrieve algorithm '" +
                name + "' from the embedded Gaudi kernel");
        }
        return alg;
    }

    /// Embed a Python interpreter and call TracccTritonBackend.TracccTritonBootstrap.bootstrap()
    void bootstrapPython(std::size_t nSlots) {
        if (!Py_IsInitialized()) {
            if (!dlopen(TRACCC_TRITON_PYTHON_LIBRARY, RTLD_NOW | RTLD_GLOBAL)) {
                throw std::runtime_error(
                    std::string(
                        "TracccTritonInitializer: failed to re-load '"
                        TRACCC_TRITON_PYTHON_LIBRARY "' with RTLD_GLOBAL: ") +
                    dlerror());
            }

            Py_Initialize();
            if (!Py_IsInitialized()) {
                throw std::runtime_error(
                    "TracccTritonInitializer: Py_Initialize() failed");
            }
        }

        PyObject* module =
            PyImport_ImportModule("TracccTritonBackend.TracccTritonBootstrap");
        if (!module) {
            PyErr_Print();
            throw std::runtime_error(
                "TracccTritonInitializer: failed to import "
                "TracccTritonBackend.TracccTritonBootstrap -- check "
                "PYTHONPATH");
        }

        PyObject* func = PyObject_GetAttrString(module, "bootstrap");
        Py_DECREF(module);
        if (!func || !PyCallable_Check(func)) {
            Py_XDECREF(func);
            PyErr_Print();
            throw std::runtime_error(
                "TracccTritonInitializer: TracccTritonBootstrap.bootstrap "
                "not found or not callable");
        }

        PyObject* result =
            PyObject_CallFunction(func, "n", static_cast<Py_ssize_t>(nSlots));
        Py_DECREF(func);
        if (!result) {
            PyErr_Print();
            throw std::runtime_error(
                "TracccTritonInitializer: TracccTritonBootstrap.bootstrap() "
                "raised a Python exception (see stderr above)");
        }
        Py_DECREF(result);

        pyMainThreadState = PyEval_SaveThread();
    }
};

TracccTritonInitializer::TracccTritonInitializer()
    : m_impl(std::make_unique<Impl>()) {}

TracccTritonInitializer::~TracccTritonInitializer() {
    try {
        finalize();
    } catch (...) {
    }
}

TracccTritonInitializer&
TracccTritonInitializer::instance() {
    static TracccTritonInitializer theInstance;
    return theInstance;
}

void
TracccTritonInitializer::initialize(const Config& config) {
    static std::mutex initMutex;
    std::lock_guard<std::mutex> lock(initMutex);

    if (m_impl->ready) {
        // Gaudi is a process singleton; a second initialize() with a
        // different device id cannot be honored.
        if (config.deviceId != m_impl->config.deviceId) {
            throw std::runtime_error(
                "TracccTritonInitializer: already initialized on device " +
                std::to_string(m_impl->config.deviceId) +
                ", cannot re-initialize on device " +
                std::to_string(config.deviceId) +
                ". Pin the model to a single GPU in instance_group.");
        }
        if (config.nSlots != m_impl->config.nSlots) {
            throw std::runtime_error(
                "TracccTritonInitializer: already initialized with " +
                std::to_string(m_impl->config.nSlots) +
                " event slots, cannot re-initialize with " +
                std::to_string(config.nSlots));
        }
        return;
    }

    m_impl->config = config;

    promoteSelfToGlobal();

    m_impl->bootstrapPython(config.nSlots);

    // Fetch the (already-initialized) singleton kernel
    m_impl->app = Gaudi::createApplicationMgr();
    if (!m_impl->app) {
        throw std::runtime_error(
            "TracccTritonInitializer: Gaudi::createApplicationMgr() "
            "returned null after the Python bootstrap");
    }
    if (m_impl->app->FSMState() != Gaudi::StateMachine::INITIALIZED) {
        throw std::runtime_error(
            "TracccTritonInitializer: application manager is not "
            "INITIALIZED after the Python bootstrap (FSM state " +
            std::to_string(static_cast<int>(m_impl->app->FSMState())) + ")");
    }

    m_impl->svcLocator = SmartIF<ISvcLocator>(m_impl->app);
    if (!m_impl->svcLocator) {
        throw std::runtime_error(
            "TracccTritonInitializer: could not obtain ISvcLocator");
    }

    // Start CoreDumpSvc, the one service that needs it.
    if (SmartIF<IService> coreDumpSvc =
            m_impl->svcLocator->service<IService>("CoreDumpSvc",
                                                  /*createIf*/ false)) {
        coreDumpSvc->sysStart().ignore();
    }

    // Fetch the surface id mapping from the DetectorStore.
    SmartIF<StoreGateSvc> detStore =
        m_impl->svcLocator->service<StoreGateSvc>("DetectorStore",
                                                 /*createIf*/ false);
    if (!detStore) {
        throw std::runtime_error(
            "TracccTritonInitializer: DetectorStore not available");
    }
    if (detStore->retrieve(m_impl->geoIdMapping, config.geoIdMappingKey)
                .isFailure() ||
        m_impl->geoIdMapping == nullptr) {
        throw std::runtime_error(
            "TracccTritonInitializer: no ActsTrk::GeometryIdMapping in the "
            "DetectorStore under '" + config.geoIdMappingKey + "'");
    }

    // Sanity-check that every algorithm of the chain exists.
    m_impl->algorithm(kClusterizationAlg);
    m_impl->algorithm(kSPFormationAlg);
    m_impl->algorithm(kTripletSeedingAlg);
    m_impl->algorithm(kTrkParamEstimationAlg);
    m_impl->algorithm(kTrackFindingAlg);

    m_impl->ready = true;
}

void
TracccTritonInitializer::finalize() {
    if (!m_impl->ready) return;

    if (m_impl->svcLocator) {
        if (SmartIF<IService> coreDumpSvc =
                m_impl->svcLocator->service<IService>("CoreDumpSvc",
                                                      /*createIf*/ false)) {
            coreDumpSvc->sysStop().ignore();
        }
    }

    if (m_impl->app) {
        m_impl->app->finalize().ignore();
        m_impl->app->terminate().ignore();
    }

    m_impl->geoIdMapping = nullptr;
    m_impl->svcLocator = nullptr;
    m_impl->app = nullptr;
    m_impl->ready = false;
}

bool
TracccTritonInitializer::isReady() const {
    return m_impl->ready;
}

const TracccTritonInitializer::Config&
TracccTritonInitializer::config() const {
    return m_impl->config;
}

std::size_t
TracccTritonInitializer::acquireSlot() {
    if (!m_impl->ready) {
        throw std::runtime_error(
            "TracccTritonInitializer::acquireSlot: not initialized");
    }
    const std::size_t slot = m_impl->nextSlot.fetch_add(1);
    if (slot >= m_impl->config.nSlots) {
        throw std::runtime_error(
            "TracccTritonInitializer: Triton created more model instances "
            "than the " + std::to_string(m_impl->config.nSlots) +
            " event slot(s) the kernel was brought up with");
    }
    return slot;
}

int
TracccTritonInitializer::deviceId() const {
    return m_impl->config.deviceId;
}

ISvcLocator&
TracccTritonInitializer::serviceLocator() const {
    if (!m_impl->svcLocator) {
        throw std::runtime_error(
            "TracccTritonInitializer::serviceLocator: not initialized");
    }
    return *m_impl->svcLocator;
}

const ActsTrk::GeometryIdMapping&
TracccTritonInitializer::geometryIdMapping() const {
    if (!m_impl->geoIdMapping) {
        throw std::runtime_error(
            "TracccTritonInitializer::geometryIdMapping: not initialized");
    }
    return *m_impl->geoIdMapping;
}

}}}  // namespace triton::backend::traccc
