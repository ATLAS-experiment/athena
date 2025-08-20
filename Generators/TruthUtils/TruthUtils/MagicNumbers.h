/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
/* Author: Andrii Verbytskyi andrii.verbytskyi@mpp.mpg.de */

#ifndef TRUTHUTILS_MAGICNUMBERS_H
#define TRUTHUTILS_MAGICNUMBERS_H

#include <limits>
namespace HepMC {
/// @brief Constant defining the barcode threshold distinguishing generator record entries from detector sim ones
/// @todo The sim barcodes start at 1M in MC15, so we should update the 200k threshold,
///   but >= 200k is still a valid test for b = 1M so let's keep it this way until MC12 is long-dead.
constexpr int SIM_BARCODE_THRESHOLD = 200000;
constexpr int SIM_REGENERATION_INCREMENT = 1000000;

/// @brief Constant defining the barcode threshold for regenerated particles, i.e. particles surviving an interaction
constexpr int SIM_STATUS_INCREMENT = 100000;

/// @brief Constant definiting the status threshold for simulated particles, eg. can be used to separate generator event record entries from simulated particles
constexpr int SIM_STATUS_THRESHOLD = 20000;

constexpr int PARTONPDGMAX = 43;
constexpr int NPPDGMIN = 1000000;
constexpr int NPPDGMAX = 8999999;
constexpr int PHOTOSMIN = 10000;

/// @brief This barcode is used by objects matched to particles from pile-up
/// interactions in standard MC Production
constexpr int crazyParticleBarcode(std::numeric_limits<int32_t>::max());

constexpr int INVALID_PARTICLE_BARCODE = -1;

template <class T>  inline bool is_simulation_particle(const T& p){ return (p->barcode()>SIM_BARCODE_THRESHOLD);}
template <>  inline bool is_simulation_particle(const int& b){ return (b>SIM_BARCODE_THRESHOLD);}

template <class T>  inline bool is_simulation_vertex(const T& p){ return (p->barcode()<-SIM_BARCODE_THRESHOLD);}
template <>  inline bool is_simulation_vertex(const int& b){ return (b<-SIM_BARCODE_THRESHOLD);}

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
