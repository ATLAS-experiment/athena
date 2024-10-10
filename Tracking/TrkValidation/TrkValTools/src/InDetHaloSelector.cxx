/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

//////////////////////////////////////////////////////////////////
// InDetHaloSelector.cxx
//   Source file for class InDetHaloSelector
///////////////////////////////////////////////////////////////////

#include "TrkValTools/InDetHaloSelector.h"
#include "AtlasHepMC/GenVertex.h"
#include "CLHEP/Geometry/Point3D.h"
#include "AtlasHepMC/GenParticle.h"
#include "TruthUtils/HepMCHelpers.h"
#include "GeneratorObjects/McEventCollection.h"

Trk::InDetHaloSelector::InDetHaloSelector(const std::string& type, const std::string& name,
                                              const IInterface* parent)
  : AthAlgTool (type,name,parent)

{
  declareInterface<IGenParticleSelector>(this);

}

///////////////////////////////
/// initialize
///////////////////////////////
StatusCode Trk::InDetHaloSelector::initialize() {
  ATH_MSG_DEBUG ("initialise in " << name());
  return StatusCode::SUCCESS;
}

StatusCode Trk::InDetHaloSelector::finalize() {
  ATH_MSG_DEBUG ("starting finalize() in " << name());
  return StatusCode::SUCCESS;
}

std::vector<HepMC::ConstGenParticlePtr>*
Trk::InDetHaloSelector::selectGenSignal (const McEventCollection* SimTracks) const {

  if (! SimTracks) return nullptr;

  std::vector<HepMC::ConstGenParticlePtr>* genSignal = 
    new std::vector<HepMC::ConstGenParticlePtr>;

  // pile-up: vector of MCEC has more than one entry
  DataVector<HepMC::GenEvent>::const_iterator itCollision = SimTracks->begin();
  
  for( ; itCollision != SimTracks->end(); ++itCollision ) {
    const HepMC::GenEvent*    genEvent = *itCollision;
    
    for (const auto& particle: *genEvent) {

      // 1) require stable particle from generation or simulation
      if (!MC::isStable(particle))    continue;

      int   pdgCode         = particle->pdg_id();
      if (MC::isNucleus(pdgCode)) continue; // ignore nuclei from hadronic interactions
      ATH_MSG_DEBUG( "Checking particle " <<  particle );
      float charge          = MC::charge(pdgCode);
      ATH_MSG_DEBUG( "particle charge = " << charge );
      if (std::fabs(charge)<0.5) continue;
      
      genSignal->push_back(particle);
    
    } // loop and select particles
  }   // loop and select pile-up vertices
  return genSignal;
}

