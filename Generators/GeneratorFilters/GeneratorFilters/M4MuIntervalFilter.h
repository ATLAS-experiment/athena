/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#ifndef GENERATORFILTERSM4MUINTERVALFILTER_H
#define GENERATORFILTERSM4MUINTERVALFILTER_H

#include "GeneratorModules/GenFilter.h"
#include "GaudiKernel/ServiceHandle.h"
#include "AthenaKernel/IAthRNGSvc.h"

namespace CLHEP {
  class HepRandomEngine;
}

class M4MuIntervalFilter : public GenFilter {
public:

  M4MuIntervalFilter(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~M4MuIntervalFilter();
  virtual StatusCode filterInitialize();
  virtual StatusCode filterFinalize();
  virtual StatusCode filterEvent(const EventContext& ctx);

private:

  CLHEP::HepRandomEngine* getRandomEngine(const std::string& streamName,
                                          const EventContext& ctx) const;

  double m_maxEta;
  double m_minPt;                           // Rapidity acceptance

  ServiceHandle<IAthRNGSvc> m_rndmSvc{this, "RndmSvc", "AthRNGSvc"};// Random number generator

  double m_prob2medium;
  double m_prob2low;
  double m_prob2high;
  double m_m4mulow;
  double m_m4muhigh;
  bool m_ApplyReWeighting; 

public:
  double getEventWeight(double m) const;
};

#endif
