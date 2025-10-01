/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
/* Author: Andrii Verbytskyi andrii.verbytskyi@mpp.mpg.de */

#ifndef TRUTHUTILS_MAGICNUMBERS_H
#define TRUTHUTILS_MAGICNUMBERS_H

#include <limits>
#include <cstdint>
#include <memory>
#include <deque>
#include <type_traits>
#if !defined(XAOD_STANDALONE)
#include "AtlasHepMC/GenEvent.h"
#include "AtlasHepMC/GenParticle.h"
#include "AtlasHepMC/GenVertex.h"
#endif
namespace xAOD {
  // Temporarily specialize for xAOD::Truth classes ahead of the barcode migration - TODO remove this
  class TruthParticle_v1;
  class TruthVertex_v1;
}
class TrackRecord;
class HepMcParticleLink;
class CaloCalibrationHit;

enum EBC_SUPPRESSED_TRUTH : unsigned char {
  EBC_UNSUPPRESSED = 0, // Truth particle expected to be found in McEventCollection
  EBC_PU_SUPPRESSED, // Link points to a suppressed pile-up truth particle do not attempt to resolve it.
  EBC_NSUPP
};

namespace HepMC {

  /// @brief Constant defining the barcode threshold for simulated particles, eg. can be used to separate generator event record entries from simulated particles
  constexpr int SIM_BARCODE_THRESHOLD = 200000;

  /// @brief Constant defining the barcode threshold for regenerated particles, i.e. particles surviving an interaction
  constexpr int SIM_REGENERATION_INCREMENT = 1000000;

  /// @brief Constant defining the barcode threshold for regenerated particles, i.e. particles surviving an interaction
  constexpr int SIM_STATUS_INCREMENT = 100000;

  /// @brief Constant definiting the status threshold for simulated particles, eg. can be used to separate generator event record entries from simulated particles
  constexpr int SIM_STATUS_THRESHOLD = 20000;

  /// @brief Special Forward transport Geant process for vertices
  constexpr int FORWARD_TRANSPORT_MODEL_PROCESS = 212;

  /// @brief This barcode is used by objects matched to particles from pile-up interactions in standard MC Production
  constexpr int SUPPRESSED_PILEUP_BARCODE(std::numeric_limits<int32_t>::max());

  constexpr int INVALID_PARTICLE_BARCODE = -1;

  constexpr int UNDEFINED_ID = 0;
  constexpr int INVALID_PARTICLE_ID = -1;
  constexpr int INVALID_VERTEX_ID = 1;

  // TODO The definitions of is_smart_ptr and remove_smart_pointer
  // below are probably too generic for this header, but putting them
  // here initially.
  template <typename T> struct is_smart_ptr : std::false_type {};
  template <typename T> struct is_smart_ptr<std::shared_ptr<T>> : std::true_type {};
  template <typename T> struct is_smart_ptr<std::unique_ptr<T>> : std::true_type {};
  template <typename T> struct is_smart_ptr<std::weak_ptr<T>> : std::true_type {};
  template <class T> inline constexpr bool is_smart_ptr_v = is_smart_ptr<T>::value;

  template <class T> struct remove_smart_pointer { typedef T type; };
  template <class T> struct remove_smart_pointer<std::shared_ptr<T>> { typedef T type; };
  template <class T> struct remove_smart_pointer<std::unique_ptr<T>> { typedef T type; };
  template <class T> struct remove_smart_pointer<std::weak_ptr<T>> { typedef T type; };
  template <class T> using remove_smart_pointer_t = typename remove_smart_pointer<T>::type;

#if defined(XAOD_STANDALONE)
  // Needed as we can't pick up the helper functions from AtlasHepMC in this case
  template <class T>  inline int barcode(const T& p){
    if constexpr (std::is_pointer_v<T> || is_smart_ptr_v<T>){ //T is ptr
      return p->barcode();
    }
    else {
      return p.barcode();
    }
  }
  template <>  inline int barcode(const int& p){ return p;}
#endif
  // Temporarily specialize uniqueID for xAOD::Truth classes ahead of the barcode migration - TODO remove this
#if defined(HEPMC3)
  template <typename T> inline int uniqueID(const T&  p) {
    if constexpr (std::is_integral_v<T>) {
      return p;
    }
    else if constexpr (std::is_integral_v<std::remove_pointer_t<T>>) {
      return *p;
    }
    else if constexpr (std::is_same_v<T, xAOD::TruthParticle_v1> || std::is_same_v<T, xAOD::TruthVertex_v1>) {
      return p.uid();
    }
    else if constexpr (std::is_same_v<std::remove_const_t<remove_smart_pointer_t<std::remove_pointer_t<T>>>, xAOD::TruthParticle_v1> || std::is_same_v<std::remove_const_t<remove_smart_pointer_t<std::remove_pointer_t<T>>>, xAOD::TruthVertex_v1>) {
      return p->uid();
    }
    else if constexpr (std::is_same_v<T, CaloCalibrationHit>) {
      return p.particleUID();
    }
    else if constexpr (std::is_same_v<std::remove_const_t<remove_smart_pointer_t<std::remove_pointer_t<T>>>, CaloCalibrationHit>) {
      return p->particleUID();
    }
    else if constexpr (std::is_pointer_v<T> || is_smart_ptr_v<T>){ //T is ptr
      return p->id();
    }
    else {
      return p.id();
    }
  }
#else
  template <typename T> inline int uniqueID(const T&  p) {
    if constexpr (std::is_integral_v<T>) {
      return p;
    }
    else if constexpr (std::is_integral_v<std::remove_pointer_t<T>>) {
      return *p;
    }
    else if constexpr (std::is_same_v<T, xAOD::TruthParticle_v1> || std::is_same_v<T, xAOD::TruthVertex_v1>) {
      return p.uid();
    }
    else if constexpr (std::is_same_v<std::remove_const_t<remove_smart_pointer_t<std::remove_pointer_t<T>>>, xAOD::TruthParticle_v1> || std::is_same_v<std::remove_const_t<remove_smart_pointer_t<std::remove_pointer_t<T>>>, xAOD::TruthVertex_v1>) {
      return p->uid();
    }
    else if constexpr (std::is_same_v<T, CaloCalibrationHit>) {
      return p.particleUID();
    }
    else if constexpr (std::is_same_v<std::remove_const_t<remove_smart_pointer_t<std::remove_pointer_t<T>>>, CaloCalibrationHit>) {
      return p->particleUID();
    }
    else if constexpr (std::is_pointer_v<T> || is_smart_ptr_v<T>){ //T is ptr
      return p->barcode();
    }
    else {
      return p.barcode();
    }
  }
#endif
  template <typename T> inline int status(const T&  p) {
    if constexpr (std::is_integral_v<T>) {
      return p;
    }
    else if constexpr (std::is_integral_v<std::remove_pointer_t<T>>) {
      return *p;
    }
    else if constexpr (std::is_pointer_v<T> || is_smart_ptr_v<T>){ //T is ptr
      return p->status();
    }
    else {
      return p.status();
    }
  }
#if !defined(HEPMC3) && !defined(XAOD_STANDALONE)
  template <>  inline int status(const ConstGenVertexPtr& v1){ return v1->id();}
  template <>  inline int status(const GenVertexPtr& v1){ return v1->id();}
#endif

  /// @brief Function to calculate all the descendants(direction=1)/ancestors(direction=-1) of the particle.
  template <class T> inline void get_particle_history(const T& p, std::deque<int>& out, const int direction = 0) {
    if (direction < 0) {
      if (p->status()>SIM_STATUS_INCREMENT) {
        auto pv = p->production_vertex();
        if (pv) {
          for (auto pa: pv->particles_in()) {
            if (!pa || pa->pdg_id() != p->pdg_id()) continue;
            out.push_front(uniqueID(p));
            get_particle_history(pa, out, -1);
            break;
          }
        }
      }
    }
    if (direction > 0) {
      if (p->status()>SIM_STATUS_INCREMENT) {
        auto pv = p->end_vertex();
        if (pv) {
          for (auto pa: pv->particles_out()) {
            if (!pa || pa->pdg_id() != p->pdg_id()) continue;
            out.push_back(uniqueID(p));
            get_particle_history(pa, out, 1);
            break;
          }
        }
      }
    }
  }
  /// @brief Function to calculate all the descendants(direction=1)/ancestors(direction=-1) of the particle.
  template <class T>  inline std::deque<int> simulation_history(const T& p, const int direction ) { std::deque<int> res; res.push_back(uniqueID(p)); get_particle_history(p, res, direction); return res;}

  namespace BarcodeBased {
    /// @brief Method to establish if a particle (or barcode) corresponds to truth-suppressed pile-up
    template <class T>  inline bool is_truth_suppressed_pileup(const T& p){ return (barcode(p) == SUPPRESSED_PILEUP_BARCODE);}

    /// @brief Method to establish if a if the object is linked to something which was never saved to the HepMC Truth - for example particle was too low energy to be recorded
    template <class T>  inline bool no_truth_link(const T& p){ return (barcode(p) == UNDEFINED_ID);}

    /// @brief Helper function for SDO creation in PileUpTools
    template <class T>  inline bool ignoreTruthLink(const T& p, bool vetoPileUp){ const int b = barcode(p);  return no_truth_link(b) || (vetoPileUp && is_truth_suppressed_pileup(b)); }

    /// @brief Method to establish if a particle (or barcode) was created during the simulation (only to be used in legacy TP converters)
    template <class T>  inline bool is_simulation_particle(const T& p){ return (barcode(p)>SIM_BARCODE_THRESHOLD);}

    /// @brief Method to establish if a particle (or barcode) is a new seondary created during the simulation (only to be used in legacy TP converters)
    template <class T>  inline bool is_sim_secondary(const T& p){ return (barcode(p)%SIM_REGENERATION_INCREMENT > SIM_BARCODE_THRESHOLD); }

    /// @brief Method to return how many interactions a particle has undergone during simulation (only to be used in legacy TP converters).
    template <class T>  inline int generations(const T& p){ return (barcode(p)/SIM_REGENERATION_INCREMENT);}

    /// @brief Method to establish if the vertex was created during simulation (only to be used in legacy TP converters)
    template <class T>  inline bool is_simulation_vertex(const T& v){ return (barcode(v)<-SIM_BARCODE_THRESHOLD);}

    /// @brief Method to establish if two particles in the GenEvent actually represent the same generated particle
    template <class T1, class T2> inline bool is_same_generator_particle(const T1& p1,const T2& p2) { int b1 = barcode(p1); int b2 = barcode(p2); return  b1% SIM_REGENERATION_INCREMENT == b2 % SIM_REGENERATION_INCREMENT; }

    /// @brief Method to establish if two particles/vertices in the GenEvent actually represent the same generated particle
    template <class T1, class T2> inline bool is_same_object(const T1& p1,const T2& p2) { int b1 = uniqueID(p1); int b2 = uniqueID(p2); return  b1 == b2; } // NB Still need to adapt CaloCalibrationHits

    /// @brief Method to check if the first particle is a descendant of the second in the simulation, i.e. particle p1 was produced simulations particle p2.
    template <class T1, class T2> inline bool is_sim_descendant(const T1& p1,const T2& p2) { int b1 = barcode(p1); int b2 = barcode(p2); return b1 % SIM_REGENERATION_INCREMENT == b2;}
  }

  namespace StatusBased {
    /// @brief Method to establish if a particle corresponds to truth-suppressed pile-up
    template <class T>  inline bool is_truth_suppressed_pileup(const T& p){
      if constexpr (std::is_same_v<std::remove_const_t<T>, HepMcParticleLink>) {
        return p.getTruthSuppressionType() == EBC_PU_SUPPRESSED;
      }
      else if constexpr (std::is_same_v<std::remove_const_t<remove_smart_pointer_t<std::remove_pointer_t<T>>>, HepMcParticleLink>) {
        return p->getTruthSuppressionType() == EBC_PU_SUPPRESSED;
      }
      else {
        return p == EBC_PU_SUPPRESSED;
      }
    }

    /// @brief Method to establish if a if the object is linked to something which was never saved to the HepMC Truth - for example particle was too low energy to be recorded
    template <class T>  inline bool no_truth_link(const T& p){
      if constexpr (std::is_same_v<std::remove_const_t<T>, HepMcParticleLink>) {
        return p.linkIsNull();
      }
      else if constexpr (std::is_same_v<std::remove_const_t<remove_smart_pointer_t<std::remove_pointer_t<T>>>, HepMcParticleLink>) {
        return p->linkIsNull();
      }
      else {
        return (uniqueID(p) == UNDEFINED_ID);
      }
    }

    /// @brief Helper function for SDO creation in PileUpTools
    template <class T>  inline bool ignoreTruthLink(const T& p, bool vetoPileUp){
      if constexpr (std::is_same_v<std::remove_const_t<remove_smart_pointer_t<std::remove_pointer_t<T>>>, HepMcParticleLink>) {
        return no_truth_link(p) || (vetoPileUp && is_truth_suppressed_pileup(p));
      }
      else {
        const int u = uniqueID(p);  return no_truth_link(u) || (vetoPileUp && is_truth_suppressed_pileup(u));
      }
    }

    /// @brief Method to establish if a particle was created during the simulation based on the status value
    template <class T>  inline bool is_simulation_particle(const T& p){ return (status(p)>SIM_STATUS_THRESHOLD);}

    /// @brief Method to establish if a particle is a new seondary created during the simulation based on the status value
    template <class T>  inline bool is_sim_secondary(const T& p){ return (status(p)%SIM_STATUS_INCREMENT > SIM_STATUS_THRESHOLD); }

    /// @brief Method to return how many interactions a particle has undergone during simulation based on the status value
    template <class T>  inline int generations(const T& p){ return (status(p)/SIM_STATUS_INCREMENT);}

    /// @brief Method to establish if the vertex was created during simulation from the status
    template <class T>  inline bool is_simulation_vertex(const T& v){ return (status(v)>SIM_STATUS_THRESHOLD);}

    /// @brief Method to establish if two particles in the GenEvent actually represent the same generated particle
    template <class T1, class T2> inline bool is_same_generator_particle(const T1& p1,const T2& p2) {
      const int id1 = uniqueID(p1);
      const int id2 = uniqueID(p2);
      if (id1 == id2) { return true;} // simplest case
      const int generations1 = generations(p1);
      const int generations2 = generations(p2);
      if (generations1 == generations2) { return false; } // if the id values don't match and the particles have the same generation number then they cannot be the same particle.
      if constexpr (std::is_same_v<std::remove_const_t<remove_smart_pointer_t<std::remove_pointer_t<T1>>>, TrackRecord>) {
        // No choice, but to get the history of one of the particles:
        const int direction = (generations2 > generations1) ? -1 : 1;
        std::deque<int> history2 = simulation_history( p2, direction );
        if (std::find(history2.begin(),history2.end(), id1) == history2.end()) { return false; }
      }
      else {
        // No choice, but to get the history of one of the particles:
        const int direction = (generations1 > generations2) ? -1 : 1;
        std::deque<int> history1 = simulation_history( p1, direction );
        if (std::find(history1.begin(),history1.end(), id2) == history1.end()) { return false; }
      }
      return true;
    }

    /// @brief Method to establish if two particles/vertices in the GenEvent actually represent the same particle
    template <class T1, class T2> inline bool is_same_object(const T1& p1,const T2& p2) {
      const int id1 = uniqueID(p1);
      const int id2 = uniqueID(p2);
      return (id1 == id2);
    }


    /// @brief Method to check if the first particle is a descendant of the second in the simulation, i.e. particle p1 was produced simulations particle p2.
    template <class T1, class T2> inline bool is_sim_descendant(const T1& p1,const T2& p2) {
      const int id1 = uniqueID(p1);
      const int id2 = uniqueID(p2);
      if (id1 == id2) { return true;} // simplest case
      const int generations1 = generations(p1);
      const int generations2 = generations(p2);
      if (generations1 == generations2) { return false; } // if the id values don't match and the particles have the same generation number then they cannot be the same particle.
      if constexpr (std::is_same_v<std::remove_const_t<remove_smart_pointer_t<std::remove_pointer_t<T1>>>, TrackRecord>) {
        // No choice, but to get the descendents of p2
        constexpr int descendents = 1;
        std::deque<int> history2 = simulation_history( p2, descendents );
        if (std::find(history2.begin(),history2.end(), id1) == history2.end()) { return false; }
       }
      else {
        // No choice, but to get the history of p1
        constexpr int ancestors = -1;
        std::deque<int> history1 = simulation_history( p1, ancestors );
        if (std::find(history1.begin(),history1.end(), id2) == history1.end()) { return false; }
      }
      return true;
    }
  }

  /// @brief Method to establish if a particle (or barcode) corresponds to truth-suppressed pile-up (TODO update to be status based)
  template <class T>  inline bool is_truth_suppressed_pileup(const T& p){
    if constexpr (std::is_same_v<std::remove_const_t<remove_smart_pointer_t<std::remove_pointer_t<T>>>, HepMcParticleLink>) {
      return StatusBased::is_truth_suppressed_pileup(p);
    }
    else {
      return BarcodeBased::is_truth_suppressed_pileup(p);
    }
  }

  /// @brief Method to establish if a if the object is linked to something which was never saved to the HepMC Truth - for example particle was too low energy to be recorded
  template <class T>  inline bool no_truth_link(const T& p){ return StatusBased::no_truth_link(p);}

  /// @brief Helper function for SDO creation in PileUpTools
  template <class T>  inline bool ignoreTruthLink(const T& p, bool vetoPileUp){
    if constexpr (std::is_same_v<std::remove_const_t<remove_smart_pointer_t<std::remove_pointer_t<T>>>, HepMcParticleLink>) {
      return StatusBased::ignoreTruthLink(p, vetoPileUp);
    }
    else {
      return BarcodeBased::ignoreTruthLink(p, vetoPileUp);
    }
  }

  /// @brief Method to establish if a particle (or barcode) was created during the simulation (TODO update to be status based)
  template <class T>  inline bool is_simulation_particle(const T& p){ return StatusBased::is_simulation_particle(p);}

  /// @brief Method to return how many interactions a particle has undergone during simulation (TODO migrate to be based on status).
  template <class T>  inline int generations(const T& p){ return StatusBased::generations(p);}

  /// @brief Method to establish if the vertex was created during simulation (TODO migrate to be based on status).
  template <class T>  inline bool is_simulation_vertex(const T& v){ return StatusBased::is_simulation_vertex(v);}

  /// @brief Method to establish if two particles in the GenEvent actually represent the same generated particle
  template <class T1,class T2> inline bool is_same_generator_particle(const T1& p1,const T2& p2) { return StatusBased::is_same_generator_particle(p1, p2); }

  /// @brief Method to establish if two particles in the GenEvent actually represent the same  particle
  template <class T1,class T2>  inline bool is_same_particle(const T1& p1,const T2& p2) { return StatusBased::is_same_object(p1, p2); }

  /// @brief Method to establish if two particles in the GenEvent actually represent the same  vertex
  template <class T1,class T2>  inline bool is_same_vertex(const T1& p1,const T2& p2) { return StatusBased::is_same_object(p1, p2); }

  /// @brief Method to check if the first particle is a descendant of the second in the simulation, i.e. particle p1 was produced simulations particle p2.
  template <class T1,class T2> inline bool is_sim_descendant(const T1& p1,const T2& p2) { return StatusBased::is_sim_descendant(p1, p2);}

  /// @brief Function that converts the old scheme of labeling the simulation particles (barcodes) into the new scheme (statuses).
  template <class T> void old_to_new_simulation_scheme(T& evt) {
    auto particle_status = [] (int barcode, int status) {
      if ((barcode % SIM_REGENERATION_INCREMENT) > SIM_BARCODE_THRESHOLD)
        status += SIM_STATUS_THRESHOLD;
      status += SIM_STATUS_INCREMENT * (barcode / SIM_REGENERATION_INCREMENT);
      return status;
    };
    auto vertex_status = [] (int barcode, int status) {
      if (-barcode > SIM_BARCODE_THRESHOLD) status += SIM_STATUS_THRESHOLD;
      return status;
    };
#ifdef HEPMC3
    for (auto p: evt->particles())  {
      p->set_status (particle_status (HepMC::barcode(p), p->status()));
    }
    for (auto v: evt->vertices()) {
      v->set_status (vertex_status (HepMC::barcode(v), v->status()));
    }
#else
    for (auto p = evt->particles_begin(); p != evt->particles_end(); ++p) {
      (*p)->set_status (particle_status ((*p)->barcode(), (*p)->status()));
    }
    for (auto v = evt->vertices_begin(); v != evt->vertices_end(); ++v)  {
      (*v)->set_id (vertex_status ((*v)->barcode(), (*v)->id()));
    }
#endif
  }

  /// @brief Get particle status in the new scheme from the barcode and status in the old scheme
  inline int new_particle_status_from_old(const int oldStatus,const  int barcode) {
    int generations_barcode_based = (barcode/SIM_REGENERATION_INCREMENT);
    bool is_sim_secondary_barcode_based = (barcode%SIM_REGENERATION_INCREMENT > SIM_BARCODE_THRESHOLD);
    return oldStatus + SIM_STATUS_INCREMENT*generations_barcode_based + (is_sim_secondary_barcode_based? SIM_STATUS_THRESHOLD : 0); 
  }
  
  /// @brief Get particle status in the old scheme from the status in the new scheme
  inline int old_particle_status_from_new(const int newStatus) { return newStatus%SIM_STATUS_THRESHOLD; }

  /// @brief Get vertex status in the new scheme from the barcode and status in the old scheme
  inline int new_vertex_status_from_old(const int oldStatus,const  int barcode) {
    bool is_simulation_vertex_barcode_based =  (barcode<-SIM_BARCODE_THRESHOLD);
    return (is_simulation_vertex_barcode_based? SIM_STATUS_THRESHOLD : 0) + oldStatus;
  }

  /// @brief Get vertex status in the old scheme from the status in the new scheme
  inline int old_vertex_status_from_new(const int newStatus) {
    bool is_simulation_vertex_status_based = (newStatus>SIM_STATUS_THRESHOLD);
    return ( is_simulation_vertex_status_based ? -SIM_STATUS_THRESHOLD : 0) + newStatus; }
}
#if !defined(XAOD_STANDALONE)
namespace HepMC {
/// @brief Get the maximal value of barcode of particle present in the event
inline int  maxGeneratedParticleBarcode(const HepMC::GenEvent *genEvent) {
  int maxBarcode = 0;
#ifdef HEPMC3
  auto allbarcodes = genEvent->attribute<HepMC::GenEventBarcodes>("barcodes");
  for (const auto& bp: allbarcodes->barcode_to_particle_map()) {
    if (!HepMC::BarcodeBased::is_simulation_particle(bp.first)) { maxBarcode=std::max(maxBarcode,bp.first); }
  }
#else
  for (auto currentGenParticle: *genEvent) {
    const int barcode=HepMC::barcode(currentGenParticle);
    if (barcode > maxBarcode &&  !HepMC::BarcodeBased::is_simulation_particle(barcode)) { maxBarcode=barcode; }
  }
#endif
  return maxBarcode;
}

/// @brief Get the maximal absolute value of barcode of vertex present in the event. Returns a negative number.
inline int maxGeneratedVertexBarcode(const HepMC::GenEvent *genEvent) {
  int maxBarcode=0;
#ifdef HEPMC3
  auto allbarcodes = genEvent->attribute<HepMC::GenEventBarcodes>("barcodes");
  for (const auto& bp: allbarcodes->barcode_to_vertex_map()) {
    if (!HepMC::BarcodeBased::is_simulation_vertex(bp.first)) { maxBarcode=std::min(maxBarcode,bp.first); }
  }
#else
  HepMC::GenEvent::vertex_const_iterator currentGenVertexIter;
  for (currentGenVertexIter= genEvent->vertices_begin();
       currentGenVertexIter!= genEvent->vertices_end();
       ++currentGenVertexIter) {
    const int barcode((*currentGenVertexIter)->barcode());
    if (barcode < maxBarcode && !HepMC::BarcodeBased::is_simulation_vertex(barcode)) { maxBarcode=barcode; }
  }
#endif
  return maxBarcode;
}
}
#endif

#endif
