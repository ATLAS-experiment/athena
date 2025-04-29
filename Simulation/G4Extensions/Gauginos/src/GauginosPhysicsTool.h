/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GAUGINOS_GauginosPhysicsTool_H
#define GAUGINOS_GauginosPhysicsTool_H

// Include files
#include <G4ParticleDefinition.hh>

#include "AthenaBaseComps/AthAlgTool.h"
#include "G4AtlasInterfaces/IPhysicsOptionTool.h"
#include "G4AtlasInterfaces/IPhysicsConstructor.h"

/** @class GauginosPhysicsTool GauginosPhysicsTool.h "G4AtlasInfrstructure/GauginosPhysicsTool.h"
 *
 *  Tool for the concrete implementation of a Physics List selection class
 *
 *  @author Edoardo Farina
 *  @date   15-05-2015
 */
class GauginosPhysicsTool : public extends<AthAlgTool, IPhysicsOptionTool> {
 public:
  /// Standard constructor
  GauginosPhysicsTool( const std::string& type , const std::string& name,
                       const IInterface* parent ) ;

  virtual ~GauginosPhysicsTool( ); ///< Destructor

  /// Initialize method
  virtual StatusCode initialize() override final;

  /** Implements
   */

  virtual UPPhysicsConstructor GetPhysicsOption() override final;

  class PhysicsConstructor : public IPhysicsContructor {
   public:
    PhysicsConstructor(const std::string& name, MSG::Level level,
                       GauginosPhysicsTool const& gauginosPhysicsTool)
        : IPhysicsContructor(name, level),
          m_GravitinoParams(gauginosPhysicsTool.m_GravitinoParams),
          m_NeutralinoParams(gauginosPhysicsTool.m_NeutralinoParams) {}

    virtual void ConstructParticle() override;
    virtual void ConstructProcess() override;

   private:
    ParticleDefinitionParams const& m_GravitinoParams;
    ParticleDefinitionParams const& m_NeutralinoParams;
  };

protected:
 ParticleDefinitionParams m_GravitinoParams;
 ParticleDefinitionParams m_NeutralinoParams;
};

#endif //GAUGINOS_GauginosPhysicsTool_H
