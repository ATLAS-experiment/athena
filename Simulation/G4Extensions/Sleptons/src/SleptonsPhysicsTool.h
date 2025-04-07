/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef SLEPTONS_SleptonsPhysicsTool_H
#define SLEPTONS_SleptonsPhysicsTool_H

// Include files
#include <G4ParticleDefinition.hh>

#include "AthenaBaseComps/AthAlgTool.h"
#include "G4AtlasInterfaces/IPhysicsOptionTool.h"
#include "G4AtlasInterfaces/IPhysicsConstructor.h"

// Hold all particles parameters (to reduce verbosity)
struct SleptonsParticlesConfig {
  using ParticleParams = ParticleDefinitionParams;
  ParticleParams SElectronLMinus;
  ParticleParams SElectronLPlus;
  ParticleParams SMuonLMinus;
  ParticleParams SMuonLPlus;
  ParticleParams STau1Minus;
  ParticleParams STau1Plus;
  ParticleParams SElectronRMinus;
  ParticleParams SElectronRPlus;
  ParticleParams SMuonRMinus;
  ParticleParams SMuonRPlus;
  ParticleParams STau2Minus;
  ParticleParams STau2Plus;
};

/** @class SleptonsPhysicsTool SleptonsPhysicsTool.h "G4AtlasInfrstructure/SleptonsPhysicsTool.h"
 *
 *  Tool for the concrete implementation of a Physics List selection class
 *
 *  @author Edoardo Farina
 *  @date   15-05-2015
 */
class SleptonsPhysicsTool : public extends<AthAlgTool, IPhysicsOptionTool> {
 public:
  /// Standard constructor
  SleptonsPhysicsTool( const std::string& type , const std::string& name,
                       const IInterface* parent ) ;

  virtual ~SleptonsPhysicsTool( ); ///< Destructor

  /// Initialize method
  virtual StatusCode initialize() override final;

  /** Implements
   */

  virtual UPPhysicsConstructor GetPhysicsOption() override final;

  class PhysicsConstructor : public IPhysicsContructor {
   public:
    PhysicsConstructor(const std::string &name, MSG::Level level,
                       SleptonsPhysicsTool const &sleptonsPhysicsTool)
        : IPhysicsContructor(name, level),
          m_particlesConfig(sleptonsPhysicsTool.m_particlesConfig) {}

    virtual void ConstructParticle() override;
    virtual void ConstructProcess() override;

   private:
    SleptonsParticlesConfig const &m_particlesConfig;

    G4ParticleDefinition *m_theSElectronLMinus{nullptr};
    G4ParticleDefinition *m_theSElectronLPlus{nullptr};
    G4ParticleDefinition *m_theSMuonLMinus{nullptr};
    G4ParticleDefinition *m_theSMuonLPlus{nullptr};
    G4ParticleDefinition *m_theSTau1Minus{nullptr};
    G4ParticleDefinition *m_theSTau1Plus{nullptr};

    G4ParticleDefinition *m_theSElectronRMinus{nullptr};
    G4ParticleDefinition *m_theSElectronRPlus{nullptr};
    G4ParticleDefinition *m_theSMuonRMinus{nullptr};
    G4ParticleDefinition *m_theSMuonRPlus{nullptr};
    G4ParticleDefinition *m_theSTau2Minus{nullptr};
    G4ParticleDefinition *m_theSTau2Plus{nullptr};
  };

protected:
 SleptonsParticlesConfig m_particlesConfig;
};

#endif // SLEPTONS_SleptonsPhysicsTool_H
