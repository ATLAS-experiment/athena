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

using namespace SG;
void testDataInView( StoreGateSvc* /*sg*/ , MsgStream& log ) {

  // Make parent view
  auto parentView = new View( "ParentView", -1 );
  auto t1 = std::make_unique<TestClass>();
  t1->value = 1;
  {
    SG::WriteHandle<TestClass> wh( "test1" );
    wh.setProxyDict( parentView ).ignore();
    auto status = wh.record( std::move( t1 ) );
    EXPECT_TRUE( status.isSuccess() );
  }

  // Make child view
  auto childView = new View( "ChildView", -1 );
  auto t2 = std::make_unique<TestClass>();
  t2->value = 2;
  {
    SG::WriteHandle<TestClass> wh( "test2" );
    wh.setProxyDict( childView ).ignore();
    auto status = wh.record( std::move( t2 ) );
    EXPECT_TRUE( status.isSuccess() );
  }

  // All prepared, will start testing if queries respond correctly
  {
    // Ask for an object that doesn't exist
    SG::ReadHandle<TestClass> rh( "test" );
    rh.setProxyDict( childView ).ignore();
    EXPECT_FALSE( rh.isValid() );
  }
  {
    // Ask for object in the child view
    SG::ReadHandle<TestClass> rh( "test2" );
    rh.setProxyDict( childView ).ignore();
    EXPECT_TRUE( rh.isValid() );
    EXPECT_EQ( rh->value, 2 );
  }
  {
    // Ask child view for object that only exists in the parent
    SG::ReadHandle<TestClass> rh( "test1" );
    rh.setProxyDict( childView ).ignore();
    EXPECT_FALSE( rh.isValid() );
  }
  log << MSG::INFO << "Views that are not linked behave correctly" << endmsg;

  // Link views and see if data object is accessible
  childView->linkParent( parentView );
  {
    // Is the original object still there?
    SG::ReadHandle<TestClass> rh( "test2" );
    rh.setProxyDict( childView ).ignore();
    EXPECT_TRUE( rh.isValid() );
    EXPECT_EQ( rh->value, 2 );
  }
  {
    // Is the object from the parent now also visible?
    SG::ReadHandle<TestClass> rh( "test1" );
    rh.setProxyDict( childView ).ignore();
    EXPECT_TRUE( rh.isValid() );
    EXPECT_EQ( rh->value, 1);
  }
  log << MSG::INFO << "Views that are linked behave correctly" << endmsg;

  // Hide object from parent by adding one with same name to the Child
  auto t3 = std::make_unique<TestClass>();
  t3->value = 3;
  {
    // Can it be recorded? (should be allowed)
    SG::WriteHandle<TestClass> wh( "test1" );
    wh.setProxyDict( childView ).ignore();
    auto status = wh.record( std::move( t3 ) );
    EXPECT_TRUE( status.isSuccess() );
  }
  {
    // Do we now see the child object in preference to the parent?
    SG::ReadHandle<TestClass> rh( "test1" );
    rh.setProxyDict( childView ).ignore();
    EXPECT_TRUE( rh.isValid() );
    EXPECT_EQ( rh->value, 3);
  }
  log << MSG::INFO << "Hiding works as expected" << endmsg;
}

void testFallThrough( StoreGateSvc* sg , MsgStream& log) {
  auto t = std::make_unique<TestClass>();
  SG::WriteHandle<TestClass> wh( "inStore" );
  wh.setProxyDict( sg ).ignore();
  auto status = wh.record( std::move( t ) );
  EXPECT_TRUE( status.isSuccess() );

  // the whole trick is that the read handle is pointed to the view,
  // but should read from the main store if the fall though
  // is enabled
  {
    auto opaqueView = new View( "OpaqueView", -1, false );
    SG::ReadHandle<TestClass> rh( "inStore" );
    rh.setProxyDict( opaqueView ).ignore();
    EXPECT_FALSE( rh.isValid() );
  }
  {
    auto transparentView = new View( "TransparentView", -1 );
    SG::ReadHandle<TestClass> rh( "inStore" );
    rh.setProxyDict( transparentView ).ignore();
    EXPECT_TRUE( rh.isValid() );
  }
  log << MSG::INFO << "Fall through works as expected" << endmsg;
}

void testFallThroughLinks( StoreGateSvc* sg , MsgStream& log ) {

  // Have to make a container to test element links
  auto t = std::make_unique<TestContainer>();
  t->push_back( new TestClass() );
  t->back()->value = 5;
  t->push_back( new TestClass() );
  t->back()->value = 4;

  // Store the container in the event-level store
  SG::WriteHandle<TestContainer> wh( "inStore" );
  wh.setProxyDict( sg ).ignore();
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
  SG::WriteHandle<TestContainer> wh2( "inParent" );
  wh2.setProxyDict( parentView ).ignore();
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

  testDataInView( pStore, log );
  testFallThrough( pStore, log );
  testFallThroughLinks( pStore, log );

  return 0;
}
