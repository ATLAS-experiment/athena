/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// ISF_Algs includes
#include "TruthPreselectionTool.h"
#include "AtlasHepMC/HeavyIon.h"

///////////////////////////////////////////////////////////////////
// Public methods:
///////////////////////////////////////////////////////////////////

// Constructors
////////////////
ISF::TruthPreselectionTool::TruthPreselectionTool( const std::string& t, const std::string& n, const IInterface* p ) :
  base_class( t, n, p )
{
}

// Athena Algorithm's Hooks
////////////////////////////
StatusCode ISF::TruthPreselectionTool::initialize()
{
  ATH_MSG_VERBOSE ( "Initializing TruthPreselectionTool Algorithm" );

  if (!m_genParticleFilters.empty()) ATH_CHECK(m_genParticleFilters.retrieve());
  ATH_CHECK(m_quasiStableFilter.retrieve());

  // intialziation successful
  return StatusCode::SUCCESS;
}

/** check if the given particle passes all filters */
bool ISF::TruthPreselectionTool::passesFilters(HepMC::ConstGenParticlePtr& part, const ToolHandleArray<IGenParticleFilter>& filters) const
{
  // TODO: implement this as a std::find_if with a lambda function
  for ( const auto& filter : filters ) {
    // determine if the particle passes current filter
    bool passFilter = filter->pass(part);
    ATH_MSG_VERBOSE("Filter '" << filter.typeAndName() << "' returned: "
                    << (passFilter ? "true, will keep particle."
                        : "false, will remove particle."));

    if (!passFilter) return false;
  }

  return true;
}

bool ISF::TruthPreselectionTool::identifiedQuasiStableParticleForSim(HepMC::ConstGenParticlePtr& part) const
{
  bool b_sim = false;
  if (m_quasiStableFilter->pass(part)) {
    b_sim = passesFilters(part, m_genParticleFilters);
  }
  return b_sim;
}

bool ISF::TruthPreselectionTool::hasQuasiStableAncestorParticle(HepMC::ConstGenParticlePtr& part) const
{
  // TODO: investigate making this more efficient
  // Recursively loop over ancestral particles looking for a quasi-stable particle
  if (!part->production_vertex() || part->production_vertex()->particles_in().empty()) { return false; }
  for ( auto ancestor: part->production_vertex()->particles_in() ) {
    // Check ancestor particle for Attribute
    if ( ancestor->attribute<HepMC3::IntAttribute>(HepMCStr::ShadowParticleId) ) { return true; }
    if (hasQuasiStableAncestorParticle(ancestor)) { return true; }
  }
  return false;
}

bool ISF::TruthPreselectionTool::isPostQuasiStableParticleVertex(HepMC::ConstGenVertexPtr& vtx) const
{
  // All outgoing particles have already been removed.
  if ( vtx->particles_out().empty() ) { return true; }
  return false;
}

std::unique_ptr<HepMC::GenEvent> ISF::TruthPreselectionTool::filterGenEvent(const HepMC::GenEvent& inputEvent) const {
   /// The algorithm makes a deep copy of the event and drops the particles and vertices created by the simulation
   /// from the copied event.
   std::unique_ptr<HepMC::GenEvent>  outputEvent = std::make_unique<HepMC::GenEvent>(inputEvent);
   if (inputEvent.run_info()) {
     outputEvent->set_run_info(std::make_shared<HepMC3::GenRunInfo>(*(inputEvent.run_info().get())));
   }
   if (inputEvent.heavy_ion()) {
     outputEvent->set_heavy_ion(std::make_shared<HepMC::GenHeavyIon>(*(inputEvent.heavy_ion())));
   }
   HepMC::fillBarcodesAttribute(outputEvent.get());

   // First loop: flag the particles which should be passed to simulation
   for (auto& particle: outputEvent->particles()) {
       HepMC::ConstGenParticlePtr cparticle = particle;
       if (passesFilters(cparticle, m_genParticleFilters)) {
       // Particle to be simulated
       const int shadowId = particle->id();
       // Version 1 Use the Id
      particle->add_attribute(HepMCStr::ShadowParticleId,
                               std::make_shared<HepMC3::IntAttribute>(shadowId));
       // Version 2 Directly save the ConstGenParticlePtr - needs to link to a version of the GenEvent after zero-lifetime positioner as been applied.
       // HepMC::ConstGenParticlePtr& shadow = inputEvent.particles().at(shadowId);
      // particle->add_attribute(HepMCStr::ShadowParticle,
       //                         std::make_shared<HepMC::ShadowParticle>(particle));
     }
   }
   // Second loop(s): flag particles (and vertices) to be removed (i.e. those
   // with ancestor particle flagged to be passed to simulation).
   for (;;) {
     std::vector<HepMC::GenParticlePtr> p_to_remove;
     std::vector<HepMC::GenVertexPtr> v_to_remove;
     for (auto& particle: outputEvent->particles()) {
       HepMC::ConstGenParticlePtr cparticle = particle;
       if (hasQuasiStableAncestorParticle(cparticle)) {
         p_to_remove.push_back(particle);
       }
     }
     for (auto& particle: p_to_remove) outputEvent->remove_particle(particle);
     for (auto& vertex: outputEvent->vertices()) {
       HepMC::ConstGenVertexPtr cvertex = vertex;
       if (isPostQuasiStableParticleVertex(cvertex)) {
         v_to_remove.push_back(vertex);
       }
     }
     for (auto& vertex: v_to_remove) outputEvent->remove_vertex(vertex);
     if (p_to_remove.empty() && v_to_remove.empty()) break;
   }

  return outputEvent;
}
