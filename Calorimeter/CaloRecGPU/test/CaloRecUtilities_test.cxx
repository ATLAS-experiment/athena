/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file CaloRecGPU/test/CaloRecUtilities_test.cxx
 * @author Shaun Roe
 * @date December, 2025
 */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_CALORECGPU

#include <boost/test/unit_test.hpp>
#include "CxxUtils/checker_macros.h"
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;
//
#include "../src/CaloRecUtilities.h"
#include <vector>
#include <string>
#include <map>
using namespace CaloRecGPU;
using std::string_literals::operator""s;

BOOST_AUTO_TEST_SUITE(UtilitiesTest)
 BOOST_AUTO_TEST_CASE(protect_from_zero_test){
    const float nonzero_f(3.5f);
    const float zero_f(0.0f);
    BOOST_CHECK(protect_from_zero(nonzero_f) == nonzero_f);
    BOOST_CHECK(protect_from_zero(zero_f) != zero_f);
    const double nonzero_d(3.6);
    const double zero_d(0.0);
    BOOST_CHECK(protect_from_zero(nonzero_d) == nonzero_d);
    BOOST_CHECK(protect_from_zero(zero_d) != zero_d);
 }
 BOOST_AUTO_TEST_CASE(float_unhack_test){
    const float twopoint5_f(2.5f);
    const unsigned twopoint5_bits{1075838976};
    BOOST_CHECK(float_unhack(twopoint5_bits) == twopoint5_f);
 }
 
 BOOST_AUTO_TEST_CASE(apply_to_multi_class_test){
 
 //utility 'contains': true if 'str' contains 'search'
 auto contains = [](const std::string & str, const std::string & search)->bool{
   return (str.find(search)!=std::string::npos);
 };
 //Mixing up Cell/Cluster properties here,but this isn't important for the test
 //two types with static method "name()"
 struct Property1{
   static std::string name() {return "property1";};
   static std::string get_property(float f){return "prop1 "s+std::to_string(f);}
 };
 struct Property2{
   static std::string name() {return "property2";};
   static std::string get_property(float f){return "prop2 "s+std::to_string(f);}
 };
 //
 using  BasicCellProperties = multi_class_holder <Property1, Property2 >;
 //example lambda used in CaloGPUClusterAndCellDataMonitor.cxx
 auto search_lambda = [&](const auto & prop, const size_t count, bool & check,
    const std::string & str, std::vector<bool> & to_do, const std::string & prefix = "", 
    const std::string & suffix = ""){
    if (contains(str, prefix + prop.name() + suffix)){
      to_do[count] = true;
      check = true;
    }
  };
  bool found{};
  std::vector<bool> clusterPropertiesToDo(2,false);
  std::string str{"myHisto_property1"};
  apply_to_multi_class(search_lambda, BasicCellProperties{}, found, str, clusterPropertiesToDo, "_");
  BOOST_CHECK(clusterPropertiesToDo.size() == 2);
  std::vector<bool> expected{true, false};
  BOOST_TEST(clusterPropertiesToDo == expected);
  BOOST_CHECK(found);
  //run following test after first one, to set up clusterPropertiesToDo
  std::map<std::string, std::string> cluster_properties;
  //another example, similar to CaloGPUClusterAndCellDataMonitor.cxx#1689
  float f{345.9f};
  apply_to_multi_class([&](const auto & prop, const size_t i){
    if (clusterPropertiesToDo[i]){
      //modified from original
      cluster_properties[prop.name()]=prop.get_property(f);
    }
    }, BasicCellProperties{});
    BOOST_TEST_MESSAGE(cluster_properties["property1"]);
    BOOST_TEST(cluster_properties.size() == 1);
  }

BOOST_AUTO_TEST_SUITE_END()

