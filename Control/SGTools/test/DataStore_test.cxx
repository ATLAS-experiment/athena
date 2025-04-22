/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file DataStore_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Nov, 2013
 * @brief Regression tests for DataStore.
 */

#undef NDEBUG
#include "SGTools/DataStore.h"
#include "SGTools/StringPool.h"
#include "SGTools/DataProxy.h"
#include "SGTools/TestStore.h"
#include "SGTools/TransientAddress.h"
#include "AthenaKernel/IProxyDict.h"
#include "AthenaKernel/getMessageSvc.h"
#include "AthenaKernel/IAddressProvider.h"
#include "AthenaKernel/CLASS_DEF.h"
#include "AthenaKernel/BaseInfo.h"
#include "CxxUtils/checker_macros.h"
#include "TestTools/random.h"
#include "boost/timer/timer.hpp"
#include <iostream>
#include <cstdlib>
#include <cassert>


struct Base {};
struct Derived : public Base {};
static constexpr CLID Base_CLID = 89472398;
static constexpr CLID Derived_CLID = 89472399;
CLASS_DEF( Base, Base_CLID, 1 )
CLASS_DEF( Derived, Derived_CLID, 1 )
SG_BASES( Derived, Base );


class TestProvider
  : public implements<IAddressProvider>
{
public:

  virtual StatusCode updateAddress(StoreID::type /*storeID*/,
				   SG::TransientAddress* /*pTAd*/,
                                   const EventContext& /*ctx*/) override
  { return StatusCode::SUCCESS; }
};


SG::DataProxy* make_proxy (CLID clid,
                           const std::string& name,
                           SG::sgkey_t sgkey = 0)
{
  auto tad = std::make_unique<SG::TransientAddress> (clid, name);
  if (sgkey) {
    tad->setSGKey (sgkey);
  }
  return new SG::DataProxy (std::move(tad), static_cast<IConverter*>(nullptr));
}


void test_ctor()
{
  std::cout << "test_ctor\n";
  SGTest::TestStore pool;
  SG::DataStore store (pool);
  assert (store.storeID() == StoreID::UNKNOWN);
  store.setStoreID (StoreID::EVENT_STORE);
  assert (store.storeID() == StoreID::EVENT_STORE);
}


void test_addToStore ATLAS_NOT_THREAD_SAFE ()
{
  std::cout << "test_addToStore\n";

  SGTest::TestStore pool;
  SG::DataStore store (pool);
  SG::StringPool::sgkey_t sgkey = pool.stringToKey ("dp1", 123);

  assert (store.addToStore (123, 0).isFailure());
  SG::DataProxy* dp1 = make_proxy (123, "dp1");
  assert (dp1->refCount() == 0);
  assert (store.addToStore (123, dp1).isSuccess());
  assert (dp1->store() == &pool);
  assert (dp1->refCount() == 1);
  assert (dp1->sgkey() == sgkey);
  assert (store.proxy_exact (sgkey) == dp1);
  assert (store.proxy (123, "dp1") == dp1);

  int trans1;
  dp1->registerTransient (&trans1);
  assert (store.locatePersistent (&trans1) == dp1);

  SG::DataProxy* dp2 = make_proxy (123, "dp2");
  assert (store.addToStore (124, dp2).isSuccess());
  SG::StringPool::sgkey_t sgkey2a = pool.stringToKey ("dp2", 123);
  SG::StringPool::sgkey_t sgkey2b = pool.stringToKey ("dp2", 124);
  assert (store.proxy_exact (sgkey2b) == dp2);
  assert (store.proxy_exact (sgkey2a) == 0);
  assert (dp2->sgkey() == sgkey2b);
}


void test_addAlias ATLAS_NOT_THREAD_SAFE ()
{
  std::cout << "test_addAlias\n";

  SGTest::TestStore pool;
  SG::DataStore store (pool);

  SG::DataProxy* dp1 = make_proxy (123, "dp1");
  assert (store.addToStore (123, dp1).isSuccess());

  assert (store.addAlias ("dp1a", dp1).isSuccess());
  assert (store.addAlias ("dp1b", dp1).isSuccess());
  assert (dp1->refCount() == 3);

  assert (store.proxy (123, "dp1a") == dp1);
  assert (store.proxy_exact (pool.stringToKey ("dp1a", 123)) == dp1);

  assert (dp1->hasAlias("dp1a"));

  SG::DataProxy* dp2 = make_proxy (123, "dp2");
  assert (store.addToStore (123, dp2).isSuccess());
  assert (store.addAlias ("dpx", dp2).isSuccess());
  assert (dp2->refCount() == 2);
  assert (dp2->hasAlias("dpx"));

  assert (store.addAlias ("dpx", dp1).isSuccess());
  assert (dp1->refCount() == 4);
  assert (dp2->refCount() == 1);
  assert (!dp2->hasAlias("dpx"));
  assert (dp1->hasAlias("dpx"));
  assert (store.addAlias ("dpx", dp1).isSuccess());
  assert (dp1->refCount() == 4);
  assert (dp2->refCount() == 1);
}


void test_addSymLink()
{
  std::cout << "test_addSymLink\n";

  SGTest::TestStore pool;
  SG::DataStore store (pool);

  SG::DataProxy* dp1 = make_proxy (123, "dp1");
  assert (store.addToStore (123, dp1).isSuccess());
  assert (store.addSymLink (124, dp1).isSuccess());

  assert (store.proxy_exact (123, "dp1") == dp1);
  assert (store.proxy_exact (124, "dp1") == dp1);
  assert (dp1->transientID(123));
  assert (dp1->transientID(124));

  assert (store.addSymLink (124, dp1).isSuccess());
  assert (store.proxy_exact (124, "dp1") == dp1);

  SG::DataProxy* dp2 = make_proxy (125, "dp1");
  assert (store.addToStore (125, dp2).isSuccess());
  assert (store.addSymLink (125, dp1).isFailure());
  assert (!dp1->transientID(125));
}


// Testing handling of an existing proxy.
void test_addSymLink2()
{
  std::cout << "test_addSymLink2\n";

  SGTest::TestStore pool;
  SG::DataStore store (pool);

  // Existing proxy referencing the same DO.
  {
    auto doa = new DataObject;
    SG::DataProxy* dpa1 = make_proxy (Base_CLID, "dpa");
    dpa1->setObject (doa);
    assert (store.addToStore (Base_CLID, dpa1).isSuccess());
    SG::DataProxy* dpa2 = make_proxy (Derived_CLID, "dpa");
    dpa2->setObject (doa);
    assert (store.addToStore (Derived_CLID, dpa2).isSuccess());
    assert (store.addSymLink (Base_CLID, dpa2).isSuccess());

    assert (store.proxy_exact (Base_CLID, "dpa") == dpa1);
    assert (store.proxy_exact (Derived_CLID, "dpa") == dpa2);
    assert (dpa1->transientID(Base_CLID));
    assert (dpa2->transientID(Base_CLID));
    assert (dpa2->transientID(Derived_CLID));
  }

  // Existing proxy with empty new proxy.
  {
    auto dob = new DataObject;
    SG::DataProxy* dpb1 = make_proxy (Base_CLID, "dpb");
    dpb1->setObject (dob);
    assert (store.addToStore (Base_CLID, dpb1).isSuccess());
    SG::DataProxy* dpb2 = make_proxy (Derived_CLID, "dpb");
    assert (store.addToStore (Derived_CLID, dpb2).isSuccess());
    assert (store.addSymLink (Base_CLID, dpb2).isFailure());
  }

  // Empty existing proxy; new proxy with no Base/derived relation.
  {
    auto doc = new DataObject;
    SG::DataProxy* dpc1 = make_proxy (Base_CLID, "dpc");
    assert (store.addToStore (Base_CLID, dpc1).isSuccess());
    SG::DataProxy* dpc2 = make_proxy (125, "dpc");
    dpc2->setObject (doc);
    assert (store.addToStore (125, dpc2).isSuccess());
    assert (store.addSymLink (Base_CLID, dpc2).isFailure());
  }

  // Empty existing proxy; new proxy derived object.
  {
    auto dod = new DataObject;
    SG::DataProxy* dpd1 = make_proxy (Base_CLID, "dpd");
    assert (store.addToStore (Base_CLID, dpd1).isSuccess());
    SG::DataProxy* dpd2 = make_proxy (Derived_CLID, "dpd");
    dpd2->setObject (dod);
    assert (store.addToStore (Derived_CLID, dpd2).isSuccess());
    assert (store.addSymLink (Base_CLID, dpd2).isSuccess());

    assert (store.proxy_exact (Base_CLID, "dpd") == dpd1);
    assert (store.proxy_exact (Derived_CLID, "dpd") == dpd2);
    assert (dpd1->object() == dod);
    assert (dpd2->object() == dod);
    assert (dod->refCount() == 2);
    assert (dpd1->transientID(Base_CLID));
    assert (dpd2->transientID(Base_CLID));
    assert (dpd2->transientID(Derived_CLID));
  }
}


void test_proxy_exact ATLAS_NOT_THREAD_SAFE ()
{
  std::cout << "test_proxy_exact\n";

  SGTest::TestStore pool;
  SG::DataStore store (pool);

  SG::DataProxy* dp1 = make_proxy (123, "dp1");
  assert (store.addToStore (123, dp1).isSuccess());

  assert (store.proxy_exact (123, "dp1") == dp1);
  assert (store.proxy_exact (124, "dp1") == 0);
  assert (store.proxy_exact (123, "dp2") == 0);
  SG::StringPool::sgkey_t sgkey = pool.stringToKey ("dp1", 123);
  assert (store.proxy_exact (sgkey) == dp1);
  assert (store.proxy_exact (sgkey+1) == 0);
}


void test_proxy()
{
  std::cout << "test_proxy\n";

  SGTest::TestStore pool;
  SG::DataStore store (pool);

  SG::DataProxy* dp1 = make_proxy (123, "dp1");
  assert (store.addToStore (123, dp1).isSuccess());

  assert (store.proxy (123, "dp1") == dp1);
  assert (store.proxy (123, "") == dp1);

  {
    SG::TransientAddress ta (123, "dp1");
    assert (store.proxy (&ta) == dp1);
  }

  assert (store.addAlias ("dp1a", dp1).isSuccess());
  assert (store.addAlias ("dp1b", dp1).isSuccess());
  assert (store.proxy (123, "") == dp1);

  SG::DataProxy* dp2 = make_proxy (124, "dp2");
  assert (store.addToStore (123, dp2).isSuccess());
  assert (store.proxy (123, "") == dp1);

  SG::DataProxy* dp3 = make_proxy (123, "dp3");
  assert (store.addToStore (123, dp3).isSuccess());
  assert (store.proxy (123, "") == nullptr);
  assert (store.proxy (123, "dp3") == dp3);
}


void test_typeCount()
{
  std::cout << "test_typeCount\n";

  SGTest::TestStore pool;
  SG::DataStore store (pool);

  SG::DataProxy* dp1 = make_proxy (123, "dp1");
  assert (store.addToStore (123, dp1).isSuccess());
  assert (store.addSymLink (124, dp1).isSuccess());

  SG::DataProxy* dp2 = make_proxy (123, "dp2");
  assert (store.addToStore (123, dp2).isSuccess());

  assert (store.typeCount (123) == 2);
  assert (store.typeCount (124) == 1);
  assert (store.typeCount (125) == 0);
}


void test_tRange()
{
  std::cout << "test_tRange\n";

  SGTest::TestStore pool;
  SG::DataStore store (pool);

  SG::DataProxy* dp1 = make_proxy (123, "dp1");
  assert (store.addToStore (123, dp1).isSuccess());
  assert (store.addSymLink (124, dp1).isSuccess());

  SG::DataProxy* dp2 = make_proxy (123, "dp2");
  assert (store.addToStore (123, dp2).isSuccess());

  SG::DataStore::ConstStoreIterator tbeg;
  SG::DataStore::ConstStoreIterator tend;
  assert (store.tRange (tbeg, tend).isSuccess());

  assert (tbeg->first == 123);
  assert (tbeg->second.size() == 2);
  ++tbeg;
  assert (tbeg->first == 124);
  assert (tbeg->second.size() == 1);
  ++tbeg;
  assert (tbeg == tend);
}


void test_pRange()
{
  std::cout << "test_pRange\n";

  SGTest::TestStore pool;
  SG::DataStore store (pool);

  SG::DataProxy* dp1 = make_proxy (123, "dp1");
  assert (store.addToStore (123, dp1).isSuccess());
  assert (store.addSymLink (124, dp1).isSuccess());

  SG::DataProxy* dp2 = make_proxy (123, "dp2");
  assert (store.addToStore (123, dp2).isSuccess());

  SG::ConstProxyIterator pbeg;
  SG::ConstProxyIterator pend;

  assert (store.pRange (123, pbeg, pend).isSuccess());
  assert (pbeg->first == "dp1");
  assert (pbeg->second == dp1);
  ++pbeg;
  assert (pbeg->first == "dp2");
  assert (pbeg->second == dp2);
  ++pbeg;
  assert (pbeg == pend);

  assert (store.pRange (124, pbeg, pend).isSuccess());
  assert (pbeg->first == "dp1");
  assert (pbeg->second == dp1);
  ++pbeg;
  assert (pbeg == pend);

  assert (store.pRange (125, pbeg, pend).isFailure());
  assert (pbeg == pend);
}


void test_keys()
{
  std::cout << "test_keys\n";

  SGTest::TestStore pool;
  SG::DataStore store (pool);

  SG::DataProxy* dp1 = make_proxy (123, "dp1");
  assert (store.addToStore (123, dp1).isSuccess());
  assert (store.addSymLink (124, dp1).isSuccess());

  SG::DataProxy* dp2 = make_proxy (123, "dp2");
  assert (store.addToStore (123, dp2).isSuccess());

  std::vector<std::string> v;
  store.keys (123, v, true, false);
  assert (v.size() == 2);
  assert (v[0] == "dp1");
  assert (v[1] == "dp2");

  store.keys (124, v, true, false);
  assert (v.size() == 1);
  assert (v[0] == "dp1");

  assert (store.addAlias ("dp1a", dp1).isSuccess());
  store.keys (123, v, false, false);
  assert (v.size() == 2);
  assert (v[0] == "dp1");
  assert (v[1] == "dp2");

  store.keys (123, v, true, false);
  assert (v.size() == 3);
  assert (v[0] == "dp1");
  assert (v[1] == "dp1a");
  assert (v[2] == "dp2");

  SG::DataProxy* dp3 = make_proxy (125, "dp3");
  SG::DataProxy* dp4 = make_proxy (125, "dp4");
  assert (store.addToStore (125, dp3).isSuccess());
  assert (store.addToStore (125, dp4).isSuccess());
  TestProvider prov;
  dp4->setProvider (&prov, StoreID::EVENT_STORE);

  store.keys (125, v, true, false);
  assert (v.size() == 2);
  assert (v[0] == "dp3");
  assert (v[1] == "dp4");

  store.keys (125, v, true, true);
  assert (v.size() == 1);
  assert (v[0] == "dp4");
}


void test_removeProxy ATLAS_NOT_THREAD_SAFE ()
{
  std::cout << "test_removeProxy\n";

  SGTest::TestStore pool;
  SG::DataStore store (pool);

  assert (store.removeProxy (0, false, false).isFailure());

  SG::DataProxy* dp1 = make_proxy (123, "dp1");
  dp1->addRef();
  assert (store.addToStore (123, dp1).isSuccess());
  assert (store.addSymLink (124, dp1).isSuccess());
  assert (store.addAlias ("dp1x", dp1).isSuccess());

  assert (store.proxy_exact (pool.stringToKey ("dp1", 123)) == dp1);
  assert (store.proxy_exact (pool.stringToKey ("dp1", 124)) == dp1);
  assert (store.proxy_exact (pool.stringToKey ("dp1x", 123)) == dp1);
  assert (store.proxy (123, "dp1") == dp1);
  assert (store.proxy (124, "dp1") == dp1);
  assert (store.proxy (123, "dp1x") == dp1);

  assert (dp1->refCount() == 5);

  dp1->resetOnly (true);
  assert (store.removeProxy (dp1, false, false).isSuccess());
  assert (dp1->refCount() == 5);

  dp1->resetOnly (false);
  assert (store.removeProxy (dp1, false, false).isSuccess());
  assert (dp1->refCount() == 1);
  assert (store.proxy_exact (pool.stringToKey ("dp1", 123)) == 0);
  assert (store.proxy_exact (pool.stringToKey ("dp1", 124)) == 0);
  assert (store.proxy_exact (pool.stringToKey ("dp1x", 123)) == 0);
  assert (store.proxy (123, "dp1") == 0);
  assert (store.proxy (124, "dp1") == 0);
  assert (store.proxy (123, "dp1x") == 0);
  dp1->release();

  //==============================================

  // Test recording with a secondary CLID first.
  SG::DataProxy* dp2 = make_proxy (223, "dp2");
  dp2->resetOnly (false);
  dp2->addRef();
  dp2->setTransientID (223);
  dp2->setTransientID (224);
  assert (store.addToStore (224, dp2).isSuccess());
  assert (store.addToStore (223, dp2).isSuccess());
  assert (dp2->refCount() == 3);
  assert (store.removeProxy (dp2, false, false).isSuccess());
  assert (dp2->refCount() == 1);
  assert (store.proxy_exact (pool.stringToKey ("dp2", 223)) == 0);
  assert (store.proxy_exact (pool.stringToKey ("dp2", 224)) == 0);
  dp2->release();
}


void test_clearStore ATLAS_NOT_THREAD_SAFE ()
{
  std::cout << "test_clearStore\n";

  SGTest::TestStore pool;
  SG::DataStore store (pool);

  SG::DataProxy* dp1 = make_proxy (123, "dp1");
  DataObject* o1 = new DataObject;
  dp1->setObject (o1);
  dp1->addRef();
  assert (store.addToStore (123, dp1).isSuccess());
  assert (store.addSymLink (124, dp1).isSuccess());
  dp1->resetOnly (false);

  SG::DataProxy* dp2 = make_proxy (123, "dp2");
  assert (store.addToStore (123, dp2).isSuccess());
  DataObject* o2 = new DataObject;
  dp2->setObject (o2);
  dp2->addRef();

  assert (dp1->refCount() == 3);
  assert (dp2->refCount() == 2);

  int trans1;
  dp1->registerTransient (&trans1);
  assert (store.locatePersistent (&trans1) == dp1);
  int trans2;
  dp2->registerTransient (&trans2);
  assert (store.locatePersistent (&trans2) == dp2);

  store.clearStore (false, false, nullptr);

  assert (store.locatePersistent (&trans1) == 0);
  assert (store.locatePersistent (&trans2) == 0);

  assert (dp1->refCount() == 1);
  assert (dp1->object() == o1);
  assert (store.proxy (123, "dp1") == 0);
  assert (store.proxy (124, "dp1") == 0);
  assert (store.proxy_exact (pool.stringToKey ("dp1", 123)) == 0);
  assert (store.proxy_exact (pool.stringToKey ("dp1", 124)) == 0);

  assert (dp2->refCount() == 2);
  assert (dp2->object() == 0);
  assert (store.proxy (123, "dp2") == dp2);
  assert (store.proxy_exact (pool.stringToKey ("dp2", 123)) == dp2);

  store.clearStore (true, false, nullptr);
  assert (dp2->refCount() == 1);
  assert (store.proxy (123, "dp2") == 0);
  assert (store.proxy_exact (pool.stringToKey ("dp2", 123)) == 0);
}


void test_t2p()
{
  std::cout << "test_t2p\n";

  SGTest::TestStore pool;
  SG::DataStore store (pool);

  SG::DataProxy* dp1 = make_proxy (123, "dp1");
  assert (store.addToStore (123, dp1).isSuccess());

  int trans1 = 0;
  assert (store.locatePersistent (&trans1) == 0);
  assert (store.t2pRegister (&trans1, dp1).isSuccess());
  assert (store.locatePersistent (&trans1) == dp1);
  assert (store.locatePersistent (&trans1) == dp1);
  assert (store.t2pRegister (&trans1, dp1).isFailure());
  store.t2pRemove (&trans1);
  assert (store.locatePersistent (&trans1) == 0);
}


void test_dummy ATLAS_NOT_THREAD_SAFE ()
{
  std::cout << "test_dummy\n";

  SGTest::TestStore pool;
  SG::DataStore store (pool);

  SG::StringPool::sgkey_t sgkey = pool.stringToKey ("dp1", 456);

  SG::DataProxy* dp1 = make_proxy (0, "", sgkey);
  assert (store.addToStore (0, dp1).isSuccess());
  assert (dp1->refCount() == 1);
  assert (store.proxy_exact (sgkey) == dp1);

  assert (store.proxy (456, "dp1") == dp1);
  assert (dp1->clID() == 456);
  assert (dp1->name() == "dp1");
  assert (dp1->refCount() == 1);

  SG::StringPool::sgkey_t sgkey2 = pool.stringToKey ("dp2", 456);
  SG::DataProxy* dp2 = make_proxy (0, "", sgkey2);
  assert (store.addToStore (0, dp2).isSuccess());
  assert (dp2->refCount() == 1);
  assert (store.proxy_exact (sgkey2) == dp2);

  assert (store.proxy (456, "dp2") == dp2);
  assert (dp2->clID() == 456);
  assert (dp2->name() == "dp2");
  assert (dp2->refCount() == 1);

  SG::StringPool::sgkey_t sgkey3 = pool.stringToKey ("dp3", 456);
  SG::DataProxy* dp3 = make_proxy (0, "", sgkey3);
  assert (store.addToStore (0, dp3).isSuccess());
  assert (dp3->refCount() == 1);
  assert (store.proxy_exact (sgkey3) == dp3);

  dp1->addRef();
  dp2->addRef();
  dp3->addRef();
  assert (dp1->refCount() == 2);
  assert (dp2->refCount() == 2);
  assert (dp3->refCount() == 2);

  store.clearStore (true, false, nullptr);
  assert (dp1->refCount() == 1);
  assert (dp2->refCount() == 1);
  assert (dp3->refCount() == 1);
}


//*****************************************************************************
// Timing test for clearStore().


std::string make_name (size_t& icount)
{
  return "prox" + std::to_string (++icount);
}


std::vector<SG::DataProxy*> make_proxies (size_t n,
                                          bool resetOnly,
                                          size_t& icount,
                                          uint32_t& seed,
                                          IStringPool& pool)
{
  std::vector<SG::DataProxy*> ret;
  ret.reserve (n);
  for (size_t i = 0; i < n; i++) {
    std::string name = make_name (icount);
    CLID clid = 123;
    SG::StringPool::sgkey_t sgkey = pool.stringToKey (name, clid);
    SG::DataProxy* dp = make_proxy (clid, name, sgkey);
    dp->resetOnly (resetOnly);
    dp->addRef();

    std::string alias_name;
    if (Athena_test::randi_seed (seed, 10) == 0) {
      alias_name = make_name (icount);
      dp->setAlias (alias_name);
    }
    
    int nsymlink = Athena_test::randi_seed (seed, 4);
    for (int i = 0; i < nsymlink; i++) {
      dp->setTransientID (++clid);
    }
    
    ret.push_back (dp);
  }
  return ret;
}


void addProxies (const std::vector<SG::DataProxy*>& dps, SG::DataStore& store)
{
  for (SG::DataProxy* dp : dps) {
    for (CLID clid : dp->transientID()) {
      assert (store.addToStore (clid, dp).isSuccess());
    }
    for (const std::string& alias : dp->alias()) {
      assert (store.addAlias (alias, dp).isSuccess());
    }
  }
}


void check_proxy (const SG::DataProxy& dp)
{
  if (!dp.isResetOnly()) {
    assert (dp.refCount() == 1);
  }
  else {
    size_t nclid = dp.transientID().size();
    size_t nalias = dp.alias().size();
    size_t nref = 1 + nclid * (nalias+1);
    assert (dp.refCount() == nref);
  }
}


class Timer
{
public:
  Timer();

  class RunTimer
  {
  public:
    RunTimer (boost::timer::cpu_timer& timer) : m_timer (&timer)
    { timer.resume(); }
    RunTimer (RunTimer&& other) : m_timer (other.m_timer) { other.m_timer = nullptr; }
    ~RunTimer() { if (m_timer) m_timer->stop(); }
  private:
    boost::timer::cpu_timer* m_timer;
  };
  RunTimer run() { return RunTimer (m_timer); }

  std::string format() const { return m_timer.format(3); }

private:
  boost::timer::cpu_timer m_timer;
};


Timer::Timer()
{
  m_timer.stop();
}


void time_clear (size_t niter)
{
  SGTest::TestStore pool;
  SG::DataStore store (pool);
  size_t icount = 0;
  uint32_t seed = 123;
  std::vector<SG::DataProxy*> persProxies = make_proxies (300, true, icount,
                                                          seed, pool);
  std::vector<SG::DataProxy*> transProxies = make_proxies (700, false,
                                                           icount, seed, pool);
  addProxies (persProxies, store);

  Timer tclear;
  Timer ttotal;

  {
    Timer::RunTimer rt_total = ttotal.run();
    for (size_t i = 0; i < niter; i++) {
      addProxies (transProxies, store);
      {
        Timer::RunTimer rt_clear = tclear.run();
        store.clearStore (false, false, nullptr);
      }
      for (const SG::DataProxy* dp : persProxies) check_proxy (*dp);
      for (const SG::DataProxy* dp : transProxies) check_proxy (*dp);
    }
  }
  std::cout << "clear " << tclear.format();
  std::cout << "total " << ttotal.format();
}


//*****************************************************************************


int main ATLAS_NOT_THREAD_SAFE (int argc, char** argv)
{
  Athena::getMessageSvcQuiet = true;

  if (argc > 1 && strncmp (argv[1], "--timeClear", 11) == 0) {
    size_t niter = 1000;
    if (argv[1][11] == '=') {
      niter = std::stoul (std::string (argv[1] + 12));
    }
    time_clear (niter);
    return 0;
  }
  test_ctor();
  test_addToStore();
  test_addAlias();
  test_addSymLink();
  test_addSymLink2();
  test_proxy_exact();
  test_proxy();
  test_typeCount();
  test_tRange();
  test_pRange();
  test_keys();
  test_removeProxy();
  test_clearStore();
  test_t2p();
  // pac routines not tested.
  test_dummy();
  return 0;
}


// handle retrieval of dummied dp
// insert dummied dp
