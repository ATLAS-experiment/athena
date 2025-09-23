/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "GaudiKernel/ClassID.h"
#include "GaudiKernel/MsgStream.h"
#include "StoreGate/StoreGateSvc.h"
#include "AthenaKernel/getMessageSvc.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

#include "TestTools/initGaudi.h"
#include "AthViews/View.h"
#include "AthViews/ViewHelper.h"

#include "gtest/gtest.h"

struct TestClass {
  int value = 0;
};
CLASS_DEF( TestClass, 16530831, 1 )

typedef std::vector<TestClass*> TestContainer;
CLASS_DEF( TestContainer, 16530833, 1 )

using SG::View;


void testProxy() {
  // Make view
  auto view = new View( "MyView", -1 );
  auto t1 = std::make_unique<TestClass>();
  t1->value = 1;

  // Write data
  {
    SG::WriteHandle<TestClass> wh( "test" );
    wh.setProxyDict( view ).ignore();
    auto status = wh.record( std::move( t1 ) );
    EXPECT_TRUE( status.isSuccess() );
  }

  // Read data
  {
    SG::ReadHandleKey<TestClass> rhk( "test" );
    EXPECT_TRUE( rhk.initialize().isSuccess() );

    auto rh = SG::makeHandle(rhk);
    rh.setProxyDict( view ).ignore();

    // Retrieve via CLID and name (in view)
    SG::DataProxy* proxy1 = view->proxy(rhk.clid(), "test");
    EXPECT_TRUE( proxy1 && proxy1->isValid() );

    SG::ReadHandleKey<TestClass> rhk2( "test" );
    EXPECT_TRUE( rhk2.initialize().isSuccess() );

    // Retrieve via hashed key
    SG::DataProxy* proxy2 = view->proxy_exact(rhk.hashedKey());
    EXPECT_TRUE( proxy2 && proxy2->isValid() );
    EXPECT_TRUE( proxy1 == proxy2 );
  }
}


void testDataInView( const EventContext& ctx, MsgStream& log ) {
  // Make parent view
  auto parentView = new View( "ParentView", -1 );
  auto t1 = std::make_unique<TestClass>();
  t1->value = 1;
  {
    SG::WriteHandleKey<TestClass> whk( "test1" );
    EXPECT_TRUE( whk.initialize().isSuccess() );
    auto wh = ViewHelper::makeHandle( parentView, whk, ctx );
    auto status = wh.record( std::move( t1 ) );
    EXPECT_TRUE( status.isSuccess() );
  }

  // Make child view
  auto childView = new View( "ChildView", -1 );
  auto t2 = std::make_unique<TestClass>();
  t2->value = 2;
  {
    SG::WriteHandleKey<TestClass> whk( "test2" );
    EXPECT_TRUE( whk.initialize().isSuccess() );
    auto wh = ViewHelper::makeHandle( childView, whk, ctx );
    auto status = wh.record( std::move( t2 ) );
    EXPECT_TRUE( status.isSuccess() );
  }

  // All prepared, will start testing if queries respond correctly
  {
    // Ask for an object that doesn't exist
    SG::ReadHandleKey<TestClass> rhk( "test" );
    EXPECT_TRUE( rhk.initialize().isSuccess() );
    auto rh = ViewHelper::makeHandle( childView, rhk, ctx );
    EXPECT_FALSE( rh.isValid() );
  }
  {
    // Ask for object in the child view
    SG::ReadHandleKey<TestClass> rhk( "test2" );
    EXPECT_TRUE( rhk.initialize().isSuccess() );
    auto rh = ViewHelper::makeHandle( childView, rhk, ctx );
    EXPECT_TRUE( rh.isValid() );
    EXPECT_EQ( rh->value, 2 );
  }
  {
    // Ask child view for object that only exists in the parent
    SG::ReadHandleKey<TestClass> rhk( "test1" );
    EXPECT_TRUE( rhk.initialize().isSuccess() );
    auto rh = ViewHelper::makeHandle( childView, rhk, ctx );
    EXPECT_FALSE( rh.isValid() );
  }
  log << MSG::INFO << "Views that are not linked behave correctly" << endmsg;

  // Link views and see if data object is accessible
  childView->linkParent( parentView );
  {
    // Is the original object still there?
    SG::ReadHandleKey<TestClass> rhk( "test2" );
    EXPECT_TRUE( rhk.initialize().isSuccess() );
    auto rh = ViewHelper::makeHandle( childView, rhk, ctx );
    EXPECT_TRUE( rh.isValid() );
    EXPECT_EQ( rh->value, 2 );
  }
  {
    // Is the object from the parent now also visible?
    SG::ReadHandleKey<TestClass> rhk( "test1" );
    EXPECT_TRUE( rhk.initialize().isSuccess() );
    auto rh = ViewHelper::makeHandle( childView, rhk, ctx );
    EXPECT_TRUE( rh.isValid() );
    EXPECT_EQ( rh->value, 1);
  }
  log << MSG::INFO << "Views that are linked behave correctly" << endmsg;

  // Hide object from parent by adding one with same name to the Child
  auto t3 = std::make_unique<TestClass>();
  t3->value = 3;
  {
    // Can it be recorded? (should be allowed)
    SG::WriteHandleKey<TestClass> whk( "test1" );
    EXPECT_TRUE( whk.initialize().isSuccess() );
    auto wh = ViewHelper::makeHandle( childView, whk, ctx );
    auto status = wh.record( std::move( t3 ) );
    EXPECT_TRUE( status.isSuccess() );
  }
  {
    // Do we now see the child object in preference to the parent?
    SG::ReadHandleKey<TestClass> rhk( "test1" );
    EXPECT_TRUE( rhk.initialize().isSuccess() );
    auto rh = ViewHelper::makeHandle( childView, rhk, ctx );
    EXPECT_TRUE( rh.isValid() );
    EXPECT_EQ( rh->value, 3);
  }
  log << MSG::INFO << "Hiding works as expected" << endmsg;
}

void testFallThrough( const EventContext& ctx, MsgStream& log) {
  auto t = std::make_unique<TestClass>();
  SG::WriteHandleKey<TestClass> whk( "inStore" );
  EXPECT_TRUE( whk.initialize().isSuccess() );
  auto wh = SG::makeHandle(whk, ctx);
  auto status = wh.record( std::move( t ) );
  EXPECT_TRUE( status.isSuccess() );

  // the whole trick is that the read handle is pointed to the view,
  // but should read from the main store if the fall though
  // is enabled
  {
    auto opaqueView = new View( "OpaqueView", -1, false );
    SG::ReadHandleKey<TestClass> rhk( "inStore" );
    EXPECT_TRUE( rhk.initialize().isSuccess() );
    auto rh = ViewHelper::makeHandle( opaqueView, rhk, ctx );
    EXPECT_FALSE( rh.isValid() );
  }
  {
    auto transparentView = new View( "TransparentView", -1 );
    SG::ReadHandleKey<TestClass> rhk( "inStore" );
    EXPECT_TRUE( rhk.initialize().isSuccess() );
    auto rh = ViewHelper::makeHandle( transparentView, rhk, ctx );
    EXPECT_TRUE( rh.isValid() );
  }
  log << MSG::INFO << "Fall through works as expected" << endmsg;
}

void testFallThroughLinks( const EventContext& ctx, MsgStream& log ) {

  // Have to make a container to test element links
  auto t = std::make_unique<TestContainer>();
  t->push_back( new TestClass() );
  t->back()->value = 5;
  t->push_back( new TestClass() );
  t->back()->value = 4;

  // Store the container in the event-level store
  SG::WriteHandleKey<TestContainer> whk( "inStore" );
  EXPECT_TRUE( whk.initialize().isSuccess() );
  auto wh = SG::makeHandle(whk, ctx);
  auto status = wh.record( std::move( t ) );
  EXPECT_TRUE( status.isSuccess() );

  // Make another container for testing parent/child links
  auto t2 = std::make_unique<TestContainer>();
  t2->push_back( new TestClass() );
  t2->back()->value = 3;
  t2->push_back( new TestClass() );
  t2->back()->value = 2;

  // Make a parent view and store the container
  auto parentView = new View( "parentView", -1 );
  SG::WriteHandleKey<TestContainer> whk2( "inParent" );
  EXPECT_TRUE( whk2.initialize().isSuccess() );
  auto wh2 = ViewHelper::makeHandle( parentView, whk2, ctx );
  status = wh2.record( std::move( t2 ) );
  EXPECT_TRUE( status.isSuccess() );

  // Just test a straightforward element link to the parent
  {
    SG::ReadHandle<TestContainer> rh( "inParent" );
    auto link = ViewHelper::makeLink( parentView, rh, 0 );
    EXPECT_TRUE( link.isValid() );
    EXPECT_EQ( ( *link )->value, 3 );
    EXPECT_EQ( link.proxy()->name(), "_parentView_inParent" );
  }

  // Element links need to point to the right object
  // even if it's not in the current view
  {
    // Child to parent
    auto childView = new View( "childView", -1 );
    childView->linkParent( parentView );
    SG::ReadHandle<TestContainer> rh( "inParent" );
    auto link = ViewHelper::makeLink( childView, rh, 1 );
    EXPECT_TRUE( link.isValid() );
    EXPECT_EQ( ( *link )->value, 2 );
    EXPECT_EQ( link.proxy()->name(), "_parentView_inParent" );
  }
  {
    // Parent to store
    SG::ReadHandle<TestContainer> rh( "inStore" );
    auto link = ViewHelper::makeLink( parentView, rh, 0 );
    EXPECT_TRUE( link.isValid() );
    EXPECT_EQ( ( *link )->value, 5 );
    EXPECT_EQ( link.proxy()->name(), "inStore" );
  }
  {
    // Child to store
    auto childView = new View( "childView", -1 );
    childView->linkParent( parentView );
    SG::ReadHandle<TestContainer> rh( "inStore" );
    auto link = ViewHelper::makeLink( childView, rh, 1 );
    EXPECT_TRUE( link.isValid() );
    EXPECT_EQ( ( *link )->value, 4 );
    EXPECT_EQ( link.proxy()->name(), "inStore" );
  }
  log << MSG::INFO << "Fall through works with links as expected" << endmsg;
}

int main() {
  using namespace std;

  MsgStream log(Athena::getMessageSvc(), "ViewLinking_test");

  ISvcLocator* pSvcLoc;
  if (!Athena_test::initGaudi("",  pSvcLoc)) {
    log << MSG::ERROR << "Can not intit Gaudi" << endmsg;
    return -1;
  }
  assert(pSvcLoc);

  SmartIF<StoreGateSvc> pStore{pSvcLoc->service("StoreGateSvc")};

  if( !pStore ) {
    log << MSG::ERROR << "SG not available" << endmsg;
    return -1;
  }

  EventContext ctx;
  Atlas::setExtendedEventContext (ctx, Atlas::ExtendedEventContext(pStore.get()) );

  testProxy();
  testDataInView( ctx, log );
  testFallThrough( ctx, log );
  testFallThroughLinks( ctx, log );

  return 0;
}
