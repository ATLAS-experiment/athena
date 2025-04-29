/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_READOUTGEOMETRYBASE

#include "CxxUtils/checker_macros.h"
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

#include <boost/test/unit_test.hpp>

#include "ReadoutGeometryBase/SiCellId.h"

using namespace InDetDD;

BOOST_AUTO_TEST_SUITE(SiCellIdTest)
  BOOST_AUTO_TEST_CASE(Construction){
    BOOST_CHECK_NO_THROW([[maybe_unused]] SiCellId s);//default
    BOOST_CHECK_NO_THROW([[maybe_unused]] SiCellId t(20));//construction with int
    BOOST_CHECK_NO_THROW([[maybe_unused]] SiCellId u(1,-2));//construction with phi,eta indices
    //implicitly defined copy constructor
    SiCellId v(1,-2);
    BOOST_CHECK_NO_THROW([[maybe_unused]]SiCellId w(v));
    //implicitly defined move constructor
    BOOST_CHECK_NO_THROW([[maybe_unused]]SiCellId x(std::move(v)));
  }
  BOOST_AUTO_TEST_CASE(Validity){
    SiCellId s;
    BOOST_TEST(not s.isValid(), "default constructed object is not valid");
    SiCellId t(20);
    BOOST_TEST(t.isValid(), "strip (int) constructed object is valid");
    SiCellId u(1,-2);
    BOOST_TEST(u.isValid(), "phi(int), eta(int) constructed object is valid");
  }
  
  BOOST_AUTO_TEST_CASE(Accessors){
    SiCellId defaultConstructed;
    BOOST_TEST(defaultConstructed.strip() == -32768);
    BOOST_TEST(defaultConstructed.etaIndex() == -16384);
    BOOST_TEST(defaultConstructed.word() == 3221258240);
    //
    SiCellId stripConstructed(20);
    BOOST_TEST(stripConstructed.strip() == 20);
    BOOST_TEST(stripConstructed.etaIndex() == 0);
    BOOST_TEST(stripConstructed.word() == 20);
    //
    SiCellId etaphiConstructed(1,-2);
    BOOST_TEST(etaphiConstructed.strip() == 1);
    BOOST_TEST(etaphiConstructed.phiIndex() == 1);
    BOOST_TEST(etaphiConstructed.etaIndex() == -2);
    BOOST_TEST(etaphiConstructed.word() == 2147352577);
  }
  BOOST_AUTO_TEST_CASE(Comparison){
    SiCellId defaultConstructed;
    BOOST_TEST(defaultConstructed == SiCellId());
    //
    SiCellId stripConstructed(20);
    SiCellId differentStrip(21);
    BOOST_TEST(stripConstructed == SiCellId(20));
    BOOST_TEST(stripConstructed != differentStrip);
    //
    SiCellId loStrip(20);
    SiCellId hiStrip(200);
    BOOST_TEST(loStrip < hiStrip);
    //BOOST_TEST(hiStrip > loStrip); will not compile, same for >= or <=
    //possibly undesirable effect of not making the int constructor explicit?
    SiCellId strip20(20);
    BOOST_TEST(strip20 == 20, "SiCellId can be equality compared with an int");
  }
  BOOST_AUTO_TEST_CASE(Assignment){
    SiCellId copyAssign;
    SiCellId original(1,-2);
    BOOST_CHECK_NO_THROW(copyAssign = original);
    BOOST_TEST(copyAssign == original);
    SiCellId moveAssign;
    BOOST_CHECK_NO_THROW(moveAssign = std::move(original));
    BOOST_TEST(moveAssign == copyAssign);
    //possibly undesirable effect of not making the int constructor explicit?
    SiCellId strip19;
    BOOST_CHECK_NO_THROW(strip19 = 19);
    BOOST_TEST(strip19 == 19, "SiCellId can be assigned to an int and then compared");
  }
  BOOST_AUTO_TEST_CASE(StreamInsertion){
    SiCellId etaphiConstructed(1,-2);
    std::stringstream streamInserted;
    streamInserted<<etaphiConstructed;
    BOOST_TEST(streamInserted.str() == "[1.-2]");
    SiCellId defaultConstructed;
    std::stringstream invalidInserted;
    invalidInserted<<defaultConstructed;
    BOOST_TEST(invalidInserted.str() == "[INVALID]");
  }
BOOST_AUTO_TEST_SUITE_END()