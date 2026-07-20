// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

// Local include(s).
#include "ActsGPUEventTestFixture.h"
#include "TestedCollectionTypes.h"

// Boost include(s).
#define BOOST_TEST_MODULE ActsGPUEvent
#include <boost/test/included/unit_test.hpp>

// System include(s).
#include <memory>

BOOST_FIXTURE_TEST_CASE_TEMPLATE(DataToConstView, COLLECTION,
                                 TestedCollectionTypes,
                                 ActsGPUEventTestFixture) {

  // Type specific name for the recorded object.
  const std::string key = typeid(COLLECTION).name();

  // Create a data object.
  auto data = std::make_unique<typename COLLECTION::data>();

  // Record it into StoreGate.
  constexpr bool allowMods = false;
  BOOST_TEST(m_sg->record(std::move(data), key, allowMods).isSuccess());

  // Try to retrieve it as a const view object.
  const typename COLLECTION::const_view* const_view = nullptr;
  BOOST_TEST(m_sg->retrieve(const_view, key).isSuccess());
}
