/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

//////////////////////////////////////////////////////////////////
// InDetReconstructableSelector.cxx
//   Source file for class InDetReconstructableSelector
///////////////////////////////////////////////////////////////////

#include "TrkValTools/InDetReconstructableSelector.h"
#include "AtlasHepMC/GenVertex.h"
#include "CLHEP/Units/SystemOfUnits.h"
#include "AtlasHepMC/GenParticle.h"
#include "TruthUtils/HepMCHelpers.h"
#include "GeneratorObjects/McEventCollection.h"

Trk::InDetReconstructableSelector::InDetReconstructableSelector(const std::string& type, const std::string& name,
                                              const IInterface* parent)
  : AthAlgTool (type,name,parent),
    m_minPt             (1000.),
    m_maxEta            (   3.0),
    m_selectPrimariesOnly(false),
    m_maxRStartPrimary  (  25.0*CLHEP::mm),
    m_maxZStartPrimary  ( 200.0*CLHEP::mm),
    m_maxRStartAll      ( 360.0*CLHEP::mm),
    m_maxZStartAll      (2000.0*CLHEP::mm)

{
  declareInterface<IGenParticleSelector>(this);

  declareProperty("MinPt",              m_minPt);
  declareProperty("MaxEta",             m_maxEta);
  declareProperty("SelectPrimariesOnly",m_selectPrimariesOnly, "flag to restrict to primaries or not");
  declareProperty("MaxRStartPrimary",   m_maxRStartPrimary,    "production vtx r for primaries");
  declareProperty("MaxZStartPrimary",   m_maxZStartPrimary,    "production vtx z for primaries");
  declareProperty("MaxRStartAll",       m_maxRStartAll,        "production vtx r for all");
  declareProperty("MaxZStartAll",       m_maxZStartAll,        "production vtx z for all");
}

///////////////////////////////
/// initialize
///////////////////////////////
StatusCode Trk::InDetReconstructableSelector::initialize() {
  ATH_MSG_INFO ("initialise in " << name());
  return StatusCode::SUCCESS;
}

StatusCode Trk::InDetReconstructableSelector::finalize() {
  ATH_MSG_INFO ("starting finalize() in " << name());
  return StatusCode::SUCCESS;
}

std::vector<HepMC::ConstGenParticlePtr>*
Trk::InDetReconstructableSelector::selectGenSignal (const McEventCollection* SimTracks) const {

  if (! SimTracks) return nullptr;

  std::vector<HepMC::ConstGenParticlePtr>* genSignal = 
    new std::vector<HepMC::ConstGenParticlePtr>;

  // pile-up: vector of MCEC has more than one entry
  DataVector<HepMC::GenEvent>::const_iterator itCollision = SimTracks->begin();
  
  for( ; itCollision != SimTracks->end(); ++itCollision ) {
    const HepMC::GenEvent*    genEvent = *itCollision;
    
    for ( const auto& particle:   *genEvent) {


      // 1) require stable particle from generation or simulation
      if (!MC::isStable(particle))    continue;

      if(particle->production_vertex() == nullptr) {
        ATH_MSG_WARNING ("GenParticle without production vertex - simulation corrupt? ");
        ATH_MSG_DEBUG   ("It's this one: " << particle);
        continue;
      } else {
      
        // 2) require track inside ID - relaxed definition including decays of neutrals (secondaries)
        if ( std::abs(particle->production_vertex()->position().perp()) > m_maxRStartAll ||
             std::abs(particle->production_vertex()->position().z())    > m_maxZStartAll ) continue;

        // 3) if jobOption, require strict definition of particles from within beam pipe
        if ( m_selectPrimariesOnly && 
             ( std::abs(particle->production_vertex()->position().perp()) > m_maxRStartPrimary ||
               std::abs(particle->production_vertex()->position().z())    > m_maxZStartPrimary ) ) continue;

        int   pdgCode         = particle->pdg_id();
        if (MC::isNucleus(pdgCode)) continue; // ignore nuclei from hadronic interactions
        const float charge = MC::charge(pdgCode);
        if (std::abs(charge)<0.5) continue;

        if (std::abs(particle->momentum().perp()) >  m_minPt  &&  std::abs(particle->momentum().pseudoRapidity()) < m_maxEta ) {
          genSignal->push_back(particle);
        }
      }
    } // loop and select particles
  }   // loop and select pile-up vertices
  return genSignal;
}

