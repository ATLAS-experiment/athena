// Dear emacs, this is -*- C++ -*-

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHENASERVICES_TESTRANDOMSEQALG_H
#define ATHENASERVICES_TESTRANDOMSEQALG_H 1

#include "AthenaBaseComps/AthAlgorithm.h"
#include "AthenaKernel/IAtRndmGenSvc.h"
#include "Gaudi/Property.h"
#include "GaudiKernel/ServiceHandle.h"

namespace CLHEP { class HepRandomEngine; }

/** @class TestRandomSegAlg
   * @brief a trivial algorithm to test the sequence of random numbers
   * produced by an IAtRndmGenSvc
   * 
   * @author srinir@bnl.gov
   */
class TestRandomSeqAlg : public AthAlgorithm {

public:
  using AthAlgorithm::AthAlgorithm;

  virtual StatusCode initialize() override;
  virtual StatusCode execute() override;

  
private:
  /// handle to the @c IAtRndmGenSvc we want to test
  ServiceHandle<IAtRndmGenSvc> m_rndmSvc{this, "RndmSvc", "AtRanluxGenSvc", "the IAtRndmGenSvc we want to test"};
  Gaudi::Property<std::string> m_streamName{this, "StreamName", "TEST", "random number stream to use"};
  Gaudi::Property<int> m_noOfNo{this, "NoOfNo", 10, "the number of random numbers to shoot and print per event"};
  CLHEP::HepRandomEngine* m_pEng{nullptr};
};

#endif
