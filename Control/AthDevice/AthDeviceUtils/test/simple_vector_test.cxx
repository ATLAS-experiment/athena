// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

// Local include(s).
#include "AthDeviceUtils/SoACopy.h"
#include "HostMRAndCpFixture.h"
#include "VecMemTypes.h"
#include "xAODTypes.h"

// Project include(s).
#include "AthContainers/AuxStoreInternal.h"
#include "AthContainers/DataVector.h"

// Boost include(s).
#define BOOST_TEST_MODULE AthDeviceUtils
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

// Create a test suite with a global fixture that puts Gaudi/Athena in place
BOOST_FIXTURE_TEST_SUITE(SimpleVector, AthDeviceUtilsTest::HostMRAndCpFixture)

BOOST_AUTO_TEST_CASE(HostToDevice) {

  // Set up a dummy xAOD container.
  DataVector<AthDeviceUtilsTest::SimplexAODObject> dv;
  SG::AuxStoreInternal store;
  dv.setStore(&store);

  // Fill it up with some data.
  for (int i = 0; i < 10; ++i) {
    dv.push_back(std::make_unique<AthDeviceUtilsTest::SimplexAODObject>());
    dv.back()->setIntVar(i);
    dv.back()->setFloatVar(i * 0.1f);
  }

  // Set up a dummy VecMem container.
  AthDeviceUtilsTest::SimpleVecMemObjectContainer::host soa{m_mrs->mainMR()};
  soa.resize(dv.size());

  // Set up a copier object.
  static const AthDevice::SoACopy copier(AthDevice::SoAColumn<1>{"floatVar"},
                                         AthDevice::SoAColumn<0>{"intVar"});

  // Copy the data from the xAOD container to the VecMem container.
  auto soaData = vecmem::get_data(soa);
  copier(dv, soaData, EventContext{}, *m_mrs, *m_copies);

  // Compare their contents.
  BOOST_TEST(soa.size() == dv.size());
  for (size_t i = 0; i < dv.size(); ++i) {
    BOOST_TEST(soa[i].get<0>() == dv[i]->intVar());
    BOOST_TEST(soa[i].get<1>() == dv[i]->floatVar());
  }
}

BOOST_AUTO_TEST_CASE(DeviceToHost) {

  // Set up the dummy VecMem container.
  AthDeviceUtilsTest::SimpleVecMemObjectContainer::host soa{m_mrs->mainMR()};

  // Fill it up with some data.
  for (int i = 0; i < 10; ++i) {
    soa.push_back({i, i * 0.1f});
  }

  // Set up a dummy xAOD container.
  DataVector<AthDeviceUtilsTest::SimplexAODObject> dv;
  SG::AuxStoreInternal store;
  dv.setStore(&store);

  // Set up a copier object.
  static const AthDevice::SoACopy copier(AthDevice::SoAColumn<0>{"intVar"},
                                         AthDevice::SoAColumn<1>{"floatVar"});

  // Copy the data from the VecMem container to the xAOD container.
  auto soaData = vecmem::get_data(soa);
  copier(soaData, dv, EventContext{}, *m_mrs, *m_copies);

  // Compare their contents.
  BOOST_TEST(soa.size() == dv.size());
  for (size_t i = 0; i < dv.size(); ++i) {
    BOOST_TEST(soa[i].get<0>() == dv[i]->intVar());
    BOOST_TEST(soa[i].get<1>() == dv[i]->floatVar());
  }
}

BOOST_AUTO_TEST_SUITE_END()
