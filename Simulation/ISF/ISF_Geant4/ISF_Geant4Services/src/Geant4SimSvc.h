/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ISF_GEANT4SIMSVC_H
#define ISF_GEANT4SIMSVC_H 1

// STL includes
#include <string>

// Gaudi
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"

// ISF includes
#include "CxxUtils/checker_macros.h"
#include "ISF_Event/ISFParticleContainer.h"
#include "ISF_Interfaces/BaseSimulationG4Svc.h"
#include "ISF_Interfaces/BaseSimulatorG4Tool.h"
#include "ISF_Interfaces/ISimulatorTool.h"

namespace iGeant4 {

  /** @class Geant4SimSvc

  */
class ATLAS_NOT_THREAD_SAFE Geant4SimSvc : public ISF::BaseSimulationG4Svc {

 public:
  //** Constructor with parameters */
  Geant4SimSvc(const std::string& name, ISvcLocator* pSvcLocator);

  /** Destructor */
  virtual ~Geant4SimSvc();

  /** Athena algorithm's interface methods */
  virtual StatusCode initialize() override;
  virtual StatusCode finalize() override;

  using ISF::BaseSimulationG4Svc::releaseEvent;
  using ISF::BaseSimulationG4Svc::setupEvent;
  using ISF::BaseSimulationG4Svc::simulate;
  using ISF::BaseSimulationG4Svc::simulateVector;

  /** Simulation Call  */
  virtual StatusCode simulate(ISF::ISFParticle& isp,
                              McEventCollection* mcEventCollection,
                              std::shared_ptr<HitCollectionMap>) override;

  /** Simulation Call for vector of ISF particles */
  virtual StatusCode simulateVector(
      const ISF::ISFParticleVector& particles,
      McEventCollection* mcEventCollection, std::shared_ptr<HitCollectionMap> hitCollections,
      McEventCollection* shadowTruth = nullptr) override;

  /** Setup Event chain - in case of a begin-of event action is needed */
  virtual StatusCode setupEvent(HitCollectionMap&) override;

  /** Release Event chain - in case of an end-of event action is needed */
  virtual StatusCode releaseEvent(HitCollectionMap&) override;

 private:
  /** Default constructor */
  Geant4SimSvc();

  PublicToolHandle<ISF::ISimulatorTool> m_simulatorTool{this, "SimulatorTool",
                                                        "", ""};

  ISF::BaseSimulatorG4Tool*
      m_simulatorG4Tool{};  //!< pointer to the  downcasted G4 simulator tool
};
}


#endif //> !ISF_Geant4SimSvc_H
