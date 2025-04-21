/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// class header
#include "GauginosPhysicsTool.h"

#include <memory>
// package headers
#include "GMSBNeutralino.hh"
#include "GMSBGravitino.hh"
// Geant4 physics lists
#include "G4ProcessManager.hh"
#include "G4ParticleTable.hh"
#include "G4hIonisation.hh"
#include "G4hMultipleScattering.hh"
#include "G4ParticleDefinition.hh"
#include "G4Transportation.hh"
#include "G4MuIonisation.hh"
// CLHEP headers
#include "CLHEP/Units/SystemOfUnits.h"

//-----------------------------------------------------------------------------
// Implementation file for class : GauginosPhysicsTool
//
// 15-05-2015 : Edoardo Farina
//-----------------------------------------------------------------------------

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
GauginosPhysicsTool::GauginosPhysicsTool( const std::string& type,
                                          const std::string& nam,const IInterface* parent )
  : base_class ( type, nam , parent )
{
  m_physicsOptionType = G4AtlasPhysicsOption::Type::BSMPhysics;

  declareProperty("GravitinoMass",
                  m_GravitinoParams.mass = 0.108E-04 * CLHEP::GeV,
                  "Gravitino Mass");
  declareProperty("GravitinoWidth", m_GravitinoParams.width = 0. * CLHEP::GeV,
                  "Gravitino Width");
  declareProperty("GravitinoCharge", m_GravitinoParams.charge = 0,
                  "Gravitino charge");
  declareProperty("GravitinoPDGCode", m_GravitinoParams.pdgCode = 1000039,
                  "Gravitino PDG CODE");
  declareProperty("GravitinoLifetime", m_GravitinoParams.lifetime = -1,
                  "Gravitino Lifetime");
  declareProperty("GravitinoStable", m_GravitinoParams.stable = true,
                  "Gravitino Stable");
  declareProperty("GravitinoShortlived", m_GravitinoParams.shortlived = false,
                  "Gravitino Shortlived");

  declareProperty("NeutralinoMass",
                  m_NeutralinoParams.mass = 118.848 * CLHEP::GeV,
                  "Neutralino Mass");
  declareProperty("NeutralinoWidth", m_NeutralinoParams.width = 0. * CLHEP::GeV,
                  "Neutralino Width");
  declareProperty("NeutralinoCharge", m_NeutralinoParams.charge = 0,
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

GauginosPhysicsTool::~GauginosPhysicsTool()
{
}

//=============================================================================
// Initialize
//=============================================================================
StatusCode GauginosPhysicsTool::initialize( )
{
  ATH_MSG_DEBUG("GauginosPhysicsTool initialize(  )");

  return StatusCode::SUCCESS;
}

auto GauginosPhysicsTool::GetPhysicsOption() -> UPPhysicsConstructor {
  return std::make_unique<GauginosPhysicsTool::PhysicsConstructor>(
      name(), msgLevel(), *this);
}

void GauginosPhysicsTool::PhysicsConstructor::ConstructParticle() {
  ATH_MSG_DEBUG("Create particle of Gauginos" );
  GMSBNeutralino::Definition(
      m_NeutralinoParams.mass, m_NeutralinoParams.width,
      m_NeutralinoParams.charge, m_NeutralinoParams.pdgCode,
      m_NeutralinoParams.stable, m_NeutralinoParams.lifetime,
      m_NeutralinoParams.shortlived);
  GMSBGravitino::Definition(
      m_GravitinoParams.mass, m_GravitinoParams.width, m_GravitinoParams.charge,
      m_GravitinoParams.pdgCode, m_GravitinoParams.stable,
      m_GravitinoParams.lifetime, m_GravitinoParams.shortlived);
}

void GauginosPhysicsTool::PhysicsConstructor::ConstructProcess() {
  ATH_MSG_DEBUG(" Construct Process for the Gauginos being run");
}
