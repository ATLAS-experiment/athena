// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
// Python.h must be included first: it fixes up some standard headers on
// certain platforms and CPython insists on being included before anything
// else that might set feature-test macros it depends on.
#include "Python.h"

#include "TracccTritonInitializer.hpp"

#include <dlfcn.h>

#include <mutex>
#include <stdexcept>
#include <string>

#ifndef TRACCC_TRITON_PYTHON_LIBRARY
#error "TRACCC_TRITON_PYTHON_LIBRARY must be defined by CMakeLists.txt"
#endif

// Gaudi kernel embedding.
#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/IAppMgrUI.h"
#include "GaudiKernel/IAlgManager.h"
#include "GaudiKernel/IAlgorithm.h"
#include "GaudiKernel/IService.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/StateMachine.h"

// The detray/ACTS/Athena surface id mapping the detector description service
// records to the DetectorStore.
#include "ActsGPUEvent/GeometryIdMapping.h"
#include "StoreGate/StoreGateSvc.h"

namespace triton { namespace backend { namespace traccc {

// TracccTritonBootstrap.bootstrap() (see python/) brings up the Gaudi kernel
// with the device chain configured by TracccTritonDeviceRecoCfg, under these
// fixed algorithm instance names, so the Runner can look them up afterward.
namespace {
constexpr const char* kClusterizationAlg = "DeviceClusterizationAlg";
constexpr const char* kSPFormationAlg = "DeviceSPFormationAlg";
constexpr const char* kTripletSeedingAlg = "DeviceTripletSeedingAlg";
constexpr const char* kTrkParamEstimationAlg = "DeviceTrkParamEstimationAlg";
constexpr const char* kTrackFindingAlg = "DeviceTrackFindingAlg";

/// Triton dlopen()s this backend with RTLD_LOCAL. For any C++ class we
/// only ever see through a header-only/INTERFACE library (e.g.
/// ActsTrk::GeometryIdMapping, declared in the header-only part of
/// ActsGPUEventLib) but that is *also* compiled into
/// another, separately dlopen()'d Gaudi component library (via
/// Gaudi::PluginService -- the actual device-description service
/// implementation lives in one), RTLD_LOCAL can in principle leave the two
/// DSOs' copies of that class's vtable/RTTI unmerged, so a dynamic_cast or
/// StoreGate CLID lookup between them would silently fail despite being
/// nominally "the same" C++ class -- the C++-type analogue of the
/// libpython/RTLD_GLOBAL issue handled in bootstrapPython() below.
/// Promoting our own already-loaded module to RTLD_GLOBAL (re-dlopen with
/// RTLD_NOLOAD, which only changes the load flags rather than loading a
/// second copy) is a defensive fix for that scenario; it is cheap and
/// harmless to keep even though the one occurrence of this symptom we hit
/// in practice turned out to be a stale prebuilt dependency (a package
/// this backend depends on via ActsGPUInterfaces/ActsGPUEvent was being
/// pulled from an out-of-date nightly binary instead of being rebuilt
/// locally -- see package_filters.txt and the README).
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
    // Set once bootstrapPython() hands the GIL back; used to reacquire it
    // (if ever needed) during finalize(). Never set back to nullptr: we
    // intentionally never call Py_Finalize(), see finalize() below.
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

    /// Embed a Python interpreter (if not already done) and call
    /// TracccTritonBackend.TracccTritonBootstrap.bootstrap(), which builds
    /// and initializes the real ComponentAccumulator-configured Gaudi
    /// kernel. Throws std::runtime_error with the Python traceback printed
    /// to stderr on any failure.
    void bootstrapPython() {
        if (!Py_IsInitialized()) {
            // Triton dlopen()s this backend with RTLD_LOCAL, so libpython's
            // symbols are not visible process-wide. CPython's own compiled
            // C-extension modules (e.g. lib-dynload/_opcode...so, imported
            // transitively by AthenaConfiguration) are loaded through a
            // *nested* dlopen() and need those symbols (PyExc_RuntimeError,
            // etc.) already visible -- otherwise that nested dlopen fails
            // with "undefined symbol". Re-opening libpython here with
            // RTLD_GLOBAL, before Py_Initialize(), fixes that: this is the
            // standard workaround for embedding CPython inside a library
            // that is itself loaded as a plugin rather than linked into the
            // main executable.
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

        PyObject* result = PyObject_CallObject(func, nullptr);
        Py_DECREF(func);
        if (!result) {
            PyErr_Print();
            throw std::runtime_error(
                "TracccTritonInitializer: TracccTritonBootstrap.bootstrap() "
                "raised a Python exception (see stderr above)");
        }
        Py_DECREF(result);

        // Nothing else in this process calls back into Python (the device
        // algorithms are plain C++ AthReentrantAlgorithms), so release the
        // GIL rather than holding it for the life of the process -- some
        // component further down the Gaudi/ROOT stack may legitimately want
        // it on another thread.
        pyMainThreadState = PyEval_SaveThread();
    }
};

TracccTritonInitializer::TracccTritonInitializer()
    : m_impl(std::make_unique<Impl>()) {}

TracccTritonInitializer::~TracccTritonInitializer() {
    // Best-effort teardown; Triton may finalize the backend out of order.
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
        // different device id cannot be honoured.
        if (config.deviceId != m_impl->config.deviceId) {
            throw std::runtime_error(
                "TracccTritonInitializer: already initialized on device " +
                std::to_string(m_impl->config.deviceId) +
                ", cannot re-initialize on device " +
                std::to_string(config.deviceId) +
                ". Set instance_group { count: 1 } in the model config.");
        }
        return;
    }

    m_impl->config = config;

    // ---- 0. Fix up this DSO's own load flags (see promoteSelfToGlobal). ----
    promoteSelfToGlobal();

    // ---- 1. Let ComponentAccumulator build and initialize the kernel. ----
    // TracccTritonBootstrap.bootstrap() does the equivalent of
    // Gaudi::createApplicationMgr() + property injection (via
    // ComponentAccumulator.createApp(), which resolves the full provider
    // tool / memory-resource / copy / stream tool tree) + configure() +
    // initialize(), all from Python -- see TracccTritonInitializer.hpp for
    // why this isn't hand-rolled in C++.
    m_impl->bootstrapPython();

    // ---- 2. Fetch the (already-initialized) singleton kernel. ----
    // Gaudi::createApplicationMgr() returns the existing process-wide
    // ApplicationMgr instance if one already exists rather than creating a
    // second one, so this retrieves the very same kernel Python just
    // configured and initialized.
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

    // ---- 3. Fetch the surface id mapping from the DetectorStore. ----
    // JSONDeviceDetectorDescriptionProviderSvc records it there during its
    // own initialize(), which the Python bootstrap has already run.
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

    // ---- 4. Sanity-check that every algorithm of the chain exists. ----
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

    if (m_impl->app) {
        m_impl->app->finalize().ignore();
        m_impl->app->terminate().ignore();
    }
    // Release in dependency order: the DetectorStore-owned mapping first, then
    // the locator, then the application manager itself (SmartIF owns the
    // refcount).
    m_impl->geoIdMapping = nullptr;
    m_impl->svcLocator = nullptr;
    m_impl->app = nullptr;
    m_impl->ready = false;

    // Deliberately not calling Py_Finalize(): Gaudi/ROOT components created
    // via the Python bootstrap may still hold Python-side state, and
    // finalizing the interpreter while a C++ singleton could still reference
    // it is a well-known source of shutdown crashes. The interpreter is
    // simply leaked for the remaining life of the process, same as the GIL
    // release in bootstrapPython() -- neither matters once the process exits.
}

bool
TracccTritonInitializer::isReady() const {
    return m_impl->ready;
}

const TracccTritonInitializer::Config&
TracccTritonInitializer::config() const {
    return m_impl->config;
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
