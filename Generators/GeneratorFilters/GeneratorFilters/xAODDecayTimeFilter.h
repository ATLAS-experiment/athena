/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#ifndef GENERATORFILTERS_XAODDECAYTIMEFILTER_H
#define GENERATORFILTERS_XAODDECAYTIMEFILTER_H

#include "GeneratorModules/GenFilter.h"
#include "GaudiKernel/ServiceHandle.h"
#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTruth/TruthParticleAuxContainer.h"
#include "AthenaKernel/IAthRNGSvc.h"

#include "xAODTruth/TruthEvent.h"
#include "xAODTruth/TruthEventContainer.h"
#include <limits>       // std::numeric_limits

namespace CLHEP {
  class HepRandomEngine;
}

class xAODDecayTimeFilter : public GenFilter {
public:
  using GenFilter::GenFilter;

  virtual StatusCode filterInitialize() override final;
  virtual StatusCode filterEvent() override final;

private:
   CLHEP::HepRandomEngine* getRandomEngine(const std::string& streamName,
                                          const EventContext& ctx) const;

  double tau(const xAOD::TruthParticle* ptr) const;
  Gaudi::Property<float> m_lifetimeLow{this, "LifetimeLow", std::numeric_limits<float>::lowest(), "proper decay time value in ps"};
  Gaudi::Property<float> m_lifetimeHigh{this, "LifetimeHigh", std::numeric_limits<float>::max(), "proper decay time value in ps"};
  Gaudi::Property<float> m_seedlifetime{this, "Seedlifetime", std::numeric_limits<float>::lowest(), "proper decay time value in ps"};
  ServiceHandle<IAthRNGSvc> m_rndmSvc{this, "RndmSvc", "AthRNGSvc"};
  Gaudi::Property<bool> m_flatlifetime{this, "Flatlifetime", false, "proper decay time value in ps"};
  Gaudi::Property<std::vector<int>> m_particleID{this, "PDGs", {}};
  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthPartContKey{this, "TruthParticleContainerKey", "TruthGen"};
};



#endif
