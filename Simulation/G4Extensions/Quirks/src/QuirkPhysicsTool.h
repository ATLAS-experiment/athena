/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef QUIRKS_QuirksPhysicsTool_H
#define QUIRKS_QuirksPhysicsTool_H

// Include files

#include "AthenaBaseComps/AthAlgTool.h"
#include "G4AtlasInterfaces/IPhysicsOptionTool.h"
#include "G4AtlasInterfaces/IPhysicsConstructor.h"

/** @class QuirksPhysicsTool QuirksPhysicsTool.h "G4AtlasInfrstructure/QuirksPhysicsTool.h"
 *
 *  Tool for the concrete implementation of a Physics List selection class
 *
 *  @author Edoardo Farina
 *  @date   15-05-2015
 */
class QuirksPhysicsTool : public extends<AthAlgTool, IPhysicsOptionTool> {
 public:
  /// Standard constructor
  QuirksPhysicsTool( const std::string& type , const std::string& name,
                       const IInterface* parent ) ;

  virtual ~QuirksPhysicsTool( ); ///< Destructor

  /// Initialize method
  virtual StatusCode initialize() override final;

  virtual UPPhysicsConstructor GetPhysicsOption() override final;

  class PhysicsConstructor : public IPhysicsContructor {
   public:
    PhysicsConstructor(const std::string& name, MSG::Level level, G4double mass,
                       G4double charge, G4int pdgid, G4double stringForce,
                       G4double firstStringLength, G4double maxBoost,
                       G4double maxMergeT, G4double maxMergeMag)
        : IPhysicsContructor(name, level),
          m_mass(mass),
          m_charge(charge),
          m_pdgid(pdgid),
          m_stringForce(stringForce),
          m_firstStringLength(firstStringLength),
          m_maxBoost(maxBoost),
          m_maxMergeT(maxMergeT),
          m_maxMergeMag(maxMergeMag) {}

    virtual void ConstructParticle() override;
    virtual void ConstructProcess() override;

   private:
    G4double m_mass{};
    G4double m_charge{};
    G4int m_pdgid{};
    G4double m_stringForce{};
    G4double m_firstStringLength{};
    G4double m_maxBoost{};
    G4double m_maxMergeT{};
    G4double m_maxMergeMag{};
  };

private:
    G4double m_mass{};
    G4double m_charge{};
    G4int m_pdgid{};
    G4double m_stringForce{};
    G4double m_firstStringLength{};
    G4double m_maxBoost{};
    G4double m_maxMergeT{};
    G4double m_maxMergeMag{};
    G4int m_enableDebug{};
    G4double m_debugStep{};
    G4int m_numDebugSteps{};
};

#endif // QUIRKS_QuirksPhysicsTool_H
