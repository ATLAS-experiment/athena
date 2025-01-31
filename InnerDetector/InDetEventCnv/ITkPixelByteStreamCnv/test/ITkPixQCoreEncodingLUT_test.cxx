/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
/*
 */
/**
 * @file ITkPixelRodDecoder_test.cxx
 * @author Shaun Roe
 * @date July, 2024
 * @brief Some tests for functions in ITkPixelQCoreEncodingLUT.h 
 */
 
 
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_ITKPIXELBYTESTREAMCNV

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>
#pragma GCC diagnostic pop
#include <iostream>
#include "src/ITkPixQCoreEncodingLUT.h"
using namespace ITkPixEncoding;


BOOST_AUTO_TEST_SUITE(Test_ITkPixQCoreEncodingLUT)
BOOST_AUTO_TEST_CASE(encode_test){
    //encode some well behaved random integer
    uint32_t encoded{};
    uint32_t decoded = 34;
    uint32_t length = encode(decoded, encoded);
    BOOST_TEST(length == 10);
    BOOST_TEST(encoded == 780140544);
    //encode zero
    decoded = 0;
    length = encode(decoded, encoded);
    BOOST_TEST(length == 0);
    BOOST_TEST(encoded == 0);
    //encode 0xFFFFFFFF
    decoded = 0xFFFFFFFF;
    length = encode(decoded, encoded);
    BOOST_TEST(length == 30);
    BOOST_TEST(encoded == 1073741823);
}

BOOST_AUTO_TEST_CASE(create_lut_encode_test){
  const auto & lenResult  = create_lut_encode(true);
  BOOST_TEST(lenResult[0] == 0);
  BOOST_TEST(lenResult[1] == 8);
  const auto & encodeResult= create_lut_encode(false) ;
  BOOST_TEST(encodeResult[0] == 0);
  BOOST_TEST(encodeResult[1] == 713031680);
}


BOOST_AUTO_TEST_CASE(check_LUT){
  const auto & lenLUT = ITkPixV2QCoreEncodingLUT_Length;
  const auto & encodeLUT =  ITkPixV2QCoreEncodingLUT_Tree;
  BOOST_TEST(lenLUT[0] == 0);
  BOOST_TEST(lenLUT[1] == 8);
  BOOST_TEST(encodeLUT[0] == 0);
  BOOST_TEST(encodeLUT[1] == 170);
}

BOOST_AUTO_TEST_CASE(one_bit_test){
  BOOST_TEST(one_bit(0) == 0);
  BOOST_TEST(one_bit(1) == 1);
  BOOST_TEST(one_bit(-1) == 1);
}

BOOST_AUTO_TEST_CASE(prepByte_test){
  constexpr std::array<uint32_t, 8> expectedResultFor0{};
  constexpr std::array<uint32_t, 8> expectedResultForFFFF{3,3,3,3,3,3,3,3};
  constexpr std::array<uint32_t, 8> expectedResultFor5{2,2,0,0,0,0,0,0};
  //
  BOOST_TEST(prepByte(5) == expectedResultFor5);
  BOOST_TEST(prepByte(0) == expectedResultFor0);
  BOOST_TEST(prepByte(0xFFFF) == expectedResultForFFFF);
}

BOOST_AUTO_TEST_SUITE_END()

