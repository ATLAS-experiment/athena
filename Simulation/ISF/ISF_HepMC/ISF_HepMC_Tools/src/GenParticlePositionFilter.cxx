/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// class header include
#include "GenParticlePositionFilter.h"

// HepMC includes
#include "AtlasHepMC/GenParticle.h"
#include "AtlasHepMC/GenVertex.h"
#include "AtlasHepMC/SimpleVector.h"

/** Constructor **/
ISF::GenParticlePositionFilter::GenParticlePositionFilter( const std::string& t,
                                                           const std::string& n,
                                                           const IInterface* p )
  : base_class(t,n,p)
{
}


// Athena algtool's Hooks
StatusCode  ISF::GenParticlePositionFilter::initialize()
{
    ATH_MSG_VERBOSE("initialize()");

    // retrieve the GeoIDService
    ATH_CHECK( m_geoIDSvc.retrieve() );

    ATH_MSG_VERBOSE("initialize() successful");
    return StatusCode::SUCCESS;
}


/** does the given particle pass the filter? */
#ifdef HEPMC3
bool ISF::GenParticlePositionFilter::pass(const HepMC::ConstGenParticlePtr& particle) const
{
  // the GenParticle production vertex
  auto  vtx = particle->production_vertex();
#else
bool ISF::GenParticlePositionFilter::pass(const HepMC::GenParticle& particle) const
{
  // the GenParticle production vertex
  HepMC::GenVertexPtr vtx = particle.production_vertex();
#endif

  // no production vertex?
  if (!vtx) {
    ATH_MSG_DEBUG("GenParticle does not have a production vertex, filtering it out");
    return false;
  }

  // (x,y,z,t) position
  HepMC::FourVector pos = vtx->position();

  bool inside = false;
  // check if the particle position is inside (or on surface)
  // of any of the given ISF Simulation GeoIDs
  std::vector<int>::const_iterator checkRegionIt    = m_checkRegion.begin();
  std::vector<int>::const_iterator checkRegionItEnd = m_checkRegion.end();
  for ( ; (checkRegionIt!=checkRegionItEnd) && (!inside); ++checkRegionIt) {
    // consult the GeoID identification service
    ISF::InsideType insideCheck = m_geoIDSvc->inside( pos.x(),
                                                      pos.y(),
                                                      pos.z(),
                                                      AtlasDetDescr::AtlasRegion(*checkRegionIt) );
    // is inside only if ==fInside or ==fSurface
    inside |= (insideCheck==ISF::fInside) || (insideCheck==ISF::fSurface);
  }


  // return whether pos was inside any of the simulation geometries
  if (inside)
    ATH_MSG_VERBOSE("GenParticle is inside AtlasVolume and passed the filter");
  else
    ATH_MSG_VERBOSE("GenParticle is outside AtlasVolume and got fitered out");
  return inside;
}
