/**
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 **/
 
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_Identifier
#include <boost/test/unit_test.hpp>
#include <boost/test/tools/output_test_stream.hpp>
namespace utf = boost::unit_test;

#include "Identifier/Identifiable.h"
#include "Identifier/Identifier.h"
#include "Identifier/IdentifierHash.h"
#include "Identifier/IdHelper.h"


class IdentifiableStub: public Identifiable{
public:
  Identifier identify() const override {return Identifier(0);}
  IdentifierHash identifyHash() const override {return IdentifierHash(0);}
  const IdHelper* getHelper() const override {return Identifiable::getHelper();}
};


BOOST_AUTO_TEST_SUITE(IdentifiableTest)

BOOST_AUTO_TEST_CASE(IdentifiableConstructors){
  BOOST_CHECK_NO_THROW(IdentifiableStub());  
}

BOOST_AUTO_TEST_CASE(IdentifiableAccessors){
  IdentifiableStub g;
  Identifier idZero(0);
  BOOST_TEST(g.identify() == idZero);
  Identifier idHashZero(0);
  BOOST_TEST(g.identifyHash() == idHashZero);
  BOOST_TEST(g.getHelper() == nullptr);
}


BOOST_AUTO_TEST_SUITE_END()

