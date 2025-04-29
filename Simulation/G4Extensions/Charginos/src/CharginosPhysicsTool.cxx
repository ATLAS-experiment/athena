/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// Include files

// local
#include "CharginosPhysicsTool.h"
#include "AMSBCharginoPlus.hh"
#include "AMSBCharginoMinus.hh"
#include "AMSBNeutralino.hh"

// Geant4 physics lists
#include "G4ProcessManager.hh"
#include "G4ParticleTable.hh"
#include "G4hIonisation.hh"
#include "G4hMultipleScattering.hh"
#include "G4Transportation.hh"
#include "G4MuIonisation.hh"
#include "G4DecayTable.hh"
#include "G4VDecayChannel.hh"
#include "G4PhaseSpaceDecayChannel.hh"

// CLHEP headers
#include "CLHEP/Units/SystemOfUnits.h"

//-----------------------------------------------------------------------------
// Implementation file for class : CharginosPhysicsTool
//
// 15-05-2015 : Edoardo Farina
//-----------------------------------------------------------------------------



//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
CharginosPhysicsTool::CharginosPhysicsTool(const std::string& type,
                                           const std::string& name,
                                           const IInterface* parent)
    : base_class(type, name, parent) {
  m_physicsOptionType = G4AtlasPhysicsOption::Type::BSMPhysics;
  declareProperty("CharginoPlusMass",
                  m_CharginoPlusParams.mass = 101.0 * CLHEP::GeV,
                  "CharginoPlus Mass");
  declareProperty("CharginoPlusWidth",
                  m_CharginoPlusParams.width = 0.0 * CLHEP::MeV,
                  "CharginoPlus Width");
  declareProperty("CharginoPlusCharge",
                  m_CharginoPlusParams.charge = +1. * CLHEP::eplus,
                  "CharginoPlus charge");
  declareProperty("CharginoPlusPDGCode", m_CharginoPlusParams.pdgCode = 1000024,
                  "CharginoPlus PDG CODE");
  declareProperty("CharginoPlusStable", m_CharginoPlusParams.stable = true,
                  "CharginoPlus Stable");
  declareProperty("CharginoPlusLifetime", m_CharginoPlusParams.lifetime = -1,
                  "CharginoPlus Lifetime");
  declareProperty("CharginoPlusShortlived",
                  m_CharginoPlusParams.shortlived = false,
                  "CharginoPlus Shortlived");

  declareProperty("CharginoMinusMass",
                  m_CharginoMinusParams.mass = 101.0 * CLHEP::GeV,
                  "CharginoMinus Mass");
  declareProperty("CharginoMinusWidth",
                  m_CharginoMinusParams.width = 0.0 * CLHEP::MeV,
                  "CharginoMinus Width");
  declareProperty("CharginoMinusCharge",
                  m_CharginoMinusParams.charge = -1. * CLHEP::eplus,
                  "CharginoMinus charge");
  declareProperty("CharginoMinusPDGCode",
                  m_CharginoMinusParams.pdgCode = -1000024,
                  "CharginoMinus PDG CODE");
  declareProperty("CharginoMinusStable", m_CharginoMinusParams.stable = true,
                  "CharginoMinus Stable");
  declareProperty("CharginoMinusLifetime", m_CharginoMinusParams.lifetime = -1,
                  "CharginoMinus Lifetime");
  declareProperty("CharginoMinusShortlived",
                  m_CharginoMinusParams.shortlived = false,
                  "CharginoMinus Shortlived");

  declareProperty("NeutralinoMass",
                  m_NeutralinoParams.mass = 100.0 * CLHEP::GeV,
                  "Neutralino Mass");
  declareProperty("NeutralinoWidth",
                  m_NeutralinoParams.width = 0.0 * CLHEP::MeV,
                  "Neutralino Width");
  declareProperty("NeutralinoCharge",
                  m_NeutralinoParams.charge = 0. * CLHEP::eplus,
                  "Neutralino charge");
  declareProperty("NeutralinoPDGCode", m_NeutralinoParams.pdgCode = 1000022,
                  "Neutralino PDG CODE");
  declareProperty("NeutralinoStable", m_NeutralinoParams.stable = true,
                  "Neutralino Stable");
  declareProperty("NeutralinoLifetime", m_NeutralinoParams.lifetime = -1,
                  "Neutralino Lifetime");
  declareProperty("NeutralinoShortlived", m_NeutralinoParams.shortlived = false,
                  "Neutralino Shortlived");
}

//=============================================================================
// Destructor
//=============================================================================

CharginosPhysicsTool::~CharginosPhysicsTool()
{

}

//=============================================================================
// Initialize
//=============================================================================
StatusCode CharginosPhysicsTool::initialize( )
{
  ATH_MSG_INFO("CharginosPhysicsTool::initialize( )");
  return StatusCode::SUCCESS;
}

auto CharginosPhysicsTool::GetPhysicsOption() -> UPPhysicsConstructor {

  ATH_MSG_INFO("CharginosPhysicsTool::GetPhysicsOption( )");
  return std::make_unique<CharginosPhysicsTool::PhysicsConstructor>(
      name(), this->msgLevel(), *this);
}

void CharginosPhysicsTool::PhysicsConstructor::ConstructParticle() {
  ATH_MSG_DEBUG("ConstructParticle for the Charginos being run");

  m_theCharginoMinus = AMSBCharginoMinus::Definition(
      m_CharginoMinusParams.mass, m_CharginoMinusParams.width,
      m_CharginoMinusParams.charge, m_CharginoMinusParams.pdgCode,
      m_CharginoMinusParams.stable, m_CharginoMinusParams.lifetime,
      m_CharginoMinusParams.shortlived);

  m_theCharginoPlus = AMSBCharginoPlus::Definition(
      m_CharginoPlusParams.mass, m_CharginoPlusParams.width,
      m_CharginoPlusParams.charge, m_CharginoPlusParams.pdgCode,
      m_CharginoPlusParams.stable, m_CharginoPlusParams.lifetime,
      m_CharginoPlusParams.shortlived);

  m_theNeutralino = AMSBNeutralino::Definition(
      m_NeutralinoParams.mass, m_NeutralinoParams.width,
      m_NeutralinoParams.charge, m_NeutralinoParams.pdgCode,
      m_NeutralinoParams.stable, m_NeutralinoParams.lifetime,
      m_NeutralinoParams.shortlived);
}

void CharginosPhysicsTool::PhysicsConstructor::ConstructProcess() {
  ATH_MSG_DEBUG("ConstructProcess for Charginos being run");

  G4ProcessManager *charginoPlus = m_theCharginoPlus->GetProcessManager();
  G4ProcessManager *charginoMinus = m_theCharginoMinus->GetProcessManager();

  charginoPlus->AddProcess(new G4hMultipleScattering,-1,1,1);
  charginoMinus->AddProcess(new G4hMultipleScattering,-1,1,1);
  charginoPlus->AddProcess(new G4hIonisation,-1,2,2);
  charginoMinus->AddProcess(new G4hIonisation,-1,2,2);
}
