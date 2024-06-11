/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// ISF_Algs includes
#include "TruthPreselectionTool.h"
// McEventCollection
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
#ifdef HEPMC3
bool ISF::TruthPreselectionTool::passesFilters(HepMC::ConstGenParticlePtr& part, const ToolHandleArray<IGenParticleFilter>& filters) const
#else
bool ISF::TruthPreselectionTool::passesFilters(HepMC::ConstGenParticlePtr part, const ToolHandleArray<IGenParticleFilter>& filters) const
#endif
{
  // TODO: implement this as a std::find_if with a lambda function
  for ( const auto& filter : filters ) {
    // determine if the particle passes current filter
#ifdef HEPMC3
    bool passFilter = filter->pass(part);
#else
    bool passFilter = filter->pass(*part);
#endif
    ATH_MSG_VERBOSE("Filter '" << filter.typeAndName() << "' returned: "
                    << (passFilter ? "true, will keep particle."
                        : "false, will remove particle."));

    if (!passFilter) return false;
  }

  return true;
}

#ifdef HEPMC3
bool ISF::TruthPreselectionTool::identifiedQuasiStableParticleForSim(HepMC::ConstGenParticlePtr& part) const
#else
bool ISF::TruthPreselectionTool::identifiedQuasiStableParticleForSim(HepMC::ConstGenParticlePtr part) const
#endif
{
  bool b_sim = false;
#ifdef HEPMC3
  if (m_quasiStableFilter->pass(part)) {
    b_sim = passesFilters(part, m_genParticleFilters);
  }
#else
  if (m_quasiStableFilter->pass(*part)) {
    b_sim = passesFilters(part, m_genParticleFilters);
  }
#endif
  return b_sim;
}

#ifdef HEPMC3
bool ISF::TruthPreselectionTool::hasQuasiStableAncestorParticle(HepMC::ConstGenParticlePtr& part) const
#else
bool ISF::TruthPreselectionTool::hasQuasiStableAncestorParticle(HepMC::ConstGenParticlePtr part) const
#endif
{
  // TODO: investigate making this more efficient
#ifdef HEPMC3
  // Recursively loop over ancestral particles looking for a quasi-stable particle
  if (!part->production_vertex() || part->production_vertex()->particles_in().empty()) { return false; }
  for ( auto ancestor: part->production_vertex()->particles_in() ) {
    // Check ancestor particle for Attribute
    if ( ancestor->attribute<HepMC3::IntAttribute>("ShadowParticleId") ) { return true; }
#else
  if (!part->production_vertex() || part->production_vertex()->particles_in_size()==0) { return false; }
  // Recursively loop over ancestral particles looking for a quasi-stable particle
  auto  firstParent = part->production_vertex()->particles_begin(HepMC::parents);
  auto lastParent  = part->production_vertex()->particles_end(HepMC::parents);
  for (auto  pitr = firstParent; pitr != lastParent; ++pitr ) {
    HepMC::ConstGenParticlePtr ancestor = *pitr;
    if (identifiedQuasiStableParticleForSim(ancestor)) { return true; }
#endif
    if (hasQuasiStableAncestorParticle(ancestor)) { return true; }
  }
  return false;
}

#ifdef HEPMC3
bool ISF::TruthPreselectionTool::isPostQuasiStableParticleVertex(HepMC::ConstGenVertexPtr& vtx) const
{
  // All outgoing particles have already been removed.
  if ( vtx->particles_out().empty() ) { return true; }
  return false;
}
#else
bool ISF::TruthPreselectionTool::isPostQuasiStableParticleVertex(HepMC::ConstGenVertexPtr vtx) const
{
  // Recursively loop over ancestral particles looking for a quasi-stable particle
#ifdef HEPMC3
  for ( auto ancestor: vtx->particles_in() ) {
    // Check ancestor particle for Attribute
    if ( ancestor->attribute<HepMC3::IntAttribute>("ShadowParticleId") ) { return true; }
#else
  auto  firstParent = vtx->particles_in_const_begin();
  auto lastParent  = vtx->particles_in_const_end();
  for (auto  pitr = firstParent; pitr != lastParent; ++pitr ) {
    HepMC::ConstGenParticlePtr ancestor = *pitr;
    if (identifiedQuasiStableParticleForSim(ancestor)) { return true; }
#endif
    if (hasQuasiStableAncestorParticle(ancestor)) { return true; }
  }
  return false;
}
#endif

std::unique_ptr<HepMC::GenEvent> ISF::TruthPreselectionTool::filterGenEvent(const HepMC::GenEvent& inputEvent) const {
#ifdef HEPMC3
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
       particle->add_attribute("ShadowParticleId",
                               std::make_shared<HepMC3::IntAttribute>(shadowId));
       // Version 2 Directly save the ConstGenParticlePtr - needs to link to a version of the GenEvent after zero-lifetime positioner as been applied.
       // HepMC::ConstGenParticlePtr& shadow = inputEvent.particles().at(shadowId);
       // particle->add_attribute("ShadowParticle",
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
#else

  std::unique_ptr<HepMC::GenEvent> outputEvent = std::make_unique<HepMC::GenEvent>(inputEvent.signal_process_id(),
                                                                                   inputEvent.event_number());

  outputEvent->set_mpi                  ( inputEvent.mpi() );
  outputEvent->set_event_scale          ( inputEvent.event_scale() );
  outputEvent->set_alphaQCD             ( inputEvent.alphaQCD() );
  outputEvent->set_alphaQED             ( inputEvent.alphaQED() );
  if ( inputEvent.cross_section() ) {
    outputEvent->set_cross_section        ( *inputEvent.cross_section());
  }
  if (inputEvent.heavy_ion()) {
    outputEvent->set_heavy_ion(*(inputEvent.heavy_ion()));
  }
  if (inputEvent.pdf_info()) {
    outputEvent->set_pdf_info(*(inputEvent.pdf_info()));
  }
  outputEvent->define_units( inputEvent.momentum_unit(), inputEvent.length_unit() );

  // 1. create a NEW copy of all vertices from inputEvent
  //    taking care to map new vertices onto the vertices being copied
  //    and add these new vertices to this event.
  //    We do not use GenVertex::operator= because that would copy
  //    the attached particles as well.
  std::map<const HepMC::GenVertex*,HepMC::GenVertex*> inputEvtVtxToOutputEvtVtx;
  HepMC::GenEvent::vertex_const_iterator currentVertexIter(inputEvent.vertices_begin());
  const HepMC::GenEvent::vertex_const_iterator endOfCurrentListOfVertices(inputEvent.vertices_end());
  ATH_MSG_VERBOSE( "Starting a vertex loop ... " );
  for (; currentVertexIter != endOfCurrentListOfVertices; ++currentVertexIter) {
    const HepMC::GenVertex *pCurrentVertex(*currentVertexIter);
    if (isPostQuasiStableParticleVertex(pCurrentVertex)) {
      continue; // skip vertices created by the simulation
    }
    std::unique_ptr<HepMC::GenVertex> copyOfGenVertex =std::make_unique<HepMC::GenVertex>(pCurrentVertex->position(), pCurrentVertex->id(), pCurrentVertex->weights() );
    copyOfGenVertex->suggest_barcode( HepMC::barcode(pCurrentVertex) );
    inputEvtVtxToOutputEvtVtx[pCurrentVertex] = copyOfGenVertex.get();
    outputEvent->add_vertex( copyOfGenVertex.release() );
  } //vertex loop

  // 2. copy the signal process vertex info.
  if ( inputEvent.signal_process_vertex() ) {
    outputEvent->set_signal_process_vertex( inputEvtVtxToOutputEvtVtx[inputEvent.signal_process_vertex()] );
  }
  else {
    outputEvent->set_signal_process_vertex( nullptr );
  }
  //
  // 3. create a NEW copy of all particles from inputEvent
  //    taking care to attach them to the appropriate vertex
  HepMC::GenParticle* beam1{};
  HepMC::GenParticle* beam2{};
  for ( HepMC::GenEvent::particle_const_iterator particleIter = inputEvent.particles_begin();
        particleIter != inputEvent.particles_end(); ++particleIter ) {
    const HepMC::GenParticle* currentParticle = *particleIter;
    if (hasQuasiStableAncestorParticle(currentParticle)) { // TODO Consider making the threshold for this check configurable.
      continue; // skip particles created by the simulation
    }
    std::unique_ptr<HepMC::GenParticle> copyOfGenParticle = std::make_unique<HepMC::GenParticle>(*currentParticle);
    const bool isBeamParticle1(currentParticle == inputEvent.beam_particles().first);
    const bool isBeamParticle2(currentParticle == inputEvent.beam_particles().second);
    // There may (will) be particles which had end vertices added by
    // the simulation (inputEvent). Those vertices will not be copied
    // to the outputEvent, so we should not try to use them here.
    const bool shouldAddProdVertex(currentParticle->production_vertex() && inputEvtVtxToOutputEvtVtx[ currentParticle->production_vertex() ]);
    const bool shouldAddEndVertex(currentParticle->end_vertex() && inputEvtVtxToOutputEvtVtx[ currentParticle->end_vertex() ]);
    if ( isBeamParticle1 || isBeamParticle2 || shouldAddProdVertex || shouldAddEndVertex ) {
      HepMC::GenParticle* particleCopy = copyOfGenParticle.release();
      if ( isBeamParticle1 ) {
        beam1 = particleCopy;
      }
      if ( isBeamParticle2 ) {
        beam2 = particleCopy;
      }
      if ( shouldAddProdVertex || shouldAddEndVertex ) {
        if ( shouldAddEndVertex  ) {
          inputEvtVtxToOutputEvtVtx[ currentParticle->end_vertex() ]->
            add_particle_in(particleCopy);
        }
        if ( shouldAddProdVertex ) {
          inputEvtVtxToOutputEvtVtx[ currentParticle->production_vertex() ]->
            add_particle_out(particleCopy);
        }
      }
      else {
        ATH_MSG_WARNING ( "Found GenParticle with no production or end vertex! \n" << *currentParticle);
      }
    }
  }
  outputEvent->set_beam_particles( beam1, beam2 );
  //
  // 4. now that vtx/particles are copied, copy weights and random states
  outputEvent->set_random_states( inputEvent.random_states() );
  outputEvent->weights() = inputEvent.weights();
#endif
  return outputEvent;
}
