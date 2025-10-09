/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file StoreGate/test/ReadDecorHandleKeyArrau_test.cxx
 * @author Jovan Mitrevski <Jovan.Mitrevski.cern.ch>
 * @date Oct, 2018
 * @brief Tests for ReadDecorHandleKeyArray; based on the tests for WriteDecorHandleKeyArray
 */


#undef NDEBUG
#include "StoreGate/ReadDecorHandleKeyArray.h"
#include "StoreGate/exceptions.h"
#include "SGTools/TestStore.h"
#include "AthenaKernel/CLASS_DEF.h"
#include "TestTools/initGaudi.h"
#include "TestTools/expect_exception.h"
#include <cassert>
#include <iostream>


class MyObj {};
CLASS_DEF (MyObj, 293847295, 1)


class TestHolder
  : public implements<IDataHandleHolder>
{
public:
  virtual std::vector<Gaudi::DataHandle*> inputHandles() const override;
  virtual std::vector<Gaudi::DataHandle*> outputHandles() const override;
  virtual void addDependency(const DataObjID&, const Gaudi::DataHandle::Mode&) override;

  virtual const std::string& name() const override { std::abort(); }


  virtual const DataObjIDColl& extraInputDeps() const override { std::abort(); }
  virtual const DataObjIDColl& extraOutputDeps() const override { std::abort(); }

  virtual void acceptDHVisitor(IDataHandleVisitor*) const override { std::abort(); }

  virtual const DataObjIDColl& inputDataObjs() const override { std::abort(); }
  virtual const DataObjIDColl& outputDataObjs() const override { std::abort(); }

  virtual void declare(Gaudi::DataHandle&) override { std::abort(); }
  virtual void renounce(Gaudi::DataHandle&) override { std::abort(); }
  virtual bool renounceInput(const DataObjID&) override { std::abort(); }

  std::vector<Gaudi::DataHandle*> m_inputHandles;
  std::vector<Gaudi::DataHandle*> m_outputHandles;
  std::vector<DataObjID> m_deps;
};


std::vector<Gaudi::DataHandle*> TestHolder::inputHandles() const
{
  return m_inputHandles;
}


std::vector<Gaudi::DataHandle*> TestHolder::outputHandles() const
{
  return m_outputHandles;
}


void TestHolder::addDependency(const DataObjID& id, const Gaudi::DataHandle::Mode&)
{
  m_deps.push_back (id);
}



void test1()
{
  std::cout << "test1\n";

  SG::ReadDecorHandleKeyArray<MyObj> k1{"aaa.dec1", "aaa.dec2"};
  assert (k1[0].clid() == 293847295);
  assert (k1[0].key() == "aaa.dec1");
  assert (k1[0].mode() == Gaudi::DataHandle::Reader);
  assert (k1[0].storeHandle().name() == "StoreGateSvc");
  assert (!k1[0].storeHandle().isSet());

  assert (k1.initialize().isSuccess());
  assert (k1[0].storeHandle().isSet());

  k1 = {"bbb.foo1", "bbb.foo2", "bbb.foo3"};
  assert (k1[2].key() == "bbb.foo3");

  std::vector<std::string> vv {"ccc.fee1", "ccc.fee2", "ccc.fee3"};
  assert (k1.assign (vv).isSuccess());
  assert (k1[1].key() == "ccc.fee2");

  SG::ReadHandleKey<MyObj> ok ("aaa");
  SG::ReadDecorHandleKeyArray<MyObj> k2{ok, {"dec1", "dec2"}};
  assert (k2.size() == 2);
  assert (k2[0].clid() == 293847295);
  assert (k2[0].key() == "aaa.dec1");
  assert (k2[1].key() == "aaa.dec2");
  assert (k2[0].mode() == Gaudi::DataHandle::Reader);
  assert (k2[0].storeHandle().name() == "StoreGateSvc");
  assert (!k2[0].storeHandle().isSet());

  EXPECT_EXCEPTION(SG::ExcBadHandleKey,
                   (SG::ReadDecorHandleKeyArray<MyObj> {ok, {"aaa.dec1", "aaa.dec2"}}));
}

void test1a()
{
  std::cout << "test1a\n";

  SG::ReadDecorHandleKeyArray<MyObj, int> k1{"aaa.dec1", "aaa.dec2"};
  assert (k1[0].clid() == 293847295);
  assert (k1[0].key() == "aaa.dec1");
  assert (k1[0].mode() == Gaudi::DataHandle::Reader);
  assert (k1[0].storeHandle().name() == "StoreGateSvc");
  assert (!k1[0].storeHandle().isSet());

  assert (k1.initialize().isSuccess());
  assert (k1[0].storeHandle().isSet());

  k1 = {"bbb.foo1", "bbb.foo2", "bbb.foo3"};
  assert (k1[2].key() == "bbb.foo3");

  std::vector<std::string> vv {"ccc.fee1", "ccc.fee2", "ccc.fee3"};
  assert (k1.assign (vv).isSuccess());
  assert (k1[1].key() == "ccc.fee2");

}


void test2()
{
  std::cout << "test2" << std::endl;

  TestHolder h;
  SG::ReadDecorHandleKeyArray<MyObj> k1{"aaa.dec1", "aaa.dec2"};
  k1.setOwner(&h);

  SG::ReadHandleKey<MyObj> r1 ("rrr");
  SG::WriteHandleKey<MyObj> w1 ("sss");
  h.m_inputHandles.push_back (&r1);
  h.m_outputHandles.push_back (&w1);
  assert (k1.initialize().isSuccess());
}


int main()
{
  ISvcLocator* pDum;
  Athena_test::initGaudi(pDum); //need MessageSvc

  test1();
  test1a();
  test2();
  return 0;
}
