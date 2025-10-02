/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * Executable to test ReadHandle performance with and without views.
 * Author: Frank Winklmeier
 */

#include "AthViews/View.h"
#include "AthViews/ViewHelper.h"

#include "StoreGate/StoreGateSvc.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandle.h"
#include "TestTools/initGaudi.h"

#include "gtest/gtest.h"

#include <chrono>
#include <format>
#include <iostream>
#include <stdexcept>

// Number of iterations for test
const size_t N = 1e6;

// Test classes
struct TestClass {
  int value = 0;
};
CLASS_DEF( TestClass, 16530831, 1 )

typedef std::vector<TestClass*> TestContainer;
CLASS_DEF( TestContainer, 16530833, 1 )


void testView(const EventContext& ctx)
{
  // Store data within view
  auto view = new SG::View( "MyView", -1 );
  auto t1 = std::make_unique<TestClass>();
  {
    SG::WriteHandle<TestClass> wh( "testView" );
    wh.setProxyDict( view ).ignore();
    EXPECT_TRUE( wh.record( std::move( t1 ) ).isSuccess() );
  }

  // Read data
  SG::ReadHandleKey<TestClass> rhk( "testView" );
  rhk.initialize().ignore();

  for (size_t i=0; i<N; i++) {
    auto rh = ViewHelper::makeHandle(view, rhk, ctx);
    EXPECT_TRUE( rh.isValid() );
  }
}


void testSG(const EventContext& ctx)
{
  // Store data
  auto t1 = std::make_unique<TestClass>();
  {
    SG::WriteHandle<TestClass> wh( "test" );
    EXPECT_TRUE( wh.record( std::move( t1 ) ).isSuccess() );
  }

  // Read data
  SG::ReadHandleKey<TestClass> rhk( "test" );
  rhk.initialize().ignore();

  for (size_t i=0; i<N; i++) {
    auto rh = SG::makeHandle(rhk, ctx);
    EXPECT_TRUE( rh.isValid() );
  }
}


struct TimeIt {
  using clock_t = std::chrono::high_resolution_clock;
  ~TimeIt() {
    try{
      std::cout << std::format("{:10} {:>8}\n", name, std::chrono::duration_cast<std::chrono::microseconds>(clock_t::now()-t0));
    } catch ( std::format_error & e){
      std::cerr<<"Format error in ViewPerf_test\n";
    }
  }
  std::string name;
  clock_t::time_point t0{clock_t::now()};
};


int main() {
  ISvcLocator* pSvcLoc;
  EXPECT_TRUE( Athena_test::initGaudi(pSvcLoc) );

  SmartIF<StoreGateSvc> pStore{pSvcLoc->service("StoreGateSvc")};
  EXPECT_TRUE( pStore );

  EventContext ctx;
  try{
    ctx.setExtension( Atlas::ExtendedEventContext(pStore.get()) );
  } catch (std::bad_any_cast & e){
    std::cerr<<"Exception bad_any_cast in ViewPerf_test"<<std::endl;
    return 1;
  }

  {
    TimeIt t{"no View"};
    try{
      testSG(ctx);
    } catch (std::exception & e){
      std::cerr<<"Exception "<<e.what()<<" in ViewPerf_test"<<std::endl;
      return 1;
    }
  }
  {
    TimeIt t{"with View"};
    testView(ctx);
  }

  return 0;
}
