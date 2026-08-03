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

BOOST_FIXTURE_TEST_CASE_TEMPLATE(HostToConstView, COLLECTION,
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

  // Try to retrieve it as a const view object.
  const typename COLLECTION::const_view* const_view = nullptr;
  BOOST_TEST_WARN(m_sg->retrieve(const_view, key).isSuccess(),
                  "Host to const view conversion not working");
}
