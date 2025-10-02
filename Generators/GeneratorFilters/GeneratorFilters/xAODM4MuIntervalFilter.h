/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef GENERATORFILTERS_XAODM4MUINTERVALFILTER_H
#define GENERATORFILTERS_XAODM4MUINTERVALFILTER_H


#include "GeneratorModules/GenFilter.h"
#include "GaudiKernel/ServiceHandle.h"
#include "AthenaKernel/IAthRNGSvc.h"

#include "xAODTruth/TruthParticleContainer.h"

namespace CLHEP {
  class HepRandomEngine;
}

// Pt  High --> Low
namespace {
  class High2LowByPt {
  public:
    bool operator () (const HepMC::FourVector &t1, const HepMC::FourVector &t2) {
      return (t1.perp() > t2.perp());
    }
  };
} // namespace

class xAODM4MuIntervalFilter : public GenFilter {
public:
  using GenFilter::GenFilter;

  virtual StatusCode filterInitialize() override final;
  virtual StatusCode filterEvent() override final;

private:

  CLHEP::HepRandomEngine* getRandomEngine(const std::string& streamName,
                                          const EventContext& ctx) const;

  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthPartContKey{this, "TruthParticleContainerKey", "TruthGen"};
  Gaudi::Property<double> m_maxEta{this,"MaxEta",5.0," "}; // Rapidity acceptance
  Gaudi::Property<double> m_minPt{this,"MinPt",1000," "};

  ServiceHandle<IAthRNGSvc> m_rndmSvc{this, "RndmSvc", "AthRNGSvc"};// Random number generator

  Gaudi::Property<double> m_prob2low{this,"LowM4muProbability",1.0," "};
  Gaudi::Property<double> m_prob2medium{this,"MediumMj4muProbability",0.5," "};
  Gaudi::Property<double> m_prob2high{this,"HighM4muProbability",0.1," "};
  Gaudi::Property<double> m_m4mulow{this,"LowM4mu",11000," "};
  Gaudi::Property<double> m_m4muhigh{this,"HighM4mu",25000," "};
  Gaudi::Property<bool> m_ApplyReWeighting{this,"ApplyReWeighting",true," "};
public:
  double getEventWeight(double m) const;

};

#endif
