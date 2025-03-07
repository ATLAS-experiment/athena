/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
/*
 */
/**
 * @file IOVDbSvc/test/Json2Cool_test.cxx
 * @author Shaun Roe
 * @date May, 2019
 * @brief Some tests for Json2Cool class in the Boost framework
 */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_IOVDBSVC


#include <boost/test/unit_test.hpp>

#include <boost/test/tools/output_test_stream.hpp>

#include "../src/Json2Cool.h"
#include "../src/BasicFolder.h"

#include "CoralBase/AttributeList.h"
#include "CoralBase/AttributeListSpecification.h"
#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeSpecification.h"
#include "CoolKernel/StorageType.h"
//
#include "CoolKernel/RecordSpecification.h"
#include "CoolKernel/Record.h"

#include "CxxUtils/checker_macros.h"
#include "CxxUtils/base64.h"

#include <istream>
#include <string>
#include <sstream>

using namespace std::string_literals;
using namespace IOVDbNamespace;
using namespace cool;
const auto larJson=R"foo({"data":{"0":["[DB=8C26DCEB-1065-E011-8322-00145EDD7651][CNT=CollectionTree(CaloRec::CaloCellPositionShift/LArCellPositionShift)][CLID=3B3CCC72-7238-468E-B25E-6F85BA5C9D64][TECH=00000202][OID=00000003-00000000]"]}})foo";
const std::string spec="[{\"PoolRef\":\"String4k\"}]";

BOOST_AUTO_TEST_SUITE(Json2CoolTest)
  BOOST_AUTO_TEST_CASE(Constructor){
    BasicFolder b;
    std::istringstream initializerStream(larJson);
    BOOST_CHECK_NO_THROW(Json2Cool j(initializerStream,b, spec));
    BOOST_CHECK(b.empty() == false);
  }
  BOOST_AUTO_TEST_CASE(convertedProperties){
    auto *pSpec=new coral::AttributeListSpecification;
    pSpec->extend<std::string>("PoolRef");
    coral::AttributeList attrList(*pSpec, true);
    //MUST use string literal suffix 's' to set the value to a string
    attrList[0].setValue("[DB=8C26DCEB-1065-E011-8322-00145EDD7651][CNT=CollectionTree(CaloRec::CaloCellPositionShift/LArCellPositionShift)][CLID=3B3CCC72-7238-468E-B25E-6F85BA5C9D64][TECH=00000202][OID=00000003-00000000]"s);
    BasicFolder b;
    std::istringstream initializerStream(larJson);
    BOOST_CHECK_NO_THROW(Json2Cool j(initializerStream,b, spec));
    BOOST_CHECK(b.getPayload(0) == attrList);
    const std::pair<cool::ValidityKey, cool::ValidityKey> refIov(0, 9223372036854775807);
    BOOST_CHECK(b.iov() == refIov);
  }
  
  BOOST_AUTO_TEST_CASE(parsePayloadSpec){
    const std::string testSpecString="[{\"crate\":\"UChar\"},{\"ROB\":\"Int32\"},{\"BCIDOffset\":\"Int16\"},{\"AName\":\"String255\"},{\"fTest\":\"Float\"}]";
    auto *referenceSpec = new cool::RecordSpecification();
    referenceSpec->extend("crate", StorageType::UChar);
    referenceSpec->extend("ROB", StorageType::Int32);
    referenceSpec->extend("BCIDOffset", StorageType::Int16);
    referenceSpec->extend("AName", StorageType::String255);
    referenceSpec->extend("fTest", StorageType::Float);
    auto *returnedSpec = Json2Cool::parsePayloadSpec(testSpecString);
    BOOST_CHECK(*(returnedSpec) == *static_cast<const cool::IRecordSpecification*>(referenceSpec));
  }
  BOOST_AUTO_TEST_CASE(createAttributeList){
    auto *referenceSpec = new cool::RecordSpecification();
    referenceSpec->extend("crate", StorageType::UChar);
    referenceSpec->extend("ROB", StorageType::Int32);
    referenceSpec->extend("ROB1", StorageType::Int32);
    referenceSpec->extend("BCIDOffset", StorageType::Int16);
    referenceSpec->extend("AName", StorageType::String255);
    referenceSpec->extend("fnTest1", StorageType::Float);
    referenceSpec->extend("fnTest2", StorageType::Float);
    referenceSpec->extend("fsTest1", StorageType::Float);
    referenceSpec->extend("fsTest2", StorageType::Float);
    referenceSpec->extend("dnTest1", StorageType::Double);
    referenceSpec->extend("dnTest2", StorageType::Double);
    referenceSpec->extend("dsTest1", StorageType::Double);
    referenceSpec->extend("dsTest2", StorageType::Double);
    referenceSpec->extend("blobTest", StorageType::Blob16M);
    nlohmann::json j=nlohmann::json::array();
    cool::Record reference(*referenceSpec);
    //
    auto & att0 ATLAS_THREAD_SAFE = const_cast<coral::Attribute&>(reference.attributeList()[0]);
    unsigned char set0(1);
    att0.setValue<unsigned char>(set0);
    j.push_back(set0);
    //
    auto & att1 ATLAS_THREAD_SAFE = const_cast<coral::Attribute&>(reference.attributeList()[1]);
    att1.setValue<int>(2);
    j.push_back(2);
    //
    auto & att1_ ATLAS_THREAD_SAFE = const_cast<coral::Attribute&>(reference.attributeList()[2]);
    att1_.setValue<int>(4);
    std::ostringstream ss2;
    ss2<<4;
    j.push_back(ss2.str());
    //
    auto & att2 ATLAS_THREAD_SAFE = const_cast<coral::Attribute&>(reference.attributeList()[3]);
    short set2(3);
    att2.setValue<short>(set2);
    j.push_back(set2);
    //
    auto & att3 ATLAS_THREAD_SAFE = const_cast<coral::Attribute&>(reference.attributeList()[4]);
    att3.setValue<std::string>("purple");
    j.push_back("purple");
    //
    auto & att4 ATLAS_THREAD_SAFE = const_cast<coral::Attribute&>(reference.attributeList()[5]);
    float set4(1.01);
    att4.setValue<float>(set4);
    j.push_back(set4);
    //
    auto & att5 ATLAS_THREAD_SAFE = const_cast<coral::Attribute&>(reference.attributeList()[6]);
    float set5(1);
    att5.setValue<float>(set5);
    j.push_back(set5);
    //
    auto & att6 ATLAS_THREAD_SAFE = const_cast<coral::Attribute&>(reference.attributeList()[7]);
    float set6(1.02);
    att6.setValue<float>(set6);
    j.push_back(std::to_string(set6));
    //
    auto & att7 ATLAS_THREAD_SAFE = const_cast<coral::Attribute&>(reference.attributeList()[8]);
    float set7(2);
    att7.setValue<float>(set7);
    j.push_back(std::to_string(set7));
    //
    auto & att8 ATLAS_THREAD_SAFE = const_cast<coral::Attribute&>(reference.attributeList()[9]);
    double set8(1.01);
    att8.setValue<double>(set8);
    j.push_back(set8);
    //
    auto & att9 ATLAS_THREAD_SAFE = const_cast<coral::Attribute&>(reference.attributeList()[10]);
    double set9(1);
    att9.setValue<double>(set9);
    j.push_back(set9);
    //
    auto & att10 ATLAS_THREAD_SAFE = const_cast<coral::Attribute&>(reference.attributeList()[11]);
    double set10(1.02);
    att10.setValue<double>(set10);
    j.push_back(std::to_string(set10));
    //
    auto & att11 ATLAS_THREAD_SAFE = const_cast<coral::Attribute&>(reference.attributeList()[12]);
    double set11(2);
    att11.setValue<double>(set11);
    j.push_back(std::to_string(set11));
    //
    auto & att12 ATLAS_THREAD_SAFE = const_cast<coral::Attribute&>(reference.attributeList()[13]);
    std::stringstream p;
    unsigned char myString [] = "This is my BLOB";
    const coral::Blob bl(strlen((char*)myString));
    att12.setValue<coral::Blob>(bl);
    const auto address = static_cast<const unsigned char *>(bl.startingAddress());
    const unsigned int nBytes = bl.size();
    std::string str=CxxUtils::base64_encode(address, nBytes);
    j.push_back(str);


    auto record=Json2Cool::createAttributeList(referenceSpec, j);
    BOOST_CHECK(record.size() == reference.size());
    BOOST_CHECK(reference == record);
  }

BOOST_AUTO_TEST_SUITE_END()

