// -*- c++ -*-

/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ISF_ISIMULATORTOOL_H
#define ISF_ISIMULATORTOOL_H

// Gaudi
#include "GaudiKernel/IAlgTool.h"

// ISF
#include "ISF_Interfaces/SimulationFlavor.h"

#include "ISF_Event/ISFParticle.h"
#include "ISF_Event/ISFParticleContainer.h"
#include "ISF_Event/ISFParticleContainer.h"

class McEventCollection;

namespace ISF {

class ISimulatorTool : virtual public IAlgTool {
public:
  DeclareInterfaceID(ISimulatorTool, 1, 0);

  /** Simulation call for individual particles*/
  virtual StatusCode simulate(const EventContext& ctx, ISFParticle& isp, ISFParticleContainer& secondaries, McEventCollection* mcEventCollection) = 0;

  /** Simulation call for vectors of particles*/
  virtual StatusCode simulateVector(const EventContext& ctx, const ISFParticleVector &particles, ISFParticleContainer& secondaries, McEventCollection *mcEventCollection, McEventCollection *shadowTruth=nullptr) = 0;

  /** Create data containers for an event */
  virtual StatusCode setupEvent(const EventContext&) = 0;

  /** Create data containers for an event (called by ISimulationSvc) */
  virtual StatusCode setupEventST() = 0;

  /** Finalise data containers for an event */
  virtual StatusCode releaseEvent(const EventContext&) = 0;

  /** Finalise data containers for an event (called by ISimulationSvc) */
  virtual StatusCode releaseEventST() = 0;

  /** return the simulation flavor */
  virtual ISF::SimulationFlavor simFlavor() const = 0;

};

}

#endif
