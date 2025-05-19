/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file ITkStripFrontEnd/test/ITkStripAmp_test.cxx
 * @author Alexis Maloizel
 * @date Dec, 2024
 * @brief Some tests for ITkStripAmp
 */
 
 
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE StripDigitizationTest
#include <boost/test/unit_test.hpp>
namespace utf = boost::unit_test;

//
#include "TestTools/initGaudi.h"
#include "InDetSimEvent/SiCharge.h"

#include "src/ITkStripAmp.h"
#include <cmath>
#include <vector>
#include <array>

using namespace std::string_literals; //for 's' suffix

struct TestFixture {
  TestFixture() :tool("ITkStripAmp"s){
    BOOST_TEST( tool.retrieve() );
  }
  ToolHandle<IAmplifier> tool;
};

struct GaudiFixture : public Athena_test::InitGaudi {
  GaudiFixture() :Athena_test::InitGaudi("StripDigitization/ITkStripAmp_test.txt"){
   BOOST_TEST( svcLoc.as<ISvcManager>()->start() );
  }
  ~GaudiFixture() {
    svcLoc.as<ISvcManager>()->stop().ignore();
  }
};
 
BOOST_FIXTURE_TEST_SUITE( StripDigitizationTest, TestFixture,
                          * boost::unit_test::fixture<GaudiFixture>()
                          * boost::unit_test::tolerance(1.e-6) ) 
                          
BOOST_AUTO_TEST_CASE(ToolIsCalledITkStripAmp){
  BOOST_TEST(tool.name() == "ITkStripAmp"s);
}

BOOST_AUTO_TEST_CASE(ToolHandleIsNotNull){
  BOOST_TEST(tool.get() != nullptr);
}

BOOST_AUTO_TEST_CASE(DummyResponseIs1, * utf::tolerance(0.001)){
  const IAmplifier::list_t Charges_dummy;
  const float ToT_dummy = 0.;
  BOOST_TEST(tool->response(Charges_dummy, ToT_dummy) == 1.0);
}

BOOST_AUTO_TEST_CASE(ResponseIsAsExpected, * utf::tolerance(0.001)){
  //Creating a dummy charge for the test
    float charge = 10.0;             
    float time = .5;               
    SiCharge::Process processType = SiCharge::Process::track;
    SiCharge dummyCharge(charge, time, processType);
    std::vector<SiCharge> dummyCharges = {dummyCharge};
    std::array<float, 3> dummyResponse = {0., 0., 0.};
    float dummyTime = 2.;
    float peakTime = 25;
    int bin_max{std::ssize(dummyResponse)};   
    float tp{peakTime / 3.0f};            
    for (const SiCharge& charge : dummyCharges) {
        float ch{static_cast<float>(charge.charge())};         
        float ch_time{static_cast<float>(charge.time())};      
        int bin_end{bin_max - 1};        
        for (int bin{-1}; bin < bin_end; ++bin) {
            float bin_time{dummyTime + bin * 25};      
            float tC{bin_time - ch_time};     
            if (tC > 0.0) {
                tC /= tp;
                dummyResponse[bin + 1] += ch * tC * tC * tC * std::exp(-tC); 
            }
        }
    }
    for (short bin{0}; bin < bin_max; ++bin) {
        dummyResponse[bin] = dummyResponse[bin] * 1.; 
    }
    std::vector<float> response = {0., 0., 0.};
    tool->response(dummyCharges, 2., response);
    BOOST_TEST(response[1] == dummyResponse[1]);
    BOOST_TEST(response[2] == dummyResponse[2]);
}

  
BOOST_AUTO_TEST_SUITE_END()
