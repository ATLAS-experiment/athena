/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file DataModelTestDataCommon/src/xAODTestWritePVec.cxx
 * @author snyder@bnl.gov
 * @date Sep 2024
 * @brief Algorithm to test writing xAOD data with packed containers.
 */


#include "xAODTestWritePVec.h"
#include "DataModelTestDataCommon/PVec.h"
#include "DataModelTestDataCommon/P.h"
#include "DataModelTestDataCommon/PAuxContainer.h"
#include "AthContainersInterfaces/AuxDataOption.h"
#include "AthContainers/Decorator.h"
#include "AthenaKernel/errorcheck.h"


#define CHECK_OPTION(ret)                       \
  do {                                          \
    if (!ret) {                                 \
      ATH_MSG_ERROR("setOption failed");        \
      return StatusCode::FAILURE;               \
    }                                           \
  } while(0)


namespace DMTest {


/**
 * @brief Algorithm initialization; called at the beginning of the job.
 */
StatusCode xAODTestWritePVec::initialize()
{
  ATH_CHECK( m_pvecKey.initialize() );
  return StatusCode::SUCCESS;
}


/**
 * @brief Algorithm event processing.
 */
StatusCode xAODTestWritePVec::execute (const EventContext& ctx) const
{
  unsigned int count = ctx.eventID().event_number() + 1;

  auto coll = std::make_unique<DMTest::PVec>();
  auto store = std::make_unique<DMTest::PAuxContainer>();
  coll->setStore (store.get());

  const static SG::Decorator<unsigned int> dpInt1 ("dpInt1");
  const static SG::Decorator<std::vector<float> > dpvFloat ("dpvFloat");

  int nent = 10;
  if (count == 5) nent = 0;
  for (int i=0; i < nent; i++) {
    coll->push_back (new DMTest::P);
    P& p = *coll->back();
    p.setPInt (count * 500 + i+1);
    p.setPFloat (i + (float)count * 0.01);

    std::vector<int> pvi;
    for (int j=0; j<i; j++)
      pvi.push_back (j + i*10 + count*100 - 500);
    p.setPVInt (pvi);

    std::vector<float> pvf;
    for (int j=0; j<i; j++)
      pvf.push_back ((float)j*0.1 + (float)i*0.01 + (float)count*0.001 - 0.5);
    p.setPVFloat (std::move (pvf));

    dpInt1(p) = count*50 + i+1;
    //cEL(c).toIndexedElement (*coll, 9-i);

    pvf.clear();
    for (int j=0; j<i; j++)
      pvf.push_back ((float)i*0.1 + (float)count*0.01 + (float)j*0.001);
    dpvFloat(p) = std::move(pvf);
  }

  SG::WriteHandle<DMTest::PVec> pvec (m_pvecKey, ctx);
  CHECK( pvec.record (std::move(coll), std::move(store)) );

  CHECK_OPTION( pvec->setOption ("dpInt1", SG::AuxDataOption ("nbits", 13)) );
  CHECK_OPTION( pvec->setOption ("dpvFloat", SG::AuxDataOption ("nbits", 13)) );
  CHECK_OPTION( pvec->setOption ("dpvFloat", SG::AuxDataOption ("signed", 0)));
  CHECK_OPTION( pvec->setOption ("dpvFloat", SG::AuxDataOption ("nmantissa", 13)) );

  return StatusCode::SUCCESS;
}


} // namespace DMTest

