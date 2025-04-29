/**
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
 *
 * @file HGTD_RawData/test/test_HGTD_ALTIROC_RDO_Container.cxx
 * @author Alexander Leopold <alexander.leopold@cern.ch>
 * @author Rodrigo Estevam de Paula <rodrigo.estevam.de.paula@cern.ch>
 * @brief Unit test of the HGTD_ALTIROC_RDO_Container class.
 */

#include "HGTD_RawData/HGTD_ALTIROC_RDO.h"
#include "HGTD_RawData/HGTD_ALTIROC_RDO_Collection.h"
#include "HGTD_RawData/HGTD_ALTIROC_RDO_Container.h"
#include "Identifier/Identifier.h"
#include "Identifier/IdentifierHash.h"
#include "SGTools/TestStore.h"
#include "StoreGate/StoreGateSvc.h"
#include "TestTools/initGaudi.h"

#include <memory>
#include <vector>

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

std::unique_ptr<HGTD_ALTIROC_RDO> createRDO(unsigned int id, uint64_t word) {
  std::cout << "createRDO\n";

  Identifier identifier(id);

  return std::make_unique<HGTD_ALTIROC_RDO>(identifier, word);
}

BOOST_AUTO_TEST_CASE(HGTD_ALTIROC_RDO_Container_test) {

  // initialise Gaudi for testing
  ISvcLocator* pSvcLoc;
  BOOST_REQUIRE(Athena_test::initGaudi("test_HGTD_ALTIROC_RDO_Container.txt", pSvcLoc));

  ISvcLocator* svc_locator = Gaudi::svcLocator();
  // get StoreGate service

  SmartIF<StoreGateSvc> storegate{svc_locator->service("StoreGateSvc")};
  if (!storegate) {
    BOOST_TEST(false);
  }

  auto container = std::make_unique<HGTD_ALTIROC_RDO_Container>(5);

  for (unsigned int hash = 2; hash <= 3; hash++) {
    // create a collection
    auto collection =
      std::make_unique<HGTD_ALTIROC_RDO_Collection>(IdentifierHash(hash));
    // fill it with RDOs
    for (unsigned int id = 1234; id < 1244; id++) {
      std::unique_ptr<HGTD_ALTIROC_RDO> rdo =
          createRDO(id, 0x42855003);
      collection->push_back(std::move(rdo));
    }

    BOOST_CHECK(collection->identifierHash() == IdentifierHash(hash));

    StatusCode sc = container->addCollection(collection.release(), hash);
    if (sc.isFailure()) {
      std::cout << "Could not add HGTD_ALTIROC_RDO_Collection to the HGTD_ALTIROC_RDO_Container!\n";
      BOOST_TEST(false);
    }
  }

  // Get the sct helper from the detector store
  StatusCode sc = storegate->record(container.release(), "HGTD_ALTIROC_RDO_Container");
  if (sc.isFailure()) {
    std::cout << "Could not record the HGTD_ALTIROC_RDO_Container!\n";
    BOOST_TEST(false);
  }
}
