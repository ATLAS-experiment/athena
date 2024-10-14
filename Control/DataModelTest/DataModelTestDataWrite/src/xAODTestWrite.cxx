/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file DataModelTestDataWrite/src/xAODTestWrite.cxx
 * @author snyder@bnl.gov
 * @date May 2014
 * @brief Algorithm to test writing xAOD classes with auxiliary data.
 */


#include "xAODTestWrite.h"
#include "xAODTestWriteHelper.h"
#include "DataModelTestDataCommon/CVec.h"
#include "DataModelTestDataCommon/CVecWithData.h"
#include "DataModelTestDataCommon/C.h"
#include "DataModelTestDataCommon/CAuxContainer.h"
#include "DataModelTestDataCommon/CTrigAuxContainer.h"
#include "DataModelTestDataWrite/GVec.h"
#include "DataModelTestDataWrite/G.h"
#include "DataModelTestDataWrite/GAuxContainer.h"
#include "AthContainersInterfaces/AuxDataOption.h"
#include "AthLinks/ElementLink.h"
#include "AthenaKernel/errorcheck.h"


namespace DMTest {


/**
 * @brief Algorithm initialization; called at the beginning of the job.
 */
StatusCode xAODTestWrite::initialize()
{
  ATH_CHECK( m_cvecKey.initialize() );
  ATH_CHECK( m_ctrigKey.initialize() );
  ATH_CHECK( m_gvecKey.initialize (SG::AllowEmpty) );
  ATH_CHECK( m_cvecWDKey.initialize() );
  return StatusCode::SUCCESS;
}


/**
 * @brief Algorithm event processing.
 */
StatusCode xAODTestWrite::execute (const EventContext& ctx) const
{
  unsigned int count = ctx.eventID().event_number() + 1;

  SG::ReadHandle<DMTest::CVec> cvec (m_cvecKey, ctx);

  auto trig_coll = std::make_unique<DMTest::CVec>();
  auto trig_store = std::make_unique<DMTest::CTrigAuxContainer>();
  trig_coll->setStore (trig_store.get());

  static const C::Accessor<int> anInt2 ("anInt2");
  static const C::Accessor<ElementLink<DMTest::CVec> > cEL ("cEL");

  //static const C::Decorator<SG::PackedElement<unsigned int> > dpInt1 ("dpInt1");
  //static const C::Decorator<SG::PackedElement<std::vector<float> > > dpvFloat ("dpvFloat");
  static const C::Decorator<unsigned int> dpInt1 ("dpInt1");
  static const C::Decorator<std::vector<float> > dpvFloat ("dpvFloat");

  for (int i=0; i < 8; i++) {
    trig_coll->push_back (new DMTest::C);
    C& c = *trig_coll->back();
    c.setAnInt (count * 500 + i+1);
    c.setAFloat (count * 600 + (float)i*0.1);

    anInt2(c) = count*700 + i+1;
  }

  SG::WriteHandle<DMTest::CVec> ctrig (m_ctrigKey, ctx);
  CHECK( ctrig.record (std::move (trig_coll), std::move (trig_store)) );

  if (!m_gvecKey.empty()) {
    auto gcont = std::make_unique<DMTest::GVec>();
    auto gstore = std::make_unique<DMTest::GAuxContainer>();
    gcont->setStore (gstore.get());

    for (int i=0; i < 10; i++) {
      gcont->push_back (new DMTest::G);
      G& g = *gcont->back();
      g.setAnInt (count * 700 + i+1);
      g.setgFloat (count * 700 + 100 + i+0.5);
      std::vector<double> v;
      for (int j=0; j<i; j++) {
        v.push_back (count * 700 + 100*j + i+0.5);
      }
      g.setgvFloat (v);
    }

    SG::WriteHandle<DMTest::GVec> gvec (m_gvecKey, ctx);
    CHECK( gvec.record (std::move (gcont), std::move (gstore)) );
  }

  CHECK( write_cvec_with_data (count, ctx) );

  return StatusCode::SUCCESS;
}


/**
 * @brief Test writing container with additional data.
 */
StatusCode xAODTestWrite::write_cvec_with_data (unsigned int count,
                                                const EventContext& ctx) const
{
  auto coll = std::make_unique<DMTest::CVecWithData>();
  auto store = std::make_unique<DMTest::CAuxContainer>();
  coll->setStore (store.get());

  coll->meta1 = count + 1000;
  for (int i=0; i < 10; i++) {
    coll->push_back (new DMTest::C);
    C& c = *coll->back();
    c.setAnInt (count * 200 + i+1);
  }

  SG::WriteHandle<DMTest::CVecWithData> cvecWD (m_cvecWDKey, ctx);
  CHECK( cvecWD.record (std::move (coll), std::move (store)) );

  return StatusCode::SUCCESS;
}


} // namespace DMTest

