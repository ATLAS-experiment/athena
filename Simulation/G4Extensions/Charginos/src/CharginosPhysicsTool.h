/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CHARGINOS_CHARGINOSPHYSICSTOOL_H
#define CHARGINOS_CHARGINOSPHYSICSTOOL_H

// Include files

#include "AthenaBaseComps/AthAlgTool.h"
#include "G4AtlasInterfaces/IPhysicsOptionTool.h"
#include "G4AtlasInterfaces/IPhysicsConstructor.h"

/** @class CharginosPhysicsTool CharginosPhysicsTool.h "G4AtlasInfrstructure/CharginosPhysicsTool.h"
 *
 *  Tool for the concrete implementation of a Physics List selection class
 *
 *  @author Edoardo Farina
 *  @date   15-05-2015
 */
class CharginosPhysicsTool : public extends<AthAlgTool, IPhysicsOptionTool> {
 public:
  /// Standard constructor
  CharginosPhysicsTool( const std::string& type , const std::string& name,
                        const IInterface* parent ) ;

  virtual ~CharginosPhysicsTool( ); ///< Destructor

  /// Initialize method
  virtual StatusCode initialize() override final;

  /** Implements
   */

  virtual UPPhysicsConstructor GetPhysicsOption() override final;

  class PhysicsConstructor : public IPhysicsContructor {
   public:
    PhysicsConstructor(const std::string &name, MSG::Level level,
                       CharginosPhysicsTool const &charginosPhysicsTool)
        : IPhysicsContructor(name, level),
          m_CharginoMinusParams(charginosPhysicsTool.m_CharginoMinusParams),
          m_CharginoPlusParams(charginosPhysicsTool.m_CharginoPlusParams),
          m_NeutralinoParams(charginosPhysicsTool.m_NeutralinoParams) {}

    virtual void ConstructParticle() override;
    virtual void ConstructProcess() override;

   private:
    ParticleDefinitionParams const &m_CharginoMinusParams;
    ParticleDefinitionParams const &m_CharginoPlusParams;
    ParticleDefinitionParams const &m_NeutralinoParams;
    G4ParticleDefinition *m_theCharginoMinus{nullptr};
    G4ParticleDefinition *m_theCharginoPlus{nullptr};
    G4ParticleDefinition *m_theNeutralino{nullptr};
  };

protected:
 ParticleDefinitionParams m_CharginoMinusParams;
 ParticleDefinitionParams m_CharginoPlusParams;
 ParticleDefinitionParams m_NeutralinoParams;
};

#endif // CHARGINOS_CHARGINOSPHYSICSTOOL_H
