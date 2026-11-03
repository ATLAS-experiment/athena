// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

// Local include(s).
#include "HostMRAndCpFixture.h"

// Framework include(s).
#include "GaudiKernel/System.h"

// Boost include(s).
#include <boost/test/unit_test.hpp>

namespace AthDeviceUtilsTest {

HostMRAndCpFixture::HostMRAndCpFixture()
    : Athena_test::InitGaudi(System::getEnv("ATHDEVICEUTILS_TEST_JOBOPTIONS")),
      m_mrs("AthDevice::MemoryResourcesAdaptorTool/HostMemoryResources"),
      m_copies("AthDevice::CopiesAdaptorTool/HostCopies") {

  BOOST_TEST(m_mrs.retrieve().isSuccess());
  BOOST_TEST(m_copies.retrieve().isSuccess());
}

HostMRAndCpFixture::~HostMRAndCpFixture() = default;

}  // namespace AthDeviceUtilsTest
