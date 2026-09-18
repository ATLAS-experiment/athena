/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file CoralUtilities/test/ChaiCoralConverter_test.cxx
 * @brief Tests for ChaiCoralConverter, both CHAI <-> CORAL directions, in
 *        the Boost framework. Covers every chai::Type, null fields,
 *        multi-channel containers, and vector-payload (CondAttrListVec) rows.
 */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_CORALUTILITIES

#include <boost/test/unit_test.hpp>
//
#include <chai/Container.h>
#include <chai/ContainerBase.h>
#include <chai/PayloadSpec.h>
#include <chai/VectorContainer.h>
//
#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeList.h"
#include "CoralBase/AttributeListSpecification.h"
#include "CoralBase/Blob.h"
//
#include "AthenaPoolUtilities/AthenaAttributeList.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "CoralUtilities/ChaiCoralConverter.h"
//
#include <limits>
#include <memory>
#include <string>
//
#include "CxxUtils/checker_macros.h"
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

using namespace chai;

namespace {
  PayloadSpec allTypesSpec() {
    return PayloadSpec(
      {{"b",Bool}, {"i8",Int8}, {"u8",UInt8}, {"i16",Int16}, {"u16",UInt16},
       {"i32",Int32}, {"u32",UInt32}, {"i64",Int64}, {"u64",UInt64},
       {"f",Float}, {"d",Double}, {"s",String}, {"blob",Blob}},
      {{0,""}});
  }

  std::shared_ptr<Values> allTypesValues(const PayloadSpec& spec) {
    auto values = std::make_shared<Values>(spec.fields());
    values->push(true);
    values->push(static_cast<int8_t>(-5));
    values->push(static_cast<uint8_t>(5));
    values->push(static_cast<int16_t>(-500));
    values->push(static_cast<uint16_t>(500));
    values->push(static_cast<int32_t>(-50000));
    values->push(static_cast<uint32_t>(50000));
    values->push(static_cast<int64_t>(-5000000000LL));
    values->push(static_cast<uint64_t>(5000000000ULL));
    values->push(1.5f);
    values->push(2.5);
    values->push(std::string("hello"));
    values->push(BlobData(std::vector<uint8_t>{1,2,3,4}));
    return values;
  }

  // Releases a CORAL specification on scope exit, including on a BOOST_REQUIRE
  // failure that unwinds the test case early
  struct SpecReleaser {
    void operator()(coral::AttributeListSpecification* spec) const {
      if (spec) {
        spec->release();
      }
    }
  };
  using SpecHolder = std::unique_ptr<coral::AttributeListSpecification, SpecReleaser>;
}

BOOST_AUTO_TEST_SUITE(ChaiCoralConverterTest)

  BOOST_AUTO_TEST_CASE(ToCoralSpec_oneFieldPerType){
    const PayloadSpec spec = allTypesSpec();
    const SpecHolder coralSpec(ChaiCoralConverter::toCoralSpec(spec.fields()));
    BOOST_REQUIRE(coralSpec != nullptr);
    BOOST_TEST(coralSpec->size() == 13u);
  }

  // Pins the chai::Type -> CORAL C++ type table directly, rather than only
  // through the round trip below
  BOOST_AUTO_TEST_CASE(ToCoralSpec_typeMapping_matchesCoralTypes){
    const PayloadSpec spec = allTypesSpec();
    const SpecHolder coralSpec(ChaiCoralConverter::toCoralSpec(spec.fields()));
    // std::type_info has no operator<<, so wrap each comparison in bool()
    // rather than let BOOST_TEST decompose it for failure printing
    auto typeOf = [&](const char* name) -> const std::type_info& {
      return (*coralSpec)[coralSpec->index(name)].type();
    };
    BOOST_TEST(bool(typeOf("b") == typeid(bool)));
    BOOST_TEST(bool(typeOf("i8") == typeid(char)));
    BOOST_TEST(bool(typeOf("u8") == typeid(unsigned char)));
    BOOST_TEST(bool(typeOf("i16") == typeid(short)));
    BOOST_TEST(bool(typeOf("u16") == typeid(unsigned short)));
    BOOST_TEST(bool(typeOf("i32") == typeid(int)));
    BOOST_TEST(bool(typeOf("u32") == typeid(unsigned int)));
    BOOST_TEST(bool(typeOf("i64") == typeid(long long)));
    BOOST_TEST(bool(typeOf("u64") == typeid(unsigned long long)));
    BOOST_TEST(bool(typeOf("f") == typeid(float)));
    BOOST_TEST(bool(typeOf("d") == typeid(double)));
    BOOST_TEST(bool(typeOf("s") == typeid(std::string)));
    BOOST_TEST(bool(typeOf("blob") == typeid(coral::Blob)));
  }

  BOOST_AUTO_TEST_CASE(ToFieldSpec_allTypes){
    const PayloadSpec spec = allTypesSpec();
    const SpecHolder coralSpec(ChaiCoralConverter::toCoralSpec(spec.fields()));
    const FieldSpec fields = ChaiCoralConverter::toFieldSpec(*coralSpec);
    BOOST_TEST((fields == spec.fields()));
  }

  BOOST_AUTO_TEST_CASE(ToFieldSpec_unsupportedType_throwsNamingFieldAndType){
    const SpecHolder coralSpec(new coral::AttributeListSpecification());
    coralSpec->extend("weird", typeid(long));

    BOOST_CHECK_EXCEPTION(ChaiCoralConverter::toFieldSpec(*coralSpec), std::runtime_error,
      [](const std::runtime_error& e){
        const std::string what(e.what());
        return what.find("weird") != std::string::npos && what.find("long") != std::string::npos;
      });
  }

  BOOST_AUTO_TEST_CASE(ToAttributeList_allTypes_roundTrip){
    const PayloadSpec spec = allTypesSpec();
    const auto values = allTypesValues(spec);
    const SpecHolder coralSpec(ChaiCoralConverter::toCoralSpec(spec.fields()));

    const coral::AttributeList attr = ChaiCoralConverter::toAttributeList(*coralSpec, *values);
    BOOST_TEST(attr["b"].data<bool>() == true);
    // char's signedness is platform-defined (e.g. unsigned on aarch64); compare
    // against the same cast rather than the int literal so this holds everywhere
    BOOST_TEST(attr["i8"].data<char>() == static_cast<char>(-5));
    BOOST_TEST(attr["u8"].data<unsigned char>() == 5u);
    BOOST_TEST(attr["i16"].data<short>() == -500);
    BOOST_TEST(attr["u16"].data<unsigned short>() == 500u);
    BOOST_TEST(attr["i32"].data<int>() == -50000);
    BOOST_TEST(attr["u32"].data<unsigned int>() == 50000u);
    BOOST_TEST(attr["i64"].data<long long>() == -5000000000LL);
    BOOST_TEST(attr["u64"].data<unsigned long long>() == 5000000000ULL);
    BOOST_TEST(attr["f"].data<float>() == 1.5f);
    BOOST_TEST(attr["d"].data<double>() == 2.5);
    BOOST_TEST(attr["s"].data<std::string>() == "hello");
    const coral::Blob& blob = attr["blob"].data<coral::Blob>();
    BOOST_REQUIRE(blob.size() == 4);
    const auto* bytes = static_cast<const unsigned char*>(blob.startingAddress());
    BOOST_TEST(bytes[0] == 1);
    BOOST_TEST(bytes[1] == 2);
    BOOST_TEST(bytes[2] == 3);
    BOOST_TEST(bytes[3] == 4);
  }

  BOOST_AUTO_TEST_CASE(ToAttributeList_emptyBlob){
    const PayloadSpec spec({{"blob",Blob}}, {{0,""}});
    auto values = std::make_shared<Values>(spec.fields());
    values->push(BlobData(std::vector<uint8_t>{}));
    const SpecHolder coralSpec(ChaiCoralConverter::toCoralSpec(spec.fields()));

    const coral::AttributeList attr = ChaiCoralConverter::toAttributeList(*coralSpec, *values);
    BOOST_TEST(attr["blob"].data<coral::Blob>().size() == 0);
  }

  BOOST_AUTO_TEST_CASE(ToAttributeList_nullField){
    const PayloadSpec spec({{"v",Int32}}, {{0,""}});
    auto values = std::make_shared<Values>(spec.fields());
    values->push();  // Null
    const SpecHolder coralSpec(ChaiCoralConverter::toCoralSpec(spec.fields()));

    const coral::AttributeList attr = ChaiCoralConverter::toAttributeList(*coralSpec, *values);
    BOOST_TEST(attr["v"].isNull());
  }

  // A field count mismatch must be caught up front, naming both counts (see toAttributeList()).
  BOOST_AUTO_TEST_CASE(ToAttributeList_payloadShorterThanSpec_throwsWithBothCounts){
    const PayloadSpec specTwoFields({{"a",Int32}, {"b",Int32}}, {{0,""}});
    const PayloadSpec specOneField({{"a",Int32}}, {{0,""}});
    auto values = std::make_shared<Values>(specOneField.fields());
    values->push(static_cast<int32_t>(1));
    const SpecHolder coralSpec(ChaiCoralConverter::toCoralSpec(specTwoFields.fields()));

    BOOST_CHECK_EXCEPTION(ChaiCoralConverter::toAttributeList(*coralSpec, *values), std::runtime_error,
      [](const std::runtime_error& e){
        return std::string(e.what()).find("field count mismatch, payload 1 vs specification 2") != std::string::npos;
      });
  }

  BOOST_AUTO_TEST_CASE(ToAttributeList_payloadLongerThanSpec_throwsWithBothCounts){
    const PayloadSpec specTwoFields({{"a",Int32}, {"b",Int32}}, {{0,""}});
    const PayloadSpec specOneField({{"a",Int32}}, {{0,""}});
    auto values = std::make_shared<Values>(specTwoFields.fields());
    values->push(static_cast<int32_t>(1));
    values->push(static_cast<int32_t>(2));
    const SpecHolder coralSpec(ChaiCoralConverter::toCoralSpec(specOneField.fields()));

    BOOST_CHECK_EXCEPTION(ChaiCoralConverter::toAttributeList(*coralSpec, *values), std::runtime_error,
      [](const std::runtime_error& e){
        return std::string(e.what()).find("field count mismatch, payload 2 vs specification 1") != std::string::npos;
      });
  }

  // Same field count but a different name: the lookup fails on the name
  // instead of matching values by position.
  BOOST_AUTO_TEST_CASE(ToAttributeList_fieldNameNotInPayload_throwsNamingField){
    const PayloadSpec specNamedA({{"a",Int32}}, {{0,""}});
    const PayloadSpec specNamedX({{"x",Int32}}, {{0,""}});
    auto values = std::make_shared<Values>(specNamedX.fields());
    values->push(static_cast<int32_t>(1));
    const SpecHolder coralSpec(ChaiCoralConverter::toCoralSpec(specNamedA.fields()));

    BOOST_CHECK_EXCEPTION(ChaiCoralConverter::toAttributeList(*coralSpec, *values), std::runtime_error,
      [](const std::runtime_error& e){
        return std::string(e.what()).find("a") != std::string::npos;
      });
  }

  // Field order differs from the specification: values must still land on the
  // attribute of the same name, not the same position
  BOOST_AUTO_TEST_CASE(ToAttributeList_fieldOrderDiffers_mapsByName){
    const PayloadSpec specAB({{"a",Int32}, {"b",Int32}}, {{0,""}});
    const PayloadSpec specBA({{"b",Int32}, {"a",Int32}}, {{0,""}});
    auto values = std::make_shared<Values>(specBA.fields());
    values->push(static_cast<int32_t>(20));  // b
    values->push(static_cast<int32_t>(10));  // a
    const SpecHolder coralSpec(ChaiCoralConverter::toCoralSpec(specAB.fields()));

    const coral::AttributeList attr = ChaiCoralConverter::toAttributeList(*coralSpec, *values);
    BOOST_TEST(attr["a"].data<int>() == 10);
    BOOST_TEST(attr["b"].data<int>() == 20);
  }

  BOOST_AUTO_TEST_CASE(ToAttributeList_multiChannel){
    const PayloadSpec spec({{"v",Int32}}, {{0,""},{1,""}});
    Container c(spec);
    auto v0 = std::make_shared<Values>(spec.fields());
    v0->push(static_cast<int32_t>(10));
    auto v1 = std::make_shared<Values>(spec.fields());
    v1->push(static_cast<int32_t>(20));
    c.add(0, v0);
    c.add(1, v1);

    const SpecHolder coralSpec(ChaiCoralConverter::toCoralSpec(c.payloadSpec().fields()));
    const coral::AttributeList a0 = ChaiCoralConverter::toAttributeList(*coralSpec, c[0]);
    const coral::AttributeList a1 = ChaiCoralConverter::toAttributeList(*coralSpec, c[1]);
    BOOST_TEST(a0["v"].data<int>() == 10);
    BOOST_TEST(a1["v"].data<int>() == 20);
  }

  BOOST_AUTO_TEST_CASE(ToAttributeListVec_multiRow){
    const PayloadSpec spec({{"v",Int32}}, {{0,""}});
    VectorContainer vc(spec);
    std::vector<ValuesPtr> rows;
    for (int32_t val : {10, 20, 30}) {
      auto row = std::make_shared<Values>(spec.fields());
      row->push(val);
      rows.push_back(std::move(row));
    }
    vc.setRows(0, rows);

    const SpecHolder coralSpec(ChaiCoralConverter::toCoralSpec(vc.payloadSpec().fields()));
    const auto attrs = ChaiCoralConverter::toAttributeListVec(*coralSpec, vc.rows(0));
    BOOST_REQUIRE(attrs.size() == 3u);
    BOOST_TEST(attrs[0]["v"].data<int>() == 10);
    BOOST_TEST(attrs[1]["v"].data<int>() == 20);
    BOOST_TEST(attrs[2]["v"].data<int>() == 30);
  }

  BOOST_AUTO_TEST_CASE(ToAttributeListVec_emptyChannel){
    const PayloadSpec spec({{"v",Int32}}, {{0,""}});
    VectorContainer vc(spec);
    // Channel 0 is pre-created by the PayloadSpec's channel list but never
    // populated, exercising the empty-channel case chai/VectorContainer.h documents.
    const SpecHolder coralSpec(ChaiCoralConverter::toCoralSpec(vc.payloadSpec().fields()));
    const auto attrs = ChaiCoralConverter::toAttributeListVec(*coralSpec, vc.rows(0));
    BOOST_TEST(attrs.empty());
  }

  BOOST_AUTO_TEST_CASE(ToAttributeListVec_multiChannelDifferentRowCounts){
    const PayloadSpec spec({{"v",Int32}}, {{0,""},{1,""}});
    VectorContainer vc(spec);
    std::vector<ValuesPtr> rows0;
    auto r0 = std::make_shared<Values>(spec.fields());
    r0->push(static_cast<int32_t>(1));
    rows0.push_back(std::move(r0));
    vc.setRows(0, rows0);

    std::vector<ValuesPtr> rows1;
    for (int32_t val : {2, 3}) {
      auto row = std::make_shared<Values>(spec.fields());
      row->push(val);
      rows1.push_back(std::move(row));
    }
    vc.setRows(1, rows1);

    const SpecHolder coralSpec(ChaiCoralConverter::toCoralSpec(vc.payloadSpec().fields()));
    const auto attrs0 = ChaiCoralConverter::toAttributeListVec(*coralSpec, vc.rows(0));
    const auto attrs1 = ChaiCoralConverter::toAttributeListVec(*coralSpec, vc.rows(1));
    BOOST_REQUIRE(attrs0.size() == 1u);
    BOOST_REQUIRE(attrs1.size() == 2u);
    BOOST_TEST(attrs0[0]["v"].data<int>() == 1);
    BOOST_TEST(attrs1[0]["v"].data<int>() == 2);
    BOOST_TEST(attrs1[1]["v"].data<int>() == 3);
  }

  BOOST_AUTO_TEST_CASE(ToAttributeListVec_nullRow_throwsNamingIndex){
    const PayloadSpec spec({{"v",Int32}}, {{0,""}});
    const SpecHolder coralSpec(ChaiCoralConverter::toCoralSpec(spec.fields()));
    auto row0 = std::make_shared<Values>(spec.fields());
    row0->push(static_cast<int32_t>(1));
    const std::vector<ValuesPtr> rows{row0, nullptr};

    BOOST_CHECK_EXCEPTION(ChaiCoralConverter::toAttributeListVec(*coralSpec, rows), std::invalid_argument,
      [](const std::invalid_argument& e){
        return std::string(e.what()).find("index 1") != std::string::npos;
      });
  }

  BOOST_AUTO_TEST_CASE(ToValues_allTypes_roundTrip){
    const PayloadSpec spec = allTypesSpec();
    const auto values = allTypesValues(spec);
    const SpecHolder coralSpec(ChaiCoralConverter::toCoralSpec(spec.fields()));
    const coral::AttributeList attr = ChaiCoralConverter::toAttributeList(*coralSpec, *values);
    const auto fieldsPtr = std::make_shared<FieldSpec>(ChaiCoralConverter::toFieldSpec(*coralSpec));

    const ValuesPtr result = ChaiCoralConverter::toValues(fieldsPtr, attr);
    BOOST_TEST(result->get<bool>("b") == true);
    BOOST_TEST(result->get<int8_t>("i8") == -5);
    BOOST_TEST(result->get<uint8_t>("u8") == 5u);
    BOOST_TEST(result->get<int16_t>("i16") == -500);
    BOOST_TEST(result->get<uint16_t>("u16") == 500u);
    BOOST_TEST(result->get<int32_t>("i32") == -50000);
    BOOST_TEST(result->get<uint32_t>("u32") == 50000u);
    BOOST_TEST(result->get<int64_t>("i64") == -5000000000LL);
    BOOST_TEST(result->get<uint64_t>("u64") == 5000000000ULL);
    BOOST_TEST(result->get<float>("f") == 1.5f);
    BOOST_TEST(result->get<double>("d") == 2.5);
    BOOST_TEST(result->get<std::string>("s") == "hello");
    const auto& bytes = result->get<BlobData>("blob").m_bytes;
    BOOST_REQUIRE(bytes.size() == 4u);
    BOOST_TEST(bytes[0] == 1);
    BOOST_TEST(bytes[1] == 2);
    BOOST_TEST(bytes[2] == 3);
    BOOST_TEST(bytes[3] == 4);
  }

  BOOST_AUTO_TEST_CASE(ToValues_nullField){
    const PayloadSpec spec({{"v",Int32}}, {{0,""}});
    const SpecHolder coralSpec(ChaiCoralConverter::toCoralSpec(spec.fields()));
    auto values = std::make_shared<Values>(spec.fields());
    values->push();  // Null
    const coral::AttributeList attr = ChaiCoralConverter::toAttributeList(*coralSpec, *values);
    const auto fieldsPtr = std::make_shared<FieldSpec>(ChaiCoralConverter::toFieldSpec(*coralSpec));

    const ValuesPtr result = ChaiCoralConverter::toValues(fieldsPtr, attr);
    BOOST_TEST(result->isNull("v"));
  }

  BOOST_AUTO_TEST_CASE(ToValues_emptyBlob){
    const PayloadSpec spec({{"blob",Blob}}, {{0,""}});
    const SpecHolder coralSpec(ChaiCoralConverter::toCoralSpec(spec.fields()));
    auto values = std::make_shared<Values>(spec.fields());
    values->push(BlobData(std::vector<uint8_t>{}));
    const coral::AttributeList attr = ChaiCoralConverter::toAttributeList(*coralSpec, *values);
    const auto fieldsPtr = std::make_shared<FieldSpec>(ChaiCoralConverter::toFieldSpec(*coralSpec));

    const ValuesPtr result = ChaiCoralConverter::toValues(fieldsPtr, attr);
    BOOST_TEST(result->get<BlobData>("blob").m_bytes.empty());
  }

  BOOST_AUTO_TEST_CASE(ToValues_fieldCountMismatch_throwsWithBothCounts){
    const PayloadSpec specOneField({{"a",Int32}}, {{0,""}});
    const PayloadSpec specTwoFields({{"a",Int32}, {"b",Int32}}, {{0,""}});
    const SpecHolder coralSpec(ChaiCoralConverter::toCoralSpec(specOneField.fields()));
    auto values = std::make_shared<Values>(specOneField.fields());
    values->push(static_cast<int32_t>(1));
    const coral::AttributeList attr = ChaiCoralConverter::toAttributeList(*coralSpec, *values);
    const auto fieldsPtr = std::make_shared<FieldSpec>(specTwoFields.fields());

    BOOST_CHECK_EXCEPTION(ChaiCoralConverter::toValues(fieldsPtr, attr), std::runtime_error,
      [](const std::runtime_error& e){
        return std::string(e.what()).find("specification 2 vs payload 1") != std::string::npos;
      });
  }

  // Same field count but a different name: the lookup fails on the name
  // instead of matching values by position.
  BOOST_AUTO_TEST_CASE(ToValues_fieldNameNotInAttributeList_throwsNamingField){
    const PayloadSpec specNamedX({{"x",Int32}}, {{0,""}});
    const PayloadSpec specNamedA({{"a",Int32}}, {{0,""}});
    const SpecHolder coralSpec(ChaiCoralConverter::toCoralSpec(specNamedX.fields()));
    auto values = std::make_shared<Values>(specNamedX.fields());
    values->push(static_cast<int32_t>(1));
    const coral::AttributeList attr = ChaiCoralConverter::toAttributeList(*coralSpec, *values);
    const auto fieldsPtr = std::make_shared<FieldSpec>(specNamedA.fields());

    BOOST_CHECK_EXCEPTION(ChaiCoralConverter::toValues(fieldsPtr, attr), std::runtime_error,
      [](const std::runtime_error& e){
        return std::string(e.what()).find("field 'a'") != std::string::npos;
      });
  }

  // Field order differs from the field specification: values must land on
  // the field of the same name and in FieldSpec order, not attribute order
  BOOST_AUTO_TEST_CASE(ToValues_fieldOrderDiffers_mapsByName){
    const PayloadSpec specAB({{"a",Int32}, {"b",Int32}}, {{0,""}});
    const PayloadSpec specBA({{"b",Int32}, {"a",Int32}}, {{0,""}});
    const SpecHolder coralSpec(ChaiCoralConverter::toCoralSpec(specAB.fields()));
    auto values = std::make_shared<Values>(specAB.fields());
    values->push(static_cast<int32_t>(10));  // a
    values->push(static_cast<int32_t>(20));  // b
    const coral::AttributeList attr = ChaiCoralConverter::toAttributeList(*coralSpec, *values);
    const auto fieldsPtr = std::make_shared<FieldSpec>(specBA.fields());

    const ValuesPtr result = ChaiCoralConverter::toValues(fieldsPtr, attr);
    BOOST_TEST(result->get<int32_t>("a") == 10);
    BOOST_TEST(result->get<int32_t>("b") == 20);
    BOOST_TEST(result->get<int32_t>(0) == 20);  // b is first in fieldsPtr's order
    BOOST_TEST(result->get<int32_t>(1) == 10);
  }

  BOOST_AUTO_TEST_CASE(ToValues_typeMismatch_throwsNamingField){
    const PayloadSpec specInt32({{"v",Int32}}, {{0,""}});
    const PayloadSpec specInt64({{"v",Int64}}, {{0,""}});
    const SpecHolder coralSpec(ChaiCoralConverter::toCoralSpec(specInt32.fields()));
    auto values = std::make_shared<Values>(specInt32.fields());
    values->push(static_cast<int32_t>(1));
    const coral::AttributeList attr = ChaiCoralConverter::toAttributeList(*coralSpec, *values);
    const auto fieldsPtr = std::make_shared<FieldSpec>(specInt64.fields());

    BOOST_CHECK_EXCEPTION(ChaiCoralConverter::toValues(fieldsPtr, attr), std::runtime_error,
      [](const std::runtime_error& e){
        const std::string what(e.what());
        return what.find("field 'v'") != std::string::npos
            && what.find("Int64") != std::string::npos && what.find("Int32") != std::string::npos;
      });
  }

  BOOST_AUTO_TEST_CASE(ToValues_nullFieldSpec_throws){
    const PayloadSpec spec({{"v",Int32}}, {{0,""}});
    const SpecHolder coralSpec(ChaiCoralConverter::toCoralSpec(spec.fields()));
    auto values = std::make_shared<Values>(spec.fields());
    values->push(static_cast<int32_t>(1));
    const coral::AttributeList attr = ChaiCoralConverter::toAttributeList(*coralSpec, *values);

    BOOST_CHECK_THROW(ChaiCoralConverter::toValues(nullptr, attr), std::invalid_argument);
  }

  BOOST_AUTO_TEST_CASE(ToValues_sharesFieldSpec){
    const PayloadSpec spec({{"v",Int32}}, {{0,""}});
    const SpecHolder coralSpec(ChaiCoralConverter::toCoralSpec(spec.fields()));
    auto v0 = std::make_shared<Values>(spec.fields());
    v0->push(static_cast<int32_t>(1));
    auto v1 = std::make_shared<Values>(spec.fields());
    v1->push(static_cast<int32_t>(2));
    const coral::AttributeList a0 = ChaiCoralConverter::toAttributeList(*coralSpec, *v0);
    const coral::AttributeList a1 = ChaiCoralConverter::toAttributeList(*coralSpec, *v1);
    const auto fieldsPtr = std::make_shared<FieldSpec>(ChaiCoralConverter::toFieldSpec(*coralSpec));

    const ValuesPtr r0 = ChaiCoralConverter::toValues(fieldsPtr, a0);
    const ValuesPtr r1 = ChaiCoralConverter::toValues(fieldsPtr, a1);
    BOOST_TEST(r0->fieldSpecPtr().get() == fieldsPtr.get());
    BOOST_TEST(r1->fieldSpecPtr().get() == fieldsPtr.get());
  }

  BOOST_AUTO_TEST_CASE(ToValuesVec_multiRow){
    const PayloadSpec spec({{"v",Int32}}, {{0,""}});
    const SpecHolder coralSpec(ChaiCoralConverter::toCoralSpec(spec.fields()));
    std::vector<coral::AttributeList> rows;
    for (int32_t val : {10, 20, 30}) {
      auto v = std::make_shared<Values>(spec.fields());
      v->push(val);
      rows.push_back(ChaiCoralConverter::toAttributeList(*coralSpec, *v));
    }
    const auto fieldsPtr = std::make_shared<FieldSpec>(ChaiCoralConverter::toFieldSpec(*coralSpec));

    const auto result = ChaiCoralConverter::toValuesVec(fieldsPtr, rows);
    BOOST_REQUIRE(result.size() == 3u);
    BOOST_TEST(result[0]->get<int32_t>("v") == 10);
    BOOST_TEST(result[1]->get<int32_t>("v") == 20);
    BOOST_TEST(result[2]->get<int32_t>("v") == 30);
  }

  BOOST_AUTO_TEST_CASE(ToValuesVec_empty){
    const PayloadSpec spec({{"v",Int32}}, {{0,""}});
    const SpecHolder coralSpec(ChaiCoralConverter::toCoralSpec(spec.fields()));
    const auto fieldsPtr = std::make_shared<FieldSpec>(ChaiCoralConverter::toFieldSpec(*coralSpec));
    const std::vector<coral::AttributeList> rows;

    const auto result = ChaiCoralConverter::toValuesVec(fieldsPtr, rows);
    BOOST_TEST(result.empty());
  }

  // toValuesVec's rows are AttributeList values, not pointers (unlike
  // toAttributeListVec's), so there is no null-row case to test here.
  BOOST_AUTO_TEST_CASE(ToValuesVec_nullFieldSpec_throws){
    const std::vector<coral::AttributeList> rows;
    BOOST_CHECK_THROW(ChaiCoralConverter::toValuesVec(nullptr, rows), std::invalid_argument);
  }

  BOOST_AUTO_TEST_CASE(ToAthenaAttributeList_singleChannel_roundTrip){
    const PayloadSpec spec = allTypesSpec();
    Container c(spec);
    c.add(0, allTypesValues(spec));

    const auto attrList = ChaiCoralConverter::toAthenaAttributeList(c);
    BOOST_REQUIRE(attrList != nullptr);
    const Container result = ChaiCoralConverter::toContainer(*attrList, 7, "name");
    BOOST_TEST((result.channelIds() == std::vector<uint64_t>{7}));
    BOOST_TEST(result.channelSpec().channelNames().at(7) == "name");
    BOOST_TEST(result[7].get<bool>("b") == true);
    BOOST_TEST(result[7].get<int8_t>("i8") == -5);
    BOOST_TEST(result[7].get<std::string>("s") == "hello");
    BOOST_TEST(result[7].get<BlobData>("blob").m_bytes.size() == 4u);
  }

  BOOST_AUTO_TEST_CASE(ToAthenaAttributeList_zeroPopulatedChannels_throws){
    const PayloadSpec spec({{"v",Int32}}, {{0,""}});
    Container c(spec);
    // Channel 0 is pre-created by the spec but never populated

    BOOST_CHECK_EXCEPTION(ChaiCoralConverter::toAthenaAttributeList(c), std::runtime_error,
      [](const std::runtime_error& e){
        return std::string(e.what()).find("found 0") != std::string::npos;
      });
  }

  BOOST_AUTO_TEST_CASE(ToAthenaAttributeList_twoChannels_throws){
    const PayloadSpec spec({{"v",Int32}}, {{0,""},{1,""}});
    Container c(spec);
    auto v0 = std::make_shared<Values>(spec.fields());
    v0->push(static_cast<int32_t>(1));
    auto v1 = std::make_shared<Values>(spec.fields());
    v1->push(static_cast<int32_t>(2));
    c.add(0, v0);
    c.add(1, v1);

    BOOST_CHECK_EXCEPTION(ChaiCoralConverter::toAthenaAttributeList(c), std::runtime_error,
      [](const std::runtime_error& e){
        return std::string(e.what()).find("found 2") != std::string::npos;
      });
  }

  BOOST_AUTO_TEST_CASE(ToContainer_athenaAttributeList_defaultChannel){
    const PayloadSpec spec({{"v",Int32}}, {{0,""}});
    const SpecHolder coralSpec(ChaiCoralConverter::toCoralSpec(spec.fields()));
    auto values = std::make_shared<Values>(spec.fields());
    values->push(static_cast<int32_t>(42));
    const AthenaAttributeList attrList(ChaiCoralConverter::toAttributeList(*coralSpec, *values));

    const Container result = ChaiCoralConverter::toContainer(attrList);
    BOOST_TEST((result.channelIds() == std::vector<uint64_t>{0}));
    BOOST_TEST(result.channelSpec().channelNames().at(0).empty());
    BOOST_TEST(result[0].get<int32_t>("v") == 42);
  }

  BOOST_AUTO_TEST_CASE(ToContainer_emptySpecification_throws){
    const AthenaAttributeList emptyAttrList;
    BOOST_CHECK_THROW(ChaiCoralConverter::toContainer(emptyAttrList), std::runtime_error);

    CondAttrListCollection coll(true);
    coll.add(0, emptyAttrList.coralList());
    BOOST_CHECK_THROW(ChaiCoralConverter::toContainer(coll), std::runtime_error);
  }

  BOOST_AUTO_TEST_CASE(ToCondAttrListCollection_multiChannel_roundTrip){
    const PayloadSpec spec({{"v",Int32}}, {{0,"ch0"},{1,""},{2,"ch2"}});
    Container c(spec);
    auto v0 = std::make_shared<Values>(spec.fields());
    v0->push(static_cast<int32_t>(10));
    auto v1 = std::make_shared<Values>(spec.fields());
    v1->push(static_cast<int32_t>(20));
    auto v2 = std::make_shared<Values>(spec.fields());
    v2->push(static_cast<int32_t>(30));
    c.add(0, v0);
    c.add(1, v1);
    c.add(2, v2);

    const auto coll = ChaiCoralConverter::toCondAttrListCollection(c, true);
    BOOST_TEST(coll->size() == 3u);
    BOOST_TEST(coll->name_size() == 2u);
    BOOST_TEST(coll->iov_size() == 0u);
    BOOST_TEST(coll->attributeList(0)["v"].data<int>() == 10);
    BOOST_TEST(coll->attributeList(1)["v"].data<int>() == 20);
    BOOST_TEST(coll->attributeList(2)["v"].data<int>() == 30);

    // Force heap churn between building the collection and reading it back
    {
      coral::AttributeList churn(coll->attributeList(0).specification(), true);
      const std::vector<char> churnBytes(256, 'x');
      BOOST_TEST(churn.size() == coll->attributeList(0).size());
      BOOST_TEST(!churnBytes.empty());
    }

    const Container result = ChaiCoralConverter::toContainer(*coll);
    BOOST_TEST(bool(result == c));
    BOOST_TEST((result.channelSpec().channelNames() == c.channelSpec().channelNames()));
  }

  BOOST_AUTO_TEST_CASE(ToCondAttrListCollection_skipsUnpopulatedChannel){
    const PayloadSpec spec({{"v",Int32}}, {{0,""},{1,""},{2,""}});
    Container c(spec);
    auto v0 = std::make_shared<Values>(spec.fields());
    v0->push(static_cast<int32_t>(1));
    auto v1 = std::make_shared<Values>(spec.fields());
    v1->push(static_cast<int32_t>(2));
    c.add(0, v0);
    c.add(1, v1);
    // Channel 2 is left unpopulated

    const auto coll = ChaiCoralConverter::toCondAttrListCollection(c, true);
    BOOST_TEST(coll->size() == 2u);
  }

  BOOST_AUTO_TEST_CASE(ToCondAttrListCollection_unpopulatedNamedChannel_noName){
    const PayloadSpec spec({{"v",Int32}}, {{0,""},{1,"ch1"},{2,"ch2"}});
    Container c(spec);
    auto v0 = std::make_shared<Values>(spec.fields());
    v0->push(static_cast<int32_t>(1));
    auto v1 = std::make_shared<Values>(spec.fields());
    v1->push(static_cast<int32_t>(2));
    c.add(0, v0);
    c.add(1, v1);
    // Channel 2 ("ch2") is left unpopulated

    const auto coll = ChaiCoralConverter::toCondAttrListCollection(c, true);
    BOOST_TEST(coll->size() == 2u);
    BOOST_TEST(coll->name_size() == 1u);
  }

  BOOST_AUTO_TEST_CASE(ToCondAttrListCollection_nullFields){
    const PayloadSpec spec({{"v",Int32}}, {{0,""}});
    Container c(spec);
    auto v0 = std::make_shared<Values>(spec.fields());
    v0->push();  // Null
    c.add(0, v0);

    const auto coll = ChaiCoralConverter::toCondAttrListCollection(c, true);
    BOOST_TEST(coll->attributeList(0)["v"].isNull());
    const Container result = ChaiCoralConverter::toContainer(*coll);
    BOOST_TEST(result[0].isNull("v"));
  }

  BOOST_AUTO_TEST_CASE(ToCondAttrListCollection_channelIdAtUInt32Max_passes){
    const uint64_t maxId = std::numeric_limits<uint32_t>::max();
    const PayloadSpec spec({{"v",Int32}}, {{maxId,""}});
    Container c(spec);
    auto v = std::make_shared<Values>(spec.fields());
    v->push(static_cast<int32_t>(1));
    c.add(maxId, v);

    const auto coll = ChaiCoralConverter::toCondAttrListCollection(c, true);
    BOOST_TEST(coll->size() == 1u);
  }

  BOOST_AUTO_TEST_CASE(ToCondAttrListCollection_channelIdAboveUInt32_throwsNamingId){
    const uint64_t badId = static_cast<uint64_t>(std::numeric_limits<uint32_t>::max()) + 1;
    const PayloadSpec spec({{"v",Int32}}, {{badId,""}});
    Container c(spec);
    auto v = std::make_shared<Values>(spec.fields());
    v->push(static_cast<int32_t>(1));
    c.add(badId, v);

    BOOST_CHECK_EXCEPTION(ChaiCoralConverter::toCondAttrListCollection(c, true), std::runtime_error,
      [badId](const std::runtime_error& e){
        return std::string(e.what()).find(std::to_string(badId)) != std::string::npos;
      });
  }

  BOOST_AUTO_TEST_CASE(ToCondAttrListCollection_channelIdCollision_throws){
    const uint64_t highId = 1ull << 32;
    const PayloadSpec spec({{"v",Int32}}, {{0,""},{highId,""}});
    Container c(spec);
    auto v0 = std::make_shared<Values>(spec.fields());
    v0->push(static_cast<int32_t>(1));
    auto v1 = std::make_shared<Values>(spec.fields());
    v1->push(static_cast<int32_t>(2));
    c.add(0, v0);
    c.add(highId, v1);

    BOOST_CHECK_THROW(ChaiCoralConverter::toCondAttrListCollection(c, true), std::runtime_error);
  }

  BOOST_AUTO_TEST_CASE(ToContainer_emptyCollection_throws){
    CondAttrListCollection coll(true);
    BOOST_CHECK_THROW(ChaiCoralConverter::toContainer(coll), std::runtime_error);
  }

  BOOST_AUTO_TEST_CASE(ToContainer_collection_nameOnlyChannelIgnored){
    const PayloadSpec spec({{"v",Int32}}, {{0,""}});
    const SpecHolder coralSpec(ChaiCoralConverter::toCoralSpec(spec.fields()));
    auto values = std::make_shared<Values>(spec.fields());
    values->push(static_cast<int32_t>(1));
    const coral::AttributeList attr = ChaiCoralConverter::toAttributeList(*coralSpec, *values);

    CondAttrListCollection coll(true);
    coll.add(0, attr);
    coll.add(1, "orphanName");  // Named, but no attribute list for channel 1

    const Container result = ChaiCoralConverter::toContainer(coll);
    BOOST_TEST((result.channelIds() == std::vector<uint64_t>{0}));
    BOOST_TEST(!result.channelSpec().contains(1));
  }

  BOOST_AUTO_TEST_CASE(ToContainer_collection_unnamedChannel_emptyName){
    const PayloadSpec spec({{"v",Int32}}, {{0,""}});
    const SpecHolder coralSpec(ChaiCoralConverter::toCoralSpec(spec.fields()));
    auto values = std::make_shared<Values>(spec.fields());
    values->push(static_cast<int32_t>(1));
    const coral::AttributeList attr = ChaiCoralConverter::toAttributeList(*coralSpec, *values);

    CondAttrListCollection coll(true);
    coll.add(0, attr);

    const Container result = ChaiCoralConverter::toContainer(coll);
    BOOST_TEST(result.channelSpec().channelNames().at(0).empty());
  }

  BOOST_AUTO_TEST_CASE(ToContainer_collection_sharesContainerFieldSpec){
    const PayloadSpec spec({{"v",Int32}}, {{0,""}});
    const SpecHolder coralSpec(ChaiCoralConverter::toCoralSpec(spec.fields()));
    auto v0 = std::make_shared<Values>(spec.fields());
    v0->push(static_cast<int32_t>(1));
    auto v1 = std::make_shared<Values>(spec.fields());
    v1->push(static_cast<int32_t>(2));
    const coral::AttributeList a0 = ChaiCoralConverter::toAttributeList(*coralSpec, *v0);
    const coral::AttributeList a1 = ChaiCoralConverter::toAttributeList(*coralSpec, *v1);

    CondAttrListCollection coll(true);
    coll.add(0, a0);
    coll.add(1, a1);

    const Container result = ChaiCoralConverter::toContainer(coll);
    BOOST_TEST(result[0].fieldSpecPtr().get() == result[1].fieldSpecPtr().get());
  }

  // Pins CondAttrListCollection's own addShared invariant that
  // toCondAttrListCollection relies on, not converter logic
  BOOST_AUTO_TEST_CASE(ToCondAttrListCollection_specShared){
    const PayloadSpec spec({{"v",Int32}}, {{0,""},{1,""},{2,""}});
    Container c(spec);
    auto v0 = std::make_shared<Values>(spec.fields());
    v0->push(static_cast<int32_t>(1));
    auto v1 = std::make_shared<Values>(spec.fields());
    v1->push(static_cast<int32_t>(2));
    auto v2 = std::make_shared<Values>(spec.fields());
    v2->push(static_cast<int32_t>(3));
    c.add(0, v0);
    c.add(1, v1);
    c.add(2, v2);

    const auto coll = ChaiCoralConverter::toCondAttrListCollection(c, true);
    const auto& spec0 = coll->attributeList(0).specification();
    const auto& spec1 = coll->attributeList(1).specification();
    const auto& spec2 = coll->attributeList(2).specification();
    BOOST_TEST(&spec0 == &spec1);
    BOOST_TEST(&spec1 == &spec2);
  }

  BOOST_AUTO_TEST_CASE(ValuesSize_allTypes_nullCountsZero){
    const PayloadSpec spec = allTypesSpec();
    const auto values = allTypesValues(spec);
    // sizeof(bool)+int8_t+uint8_t+int16_t+uint16_t+int32_t+uint32_t+int64_t+uint64_t+float+double
    // + strlen("hello") + 4 blob bytes
    const unsigned int expectedFull = 1+1+1+2+2+4+4+8+8+4+8 + 5 + 4;
    BOOST_TEST(ChaiCoralConverter::valuesSize(*values) == expectedFull);

    // Null out one field ("d", 8 bytes) and confirm the total drops by its size
    auto valuesWithNull = std::make_shared<Values>(spec.fields());
    valuesWithNull->push(true);
    valuesWithNull->push(static_cast<int8_t>(-5));
    valuesWithNull->push(static_cast<uint8_t>(5));
    valuesWithNull->push(static_cast<int16_t>(-500));
    valuesWithNull->push(static_cast<uint16_t>(500));
    valuesWithNull->push(static_cast<int32_t>(-50000));
    valuesWithNull->push(static_cast<uint32_t>(50000));
    valuesWithNull->push(static_cast<int64_t>(-5000000000LL));
    valuesWithNull->push(static_cast<uint64_t>(5000000000ULL));
    valuesWithNull->push(1.5f);
    valuesWithNull->push();  // "d" is null
    valuesWithNull->push(std::string("hello"));
    valuesWithNull->push(BlobData(std::vector<uint8_t>{1,2,3,4}));
    BOOST_TEST(ChaiCoralConverter::valuesSize(*valuesWithNull) == expectedFull - 8u);
  }

  BOOST_AUTO_TEST_CASE(PayloadSize_container_populatedOnly){
    const PayloadSpec spec({{"v",Int32}}, {{0,""},{1,""},{2,""}});
    Container c(spec);
    auto v0 = std::make_shared<Values>(spec.fields());
    v0->push(static_cast<int32_t>(1));
    auto v1 = std::make_shared<Values>(spec.fields());
    v1->push(static_cast<int32_t>(2));
    c.add(0, v0);
    c.add(1, v1);
    // Channel 2 is left unpopulated

    const unsigned int expected = ChaiCoralConverter::valuesSize(c[0]) + ChaiCoralConverter::valuesSize(c[1]);
    BOOST_TEST(ChaiCoralConverter::payloadSize(c) == expected);
  }

  BOOST_AUTO_TEST_CASE(PayloadSize_vectorContainer_rows){
    const PayloadSpec spec({{"v",Int32}}, {{0,""},{1,""}});
    VectorContainer vc(spec);
    std::vector<ValuesPtr> rows0;
    auto r0 = std::make_shared<Values>(spec.fields());
    r0->push(static_cast<int32_t>(1));
    rows0.push_back(std::move(r0));
    vc.setRows(0, rows0);

    std::vector<ValuesPtr> rows1;
    for (int32_t val : {2, 3}) {
      auto row = std::make_shared<Values>(spec.fields());
      row->push(val);
      rows1.push_back(std::move(row));
    }
    vc.setRows(1, rows1);

    unsigned int expected{0};
    for (const auto& row : vc.rows(0)) {
      expected += ChaiCoralConverter::valuesSize(*row);
    }
    for (const auto& row : vc.rows(1)) {
      expected += ChaiCoralConverter::valuesSize(*row);
    }
    BOOST_TEST(ChaiCoralConverter::payloadSize(vc) == expected);
  }

  BOOST_AUTO_TEST_CASE(PayloadSize_vectorContainer_nullRow_throwsNamingChannelAndIndex){
    const PayloadSpec spec({{"v",Int32}}, {{0,""}});
    VectorContainer vc(spec);
    auto row0 = std::make_shared<Values>(spec.fields());
    row0->push(static_cast<int32_t>(1));
    const std::vector<ValuesPtr> rows{row0, nullptr};
    vc.setRows(0, rows);

    BOOST_CHECK_EXCEPTION(ChaiCoralConverter::payloadSize(vc), std::invalid_argument,
      [](const std::invalid_argument& e){
        return std::string(e.what()).find("channel 0 index 1") != std::string::npos;
      });
  }

BOOST_AUTO_TEST_SUITE_END()
