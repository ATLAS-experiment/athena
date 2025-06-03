/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ISF_BASESIMULATORG4TOOL_H
#define ISF_BASESIMULATORG4TOOL_H

// STL includes
#include <memory>

// FrameWork includes
#include <GaudiKernel/IAlgTool.h>
#include <GaudiKernel/StatusCode.h>

// ISF
#include "ISF_Event/ISFParticle.h"
#include "ISF_Event/ISFParticleContainer.h"
#include "ISF_Interfaces/BaseSimulatorTool.h"

class HitCollectionMap;

namespace ISF {

/*
 * @class BaseSimulatorG4Tool
 * Base class for an ISimulatorTool which requires a Geant4 UserInfo object
 */
class BaseSimulatorG4Tool : public ISF::BaseSimulatorTool {
 public:
  /** Standard BaseSimulatorTool constructor */
  using BaseSimulatorTool::BaseSimulatorTool;

  /** Simulation call for individual particles*/
  virtual StatusCode simulate(const EventContext& ctx, ISFParticle& isp,
                              ISFParticleContainer& secondaries,
                              McEventCollection* mcEventCollection,
                              std::shared_ptr<HitCollectionMap>) = 0;

  /** Simulation call for vectors of particles*/
  virtual StatusCode simulateVector(
      const EventContext& ctx, const ISFParticleVector& particles,
      ISFParticleContainer& secondaries, McEventCollection* mcEventCollection,
      std::shared_ptr<HitCollectionMap> HitCollectionMap, McEventCollection* shadowTruth = nullptr) = 0;

  /** Create data containers for an event */
  virtual StatusCode setupEvent(const EventContext&, HitCollectionMap&) = 0;

  /** Create data containers for an event (called by ISimulationSvc) */
  virtual StatusCode setupEventST(HitCollectionMap& hitCollections) {
    return setupEvent(Gaudi::Hive::currentContext(), hitCollections);
  }

  /** Finalise data containers for an event */
  virtual StatusCode releaseEvent(const EventContext&,
                                  HitCollectionMap&) = 0;

  /** Finalise data containers for an event (called by ISimulationSvc) */
  virtual StatusCode releaseEventST(HitCollectionMap& hitCollections) {
    return releaseEvent(Gaudi::Hive::currentContext(), hitCollections);
  }

  /////////////// ISF::ISimulatorTool interface methods ///////////////

  /** Simulation call for individual particles*/
  virtual StatusCode simulate(const EventContext&, ISFParticle&,
                              ISFParticleContainer&, McEventCollection*) {
    return StatusCode::FAILURE;
  };

  /** Simulation call for vectors of particles*/
  virtual StatusCode simulateVector(const EventContext&,
                                    const ISFParticleVector&,
                                    ISFParticleContainer&, McEventCollection*,
                                    McEventCollection*) {
    return StatusCode::FAILURE;
  };

  /** Create data containers for an event */
  virtual StatusCode setupEvent(const EventContext&) {
    return StatusCode::FAILURE;
  };

  /** Create data containers for an event (called by ISimulationSvc) */
  virtual StatusCode setupEventST() { return StatusCode::FAILURE; };

  /** Finalise data containers for an event */
  virtual StatusCode releaseEvent(const EventContext&) {
    return StatusCode::FAILURE;
  };

  /** Finalise data containers for an event (called by ISimulationSvc) */
  virtual StatusCode releaseEventST() { return StatusCode::FAILURE; };
};

}  // namespace ISF

#endif
