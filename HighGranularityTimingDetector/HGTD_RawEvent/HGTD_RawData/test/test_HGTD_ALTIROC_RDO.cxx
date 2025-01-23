/**
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
 *
 * @file HGTD_RawData/test/test_HGTD_ALTIROC_RDO.cxx
 * @author Alexander Leopold <alexander.leopold@cern.ch>
 * @author Rodrigo Estevam de Paula <rodrigo.estevam.de.paula@cern.ch>
 * @date December, 2024
 * @brief Unit test of the HGTD_ALTIROC_RDO class, testing initialisation
 * and all constructors.
 */

#include "HGTD_RawData/HGTD_ALTIROC_RDO.h"
#include "Identifier/Identifier.h"

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

HGTD_ALTIROC_RDO createRDO() {
  std::cout << "createRDO\n";

  Identifier id(5678);

  HGTD_ALTIROC_RDO rdo(id, 0x42855003);

  std::cout << "createRDO done\n";
  return rdo;
}

void compare(const HGTD_ALTIROC_RDO& p1, const HGTD_ALTIROC_RDO& p2) {
  std::cout << "compare HGTD_ALTIROC_RDO\n";
  BOOST_CHECK(p1.identify() == p2.identify());
  BOOST_CHECK(p1.getToA() == p2.getToA());
  BOOST_CHECK(p1.getToT() == p2.getToT());
  BOOST_CHECK(p1.getBCID() == p2.getBCID());
  BOOST_CHECK(p1.getL1ID() == p2.getL1ID());
  BOOST_CHECK(p1.getCRC() == p2.getCRC());
  BOOST_CHECK(p1.getWord() == p2.getWord());
  std::cout << "compare HGTD_ALTIROC_RDO done\n";
}

void testCopyCtor(const HGTD_ALTIROC_RDO& rdo) {
  std::cout << "testCopyCtor\n";
  HGTD_ALTIROC_RDO copied_rdo(rdo);

  compare(rdo, copied_rdo);
  std::cout << "testCopyCtor done\n";
}

void testAssignment(const HGTD_ALTIROC_RDO& rdo) {
  std::cout << "testAssignment\n";
  HGTD_ALTIROC_RDO assigned_rdo = rdo;

  compare(rdo, assigned_rdo);
  std::cout << "testAssignment done\n";
}

void testMoveCtor(HGTD_ALTIROC_RDO rdo) {
  std::cout << "testMoveCtor\n";
  HGTD_ALTIROC_RDO orig_rdo(rdo);
  HGTD_ALTIROC_RDO moved_rdo(std::move(rdo));

  compare(orig_rdo, moved_rdo);
  std::cout << "testMoveCtor done\n";
}

void testMoveAssignment(HGTD_ALTIROC_RDO rdo) {
  std::cout << "testMoveAssignment\n";
  HGTD_ALTIROC_RDO orig_rdo(rdo);
  HGTD_ALTIROC_RDO move_assign_rdo = std::move(rdo);

  compare(orig_rdo, move_assign_rdo);

  std::cout << "testMoveAssignment done\n";
}

BOOST_AUTO_TEST_CASE(HGTD_ALTIROC_RDO_test, *boost::unit_test::tolerance(1e-10)) {

  std::cout << "running test_HGTD_ALTIROC_RDO\n";

  // don't bother with default ctor, should not be used!

  Identifier id(1234);

  HGTD_ALTIROC_RDO rdo(id, 0x42855003);

  //FIXME: Should we test here if the word is equal the joint of other information?
  BOOST_CHECK(rdo.identify() == id);
  BOOST_CHECK(rdo.getToA() == 80);
  BOOST_CHECK(rdo.getToT() == 266);
  BOOST_CHECK(rdo.getBCID() == 1);
  BOOST_CHECK(rdo.getL1ID() == 2);
  BOOST_CHECK(rdo.getCRC() == 3);
  BOOST_CHECK(rdo.getWord() == 1116033027);

  HGTD_ALTIROC_RDO rdo2 = createRDO();

  testCopyCtor(rdo2);

  testAssignment(rdo2);

  testMoveCtor(rdo2);

  testMoveAssignment(rdo2);

  std::cout << "running test_HGTD_ALTIROC_RDO done\n";
}
