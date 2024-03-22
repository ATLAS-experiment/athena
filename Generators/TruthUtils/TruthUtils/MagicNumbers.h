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
#else
namespace HepMC {
template <class T>  inline int barcode(const T& p){ return p->barcode();}
template <>  inline int barcode(const int& p){ return p;}
}
#endif
namespace HepMC {

/// @brief Constant defining the barcode threshold for simulated particles, eg. can be used to separate generator event record entries from simulated particles
constexpr int SIM_BARCODE_THRESHOLD = 200000;

/// @brief Constant defining the barcode threshold for regenerated particles, i.e. particles surviving an interaction
constexpr int SIM_REGENERATION_INCREMENT = 1000000;

/// @brief Constant defining the barcode threshold for regenerated particles, i.e. particles surviving an interaction
constexpr int SIM_STATUS_INCREMENT = 100000;

/// @brief Constant definiting the status threshold for simulated particles, eg. can be used to separate generator event record entries from simulated particles
constexpr int SIM_STATUS_THRESHOLD = 20000;

/// @brief Constant that the meaning of which is currently lost, to be recovered...
constexpr int SPECIALSTATUS = 902;
constexpr int EVTGENUNDECAYEDSTATUS = 899;
constexpr int PYTHIA8LHESTATUS = 1003;
constexpr int HERWIG7INTERMEDIATESTATUS = 11;
constexpr int PYTHIA8NOENDVERTEXSTATUS = 201;
constexpr int FORWARDTRANSPORTMODELSTATUS = 212;

/// @brief This barcode is used by objects matched to particles from pile-up interactions in standard MC Production
constexpr int SUPPRESSED_PILEUP_BARCODE(std::numeric_limits<int32_t>::max());

constexpr int UNDEFINED_ID = 0;

constexpr int INVALID_PARTICLE_BARCODE = -1;

constexpr int SINGLE_PARTICLE = 10001;

/// @brief Method to establish if a particle (or barcode) corresponds to truth-suppressed pile-up (TODO update to be status based)
template <class T>  inline bool is_truth_suppressed_pileup(const T& p){ return (barcode(p) == SUPPRESSED_PILEUP_BARCODE);}

/// @brief Method to establish if a if the object is linked to something which was never saved to the HepMC Truth - for example particle was too low energy to be recorded (TODO update to be status based)
template <class T>  inline bool no_truth_link(const T& p){ return (barcode(p) == 0);} // TODO potentially this could become id()==0?

/// @brief Helper function for SDO creation in PileUpTools
template <class T>  inline bool ignoreTruthLink(const T& p, bool vetoPileUp){ const int b = barcode(p);  return no_truth_link(b) || (vetoPileUp && is_truth_suppressed_pileup(b)); }

/// @brief Method to establish if a particle (or barcode) was created during the simulation (TODO update to be status based)
template <class T>  inline bool is_simulation_particle(const T& p){ return (barcode(p)>SIM_BARCODE_THRESHOLD);}

/// @brief Method to return how many interactions a particle has undergone during simulation (TODO migrate to be based on status).
template <class T>  inline int generations(const T& p){ return (barcode(p)/SIM_REGENERATION_INCREMENT);}

  namespace BarcodeBased {
    /// @brief Method to establish if a particle (or barcode) was created during the simulation (only to be used in legacy TP converters)
    template <class T>  inline bool is_simulation_particle(const T& p){ return (barcode(p)>SIM_BARCODE_THRESHOLD);}

    /// @brief Method to establish if a particle (or barcode) is a new seondary created during the simulation (only to be used in legacy TP converters)
    template <class T>  inline bool is_sim_secondary(const T& p){ return (barcode(p)%SIM_REGENERATION_INCREMENT > SIM_BARCODE_THRESHOLD); }

    /// @brief Method to return how many interactions a particle has undergone during simulation (only to be used in legacy TP converters).
    template <class T>  inline int generations(const T& p){ return (barcode(p)/SIM_REGENERATION_INCREMENT);}
  }

  namespace StatusBased {
    /// @brief Method to establish if a particle was created during the simulation based on the status value
    template <class T>  inline bool is_simulation_particle(const T& p){ return (p->status()>SIM_STATUS_THRESHOLD);}

    /// @brief Method to establish if a particle is a new seondary created during the simulation based on the status value
    template <class T>  inline bool is_sim_secondary(const T& p){ return (p->status()%SIM_STATUS_INCREMENT > SIM_STATUS_THRESHOLD); }

    /// @brief Method to return how many interactions a particle has undergone during simulation based on the status value
    template <class T>  inline int generations(const T& p){ return (p->status()/SIM_STATUS_INCREMENT);}
  }

/// @brief Method to establish if the vertex was created during simulation (TODO migrate to be based on status).
template <class T>  inline bool is_simulation_vertex(const T& v){ return (barcode(v)<-SIM_BARCODE_THRESHOLD);}

  namespace BarcodeBased {
    /// @brief Method to establish if the vertex was created during simulation (only to be used in legacy TP converters)
    template <class T>  inline bool is_simulation_vertex(const T& v){ return (barcode(v)<-SIM_BARCODE_THRESHOLD);}
  }

  namespace StatusBased {
    /// @brief Method to establish if the vertex was created during simulation from the status
#if defined(HEPMC3)
    template <class T>  inline bool is_simulation_vertex(const T& v){ return (v->status()>SIM_STATUS_THRESHOLD);}
#else
    template <class T>  inline bool is_simulation_vertex(const T& v){ return (v->id()>SIM_STATUS_THRESHOLD);}
#endif
  }

template <class T1,class T2>
inline bool is_same_generator_particle(const T1& p1,const T2& p2) { int b1 = barcode(p1); int b2 = barcode(p2); return  b1% SIM_REGENERATION_INCREMENT == b2 % SIM_REGENERATION_INCREMENT; }

/// @brief Method to check if the first particle is a descendant of the second in the simulation, i.e. particle p1 was produced simulations particle p2.
template <class T1,class T2>
inline bool is_sim_descendant(const T1& p1,const T2& p2) { int b1 = barcode(p1); int b2 = barcode(p2); return b1 % SIM_REGENERATION_INCREMENT == b2;}

#if  defined(HEPMC3)
template <class T> inline int uniqueID(const T& p) { return p->id(); }
#else
template <class T> inline int uniqueID(const T* p) { return barcode(p); }
#endif
#if  defined(HEPMC3) && !defined(XAOD_STANDALONE)
template <>  inline int uniqueID(const ConstGenParticlePtr& p1){ return p1->id();}
template <>  inline int uniqueID(const GenParticlePtr& p1){ return p1->id();}
#endif


/// @brief Function to calculate all the descendants(direction=1)/ancestors(direction=-1) of the particle.
template <class T> inline void get_particle_history(const T& p, std::deque<int>& out, int direction=0) {
  if (direction < 0) {
    if (p->status()>SIM_STATUS_INCREMENT) {
      auto pv = p->production_vertex();
      if (pv) {
        for (auto pa: pv->particles_in()) {
          if (pa->pdg_id() != p->pdg_id()) continue;
          out.push_front(uniqueID(p));
          get_particle_history(pa,out,-1);
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
          if (pa->pdg_id() != p->pdg_id()) continue;
          out.push_back(uniqueID(p));
          get_particle_history(pa,out,1);
          break;
        }
      }
    }
  }
}
/// @brief Function to calculate all the descendants(direction=1)/ancestors(direction=-1) of the particle.
template <class T>  inline std::deque<int> simulation_history(const T& p, int direction ) { std::deque<int> res; res.push_back(uniqueID(p)); get_particle_history(p, res, direction); return res;}

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

/// @brief Functions for converting between the old and new barcode/status schemes
inline int new_particle_status_from_old(int oldStatus, int barcode) { 
  int generations_barcode_based = (barcode/SIM_REGENERATION_INCREMENT);
  bool is_sim_secondary_barcode_based = (barcode%SIM_REGENERATION_INCREMENT > SIM_BARCODE_THRESHOLD);
  return oldStatus + SIM_STATUS_INCREMENT*generations_barcode_based + (is_sim_secondary_barcode_based? SIM_STATUS_THRESHOLD : 0); }
inline int old_particle_status_from_new(int newStatus) { return newStatus%SIM_STATUS_THRESHOLD; }

inline int new_vertex_status_from_old(int oldStatus, int barcode) { 
  bool is_simulation_vertex_barcode_based =  (barcode<-SIM_BARCODE_THRESHOLD);
  return (is_simulation_vertex_barcode_based? SIM_STATUS_THRESHOLD : 0) + oldStatus; 
}
inline int old_vertex_status_from_new(int newStatus) {
  bool is_simulation_vertex_status_based = (newStatus>SIM_STATUS_THRESHOLD); 
  return ( is_simulation_vertex_status_based ? -SIM_STATUS_THRESHOLD : 0) + newStatus; }
}
#endif
