/**
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
 *
 * @file HGTD_RawData/test/test_HGTD_ALTIROC_RDO_Collection.cxx
 * @author Alexander Leopold <alexander.leopold@cern.ch>
 * @author Rodrigo Estevam de Paula <rodrigo.estevam.de.paula@cern.ch>

 * @brief Unit test of the HGTD_ALTIROC_RDO_Collection class.
 */

#include "HGTD_RawData/HGTD_ALTIROC_RDO.h"
#include "HGTD_RawData/HGTD_ALTIROC_RDO_Collection.h"
#include "Identifier/Identifier.h"
#include "Identifier/IdentifierHash.h"

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

BOOST_AUTO_TEST_CASE(HGTD_ALTIROC_RDO_Collection_test) {
  // create a collection
  auto coll = std::make_unique<HGTD_ALTIROC_RDO_Collection>(IdentifierHash(2));
  // fill it with RDOs
  for (unsigned int id = 1234; id < 1244; id++) {
    std::unique_ptr<HGTD_ALTIROC_RDO> rdo = createRDO(id, 0x42855003);
    coll->push_back(std::move(rdo));
  }

  BOOST_CHECK(coll->identifierHash() == IdentifierHash(2));
  BOOST_CHECK(coll->size() == 10);
}
