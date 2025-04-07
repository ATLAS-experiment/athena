/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EXTRA_PARTICLES__EXTRA_PARTICLES_PHYSICS_TOOL_H
#define EXTRA_PARTICLES__EXTRA_PARTICLES_PHYSICS_TOOL_H 1

// Include files
#include "AthenaBaseComps/AthAlgTool.h"
#include "G4AtlasInterfaces/IPhysicsOptionTool.h"
#include "G4AtlasInterfaces/IPhysicsConstructor.h"

/** @class ExtraParticlesPhysicsTool ExtraParticlesPhysicsTool.h
 * "ExtraParticles/ExtraParticlesPhysicsTool.h"
 *
 *  Tool for the concrete implementation of a Physics List selection class
 *
 *  @author Miha Muskinja
 *  @date   August-2019
 */
class ExtraParticlesPhysicsTool
    : public extends<AthAlgTool, IPhysicsOptionTool> {
 public:
  /// Standard constructor
  ExtraParticlesPhysicsTool(const std::string &type, const std::string &name,
                            const IInterface *parent);

  virtual ~ExtraParticlesPhysicsTool();  ///< Destructor

  /// Initialize method
  virtual StatusCode initialize() override final;

  /** Implements */
  virtual UPPhysicsConstructor GetPhysicsOption() override final;

  class PhysicsConstructor : public IPhysicsContructor {
   public:
    PhysicsConstructor(
        const std::string &name, MSG::Level level,
        const std::map<std::string, std::vector<double>> &extraParticlesConfig)
        : IPhysicsContructor(name, level),
          m_extraParticlesConfig(extraParticlesConfig) {}

    virtual void ConstructParticle() override;
    virtual void ConstructProcess() override;

   private:
    /// a set of parameters for extra particle building
    std::map<std::string, std::vector<double>> m_extraParticlesConfig;

    /// a set to hold the newly created extra particles
    std::set<G4ParticleDefinition *> m_extraParticles;
  };

 protected:
  /// a set of parameters for extra particle building
  std::map<std::string, std::vector<double>> m_extraParticlesConfig;
};

#endif // EXTRA_PARTICLES__EXTRA_PARTICLES_PHYSICS_TOOL_H
