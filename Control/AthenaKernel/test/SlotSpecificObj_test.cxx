/*
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 */

/**
 * @file AthenaKernel/test/SlotSpecificObj_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Jul, 2017
 * @brief Regression tests for SlotSpecificObj.
 */

#undef NDEBUG
#include "AthenaKernel/SlotSpecificObj.h"
#include "AthenaKernel/errorcheck.h"
#include "CxxUtils/checker_macros.h"
#include "TestTools/initGaudi.h"
#include "TestTools/expect_exception.h"
#include "GaudiKernel/Service.h"
#include "GaudiKernel/IHiveWhiteBoard.h"
#include <cassert>
#include <iostream>
#include <print>
#include <iterator>
#include <stdexcept>

const size_t nslots = 4;


class TestWhiteBoard
  : public extends<Service, IHiveWhiteBoard>
{
public:
  TestWhiteBoard (const std::string& name, ISvcLocator* svc)
    : base_class (name, svc)
  {}
  
  virtual StatusCode selectStore(size_t /*partitionIndex*/) override { std::abort(); }
  virtual StatusCode clearStore(size_t /*partitionIndex*/) override { std::abort(); }
  virtual StatusCode setNumberOfStores(size_t /*slots*/) override { std::abort(); }
  virtual bool exists( const DataObjID& ) override { std::abort(); } 
  virtual size_t allocateStore( int /*evtnumber*/ ) override { std::abort(); }
  virtual StatusCode freeStore( size_t /*partitionIndex*/ ) override { std::abort(); }
  virtual size_t getPartitionNumber(int /*eventnumber*/) const override { std::abort(); }
  virtual size_t getNumberOfStores() const override { return nslots; }
  virtual size_t freeSlots() override { std::abort(); }

};


DECLARE_COMPONENT( TestWhiteBoard )


struct Payload
{
  size_t x = 0;
};


template <SG::InvalidSlot S>
void test1()
{
  std::println ("test1");
  SG::SlotSpecificObj<Payload, S> o;
  const SG::SlotSpecificObj<Payload, S>& co = o;

  // One extra entry for testing the invalid slot
  const size_t N = (S==SG::InvalidSlot::Disabled ? nslots : nslots +1);

  for (size_t i = 0; i < N; i++) {
    EventContext ctx (0, i);
    o.get(ctx)->x = (i+1)*10;
  }

  for (size_t i = 0; i < N; i++) {
    EventContext ctx (0, i);
    assert (o.get(ctx)->x == (i+1)*10);
    assert (co.get(ctx)->x == (i+1)*10);
  }

  for (size_t i = 0; i < N; i++) {
    EventContext ctx (0, i);
    Gaudi::Hive::setCurrentContext (ctx);
    assert (o.get()->x == (i+1)*10);
    assert (co.get()->x == (i+1)*10);
    assert (o->x == (i+1)*10);
    assert (co->x == (i+1)*10);
    assert ((*o).x == (i+1)*10);
    assert ((*co).x == (i+1)*10);

    o->x = (i+2)*20;
  }

  for (size_t i = 0; i < N; i++) {
    EventContext ctx (0, i);
    assert (o.get(ctx)->x == (i+2)*20);
  }

  // Iterator
  assert (std::distance(o.begin(), o.end()) == static_cast<int>(N));

  size_t i = 0;
  for (Payload& p : o) {
    assert (p.x == (i+2)*20);
    p.x = (i+2)*30;
    ++i;
  }

  i = 0;
  for (const Payload& p : co) {
    assert (p.x == (i+2)*30);
    ++i;
  }

  // out of range
  {
    EventContext ctx (0, nslots+10);
    EXPECT_EXCEPTION (std::out_of_range, o.get(ctx));
    EXPECT_EXCEPTION (std::out_of_range, co.get(ctx));
  }

  // Invalid context
  EventContext invalid_ctx;
  if constexpr (S==SG::InvalidSlot::Disabled) {
    EXPECT_EXCEPTION (std::out_of_range, o.get(invalid_ctx));
    EXPECT_EXCEPTION (std::out_of_range, co.get(invalid_ctx));
  }
  else {
    assert (o.get(invalid_ctx) == &*std::prev(o.end()));
    assert (co.get(invalid_ctx) == &*std::prev(co.end()));
  }
}

int main ATLAS_NOT_THREAD_SAFE ()
{
  SG::setNSlotsHiveMgrName ("TestWhiteBoard");
  errorcheck::ReportMessage::hideErrorLocus();
  ISvcLocator* svcloc = 0;
  if (!Athena_test::initGaudi("SlotSpecificObj_test.txt", svcloc)) {
    std::println (std::cerr, "This test can not be run");
    return 1;
  }  
  assert(svcloc);

  test1<SG::InvalidSlot::Disabled>();
  test1<SG::InvalidSlot::Enabled>();
  return 0;
}
