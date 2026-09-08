// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#ifndef ACTSGPUEVENT_ACTSGPUEVENTTESTFIXTURE_H
#define ACTSGPUEVENT_ACTSGPUEVENTTESTFIXTURE_H

// Framework include(s).
#include "CxxUtils/checker_macros.h"
#include "StoreGate/StoreGateSvc.h"
#include "TestTools/initGaudi.h"

// System include(s).
#include <stdexcept>

/// Global service locator for the test(s)
static ISvcLocator* g_svcLoc ATLAS_THREAD_SAFE = nullptr;

/// Fixture for performing initialization for the test(s)
struct ActsGPUEventTestFixture {

  void setup() {

    // Initialize the Gaudi framework.
    if (g_svcLoc == nullptr) {
      if (!Athena_test::initGaudi("ActsGPUEvent/ActsGPUEvent_test.txt",
                                  g_svcLoc)) {
        throw std::runtime_error("Failed to initialize Gaudi");
      }
    }

    // Initialize the StoreGate service.
    m_sg = SmartIF<StoreGateSvc>(g_svcLoc->service("StoreGateSvc"));
  }

  /// StoreGate instance to run tests with.
  SmartIF<StoreGateSvc> m_sg;
};

#endif  // ACTSGPUEVENT_ACTSGPUEVENTTESTFIXTURE_H
