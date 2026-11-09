// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file  DataModelTestDataWrite/DDer.h
 * @author snyder@bnl.gov
 * @date Nov 2005
 * @brief Class used for testing the new @c DataVector inheritance scheme.
 *
 *        This is a class deriving from a @c DataVector
 *        containing the derived class, @c D.
 */

#ifndef DATAMODELTESTDATAWRITE_DDER_H
#define DATAMODELTESTDATAWRITE_DDER_H


#include "DataModelTestDataWrite/DVec.h"
#include "AthenaKernel/BaseInfo.h"
#include "AthenaKernel/CLASS_DEF.h"


namespace DMTest {


/**
 * @brief A class deriving from @c DataVector
 *        containing the derived class, @c D.
 */
struct DDer
  : public DVec
{
  using DVec::DVec;
};


}


CLASS_DEF (DMTest::DDer, 9632, 1)

// Tell StoreGate that DDer derives from DVec.
SG_BASE(DMTest::DDer, DMTest::DVec);

#endif // not DATAMODELTESTDATAWRITE_DDER_H
