/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAOD_ANALYSIS

#include "EvgenProdTools/FixHepMC.h"
#include "TruthUtils/HepMCHelpers.h"
#include "AtlasHepMC/GenVertex.h"
#include "AtlasHepMC/GenEvent.h"
#include "AtlasHepMC/AttributeNames.h"
#include <vector>


FixHepMC::FixHepMC(const std::string& name, ISvcLocator* pSvcLocator)
  : GenBase(name, pSvcLocator)
  , m_loopKilled(0)
  , m_pdg0Killed(0)
  , m_decayCleaned(0)
  , m_unstablePurged(0)
  , m_totalSeen(0)
  , m_replacedPIDs(0)
{
  declareProperty("KillLoops", m_killLoops = true, "Remove particles in loops?");
  declareProperty("KillPDG0", m_killPDG0 = true, "Remove particles with PDG ID 0?");
  declareProperty("CleanDecays", m_cleanDecays = true, "Clean decay chains from non-propagating particles?");
  declareProperty("PurgeUnstableWithoutEndVtx", m_purgeUnstableWithoutEndVtx = false, "Remove unstable particles without decay vertex?");
  declareProperty("IgnoreSemiDisconnected", m_ignoreSemiDisconnected = false, "Ignore semi-disconnected particles (normal in Sherpa)");
  declareProperty("PIDmap", m_pidmap = std::map<int,int>(), "Map of PDG IDs to replace");
  declareProperty("forced_momentum", m_forced_momentum = "MEV", "Forced momentum unit");
  declareProperty("forced_length", m_forced_length = "MM", "Forced length unit");
  declareProperty("ApplyUnitsFix", m_unitsFix = true, "Attempt to identify momentum units problems and fix them");
  declareProperty("SetHasCycles", m_setHasCycles = false, "Inform HEPMC3 that this event has cycles (loops)");
}

StatusCode FixHepMC::execute(const EventContext& /*ctx*/) {
  for (McEventCollection::const_iterator ievt = events()->begin(); ievt != events()->end(); ++ievt) {
    // FIXME: const_cast
    HepMC::GenEvent* evt = const_cast<HepMC::GenEvent*>(*ievt);
    m_looper.findLoops(evt,true);
    /* AV: To understand why some algorithms fail one would have to request debug explicitly, but that is the reviwers consensus.*/
    if (!m_looper.loop_particles().empty() || !m_looper.loop_vertices().empty()) {
      ATH_MSG_DEBUG("Found " << m_looper.loop_vertices().size() << " vertices in loops");
      ATH_MSG_DEBUG("Found " << m_looper.loop_particles().size() << " particles in loops");
      ATH_MSG_DEBUG("Please use MC::Loops::findLoops for this event to obtain all particles and vertices in the loops");
    }
    auto old_momentum = evt->momentum_unit();
    auto old_length = evt->length_unit();
    evt->set_units(
        m_forced_momentum != "" ? HepMC3::Units::momentum_unit(m_forced_momentum): old_momentum,
        m_forced_length != "" ? HepMC3::Units::length_unit(m_forced_length): old_length);
    if ( m_forced_momentum != "" && m_forced_momentum != HepMC3::Units::name(old_momentum) ) ATH_MSG_WARNING("Updated momentum units " <<  HepMC3::Units::name(old_momentum) << "->" << m_forced_momentum);
    if ( m_forced_length != "" && m_forced_length != HepMC3::Units::name(old_length) ) ATH_MSG_WARNING("Updated length units " <<  HepMC3::Units::name(old_length) << "->" << m_forced_length);

    if (!m_pidmap.empty()) {
      for (auto ip: *evt) {
        // Skip this particle if (somehow) its pointer is null
         if (!ip) continue;
         auto newpid = m_pidmap.find(ip->pdg_id());
         if (newpid == m_pidmap.end()) continue;
         ip->set_pdg_id(newpid->second);
         // Increment the counter for replaced PDG IDs
         ++m_replacedPIDs;
         m_replacedpid_counts[ip->pdg_id()]++;
      }
    }

    if (m_setHasCycles){
      // If asked, tag the event as having cycles and alert the user that this problem exists
      auto cycles = std::make_shared<HepMC3::IntAttribute>(1);
      evt->add_attribute(HepMCStr::cycles, cycles);
    }

    // Add a unit entry to the event weight vector if it's currently empty
    if (evt->weights().empty()) {
      ATH_MSG_DEBUG("Adding a unit weight to empty event weight vector");
      evt->weights().push_back(1);
    }
    // Set a (0,0,0) vertex to be the signal vertex if not already set
    if (!HepMC::signal_process_vertex(evt)) {
      const HepMC::FourVector nullpos;
      for (auto  iv: evt->vertices()) {
        if (iv->position() == nullpos) {
          ATH_MSG_DEBUG("Setting representative event position vertex");
          HepMC::set_signal_process_vertex(evt,iv);
          break;
        }
      }
    }

    // Catch cases with more than 2 beam particles (16.11.2021)
    std::vector<HepMC::GenParticlePtr> beams_t;
    for (const HepMC::GenParticlePtr& p : evt->beams()) {
      if (p->status() == 4)  beams_t.push_back(p);
    }
    if (beams_t.size() > 2) {
      ATH_MSG_INFO("Invalid number of beam particles " <<  beams_t.size() << ". Will try to fix.");
      std::vector<HepMC::GenParticlePtr> bparttoremove;
      for (const auto& bpart : beams_t) {
        if (bpart->id() == 0 && bpart->production_vertex()) bparttoremove.push_back(bpart);
      }
      for (auto bpart: bparttoremove) {
        bpart->production_vertex()->remove_particle_out(bpart);
      }
    }

    // Some generators / samples have a mis-match between the units they report and the units used for momentum, usually because we have applied
    // a correction to the momentum without also correcting the units. Here we're going to check for states that look particularly suspicious
    // and apply a correction if required. We will try to identify those issues based on the beam particles.
    if (m_unitsFix){ // Only if requested - allow folks to disable this if they know what they're doing
      double units_problem = -1.;
      // If we have beam particles, let's use them - it's much faster!
      if (beams_t.size() > 0 ){
        for (const HepMC::GenParticlePtr& p : beams_t){
          if (p->momentum().pz() > 1e9) units_problem = p->momentum().pz();
        }
      // if we didn't have beam particles, we'll go through some of the main record
      } else {
        for (const HepMC::GenParticlePtr& p : evt->particles()) {
          if (p && p->momentum().pz() > 1e9) units_problem = p->momentum().pz();
        }
      }
      if (units_problem>0){ // No particles should have momenta above 1 PeV; this must be a units issue
        ATH_MSG_INFO("Apparent units problem; beam particles have z-momentum " << units_problem << " in MeV. Will divide by 1000.");
        MC::MeVToGeV(evt);  //Only scales momenta and masses
      }
    }

    // Some heuristics to catch problematic cases
    std::vector<HepMC::GenParticlePtr> semi_disconnected, decay_loop_particles;
    for (auto ip : evt->particles()) {
      // Skip this particle if (somehow) its pointer is null
      if (!ip) continue;
      bool particle_to_fix = false;
      int abspid = std::abs(ip->pdg_id());
      auto vProd = ip->production_vertex();
      auto vEnd  = ip->end_vertex();
      /// Case 1: particles without production vertex, except beam particles (status 4)
      if ( (!vProd || vProd->id() == 0) && vEnd && ip->status() != 4) {
        particle_to_fix = true;
        ATH_MSG_DEBUG("Found particle " << ip->pdg_id() << " without production vertex! HepMC status = " << ip->status());
      }
      /// Case 2: non-final-state particles without end vertex
      if (vProd && !vEnd && ip->status() != 1) {
        particle_to_fix = true;
        ATH_MSG_DEBUG("Found particle " << ip->pdg_id() << " without decay vertex! HepMC status = " << ip->status());
      }
      if (particle_to_fix)  semi_disconnected.push_back(ip);
      // Case 3: keep track of loop particles inside decay chains (seen in H7+EvtGen)
      if (abspid == 43 || abspid == 44 || abspid == 30353 || abspid == 30343) {
        decay_loop_particles.push_back(std::move(ip));
      }
    }

    /// AV: In case we have 3 particles, we try to add a vertex 
    /// that corresponds to 1->2 and 1->1 splitting.
    /// AV: In case we have 4 particles, we can try to do that as well.

    /// YH: In the case of Sherpa with HEPMC_TREE_LIKE: 1, where the
    /// YH: incoming/outgoing particles of the signal process have no
    /// YH: production/end vertices, this treatment can produce a loop.
    /// YH: Skip it by setting IgnoreSemiDisconnected = True.
    if ( !m_ignoreSemiDisconnected && (semi_disconnected.size() == 4 || semi_disconnected.size() == 3 || semi_disconnected.size() == 2)) {
      size_t no_endv = 0;
      size_t no_prov = 0;
      HepMC::FourVector sum(0,0,0,0);
      std::set<HepMC::GenVertexPtr> standalone;
      for (const auto& part : semi_disconnected) {
        if (!part->production_vertex() || !part->production_vertex()->id()) {
          no_prov++; sum += part->momentum();
        }
        if (!part->end_vertex()) { 
          no_endv++;  sum -= part->momentum();
        }
        if (part->production_vertex()) standalone.insert(part->production_vertex());
        if (part->end_vertex()) standalone.insert(part->end_vertex());
      }
      ATH_MSG_INFO("Heuristics: found " << semi_disconnected.size() << " semi-disconnected particles. Momentum sum is " << sum << " Standalone vertices " << standalone.size());
      bool standalonevertex = (standalone.size() == 1 && (*standalone.begin())->particles_in().size() + (*standalone.begin())->particles_out().size() == semi_disconnected.size());
      /// The condition below will cover 1->1, 1->2 and 2->1 cases
      if (! standalonevertex && no_endv && no_prov  && ( no_endv + no_prov  == semi_disconnected.size() )) {
        if (std::abs(sum.px()) < 1e-2  && std::abs(sum.py()) < 1e-2  && std::abs(sum.pz()) < 1e-2 ) {
          ATH_MSG_INFO("Try " << no_endv << "->" << no_prov << " splitting/merging.");
          auto v = HepMC::newGenVertexPtr();
          for (auto part : semi_disconnected) {
            if (!part->production_vertex() || part->production_vertex()->id() == 0) v->add_particle_out(std::move(part));
          }
          for (auto part : semi_disconnected) {
            if (!part->end_vertex()) v->add_particle_in(std::move(part));
          }
          evt->add_vertex(std::move(v));
        }
      }
    }

    /// Remove loops inside decay chains
    /// AV: Please note that this approach would distort some branching ratios.
    /// If some particle would have decay products with bad PDG ids, after the operation below
    /// the visible branching ratio of these decays would be zero.
    for (auto part: decay_loop_particles) {
      /// Check the bad particles have prod and end vertices
      auto vend = part->end_vertex();
      auto vprod = part->production_vertex();
      if (!vprod || !vend)  continue;
      bool loop_in_decay = true;
      /// Check that all particles coming into the decay vertex of a
      /// decay-loop particle candidate came from the same production vertex
      auto sisters = vend->particles_in();
      for (auto sister: sisters) {
        if (vprod != sister->production_vertex()) loop_in_decay = false;
      }
      if (!loop_in_decay) continue;

      /// remove loop
      auto daughters = vend->particles_out();
      for (auto p : daughters) vprod->add_particle_out(std::move(p));
      for (auto sister : sisters) { 
        vprod->remove_particle_out(sister); 
        vend->remove_particle_in(sister); 
        evt->remove_particle(std::move(sister));
      }
      evt->remove_vertex(std::move(vend));

    }

    // Event particle content cleaning -- remove "bad" structures
    std::vector<HepMC::GenParticlePtr> toremove;
    for (auto ip: evt->particles()) {
      // Skip this particle if (somehow) its pointer is null
      if (!ip) continue;
      m_totalSeen += 1;
      // Flag to declare if a particle should be removed
      bool bad_particle = false;
      // Check for loops
      if ( m_killLoops && isSimpleLoop(ip) ) {
        bad_particle = true;
        m_loopKilled += 1;
        ATH_MSG_DEBUG( "Found a looper : " );
        if ( msgLvl( MSG::DEBUG ) ) HepMC::Print::line(ip);
      }
      // Check on PDG ID 0
      if ( m_killPDG0 && isPID0(ip) ) {
        bad_particle = true;
        m_pdg0Killed += 1;
        ATH_MSG_DEBUG( "Found PDG ID 0 : " );
        if ( msgLvl( MSG::DEBUG ) )HepMC::Print::line(ip);
      }
      // Clean decays
      int abs_pdg_id = std::abs(ip->pdg_id());
      bool is_decayed_weak_boson =  ( abs_pdg_id == 23 || abs_pdg_id == 24 || abs_pdg_id == 25 ) && ip->end_vertex();
      if ( m_cleanDecays && isNonTransportableInDecayChain(ip) && !is_decayed_weak_boson ) {
        bad_particle = true;
        m_decayCleaned += 1;
        ATH_MSG_DEBUG( "Found a bad particle in a decay chain : " );
        if ( msgLvl( MSG::DEBUG ) ) HepMC::Print::line(ip);
      }
      // Only add to the toremove vector once, even if multiple tests match
      if (bad_particle) toremove.push_back(std::move(ip));
    }

    // Properties before cleaning
    const int num_particles_orig = evt->particles().size();

    // Do the cleaning
    if (!toremove.empty()) {
      ATH_MSG_DEBUG("Cleaning event record of " << toremove.size() << " bad particles");
      for (auto part: toremove) evt->remove_particle(std::move(part));
    }

    if(m_purgeUnstableWithoutEndVtx) {
      int purged=0;
      do {
        purged=0;
        const std::vector <HepMC::GenParticlePtr> allParticles=evt->particles();
        for(auto p : allParticles) {
          HepMC::ConstGenVertexPtr end_v=p->end_vertex();
          if(p->status() == 2 && !end_v) {
            evt->remove_particle(std::move(p));
            ++purged;
            ++m_unstablePurged;
          } 
        }
      }
      while (purged>0);
    }

    const int num_particles_filt = evt->particles().size();

    if(num_particles_orig!=num_particles_filt) {
      // Write out the change in the number of particles
      ATH_MSG_INFO("Particles filtered: " << num_particles_orig << " -> " << num_particles_filt);
    }

  } // End of the loop over events in the MC event collection
  return StatusCode::SUCCESS;
}


StatusCode FixHepMC::finalize() {
  if (m_killLoops  ) ATH_MSG_INFO( "Removed " <<   m_loopKilled << " of " << m_totalSeen << " particles because of loops." );
  if (m_killPDG0   ) ATH_MSG_INFO( "Removed " <<   m_pdg0Killed << " of " << m_totalSeen << " particles because of PDG ID 0." );
  if (m_cleanDecays) ATH_MSG_INFO( "Removed " << m_decayCleaned << " of " << m_totalSeen << " particles while cleaning decay chains." );
  if(m_purgeUnstableWithoutEndVtx) ATH_MSG_INFO( "Removed " << m_unstablePurged << " of " << m_totalSeen << " unstable particles because they had no decay vertex." );
  if (!m_pidmap.empty()) {
    ATH_MSG_INFO( "Replaced " << m_replacedPIDs << " PIDs of particles." );
    for (const auto& p: m_replacedpid_counts) ATH_MSG_INFO( "Replaced " << p.first << " PIDs " << p.second << "times." );
  }  
  return StatusCode::SUCCESS;
}


/// @name Classifiers for identifying particles to be removed
//@{

// Identify PDG ID = 0 particles
bool FixHepMC::isPID0(const HepMC::ConstGenParticlePtr& p) const {
  return p->pdg_id() == 0;
}

// Identify the particles from
bool FixHepMC::fromDecay(const HepMC::ConstGenParticlePtr& p, std::shared_ptr<std::set<int> >& storage) const {
      if (!p) return false;
      auto v=p->production_vertex();
      if (!v) return false;
      for ( const auto& anc: v->particles_in()) {
        if (MC::isDecayed(anc) && (MC::isTau(anc->pdg_id()) || MC::isHadron(anc->pdg_id()))) return true;
      }
      if (storage->find(p->id()) != storage->end()) return false;
      storage->insert(p->id());
      for ( const auto& anc: v->particles_in()) {
            if (fromDecay(anc, storage)) return true;
      }
      return false;
}

// Identify non-transportable stuff _after_ hadronisation
bool FixHepMC::isNonTransportableInDecayChain(const HepMC::ConstGenParticlePtr& p) const {
  auto storage = std::make_shared<std::set<int>>();
  return !MC::isTransportable(p->pdg_id()) && fromDecay(p, storage);
}

// Identify internal "loop" particles
bool FixHepMC::isSimpleLoop(const HepMC::ConstGenParticlePtr& p) const {
  return (p->production_vertex() == p->end_vertex() && p->end_vertex() != nullptr);
}

//@}

#endif

