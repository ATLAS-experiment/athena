// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#ifndef ATHDEVICEUTILS_HOSTMRANDCPFIXTURE_H
#define ATHDEVICEUTILS_HOSTMRANDCPFIXTURE_H

// Project include(s).
#include "AthDeviceInterfaces/ICopiesTool.h"
#include "AthDeviceInterfaces/IMemoryResourcesTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "TestTools/initGaudi.h"

namespace AthDeviceUtilsTest {

/// Fixture for unit tests that need memory resources and copy objects from
/// tools
struct HostMRAndCpFixture : Athena_test::InitGaudi {

  /// Constructor, setting up the fixture
  HostMRAndCpFixture();
  /// Destructor, tearing down the fixture
  ~HostMRAndCpFixture();

  /// Tool providing memory resources
  ToolHandle<AthDevice::IMemoryResourcesTool> m_mrs;
  /// Tool providing copy objects
  ToolHandle<AthDevice::ICopiesTool> m_copies;

};  // struct HostMRAndCpFixture

}  // namespace AthDeviceUtilsTest

#endif  // ATHDEVICEUTILS_HOSTMRANDCPFIXTURE_H
