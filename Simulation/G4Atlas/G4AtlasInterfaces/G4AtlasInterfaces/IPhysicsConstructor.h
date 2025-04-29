/*
  Copyright (C) 2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4ATLASINTERFACES_IPHYSICSCONSTRUCTOR_H
#define G4ATLASINTERFACES_IPHYSICSCONSTRUCTOR_H

#include "AthenaBaseComps/AthMessaging.h"
#include "G4VPhysicsConstructor.hh"

/**
 * Struct to hold the parameters of a particle definition.
 *
 * When the physics constructor needs to create a new particle definition,
 * this struct can be used by derived classes to pass the parameters to the
 * constructor.
 *
 *  @author Julien Esseiva
 *  @date   2025-04-04
 */
struct ParticleDefinitionParams {
  double mass{0};
  double width{0};
  double charge{0};
  double pdgCode{0};
  bool stable{false};
  double lifetime{0};
  bool shortlived{false};
};

/** @class IPhysicsContructor IPhysicsContructor.h
 * "G4AtlasInterfaces/IPhysicsContructor.h"
 *
 * Abstract interface to Geant4 Physics constructor classes with logging
 * capabilities. Implemantationsa are meant to be purely Geant4 classes,
 * independant of Gaudi. The log level is passed to the constructor
 *
 *  @author Julien Esseiva
 *  @date   2025-04-04
 */
class IPhysicsContructor : public G4VPhysicsConstructor, public AthMessaging {
 public:
  /// Standard constructor
  IPhysicsContructor(const std::string& name, MSG::Level level)
      : G4VPhysicsConstructor(name), AthMessaging(name) {
    this->setLevel(level);
  }
};
#endif