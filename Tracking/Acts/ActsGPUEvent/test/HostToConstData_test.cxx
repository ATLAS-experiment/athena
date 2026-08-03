// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

// Local include(s).
#include "ActsGPUEventTestFixture.h"
#include "TestedCollectionTypes.h"

// Boost include(s).
#define BOOST_TEST_MODULE ActsGPUEvent
#include <boost/test/included/unit_test.hpp>

// System include(s).
#include <memory>
#include <memory_resource>

BOOST_FIXTURE_TEST_CASE_TEMPLATE(HostToConstData, COLLECTION,
                                 TestedCollectionTypes,
                                 ActsGPUEventTestFixture) {

  // Type specific name for the recorded object.
  const std::string key = typeid(COLLECTION).name();

  // Create a host object.
  auto host = std::make_unique<typename COLLECTION::host>(
      *(std::pmr::get_default_resource()));

  // Record it into StoreGate.
  constexpr bool allowMods = false;
  BOOST_TEST(m_sg->record(std::move(host), key, allowMods).isSuccess());

  // Try to retrieve it as a const data object.
  const typename COLLECTION::const_data* const_data = nullptr;
  BOOST_TEST(m_sg->retrieve(const_data, key).isSuccess());
}
