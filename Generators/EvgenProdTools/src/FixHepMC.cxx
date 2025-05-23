/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAOD_ANALYSIS

#include "EvgenProdTools/FixHepMC.h"
#include "TruthUtils/HepMCHelpers.h"
#include "AtlasHepMC/GenVertex.h"
#include "AtlasHepMC/GenEvent.h"
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
  declareProperty("PIDmap", m_pidmap = std::map<int,int>(), "Map of PDG IDs to replace");
}
#ifndef HEPMC3
//---->//This is copied from MCUtils
  /// @name Event reduction functions
  //@{

  /// Remove an unwanted particle from the event, collapsing the graph structure consistently
static  inline void reduce(HepMC::GenEvent* ge, HepMC::GenParticle* gp) {
    // Do nothing if for some reason this particle is not actually in this event
    if (gp->parent_event() != ge) return;

    // Get start and end vertices
    HepMC::GenVertex* vstart = gp->production_vertex();
    HepMC::GenVertex* vend = gp->end_vertex();

    // Disconnect the unwanted particle from its vertices and delete it
    if (vstart != nullptr) vstart->remove_particle(gp);
    if (vend != nullptr) vend->remove_particle(gp);
    delete gp;

    // If start/end vertices are valid and distinct, and this was the only particle that
    // connected them, then reassign the end vertex decay products to the start vertex
    // and rewrite the vertex position as most appropriate.
    /// @note The disconnected end vertex will be picked up by the final "sweeper" loop if necessary.
    /// @note We do the reassigning this way since GV::add_particle_*() modifies the end vertex
    if (vstart != nullptr && vend != nullptr && vend != vstart) {
      bool is_only_link = true;
      for (auto pchild=vstart->particles_out_const_begin();pchild!=vstart->particles_out_const_end();++pchild) {
        if ((*pchild)->end_vertex() == vend) is_only_link = false;
      }
      if (is_only_link) {
        if (vend->position() != HepMC::FourVector())
          vstart->set_position(vend->position()); //< @todo Always use end position if defined... ok?
        while (vend->particles_out_size() > 0) {
          vstart->add_particle_out(*vend->particles_out_const_begin());
        }
        while (vend->particles_in_size() > 0) {
          vstart->add_particle_in(*vend->particles_in_const_begin());
        }
      }
    }

    // Sweep up any vertices orphaned by the particle removal
    /// @todo Can we be a bit more efficient rather than having to run over all vertices every time?
    ///       Or allow disabling of this clean-up, with a single clean being run at the end of filtering.
    /// @todo Use neater looping via vertices_match (or iterated vertex_match)
    /// @todo Also look for and report changes in number of no-parent and no-child vertices
    std::vector<HepMC::GenVertex*> orphaned_vtxs;
    for (HepMC::GenEvent::vertex_const_iterator vi = ge->vertices_begin(); vi != ge->vertices_end(); ++vi) {
      if ((*vi)->particles_in_size() == 0 && (*vi)->particles_out_size() == 0) orphaned_vtxs.push_back(*vi);
    }
    for (HepMC::GenVertex* gv : orphaned_vtxs) delete gv;
  }

  /// Remove unwanted particles from the event, collapsing the graph structure consistently
  inline void reduce(HepMC::GenEvent* ge, std::vector<HepMC::GenParticlePtr> toremove) {
    while (toremove.size()) {
      auto gp = toremove.back();
      toremove.pop_back();
      reduce(ge, gp);
    }
  }
//<----//This is copied from MCUtils
#endif

StatusCode FixHepMC::execute() {
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
    if (!m_pidmap.empty()) {
      for (auto ip: *evt) {
        // Skip this particle if (somehow) its pointer is null
         if (!ip) continue;
         auto newpid = m_pidmap.find(ip->pdg_id());
         if (newpid == m_pidmap.end()) continue;
         ip->set_pdg_id(newpid->second);
      }
    }
#ifdef HEPMC3
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
        decay_loop_particles.push_back(ip);
      }
    }

    /// AV: In case we have 3 particles, we try to add a vertex 
    /// that corresponds to 1->2 and 1->1 splitting.
    /// AV: In case we have 4 particles, we can try to do that as well.
    if ( semi_disconnected.size() == 4 || semi_disconnected.size() == 3 || semi_disconnected.size() == 2) {
      size_t no_endv = 0;
      size_t no_prov = 0;
      HepMC::FourVector sum(0,0,0,0);
      for (const auto& part : semi_disconnected) {
        if (!part->production_vertex() || !part->production_vertex()->id()) {
          no_prov++; sum += part->momentum();
        }
        if (!part->end_vertex()) { 
          no_endv++;  sum -= part->momentum();
        }
      }
      ATH_MSG_INFO("Heuristics: found " << semi_disconnected.size() << " semi-disconnected particles. Momentum sum is " << sum);
      /// The condition below will cover 1->1, 1->2 and 2->1 cases
      if (no_endv && no_prov  && ( no_endv + no_prov  == semi_disconnected.size() )) {
        if (std::abs(sum.px()) < 1e-2  && std::abs(sum.py()) < 1e-2  && std::abs(sum.pz()) < 1e-2 ) {
          ATH_MSG_INFO("Try " << no_endv << "->" << no_prov << " splitting/merging.");
          auto v = HepMC::newGenVertexPtr();
          for (auto part : semi_disconnected) {
            if (!part->production_vertex() || part->production_vertex()->id() == 0) v->add_particle_out(part);
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
      for (auto p : daughters) vprod->add_particle_out(p);
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
      if (bad_particle) toremove.push_back(ip);
    }

    // Properties before cleaning
    const int num_particles_orig = evt->particles().size();

    // Do the cleaning
    if (!toremove.empty()) {
      ATH_MSG_DEBUG("Cleaning event record of " << toremove.size() << " bad particles");
      for (auto part: toremove) evt->remove_particle(part);
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
 #else

    // Add a unit entry to the event weight vector if it's currently empty
    if (evt->weights().empty()) {
      ATH_MSG_DEBUG("Adding a unit weight to empty event weight vector");
      evt->weights().push_back(1);
    }

    // Set a (0,0,0) vertex to be the signal vertex if not already set
    if (evt->signal_process_vertex() == NULL) {
      const HepMC::FourVector nullpos;
      for (HepMC::GenEvent::vertex_const_iterator iv = evt->vertices_begin(); iv != evt->vertices_end(); ++iv) {
        if ((*iv)->position() == nullpos) {
          ATH_MSG_DEBUG("Setting representative event position vertex");
          evt->set_signal_process_vertex(const_cast<HepMC::GenVertex*>(*iv));
          break;
        }
      }
    }


    // Some heuristics to catch problematic cases
    std::vector<HepMC::GenParticlePtr> semi_disconnected, decay_loop_particles;
    for (auto ip : *evt) {
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
        decay_loop_particles.push_back(ip);
      }
    }

    /// AV: In case we have 3 particles, we try to add a vertex that correspond to 1->2 and 1->1 splitting.
    /// AV: we can try to do that for 4 particles as well.
    if (semi_disconnected.size() == 4 ||semi_disconnected.size() == 3 || semi_disconnected.size() == 2) {
      size_t no_endv = 0;
      size_t no_prov = 0;
      double vsum[4] = {0,0,0,0};
      for (auto part : semi_disconnected) {
        if (!part->production_vertex() ) { 
          no_prov++; 
          vsum[0] += part->momentum().px(); 
          vsum[1] += part->momentum().py(); 
          vsum[2] += part->momentum().pz(); 
          vsum[3] += part->momentum().e();
        }
        if (!part->end_vertex()) { 
          no_endv++;  
          vsum[0] -= part->momentum().px(); 
          vsum[1] -= part->momentum().py(); 
          vsum[2] -= part->momentum().pz(); 
          vsum[3] -= part->momentum().e();
        }
      }
      HepMC::FourVector sum(vsum[0],vsum[1],vsum[2],vsum[3]);
      ATH_MSG_INFO("Heuristics: found " << semi_disconnected.size() << " semi-disconnected particles. Momentum sum is " << vsum[0] << " " << vsum[1] << " " << vsum[2] << " " << vsum[3]);
      /// The condition below will cover 1->1, 1->2 and 2->1 cases
      if (no_endv && no_prov  && ( no_endv + no_prov  == semi_disconnected.size() )) {
        if (std::abs(sum.px()) < 1e-2  && std::abs(sum.py()) < 1e-2  && std::abs(sum.pz()) < 1e-2 ) {
          ATH_MSG_INFO("Try " << no_endv << "->" << no_prov << " splitting/merging.");
          auto v = HepMC::newGenVertexPtr();
          for (auto part : semi_disconnected) {
            if (!part->production_vertex())  v->add_particle_out(part);
          }
          for (auto part : semi_disconnected) {
            if (!part->end_vertex()) v->add_particle_in(part);
          }
          evt->add_vertex(v);
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
      if (!vend || !vprod) continue;
      bool loop_in_decay = true;
      /// Check that all particles coming into the decay vertex of bad particle cam from the same production vertex.
      std::vector<HepMC::GenParticlePtr> sisters;
      for (auto p = vend->particles_begin(HepMC::parents); p!= vend->particles_end(HepMC::parents); ++p) sisters.push_back(*p);
      for (auto sister : sisters) {
        if (vprod != sister->production_vertex()) loop_in_decay = false;
      }
      if (!loop_in_decay) continue;

      std::vector<HepMC::GenParticlePtr> daughters;
      for (auto p = vend->particles_begin(HepMC::children); p!= vend->particles_end(HepMC::children); ++p) daughters.push_back(*p);
      for (auto p : daughters) vprod->add_particle_out(p);
      for (auto sister : sisters) { 
        vprod->remove_particle(sister); 
        vend->remove_particle(sister);
      }
      evt->remove_vertex(vend);

    }


    // Event particle content cleaning -- remove "bad" structures
    std::vector<HepMC::GenParticlePtr> toremove; toremove.reserve(10);
    for (HepMC::GenEvent::particle_const_iterator ip = evt->particles_begin(); ip != evt->particles_end(); ++ip) {
      // Skip this particle if (somehow) its pointer is null
      if (*ip == NULL) continue;
      m_totalSeen += 1;

      // Flag to declare if a particle should be removed
      bool bad_particle = false;

      // Check for loops
      if ( m_killLoops && isSimpleLoop(*ip) ) {
        bad_particle = true;
        m_loopKilled += 1;
        ATH_MSG_DEBUG( "Found a looper : " );
        if ( msgLvl( MSG::DEBUG ) ) (*ip)->print();
      }

      // Check on PDG ID 0
      if ( m_killPDG0 && isPID0(*ip) ) {
        bad_particle = true;
        m_pdg0Killed += 1;
        ATH_MSG_DEBUG( "Found PDG ID 0 : " );
        if ( msgLvl( MSG::DEBUG ) ) (*ip)->print();
      }

      // Clean decays
      int abs_pdg_id = std::abs((*ip)->pdg_id());
      bool is_decayed_weak_boson =  ( abs_pdg_id == 23 || abs_pdg_id == 24 || abs_pdg_id == 25 ) && (*ip)->end_vertex();
      if ( m_cleanDecays && isNonTransportableInDecayChain(*ip) && !is_decayed_weak_boson ) {
        bad_particle = true;
        m_decayCleaned += 1;
        ATH_MSG_DEBUG( "Found a bad particle in a decay chain : " );
        if ( msgLvl( MSG::DEBUG ) ) (*ip)->print();
      }

      // Only add to the toremove vector once, even if multiple tests match
      if (bad_particle) toremove.push_back(*ip);
    }

    // Properties before cleaning
    const int num_particles_orig = evt->particles_size();
    int num_orphan_vtxs_orig = 0;
    int num_noparent_vtxs_orig = 0;
    int num_nochild_vtxs_orig = 0;
    for (auto v = evt->vertices_begin(); v != evt->vertices_end(); ++v) {
      if ((*v)->particles_in_size()==0&&(*v)->particles_out_size()==0) num_orphan_vtxs_orig++;
      if ((*v)->particles_in_size()==0) num_noparent_vtxs_orig++;
      if ((*v)->particles_out_size()==0) num_nochild_vtxs_orig++;
    }

    // Do the cleaning
    if (!toremove.empty()) {
      ATH_MSG_DEBUG("Cleaning event record of " << toremove.size() << " bad particles");
      // Clean!
      int signal_vertex_bc = evt->signal_process_vertex() ? evt->signal_process_vertex()->barcode() : 0;
      //This is the only place where reduce is used.
      reduce(evt , toremove);
      if (evt->barcode_to_vertex (signal_vertex_bc) == nullptr) {
        evt->set_signal_process_vertex (nullptr);
      }
    }

    if(m_purgeUnstableWithoutEndVtx) {
      int purged=0;
      do {
        for (HepMC::GenParticle* p : *evt) {
          HepMC::ConstGenVertexPtr end_v = p->end_vertex();
          if (p->status() == 2 && !end_v) {
            delete p->production_vertex()->remove_particle(p);
            ++purged;
            ++m_unstablePurged;
          }
        }
      }
      while (purged>0);
    }

    // Properties after cleaning
    const int num_particles_filt = evt->particles_size();
    if(num_particles_orig!=num_particles_filt) {
      int num_orphan_vtxs_filt = 0;
      int num_noparent_vtxs_filt = 0;
      int num_nochild_vtxs_filt = 0;
      for (auto v = evt->vertices_begin(); v != evt->vertices_end(); ++v) {
        if ((*v)->particles_in_size()==0&&(*v)->particles_out_size()==0) num_orphan_vtxs_filt++;
        if ((*v)->particles_in_size()==0) num_noparent_vtxs_filt++;
        if ((*v)->particles_out_size()==0) num_nochild_vtxs_filt++;
      }

      // Write out the change in the number of particles
      ATH_MSG_INFO("Particles filtered: " << num_particles_orig << " -> " << num_particles_filt);
      // Warn if the numbers of "strange" vertices have changed
      if (num_orphan_vtxs_filt != num_orphan_vtxs_orig)
        ATH_MSG_WARNING("Change in orphaned vertices: " << num_orphan_vtxs_orig << " -> " << num_orphan_vtxs_filt);
      if (num_noparent_vtxs_filt != num_noparent_vtxs_orig)
        ATH_MSG_WARNING("Change in no-parent vertices: " << num_noparent_vtxs_orig << " -> " << num_noparent_vtxs_filt);
      if (num_nochild_vtxs_filt != num_nochild_vtxs_orig)
        ATH_MSG_WARNING("Change in no-parent vertices: " << num_nochild_vtxs_orig << " -> " << num_nochild_vtxs_filt);
    }
#endif
  }
  return StatusCode::SUCCESS;
}


StatusCode FixHepMC::finalize() {
  if (m_killLoops  ) ATH_MSG_INFO( "Removed " <<   m_loopKilled << " of " << m_totalSeen << " particles because of loops." );
  if (m_killPDG0   ) ATH_MSG_INFO( "Removed " <<   m_pdg0Killed << " of " << m_totalSeen << " particles because of PDG ID 0." );
  if (m_cleanDecays) ATH_MSG_INFO( "Removed " << m_decayCleaned << " of " << m_totalSeen << " particles while cleaning decay chains." );
  if(m_purgeUnstableWithoutEndVtx) ATH_MSG_INFO( "Removed " << m_unstablePurged << " of " << m_totalSeen << " unstable particles because they had no decay vertex." );
  if (!m_pidmap.empty()) ATH_MSG_INFO( "Replaced " << m_replacedPIDs << "PIDs of particles." );
  return StatusCode::SUCCESS;
}


/// @name Classifiers for identifying particles to be removed
//@{

// Identify PDG ID = 0 particles, usually from HEPEVT padding
bool FixHepMC::isPID0(const HepMC::ConstGenParticlePtr& p) const {
  return p->pdg_id() == 0;
}

// Identify the particles from
bool FixHepMC::fromDecay(const HepMC::ConstGenParticlePtr& p, std::shared_ptr<std::set<int> >& storage) const {
      if (!p) return false;
      auto v=p->production_vertex();
      if (!v) return false;
#ifdef HEPMC3
      for ( const auto& anc: v->particles_in()) {
        if (MC::isDecayed(anc) && (MC::isTau(anc->pdg_id()) || MC::isHadron(anc->pdg_id()))) return true;
      }
      if (storage->find(p->id()) != storage->end()) return false;
      storage->insert(p->id());
      for ( const auto& anc: v->particles_in()) {
            if (fromDecay(anc, storage)) return true;
      }
#else
      for (auto  anc=v->particles_in_const_begin(); anc != v->particles_in_const_end(); ++anc) {
        if (MC::isDecayed((*anc)) && (MC::isTau((*anc)->pdg_id()) || MC::isHadron((*anc)->pdg_id()))) return true;
      }
      if (storage->find(p->barcode()) != storage->end()) return false;
      storage->insert(p->barcode());
      for (auto  anc=v->particles_in_const_begin(); anc != v->particles_in_const_end(); ++anc){
        if (fromDecay(*anc, storage)) return true;
      }
#endif
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

