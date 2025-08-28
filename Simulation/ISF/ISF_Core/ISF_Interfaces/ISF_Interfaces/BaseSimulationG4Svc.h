/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ISF_BASESIMULATIONG4SVC_H
#define ISF_BASESIMULATIONG4SVC_H 1

// STL includes
#include <memory.h>
#include <string>

// FrameWork includes
#include "AthenaBaseComps/AthService.h"
#include <GaudiKernel/StatusCode.h>
#include "GeneratorObjects/McEventCollection.h"

// ISF includes
#include "ISF_Event/ISFParticle.h"
#include "ISF_Interfaces/BaseSimulationSvc.h"
#include "ISF_Interfaces/ISimulationSvc.h"

class HitCollectionMap;

namespace ISF {

class IParticleBroker;
class ITruthSvc;

/** @class BaseSimulationG4Svc

  Base class for an ISimulatorSvc which requires a Geant4 UserInfo object
*/
class BaseSimulationG4Svc : public ISF::BaseSimulationSvc {
 public:
  /** Standard BaseSimulationSvc constructor */
  using BaseSimulationSvc::BaseSimulationSvc;

  /** Destructor */
  virtual ~BaseSimulationG4Svc() = default;

  /** Simulation call for individual particles */
  virtual StatusCode simulate(ISFParticle&, McEventCollection*,
                              std::shared_ptr<HitCollectionMap>) = 0;

  /** Setup Event chain - in case of a begin-of event action is needed */
  virtual StatusCode setupEvent(HitCollectionMap&) = 0;

  /** Release Event chain - in case of an end-of event action is needed */
  virtual StatusCode releaseEvent(HitCollectionMap&) = 0;

  /** Simulation call for vectors of particles */
  virtual StatusCode simulateVector(const ISFParticleVector& particles,
                                    McEventCollection* mcEventCollection,
                                    std::shared_ptr<HitCollectionMap> hitCollections,
                                    McEventCollection*) {
    // this implementation is a wrapper in case the simulator does
    // implement particle-vector input
    StatusCode sc = StatusCode::SUCCESS;
    // simulate each particle individually
    for (ISF::ISFParticle* part : particles) {
      ATH_MSG_VERBOSE(m_screenOutputPrefix
                      << "Starting simulation of particle: " << part);
      if (this->simulate(*part, mcEventCollection, hitCollections).isFailure()) {
        ATH_MSG_WARNING("Simulation of particle failed!"
                        << endmsg
                        << "   -> simulator: " << this->simSvcDescriptor()
                        << "   -> particle : " << *part);
        sc = StatusCode::FAILURE;
      }
    }
    return sc;
  }

  /////////////// ISimulationSvc interface methods ///////////////

  /** Simulation call for individual particles */
  virtual StatusCode simulate(ISFParticle&, McEventCollection*) override {
    return StatusCode::FAILURE;
  }

  /** Setup Event chain - in case of a begin-of event action is needed */
  virtual StatusCode setupEvent() override { return StatusCode::FAILURE; }

  /** Release Event chain - in case of an end-of event action is needed */
  virtual StatusCode releaseEvent() override { return StatusCode::FAILURE; }

  /** Simulation call for vectors of particles */
  virtual StatusCode simulateVector(const ISFParticleVector&,
                                    McEventCollection*,
                                    McEventCollection*) override {
    return StatusCode::FAILURE;
  }
};
}  // namespace ISF

#endif  //> !ISF_BASESIMULATIONSVC_H
