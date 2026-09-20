/*
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file DataModelTestDataCommon/src/xAODTestWriteCVec.cxx
 * @author snyder@bnl.gov
 * @date Apr 2016
 * @brief Algorithm to test writing xAOD classes with auxiliary data (cvec).
 */


#include "xAODTestWriteCVec.h"
#include "DataModelTestDataCommon/CVec.h"
#include "DataModelTestDataCommon/C.h"
#include "DataModelTestDataCommon/CAuxContainer.h"
#include "AthContainersInterfaces/AuxDataOption.h"
#include "AthContainers/Accessor.h"
#include "AthContainers/Decorator.h"
#include "StoreGate/WriteDecorHandle.h"
#include "AthLinks/ElementLink.h"


namespace DMTest {


/**
 * @brief Constructor.
 * @param name The algorithm name.
 * @param svc The service locator.
 */
xAODTestWriteCVec::xAODTestWriteCVec (const std::string &name,
                                      ISvcLocator *pSvcLocator)
  : AthReentrantAlgorithm (name, pSvcLocator)
{
}
  

/**
 * @brief Algorithm initialization; called at the beginning of the job.
 */
StatusCode xAODTestWriteCVec::initialize()
{
  ATH_CHECK( m_cvecKey.initialize() );

  if (m_cvecDecorKey.key().empty())
    m_cvecDecorKey = m_cvecKey.key() + ".dtest";
  ATH_CHECK( m_cvecDecorKey.initialize() );
  
  return StatusCode::SUCCESS;
}


/**
 * @brief Algorithm event processing.
 */
StatusCode xAODTestWriteCVec::execute (const EventContext& ctx) const
{
  unsigned int count = ctx.eventID().event_number() + 1;

  auto coll = std::make_unique<DMTest::CVec>();
  auto store = std::make_unique<DMTest::CAuxContainer>();
  coll->setStore (store.get());

  const static SG::Accessor<int> anInt2 ("anInt2");
  const static SG::Decorator<int> dVar1 ("dVar1");
  const static SG::Decorator<int> dVar2 ("dVar2");
  const static SG::Accessor<ElementLink<DMTest::CVec> > cEL ("cEL");

  int nent = 10;
  if (count == 5) nent = 0;
  for (int i=0; i < nent; i++) {
    coll->push_back (new DMTest::C);
    C& c = *coll->back();
    c.setAnInt (count * 100 + i+1);
    c.setAFloat (count * 200 + (float)i*0.1);

    anInt2(c) = count*300 + i+1;
    dVar1(c) = count*450 + i+1;
    dVar2(c) = count*750 + i+1;
    cEL(c).toIndexedElement (*coll, 9-i, ctx);
  }

  SG::WriteHandle<DMTest::CVec> cvec (m_cvecKey, ctx);
  CHECK( cvec.record (std::move(coll), std::move(store)) );

  SG::WriteDecorHandle<DMTest::CVec, int> dtest (m_cvecDecorKey, ctx);
  for (int i=0; i < nent; i++) {
    dtest (*(*dtest)[i]) = i+123;
  }

  return StatusCode::SUCCESS;
}


} // namespace DMTest

