/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

//////////////////////////////////////////////////////////////////
// InDetPrimaryConversionSelector.cxx
//   Source file for class InDetPrimaryConversionSelector
///////////////////////////////////////////////////////////////////

#include "TrkValTools/InDetPrimaryConversionSelector.h"
#include "AtlasHepMC/GenVertex.h"
#include "CLHEP/Geometry/Point3D.h"
#include "CLHEP/Units/SystemOfUnits.h"
#include "AtlasHepMC/GenParticle.h"
#include "TruthUtils/HepMCHelpers.h"
#include "GeneratorObjects/McEventCollection.h"

Trk::InDetPrimaryConversionSelector::InDetPrimaryConversionSelector(const std::string& type, const std::string& name,
                                              const IInterface* parent)
  : AthAlgTool (type,name,parent),
    m_minPt             ( 500.    ),
    m_maxEta            (   2.5   ),
    m_maxRStartAll      ( 800.0*CLHEP::mm),
    m_maxZStartAll      (2000.0*CLHEP::mm)

{
  declareInterface<IGenParticleSelector>(this);

  declareProperty("MinPt",              m_minPt);
  declareProperty("MaxEta",             m_maxEta);
  declareProperty("MaxRStartAll",       m_maxRStartAll,        "production vtx r for all");
  declareProperty("MaxZStartAll",       m_maxZStartAll,        "production vtx z for all");
}

///////////////////////////////
/// initialize
///////////////////////////////
StatusCode Trk::InDetPrimaryConversionSelector::initialize() {
  ATH_MSG_INFO ("initialise in " << name());
  return StatusCode::SUCCESS;
}

StatusCode Trk::InDetPrimaryConversionSelector::finalize() {
  ATH_MSG_INFO ("starting finalize() in " << name());
  return StatusCode::SUCCESS;
}

std::vector<HepMC::ConstGenParticlePtr>*
Trk::InDetPrimaryConversionSelector::selectGenSignal (const McEventCollection* SimTracks) const {

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

      if(!particle->production_vertex()) {
        ATH_MSG_WARNING ("GenParticle without production vertex - simulation corrupt? ");
        ATH_MSG_DEBUG   ("It's this one: " << particle);
        continue;
      } else {
      
        // 2) require track inside ID - relaxed definition including decays of neutrals (secondaries)
        if ( std::fabs(particle->production_vertex()->position().perp()) > m_maxRStartAll ||
             std::fabs(particle->production_vertex()->position().z())    > m_maxZStartAll ) continue;

        int   pdgCode         = particle->pdg_id();
        if (MC::isNucleus(pdgCode)) continue; // ignore nuclei from hadronic interactions
	ATH_MSG_DEBUG ("found particle = " << particle);

	// assume for the moment we're only running over single gamma MC files ...
        auto  prodVertex = particle->production_vertex();
	if ( std::abs(pdgCode) == 11 ) {
	  ATH_MSG_DEBUG ("Electron/Positron detected -- checking for production process ...");
#ifdef HEPMC3
	  for ( const auto&  inParticle: prodVertex->particles_in()) {
#else
	  HepMC::GenVertex::particles_in_const_iterator ItinParticle     = prodVertex->particles_in_const_begin();
	  HepMC::GenVertex::particles_out_const_iterator ItinParticleEnd = prodVertex->particles_in_const_end();
	  for ( ; ItinParticle != ItinParticleEnd; ++ItinParticle) {
            auto inParticle=*ItinParticle;
#endif
	    ATH_MSG_DEBUG(" --> checking morther: " << inParticle );
	    if ( MC::isPhoton(inParticle) || MC::isElectron(inParticle) ){
	      if (std::fabs(particle->momentum().perp()) >  m_minPt  &&  std::fabs(particle->momentum().pseudoRapidity()) < m_maxEta ) {
		genSignal->push_back(particle);
		ATH_MSG_DEBUG ("Selected this electron/positron!");
		break;
	      } // if (particle pt, eta)
	    } // if (mother is electron or gamma ...)
	  } // loop over mother particles
	} // if (have electron/positron)
      } // if (have stable particle)
    } // loop and select particles
  }   // loop and select pile-up vertices
  return genSignal;
}

