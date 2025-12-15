/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file TransientAddress_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Nov, 2013
 * @brief Regression tests for TransientAddress.
 */

#undef NDEBUG
#include "SGTools/TransientAddress.h"
#include "AthenaKernel/IAddressProvider.h"
#include "GaudiKernel/IOpaqueAddress.h"
#include "GaudiKernel/EventContext.h"
#include <iostream>
#include <cstdlib>
#include <cassert>


class TestAddress
  : public IOpaqueAddress
{
public:
  TestAddress (int x) : m_x (x), m_ref(0) {}
  /// Add reference to object
  virtual unsigned long        addRef     () override;
  /// release reference to object
  virtual unsigned long        release    () override;
  /// Retrieve class information from link
  virtual const CLID&          clID       () const override { std::abort(); }
  /// Retrieve service type
  virtual long                 svcType    () const override { std::abort(); }
  /// Update branch name
  virtual IRegistry*           registry   () const override { std::abort(); }
  /// Update directory pointer
  virtual void                 setRegistry(IRegistry* /*r*/) override { std::abort(); }
  /// Retrieve String parameters
  virtual const std::string*   par        () const override { std::abort(); }
  /// Access to generic link parameters
  virtual const unsigned long* ipar       () const override { std::abort(); }

  int m_x;
  int m_ref;
};


class TestProvider
  : public implements<IAddressProvider>
{
public:

  virtual StatusCode updateAddress(StoreID::type /*storeID*/,
				   SG::TransientAddress* /*pTAd*/,
                                   const EventContext& /*ctx*/) override
  { return StatusCode::SUCCESS; }
};


unsigned long TestAddress::addRef()
{
  ++m_ref;
  std::cout << "addRef " << m_x << " " << m_ref << "\n";
  return m_ref;
}


unsigned long TestAddress::release()
{
  --m_ref;
  std::cout << "release " << m_x << " " << m_ref << "\n";
  return m_ref;
}


void test1()
{
  std::cout << "test1\n";

  SG::TransientAddress tad1;
  assert (tad1.clID() == CLID_NULL);
  assert (tad1.name() == "");
  assert (tad1.transientID().size() == 0);
  assert (tad1.alias().size() == 0);
  assert (!tad1.isValid(nullptr));
  assert (tad1.provider() == 0);
  assert (tad1.storeID() == StoreID::UNKNOWN);
  assert (tad1.address() == 0);
  assert (tad1.sgkey() == 0);
  tad1.setID (123, "key");
  assert (tad1.name() == "key");
  assert (tad1.clID() == 123);
  assert (tad1.transientID().size() == 1);
  assert (tad1.alias().size() == 0);

  SG::TransientAddress tad2 (123, "key");
  assert (tad2.name() == "key");
  assert (tad2.clID() == 123);
  assert (tad2.transientID().size() == 1);
  assert (tad2.alias().size() == 0);
  assert (!tad2.isValid(nullptr));
  assert (tad2.provider() == 0);
  assert (tad2.storeID() == StoreID::UNKNOWN);
  assert (tad2.address() == 0);
  assert (tad2.sgkey() == 0);

  tad2.setSGKey (876);
  assert (tad2.sgkey() == 876);

  assert (tad2.transientID (123));
  assert (!tad2.transientID (124));
  tad2.setTransientID (124);
  assert (tad2.transientID (124));
  assert (tad2.transientID().size() == 2);

  tad2.setAlias ("key2");
  assert (tad2.alias().size() == 1);
  std::vector<std::string> a;
  a.push_back ("key3a");
  a.push_back ("key4a");
  tad2.setAlias (std::move(a));
  assert (a.size() == 0);
  assert (tad2.alias().size() == 2);

  const std::string s_key2 = "key2";
  const std::string s_key3 = "key3";
  const std::string s_key3a = "key3a";
  assert (std::ranges::binary_search (tad2.alias(), s_key2) == false);
  assert (std::ranges::binary_search (tad2.alias(), s_key3a) == true);
  tad2.setAlias (std::vector<std::string> {"key3", "key4"});
  assert (tad2.alias().size() == 2);
  assert (std::ranges::binary_search (tad2.alias(), s_key2) == false);
  assert (std::ranges::binary_search (tad2.alias(), s_key3) == true);
  tad2.removeAlias ("key3");
  assert (tad2.alias().size() == 1);
  assert (std::ranges::binary_search (tad2.alias(), s_key3) == false);

  TestAddress ad1(1);
  SG::TransientAddress tad3 (123, "key", &ad1);
  assert (tad3.name() == "key");
  assert (tad3.clID() == 123);
  assert (tad3.isValid(nullptr));
  assert (tad3.provider() == 0);
  assert (tad3.storeID() == StoreID::UNKNOWN);
  assert (tad3.address() == &ad1);

  TestAddress ad2(2);
  tad3.setAddress (&ad2);
  assert (tad3.isValid(nullptr));
  assert (tad3.address() == &ad2);
  tad3.reset();
  assert (!tad3.isValid(nullptr));
  assert (tad3.address() == 0);

  tad3.setAddress (&ad2);
  tad3.clearAddress (false);
  tad3.reset();
  assert (tad3.isValid(nullptr));
  assert (tad3.address() == &ad2);
  tad3.clearAddress (true);
  tad3.reset();
  assert (!tad3.isValid(nullptr));
  assert (tad3.address() == 0);

  EventContext ctx;

  TestProvider tp;
  tad3.setProvider (&tp, StoreID::EVENT_STORE);
  assert (tad3.provider() == &tp);
  assert (tad3.storeID() == StoreID::EVENT_STORE);
  assert (!tad3.isValid(nullptr));
  assert (tad3.isValid(&ctx));
  tad3.consultProvider (false);
  assert (!tad3.isValid(nullptr));
  tad3.consultProvider (true);
  assert (!tad3.isValid(nullptr));
  assert (tad3.isValid(&ctx));

  SG::TransientAddress tad4(0, "");
  assert (tad4.clID() == CLID_NULL);
  assert (tad4.name() == "");
  assert (tad4.transientID().size() == 0);
  tad4.setID (123, "key");
  assert (tad4.name() == "key");
  assert (tad4.clID() == 123);
  assert (tad4.transientID().size() == 1);
  assert (tad4.alias().size() == 0);
}


// Testing move.
void test2()
{
  std::cout << "test2\n";

  TestAddress ad1(1);
  SG::TransientAddress tad1 (123, "key", &ad1);
  TestProvider tp;
  tad1.setProvider (&tp, StoreID::EVENT_STORE);
  tad1.setSGKey (876);
  tad1.setTransientID (124);
  tad1.setAlias ("key2");

  assert (tad1.name() == "key");
  assert (tad1.clID() == 123);
  assert (tad1.address() == &ad1);
  assert (tad1.storeID() == StoreID::EVENT_STORE);
  assert (tad1.sgkey() == 876);
  assert (tad1.transientID().size() == 2);
  assert (tad1.transientID (123));
  assert (tad1.transientID (124));
  assert (tad1.alias() == std::vector<std::string> {"key2"});
  assert (tad1.clearAddress());
  assert (ad1.m_ref == 1);

  SG::TransientAddress tad2 (std::move (tad1));
  assert (tad2.name() == "key");
  assert (tad2.clID() == 123);
  assert (tad2.address() == &ad1);
  assert (tad2.storeID() == StoreID::EVENT_STORE);
  assert (tad2.sgkey() == 876);
  assert (tad2.transientID().size() == 2);
  assert (tad2.transientID (123));
  assert (tad2.transientID (124));
  assert (tad2.alias() == std::vector<std::string> {"key2"});
  assert (tad2.clearAddress());
  assert (ad1.m_ref == 1);

  assert (tad1.address() == nullptr);
  assert (tad1.transientID().empty());
  assert (tad1.alias().empty());

  tad2.clearAddress (false);
  tad1 = std::move (tad2);
  assert (tad1.name() == "key");
  assert (tad1.clID() == 123);
  assert (tad1.address() == &ad1);
  assert (tad1.storeID() == StoreID::EVENT_STORE);
  assert (tad1.sgkey() == 876);
  assert (tad1.transientID().size() == 2);
  assert (tad1.transientID (123));
  assert (tad1.transientID (124));
  assert (tad1.alias() == std::vector<std::string> {"key2"});
  assert (!tad1.clearAddress());
  assert (ad1.m_ref == 1);

  assert (tad2.address() == nullptr);
  assert (tad2.transientID().empty());
  assert (tad2.alias().empty());
}


// Testing copy/assignment.
void test3()
{
  std::cout << "test3\n";

  TestAddress ad1(1);
  SG::TransientAddress tad1 (123, "key", &ad1);
  TestProvider tp;
  tad1.setProvider (&tp, StoreID::EVENT_STORE);
  tad1.setSGKey (876);
  tad1.setTransientID (124);
  tad1.setAlias ("key2");

  assert (tad1.name() == "key");
  assert (tad1.clID() == 123);
  assert (tad1.address() == &ad1);
  assert (tad1.storeID() == StoreID::EVENT_STORE);
  assert (tad1.sgkey() == 876);
  assert (tad1.transientID().size() == 2);
  assert (tad1.transientID (123));
  assert (tad1.transientID (124));
  assert (tad1.alias() == std::vector<std::string> {"key2"});
  assert (tad1.clearAddress());
  assert (ad1.m_ref == 1);

  SG::TransientAddress tad2 (tad1);
  assert (tad2.name() == "key");
  assert (tad2.clID() == 123);
  assert (tad2.address() == &ad1);
  assert (tad2.storeID() == StoreID::EVENT_STORE);
  assert (tad2.sgkey() == 876);
  assert (tad2.transientID().size() == 2);
  assert (tad2.transientID (123));
  assert (tad2.transientID (124));
  assert (tad2.alias() == std::vector<std::string> {"key2"});
  assert (tad2.clearAddress());
  assert (ad1.m_ref == 2);

  SG::TransientAddress tad3;
  tad3 = tad1;
  assert (tad3.name() == "key");
  assert (tad3.clID() == 123);
  assert (tad3.address() == &ad1);
  assert (tad3.storeID() == StoreID::EVENT_STORE);
  assert (tad3.sgkey() == 876);
  assert (tad3.transientID().size() == 2);
  assert (tad3.transientID (123));
  assert (tad3.transientID (124));
  assert (tad3.alias() == std::vector<std::string> {"key2"});
  assert (tad3.clearAddress());
  assert (ad1.m_ref == 3);
}


int main()
{
  test1();
  test2();
  test3();
  return 0;
}
