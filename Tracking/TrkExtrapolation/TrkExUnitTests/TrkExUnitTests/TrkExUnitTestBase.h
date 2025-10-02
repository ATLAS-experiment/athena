/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//////////////////////////////////////////////////////////////////
// TrkExUnitTestBase.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKDETDESCRUNITTESTS_TrkExUnitTestBase_H
#define TRKDETDESCRUNITTESTS_TrkExUnitTestBase_H

// Athena & Gaudi includes
#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/RndmGenerators.h"

namespace Trk {
  /** @class TrkExUnitTestBase

      Base class for all unit tests in the TrkEx package,
      gives access to gaussian and flat random numbers

      @author Andreas.Salzburger@cern.ch
   */

  class TrkExUnitTestBase: public AthAlgorithm  {
  public:
    /** Standard Athena-Algorithm Constructor */
    using AthAlgorithm::AthAlgorithm;

    /** standard Athena-Algorithm method */
    StatusCode initialize();

    /** standard Athena-Algorithm method */
    StatusCode execute();

    /* specify the test here */
    virtual StatusCode runTest() = 0;

    /* specify the scan here */
    virtual StatusCode runScan() = 0;

    /* book the TTree branches */
    virtual StatusCode bookTree();

    /* initalizeTest, this includes loading of tools */
    virtual StatusCode initializeTest();
  protected:
    /** Random Number setup */
    std::unique_ptr<Rndm::Numbers>            m_gaussDist;
    std::unique_ptr<Rndm::Numbers>            m_flatDist;
    std::unique_ptr<Rndm::Numbers>            m_landauDist;

    /** number of tests */
    UnsignedIntegerProperty m_numTests{this, "NumberOfTestsPerEvent", 100};

    /** enable scan mode */
    BooleanProperty m_scanMode{this, "ScanMode", false};
  };
}

#endif
