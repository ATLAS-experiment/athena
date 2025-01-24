/**
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
 *
 * @file HGTD_EventTPCnv/test/HGTD_ALTIROC_RDO_Cnv_p1_test.cxx
 * @author Alexander Leopold <alexander.leopold@cern.ch>
 * @brief Tests the conversion between transient and persistent version of the
 *        HGTD_ALTIROC_RDO class.
 */

#include "GaudiKernel/MsgStream.h"
#include "HGTD_EventTPCnv/HGTD_ALTIROC_RDO_Cnv_p1.h"

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

HGTD_ALTIROC_RDO createRDO(int id, uint64_t word ) {
  std::cout << "createRDO\n";

  Identifier identifier(id);

  return HGTD_ALTIROC_RDO(identifier, word);
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

void convertAndBack(const HGTD_ALTIROC_RDO& trans1) {

  std::cout << "convertAndBack\n";

  MsgStream log(nullptr, "test");
  HGTD_ALTIROC_RDO_Cnv_p1 cnv;
  HGTD_ALTIROC_RDO_p1 pers{};

  cnv.transToPers(&trans1, &pers, log);

  HGTD_ALTIROC_RDO trans2;
  cnv.persToTrans(&pers, &trans2, log);

  compare(trans1, trans2);
  std::cout << "convertAndBack done\n";
}

BOOST_AUTO_TEST_CASE(HGTD_ALTIROC_RDO_Cnv_p1_test) {

  std::cout << "start test\n";

  HGTD_ALTIROC_RDO rdo = createRDO(1234, 0x42855003);

  convertAndBack(rdo);
}
