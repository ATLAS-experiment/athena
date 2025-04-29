/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// class header
#include "SleptonsPhysicsTool.h"

#include <G4ParticleDefinition.hh>
#include <functional>
// package headers
#include "G4AtlasInterfaces/IPhysicsConstructor.h"
#include "G4SElectronLMinus.hh"
#include "G4SElectronLPlus.hh"
#include "G4SElectronRMinus.hh"
#include "G4SElectronRPlus.hh"
#include "G4SMuonLMinus.hh"
#include "G4SMuonLPlus.hh"
#include "G4SMuonRMinus.hh"
#include "G4SMuonRPlus.hh"
#include "G4STau1Minus.hh"
#include "G4STau1Plus.hh"
#include "G4STau2Minus.hh"
#include "G4STau2Plus.hh"
// Geant4 headers
#include "G4hIonisation.hh"
#include "G4hMultipleScattering.hh"
#include "G4Transportation.hh"
#include "G4MuIonisation.hh"
#include "G4ProcessManager.hh"
#include "G4Version.hh"
//CLHEP headers
#include "CLHEP/Units/SystemOfUnits.h"

//-----------------------------------------------------------------------------
// Implementation file for class : SleptonsPhysicsTool
//
// 15-05-2015 : Edoardo Farina
//-----------------------------------------------------------------------------

#if G4VERSION_NUMBER > 1029
#define PARTICLEITERATOR (this->GetParticleIterator())
#elif G4VERSION_NUMBER > 1009
#define PARTICLEITERATOR aParticleIterator
#else
#define PARTICLEITERATOR theParticleIterator
#endif

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
SleptonsPhysicsTool::SleptonsPhysicsTool(const std::string &type,
                                         const std::string &name,
                                         const IInterface *parent)
    : base_class(type, name, parent) {
  m_physicsOptionType = G4AtlasPhysicsOption::Type::BSMPhysics;

  auto& SElectronLMinus = m_particlesConfig.SElectronLMinus;
  declareProperty("G4SElectronLMinusMass",
                  SElectronLMinus.mass = 100.00 * CLHEP::GeV,
                  "G4SElectronLMinus Mass");
  declareProperty("G4SElectronLMinusWidth",
                  SElectronLMinus.width = 0.0 * CLHEP::MeV,
                  "G4SElectronLMinus Width");
  declareProperty("G4SElectronLMinusCharge",
                  SElectronLMinus.charge = -1. * CLHEP::eplus,
                  "G4SElectronLMinus charge");
  declareProperty("G4SElectronLMinusPDGCode", SElectronLMinus.pdgCode = 2000011,
                  "G4SElectronLMinus PDG CODE");
  declareProperty("G4SElectronLMinusStable", SElectronLMinus.stable = true,
                  "G4SElectronLMinus Stable");
  declareProperty("G4SElectronLMinusLifetime", SElectronLMinus.lifetime = -1,
                  "G4SElectronLMinus Lifetime");
  declareProperty("G4SElectronLMinusShortlived",
                  SElectronLMinus.shortlived = false,
                  "G4SElectronLMinus Shortlived");

  auto& SElectronLPlus = m_particlesConfig.SElectronLPlus;
  declareProperty("G4SElectronLPlusMass",
                  SElectronLPlus.mass = 100.0 * CLHEP::GeV,
                  "G4SElectronLPlus Mass");
  declareProperty("G4SElectronLPlusWidth",
                  SElectronLPlus.width = 0.0 * CLHEP::MeV,
                  "G4SElectronLPlus Width");
  declareProperty("G4SElectronLPlusCharge",
                  SElectronLPlus.charge = +1. * CLHEP::eplus,
                  "G4SElectronLPlus charge");
  declareProperty("G4SElectronLPlusPDGCode", SElectronLPlus.pdgCode = -2000011,
                  "G4SElectronLPlus PDG CODE");
  declareProperty("G4SElectronLPlusStable", SElectronLPlus.stable = true,
                  "G4SElectronLPlus Stable");
  declareProperty("G4SElectronLPlusLifetime", SElectronLPlus.lifetime = -1,
                  "G4SElectronLPlus Lifetime");
  declareProperty("G4SElectronLPlusShortlived",
                  SElectronLPlus.shortlived = false,
                  "G4SElectronLPlus Shortlived");

  auto& SMuonLMinus = m_particlesConfig.SMuonLMinus;
  declareProperty("G4SMuonLMinusMass", SMuonLMinus.mass = 100.0 * CLHEP::GeV,
                  "G4SMuonLMinus Mass");
  declareProperty("G4SMuonLMinusWidth", SMuonLMinus.width = 0.0 * CLHEP::MeV,
                  "G4SMuonLMinus Width");
  declareProperty("G4SMuonLMinusCharge",
                  SMuonLMinus.charge = -1. * CLHEP::eplus,
                  "G4SMuonLMinus charge");
  declareProperty("G4SMuonLMinusPDGCode", SMuonLMinus.pdgCode = 2000013,
                  "G4SMuonLMinus PDG CODE");
  declareProperty("G4SMuonLMinusStable", SMuonLMinus.stable = true,
                  "G4SMuonLMinus Stable");
  declareProperty("G4SMuonLMinusLifetime", SMuonLMinus.lifetime = -1,
                  "G4SMuonLMinus Lifetime");
  declareProperty("G4SMuonLMinusShortlived", SMuonLMinus.shortlived = false,
                  "G4SMuonLMinus Shortlived");

  auto& SMuonLPlus = m_particlesConfig.SMuonLPlus;
  declareProperty("G4SMuonLPlusMass", SMuonLPlus.mass = 100.0 * CLHEP::GeV,
                  "G4SMuonLPlus Mass");
  declareProperty("G4SMuonLPlusWidth", SMuonLPlus.width = 0.0 * CLHEP::MeV,
                  "G4SMuonLPlus Width");
  declareProperty("G4SMuonLPlusCharge", SMuonLPlus.charge = 1. * CLHEP::eplus,
                  "G4SMuonLPlus charge");
  declareProperty("G4SMuonLPlusPDGCode", SMuonLPlus.pdgCode = -2000013,
                  "G4SMuonLPlus PDG CODE");
  declareProperty("G4SMuonLPlusStable", SMuonLPlus.stable = true,
                  "G4SMuonLPlus Stable");
  declareProperty("G4SMuonLPlusLifetime", SMuonLPlus.lifetime = -1,
                  "G4SMuonLPlus Lifetime");
  declareProperty("G4SMuonLPlusShortlived", SMuonLPlus.shortlived = false,
                  "G4SMuonLPlus Shortlived");

  auto& STau1Minus = m_particlesConfig.STau1Minus;
  declareProperty("G4STau1MinusMass", STau1Minus.mass = 100.0 * CLHEP::GeV,
                  "G4STau1Minus Mass");
  declareProperty("G4STau1MinusWidth", STau1Minus.width = 0.0 * CLHEP::MeV,
                  "G4STau1Minus Width");
  declareProperty("G4STau1MinusCharge", STau1Minus.charge = -1. * CLHEP::eplus,
                  "G4STau1Minus charge");
  declareProperty("G4STau1MinusPDGCode", STau1Minus.pdgCode = 1000015,
                  "G4STau1Minus PDG CODE");
  declareProperty("G4STau1MinusStable", STau1Minus.stable = true,
                  "G4STau1Minus Stable");
  declareProperty("G4STau1MinusLifetime", STau1Minus.lifetime = -1,
                  "G4STau1Minus Lifetime");
  declareProperty("G4STau1MinusShortlived", STau1Minus.shortlived = false,
                  "G4STau1Minus Shortlived");

  auto& STau1Plus = m_particlesConfig.STau1Plus;
  declareProperty("G4STau1PlusMass", STau1Plus.mass = 100.0 * CLHEP::GeV,
                  "G4STau1Plus Mass");
  declareProperty("G4STau1PlusWidth", STau1Plus.width = 0.0 * CLHEP::MeV,
                  "G4STau1Plus Width");
  declareProperty("G4STau1PlusCharge", STau1Plus.charge = +1. * CLHEP::eplus,
                  "G4STau1Plus charge");
  declareProperty("G4STau1PlusPDGCode", STau1Plus.pdgCode = -1000015,
                  "G4STau1Plus PDG CODE");
  declareProperty("G4STau1PlusStable", STau1Plus.stable = true,
                  "G4STau1Plus Stable");
  declareProperty("G4STau1PlusLifetime", STau1Plus.lifetime = -1,
                  "G4STau1Plus Lifetime");
  declareProperty("G4STau1PlusShortlived", STau1Plus.shortlived = false,
                  "G4STau1Plus Shortlived");

  auto& SElectronRMinus = m_particlesConfig.SElectronRMinus;
  declareProperty("G4SElectronRMinusMass",
                  SElectronRMinus.mass = 100.00 * CLHEP::GeV,
                  "G4SElectronRMinus Mass");
  declareProperty("G4SElectronRMinusWidth",
                  SElectronRMinus.width = 0.0 * CLHEP::MeV,
                  "G4SElectronRMinus Width");
  declareProperty("G4SElectronRMinusCharge",
                  SElectronRMinus.charge = -1. * CLHEP::eplus,
                  "G4SElectronRMinus charge");
  declareProperty("G4SElectronRMinusPDGCode", SElectronRMinus.pdgCode = 2000011,
                  "G4SElectronRMinus PDG CODE");
  declareProperty("G4SElectronRMinusStable", SElectronRMinus.stable = true,
                  "G4SElectronRMinus Stable");
  declareProperty("G4SElectronRMinusLifetime", SElectronRMinus.lifetime = -1,
                  "G4SElectronRMinus Lifetime");
  declareProperty("G4SElectronRMinusShortlived",
                  SElectronRMinus.shortlived = false,
                  "G4SElectronRMinus Shortlived");

  auto& SElectronRPlus = m_particlesConfig.SElectronRPlus;
  declareProperty("G4SElectronRPlusMass",
                  SElectronRPlus.mass = 100.0 * CLHEP::GeV,
                  "G4SElectronRPlus Mass");
  declareProperty("G4SElectronRPlusWidth",
                  SElectronRPlus.width = 0.0 * CLHEP::MeV,
                  "G4SElectronRPlus Width");
  declareProperty("G4SElectronRPlusCharge",
                  SElectronRPlus.charge = +1. * CLHEP::eplus,
                  "G4SElectronRPlus charge");
  declareProperty("G4SElectronRPlusPDGCode", SElectronRPlus.pdgCode = -2000011,
                  "G4SElectronRPlus PDG CODE");
  declareProperty("G4SElectronRPlusStable", SElectronRPlus.stable = true,
                  "G4SElectronRPlus Stable");
  declareProperty("G4SElectronRPlusLifetime", SElectronRPlus.lifetime = -1,
                  "G4SElectronRPlus Lifetime");
  declareProperty("G4SElectronRPlusShortlived",
                  SElectronRPlus.shortlived = false,
                  "G4SElectronRPlus Shortlived");

  auto& SMuonRMinus = m_particlesConfig.SMuonRMinus;
  declareProperty("G4SMuonRMinusMass", SMuonRMinus.mass = 100.0 * CLHEP::GeV,
                  "G4SMuonRMinus Mass");
  declareProperty("G4SMuonRMinusWidth", SMuonRMinus.width = 0.0 * CLHEP::MeV,
                  "G4SMuonRMinus Width");
  declareProperty("G4SMuonRMinusCharge",
                  SMuonRMinus.charge = -1. * CLHEP::eplus,
                  "G4SMuonRMinus charge");
  declareProperty("G4SMuonRMinusPDGCode", SMuonRMinus.pdgCode = 2000013,
                  "G4SMuonRMinus PDG CODE");
  declareProperty("G4SMuonRMinusStable", SMuonRMinus.stable = true,
                  "G4SMuonRMinus Stable");
  declareProperty("G4SMuonRMinusLifetime", SMuonRMinus.lifetime = -1,
                  "G4SMuonRMinus Lifetime");
  declareProperty("G4SMuonRMinusShortlived", SMuonRMinus.shortlived = false,
                  "G4SMuonRMinus Shortlived");

  auto& SMuonRPlus = m_particlesConfig.SMuonRPlus;
  declareProperty("G4SMuonRPlusMass", SMuonRPlus.mass = 100.0 * CLHEP::GeV,
                  "G4SMuonRPlus Mass");
  declareProperty("G4SMuonRPlusWidth", SMuonRPlus.width = 0.0 * CLHEP::MeV,
                  "G4SMuonRPlus Width");
  declareProperty("G4SMuonRPlusCharge", SMuonRPlus.charge = 1. * CLHEP::eplus,
                  "G4SMuonRPlus charge");
  declareProperty("G4SMuonRPlusPDGCode", SMuonRPlus.pdgCode = -2000013,
                  "G4SMuonRPlus PDG CODE");
  declareProperty("G4SMuonRPlusStable", SMuonRPlus.stable = true,
                  "G4SMuonRPlus Stable");
  declareProperty("G4SMuonRPlusLifetime", SMuonRPlus.lifetime = -1,
                  "G4SMuonRPlus Lifetime");
  declareProperty("G4SMuonRPlusShortlived", SMuonRPlus.shortlived = false,
                  "G4SMuonRPlus Shortlived");

  auto& STau2Minus = m_particlesConfig.STau2Minus;
  declareProperty("G4STau2MinusMass", STau2Minus.mass = 100.0 * CLHEP::GeV,
                  "G4STau2Minus Mass");
  declareProperty("G4STau2MinusWidth", STau2Minus.width = 0.0 * CLHEP::MeV,
                  "G4STau2Minus Width");
  declareProperty("G4STau2MinusCharge", STau2Minus.charge = -1. * CLHEP::eplus,
                  "G4STau2Minus charge");
  declareProperty("G4STau2MinusPDGCode", STau2Minus.pdgCode = 1000015,
                  "G4STau2Minus PDG CODE");
  declareProperty("G4STau2MinusStable", STau2Minus.stable = true,
                  "G4STau2Minus Stable");
  declareProperty("G4STau2MinusLifetime", STau2Minus.lifetime = -1,
                  "G4STau2Minus Lifetime");
  declareProperty("G4STau2MinusShortlived", STau2Minus.shortlived = false,
                  "G4STau2Minus Shortlived");

  auto& STau2Plus = m_particlesConfig.STau2Plus;
  declareProperty("G4STau2PlusMass", STau2Plus.mass = 100.0 * CLHEP::GeV,
                  "G4STau2Plus Mass");
  declareProperty("G4STau2PlusWidth", STau2Plus.width = 0.0 * CLHEP::MeV,
                  "G4STau2Plus Width");
  declareProperty("G4STau2PlusCharge", STau2Plus.charge = +1. * CLHEP::eplus,
                  "G4STau2Plus charge");
  declareProperty("G4STau2PlusPDGCode", STau2Plus.pdgCode = -1000015,
                  "G4STau2Plus PDG CODE");
  declareProperty("G4STau2PlusStable", STau2Plus.stable = true,
                  "G4STau2Plus Stable");
  declareProperty("G4STau2PlusLifetime", STau2Plus.lifetime = -1,
                  "G4STau2Plus Lifetime");
  declareProperty("G4STau2PlusShortlived", STau2Plus.shortlived = false,
                  "G4STau2Plus Shortlived");
}

//=============================================================================
// Destructor
//=============================================================================

SleptonsPhysicsTool::~SleptonsPhysicsTool()
{

}

//=============================================================================
// Initialize
//=============================================================================
StatusCode SleptonsPhysicsTool::initialize( )
{
  ATH_MSG_DEBUG("SleptonsPhysicsTool initialize( )");
  return StatusCode::SUCCESS;
}

auto SleptonsPhysicsTool::GetPhysicsOption() -> UPPhysicsConstructor {
  return std::make_unique<SleptonsPhysicsTool::PhysicsConstructor>(
      name(), msgLevel(), *this);
}

void SleptonsPhysicsTool::PhysicsConstructor::ConstructParticle() {
  ATH_MSG_DEBUG("ConstructParticle for the Sleptons being run");
  // Lambda to handle particle definition with consistent logic and reduce
  // duplication
  using ParticleDefinitionFn = std::function<G4ParticleDefinition*(
      double, double, double, double, bool, double, bool)>;
  auto defineParticle = [](ParticleDefinitionParams const& particleDef,
                           G4ParticleDefinition** particle,
                           ParticleDefinitionFn definitionFn,
                           bool requireLargePDG) {
    if (particleDef.mass <= 0. ||
        !(requireLargePDG ? (std::abs(particleDef.pdgCode) >= 2000000)
                          : (std::abs(particleDef.pdgCode) < 2000000))) {
      return;
    }

    *particle =
        definitionFn(particleDef.mass, particleDef.width, particleDef.charge,
                     particleDef.pdgCode, particleDef.stable,
                     particleDef.lifetime, particleDef.shortlived);
  };
  // R-types (large PDG code)
  defineParticle(m_particlesConfig.SElectronRMinus, &m_theSElectronRMinus,
                 G4SElectronRMinus::Definition, true);
  defineParticle(m_particlesConfig.SElectronRPlus, &m_theSElectronRPlus,
                 G4SElectronRPlus::Definition, true);
  defineParticle(m_particlesConfig.SMuonRMinus, &m_theSMuonRMinus,
                 G4SMuonRMinus::Definition, true);
  defineParticle(m_particlesConfig.SMuonRPlus, &m_theSMuonRPlus,
                 G4SMuonRPlus::Definition, true);
  defineParticle(m_particlesConfig.STau2Minus, &m_theSTau2Minus,
                 G4STau2Minus::Definition, true);
  defineParticle(m_particlesConfig.STau2Plus, &m_theSTau2Plus,
                 G4STau2Plus::Definition, true);

  // L-types (small PDG code)
  defineParticle(m_particlesConfig.STau1Minus, &m_theSTau1Minus,
                 G4STau1Minus::Definition, false);
  defineParticle(m_particlesConfig.STau1Plus, &m_theSTau1Plus,
                 G4STau1Plus::Definition, false);
  defineParticle(m_particlesConfig.SElectronLMinus, &m_theSElectronLMinus,
                 G4SElectronLMinus::Definition, false);
  defineParticle(m_particlesConfig.SElectronLPlus, &m_theSElectronLPlus,
                 G4SElectronLPlus::Definition, false);
  defineParticle(m_particlesConfig.SMuonLMinus, &m_theSMuonLMinus,
                 G4SMuonLMinus::Definition, false);
  defineParticle(m_particlesConfig.SMuonLPlus, &m_theSMuonLPlus,
                 G4SMuonLPlus::Definition, false);
}

void SleptonsPhysicsTool::PhysicsConstructor::ConstructProcess() {
  ATH_MSG_DEBUG(" ConstructProcess for Sleptons being run");
  PARTICLEITERATOR->reset();
  while( (*PARTICLEITERATOR)() ){
    G4ParticleDefinition* particle = PARTICLEITERATOR->value();
    int pdgcode=std::abs(particle->GetPDGEncoding());
    if (pdgcode==2000011 || pdgcode==2000013 || pdgcode==1000015 || pdgcode==1000011 || pdgcode==1000013 || pdgcode==2000015)
      {
        G4ProcessManager* proc=particle->GetProcessManager();
        proc->AddProcess(new G4hMultipleScattering,-1,1,1);
        proc->AddProcess(new G4hIonisation,-1,2,2);
      }
  }
}
