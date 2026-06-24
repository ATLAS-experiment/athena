// This file's extension implies that it's C, but it's really -*- C++ -*-.

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file  DataModelTestDataRead/ELVec.h
 * @author snyder@bnl.gov
 * @date May 2007
 * @brief Class used for testing new @c ElementLink.
 */

#ifndef DATAMODELTESTDATAREAD_ELVEC_H
#define DATAMODELTESTDATAREAD_ELVEC_H


#include "DataModelTestDataRead/BVec.h"
#include "AthLinks/ElementLink.h"
#include "AthLinks/ElementLinkVector.h"
#include "AthLinks/DataLink.h"
#include "DataModelAthenaPool/ElementLinkVector_p1.h"
#include "DataModelAthenaPool/ElementLink_p3.h"
#include "DataModelAthenaPool/DataLink_p1.h"
#include "AthenaKernel/CLASS_DEF.h"
#include "GaudiKernel/ThreadLocalContext.h"


namespace DMTest {


/**
 * @brief For testing @c ElementLink.
 */
struct ELVec
{
  std::vector<ElementLink<BVec> > m_el;
  std::vector<DataLink<BVec> > m_dl;
  ElementLinkVector<BVec> m_elv;

  ElementLinkIntVector_p1 m_elv2_p;
  std::vector<ElementLinkInt_p3> m_el2_p;
  std::vector<DataLink_p1> m_dl2_p;
};


}


namespace SG {

// We need this because we test reading and writing this class
// without T/P separation.
template <>
class ToTransient<DMTest::ELVec>
{
public:
  static bool toTransient (DMTest::ELVec& elv, const EventContext& ctx)
  {
    SG::ToTransient<std::vector<ElementLink<DMTest::BVec> > >::toTransient (elv.m_el, ctx);
    SG::ToTransient<std::vector<DataLink<DMTest::BVec> > >::toTransient (elv.m_dl, ctx);
    return true;
  }
  static bool toTransient (DMTest::ELVec& elv)
  {
    return toTransient (elv, Gaudi::Hive::currentContext());
  }
};

}


CLASS_DEF (DMTest::ELVec, 9639, 1)


#endif // not DATAMODELTESTDATAREAD_ELVEC_H
