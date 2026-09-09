#ifndef ACTSTRK_TRUTHTRAJECTORYCONTAINER_H
#define ACTSTRK_TRUTHTRAJECTORYCONTAINER_H

#include "xAODTruth/TruthParticleContainer.h"
#include "ActsEvent/MeasurementToTruthParticleAssociation.h"

#include "StoreGate/ReadHandleKeyArray.h"
#include "StoreGate/ReadHandleKey.h"

#include <vector>
#include <cassert>

namespace ActsTrk {
// container of truth trajectories
struct TruthTrajectoryContainer {
   void reserve(unsigned int n_truth_trajectories,
                unsigned int n_hits_total) {
      m_truthParticle.reserve(n_truth_trajectories);
      m_hitIdOffset.reserve(n_truth_trajectories+1);
      m_hitId.reserve(n_hits_total);
   }
   // get the number of truth trajectories.
   std::size_t size() const {
      assert( m_truthParticle.size()+1 == m_hitIdOffset.size());
      return m_truthParticle.size();
   }
   static const unsigned int MEASUREMENT_INDEX_BIT=26;
   static unsigned int makeMeasurementId(unsigned int measurement_type, unsigned int measurement_index) {
      assert( measurement_index < (1<<MEASUREMENT_INDEX_BIT) && measurement_type < (1<<6) );
      return (measurement_type<<MEASUREMENT_INDEX_BIT) | measurement_index;
   }
   static unsigned int getMeasurementType(unsigned int measurement_id) {
      return measurement_id >> MEASUREMENT_INDEX_BIT;
   }
   static unsigned int getMeasurementIndex(unsigned int measurement_id) {
      return ((1u<<MEASUREMENT_INDEX_BIT)-1) & measurement_id;
   }

   // get the measurement ids associated to the given truth trajectory.
   // the  "identifier" is a bit combination of the container index (bits 26-31) and the measurement index.
   std::span<const unsigned int> measurements(unsigned int trajectory_i) const {
      assert(trajectory_i+1<m_hitIdOffset.size());
      return std::span( m_hitId.data()+m_hitIdOffset[trajectory_i],
                        static_cast<std::size_t>(m_hitIdOffset[trajectory_i+1] - m_hitIdOffset[trajectory_i]));
   }
   // get the truth particle associated to the given truth trajectory.
   const xAOD::TruthParticle *truthParticle(unsigned int trajectory_i) const {
      assert(trajectory_i<m_truthParticle.size() && m_truthParticle[trajectory_i] != nullptr);
      return m_truthParticle[trajectory_i];
   }

   unsigned int truthTrajectoryIndex(const xAOD::TruthParticle *truth_particle) const {
      assert( truth_particle && truth_particle->index() <= m_truthTrajectoryIndex.size());
      return m_truthTrajectoryIndex[truth_particle->index()];
   }

   std::vector<const xAOD::TruthParticle *> m_truthParticle; //< vector of truth particles associated to a truth trajectory
   std::vector<unsigned int> m_hitIdOffset;  //< offset to the first hit associated to truth particle of the same index (last element is the end index)
   std::vector<unsigned int> m_hitId; //< "identifier" of associated measurements which is a bit combination of the container index (bits 26-31) and
                                      //< the measurement index.
   std::vector<unsigned int> m_truthTrajectoryIndex; //< truth trajectory index per truth particle
};

// Get a vector with one element per truth container pointing to a truth association container if available.
// @param ctx the current event context
// @param measurementToTruthKeys keys for the measurement-to-truth association maps.
std::vector<const MeasurementToTruthParticleAssociation *>
getMeasurementToTruthContainer(const EventContext &ctx,
                               const SG::ReadHandleKeyArray<MeasurementToTruthParticleAssociation> &measurementToTruthKeys);

// Build truth trajectories using the given association maps.
// Truth trajectories are composed of measurements associated to the same truth particle, where truth particles are
// considered identical if the parent according to the elastic decay model is the same.
// If a measurement_mask is given than for every measurement type the provided vector is not empty measurements are only
// considered for a truth trajectory if the corresponding value is true.
TruthTrajectoryContainer getTruthTrajectories(const std::vector<const MeasurementToTruthParticleAssociation *> &measurement_to_truth,
                                              unsigned int min_hits,
                                              float max_energy_loss,
                                              const std::vector<std::vector<bool> > &measurement_mask=std::vector<std::vector<bool> >());

// Get a measurement masks based on the given measurement containers.
// @param ctx the current event context
// @param measurementKeys keys for the measurement containers which contain the measurements that should not be masked out
// @param measurementToTruth the full measurement-to-truth association maps (will eventually be used to dimension the masks).
// The returned vector will contain one vector per measurement type, which can be empty or
// will contain a boolean for each measurement (full container) which is true if the measurement was contained in the given container(subset).
std::vector<std::vector<bool> >
getMeasurementMask(const EventContext &ctx,
                   const SG::ReadHandleKeyArray<xAOD::UncalibratedMeasurementContainer> &measurementKeys,
                   const std::vector<const MeasurementToTruthParticleAssociation *> &measurementToTruth);
}

#endif
