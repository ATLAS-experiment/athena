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

template <typename T1, typename T2, typename ALLOC2>
bool isEqual(const vecmem::device_vector<T1>& lhs,
             const std::vector<T2, ALLOC2>& rhs) {

  if (lhs.size() != rhs.size()) {
    return false;
  }
  for (std::size_t i = 0; i < lhs.size(); ++i) {
    if (lhs[i] != rhs[i]) {
      return false;
    }
  }
  return true;
}

template <typename T1, typename ALLOC1, typename T2, typename ALLOC2>
bool isEqual(const std::vector<T1, ALLOC1>& lhs,
             const std::vector<T2, ALLOC2>& rhs) {

  if (lhs.size() != rhs.size()) {
    return false;
  }
  for (std::size_t i = 0; i < lhs.size(); ++i) {
    if (lhs[i] != rhs[i]) {
      return false;
    }
  }
  return true;
}

// Create a test suite with a global fixture that puts Gaudi/Athena in place
BOOST_FIXTURE_TEST_SUITE(JaggedVector, AthDeviceUtilsTest::HostMRAndCpFixture)

BOOST_AUTO_TEST_CASE(HostToDevice) {

  // Set up a dummy xAOD container.
  DataVector<AthDeviceUtilsTest::JaggedxAODObject> dv;
  SG::AuxStoreInternal store;
  dv.setStore(&store);

  // Fill it up with some data.
  for (int i = 0; i < 10; ++i) {
    dv.push_back(std::make_unique<AthDeviceUtilsTest::JaggedxAODObject>());
    dv.back()->setIntVar(i);
    dv.back()->setFloatVar(i * 0.1f);
    dv.back()->setUnsignedVectorVar({i * 2u, i * 3u, i * 4u});
    dv.back()->setDoubleVectorVar({i * 4., i * 5., i * 6.});
  }

  // Set up a dummy VecMem container.
  AthDeviceUtilsTest::JaggedVecMemObjectContainer::buffer soa;

  // Set up a copier object.
  static const AthDevice::SoACopy copier(
      AthDevice::SoAColumn<1>{"floatVar"}, AthDevice::SoAColumn<0>{"intVar"},
      AthDevice::SoAColumn<2>{"unsignedVectorVar"},
      AthDevice::SoAColumn<4>{"doubleVectorVar"});

  // Copy the data from the xAOD container to the VecMem container.
  copier(dv, soa, EventContext{}, *m_mrs, *m_copies);

  // Set up an object to investigate the VecMem container with.
  const AthDeviceUtilsTest::JaggedVecMemObjectContainer::const_device soaDevice{
      soa};

  // Compare their contents.
  BOOST_TEST(soaDevice.size() == dv.size());
  for (size_t i = 0; i < dv.size(); ++i) {
    BOOST_TEST(soaDevice[i].get<0>() == dv[i]->intVar());
    BOOST_TEST(soaDevice[i].get<1>() == dv[i]->floatVar());
    BOOST_TEST(isEqual(soaDevice[i].get<2>(), dv[i]->unsignedVectorVar()));
    BOOST_TEST(isEqual(soaDevice[i].get<4>(), dv[i]->doubleVectorVar()));
  }
}

BOOST_AUTO_TEST_CASE(DeviceToHost) {

  // Set up the dummy VecMem container.
  AthDeviceUtilsTest::JaggedVecMemObjectContainer::host soa{m_mrs->mainMR()};

  // Fill it up with some data.
  for (int i = 0; i < 10; ++i) {
    soa.push_back({i,
                   i * 0.1f,
                   {i * 2u, i * 3u, i * 4u},
                   0.0f,
                   {i * 4., i * 5., i * 6.}});
  }

  // Set up a dummy xAOD container.
  DataVector<AthDeviceUtilsTest::JaggedxAODObject> dv;
  SG::AuxStoreInternal store;
  dv.setStore(&store);

  // Set up a copier object.
  static const AthDevice::SoACopy copier(
      AthDevice::SoAColumn<0>{"intVar"},
      AthDevice::SoAColumn<4>{"doubleVectorVar"},
      AthDevice::SoAColumn<1>{"floatVar"},
      AthDevice::SoAColumn<2>{"unsignedVectorVar"});

  // Copy the data from the VecMem container to the xAOD container.
  copier(vecmem::get_data(soa), dv, EventContext{}, *m_mrs, *m_copies);

  // Compare their contents.
  BOOST_TEST(soa.size() == dv.size());
  for (size_t i = 0; i < dv.size(); ++i) {
    BOOST_TEST(soa[i].get<0>() == dv[i]->intVar());
    BOOST_TEST(soa[i].get<1>() == dv[i]->floatVar());
    BOOST_TEST(isEqual(soa[i].get<2>(), dv[i]->unsignedVectorVar()));
    BOOST_TEST(isEqual(soa[i].get<4>(), dv[i]->doubleVectorVar()));
  }
}

BOOST_AUTO_TEST_SUITE_END()
