#include "TruthTrajectory.h"
#include <set>
#include "ActsTruth/ElasticDecayUtil.h"
#include "Acts/Utilities/Zip.hpp"
#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODInDetMeasurement/StripClusterContainer.h"
#include "xAODInDetMeasurement/HGTDClusterContainer.h"

#include <format>
#include <iostream>
#include <stdexcept>

namespace ActsTrk {

   std::vector<const MeasurementToTruthParticleAssociation *>
   getMeasurementToTruthContainer(const EventContext &ctx,
                                  const SG::ReadHandleKeyArray<MeasurementToTruthParticleAssociation> &measurementToTruthKeys) {
      std::vector<const MeasurementToTruthParticleAssociation *> measurement_to_truth(static_cast<std::size_t>(xAOD::UncalibMeasType::nTypes),nullptr);

      for (const SG::ReadHandleKey<MeasurementToTruthParticleAssociation> &key :  measurementToTruthKeys) {
         SG::ReadHandle<MeasurementToTruthParticleAssociation> handle(key,ctx);
         if (!handle.isValid()) {
            throw std::runtime_error(std::format("Failed to get measurement-to-truth-association container named: {}",key.key().c_str()));
         }
         if (handle->sourceContainer() && !handle->sourceContainer()->empty() && handle->sourceContainer()->front()) {
            measurement_to_truth.at(static_cast<std::size_t>(handle->sourceContainer()->front()->type())) = handle.cptr();
         }
      }
      return measurement_to_truth;
   }
   std::vector<std::vector<bool> >
   getMeasurementMask(const EventContext &ctx,
                      const SG::ReadHandleKeyArray<xAOD::UncalibratedMeasurementContainer> &measurementKeys,
                      const std::vector<const MeasurementToTruthParticleAssociation *> &measurementToTruth) {
      std::vector<std::vector<bool> > measurement_mask(static_cast<std::size_t>(xAOD::UncalibMeasType::nTypes));

      for (const SG::ReadHandleKey<xAOD::UncalibratedMeasurementContainer> &key :  measurementKeys) {
         SG::ReadHandle<xAOD::UncalibratedMeasurementContainer> handle(key,ctx);
         if (!handle.isValid()) {
            throw std::runtime_error(std::format("Failed to get measurement container named: {}",key.key().c_str()));
         }
         if (!handle->empty()) {
            std::size_t measurement_type = static_cast<std::size_t>(handle->front()->type());
            std::size_t n_measurements_orig = handle->front()->container()->size_v();
            measurement_mask.at(measurement_type).resize(n_measurements_orig,false);
            for (const xAOD::UncalibratedMeasurement *measurement : *handle) {
               measurement_mask[measurement_type].at(measurement->index())=true;
            }
         }
         else {
            std::size_t measurement_type=static_cast<std::size_t>(xAOD::UncalibMeasType::nTypes);
            if (dynamic_cast<const xAOD::PixelClusterContainer *>(handle.cptr())) {
               measurement_type = static_cast<std::size_t>(xAOD::UncalibMeasType::PixelClusterType);
            }
            else if (dynamic_cast<const xAOD::StripClusterContainer *>(handle.cptr())) {
               measurement_type = static_cast<std::size_t>(xAOD::UncalibMeasType::StripClusterType);
            }
            else if (dynamic_cast<const xAOD::HGTDClusterContainer *>(handle.cptr())) {
               measurement_type = static_cast<std::size_t>(xAOD::UncalibMeasType::HGTDClusterType);
            }

            if (measurement_type < measurement_mask.size()) {
               if (measurementToTruth.at(measurement_type)) {
                  measurement_mask[measurement_type].resize(measurementToTruth[measurement_type]->size(),false);
               }
            }
            else {
               throw std::runtime_error(std::format("Cannot determine measurement type for empty container {}",key.key().c_str()));
            }
         }
      }
      return measurement_mask;
   }


   TruthTrajectoryContainer getTruthTrajectories(const std::vector<const MeasurementToTruthParticleAssociation *> &measurement_to_truth,
                                                 unsigned int min_hits,
                                                 float max_energy_loss,
                                                 const std::vector<std::vector<bool> > &measurement_mask) {
      TruthTrajectoryContainer truth_trajectories;
      std::size_t n_truth_max=0ul;
      // get maximum number of truth particles / largest truth particle index
      for (const MeasurementToTruthParticleAssociation *measurement_to_truth_container : measurement_to_truth) {
         if (measurement_to_truth_container) {
            for (const ActsTrk::ParticleVector &associated_truth : *measurement_to_truth_container) {
               for (const xAOD::TruthParticle *truth_particle : associated_truth) {
                  if (truth_particle) {
                     n_truth_max=truth_particle->container()->size_v();
                     break;
                  }
               }
               if (n_truth_max>0ul) {
                  break;
               }
            }
            if (n_truth_max>0ul) {
               break;
            }
         }
      }
      // count hits per truth particle assuiming truth particle is the same if only decays elastically
      std::vector<bool> empty_mask;
      std::vector<unsigned int> hit_counts(n_truth_max,0u);
      ElasticDecayUtil<> elastic_decay;
      for (auto [/*const MeasurementToTruthParticleAssociation * */measurement_to_truth_container, measurement_type_i] :
              Acts::zip(measurement_to_truth,
                        std::ranges::views::iota(0u,static_cast<unsigned int>(measurement_to_truth.size())))) {

         if (measurement_to_truth_container) {
            const std::vector<bool> &the_mask = measurement_type_i < measurement_mask.size() ? measurement_mask[measurement_type_i] : empty_mask;
            for (auto [/*const ActsTrk::ParticleVector &*/ associated_truth, measurement_i] :
                    Acts::zip(*measurement_to_truth_container,
                              std::ranges::views::iota(0u,static_cast<unsigned int>(measurement_to_truth_container->size())))) {
               // skipped masked measurements.
               if (!the_mask.empty() && !the_mask.at(measurement_i)) continue;

               for (const xAOD::TruthParticle *truth_particle : associated_truth) {
                  if (truth_particle) {
                     const xAOD::TruthParticle *mother = elastic_decay.getMother(*truth_particle, max_energy_loss);
                     assert(mother);
                     // @TODO weight differently depending on measurement degrees of freedom / type ?
                     ++hit_counts.at(mother->index());
                  }
               }
            }
         }
      }
      // count maximum number of hits associated to all truth trajectories
      // count truth trajectories i.e. truth particles with a minimum number of associated hits.
      unsigned int n_truth_trajectories=0u;
      unsigned int n_hits_total=0u;
      std::vector<unsigned int> truth_to_truth_trajectory;
      truth_to_truth_trajectory.reserve(n_truth_max);
      for (unsigned int count : hit_counts) {
         truth_to_truth_trajectory.push_back(n_truth_trajectories);
         if (count>=min_hits) {
            ++n_truth_trajectories;
            n_hits_total += count;
         }
      }

      // pre partition flat storage for measurement ids
      //         truth_trajectories.reserve(n_truth_trajectories, n_hits_total);
      unsigned int offset=0u;
      truth_trajectories.m_truthParticle.resize(n_truth_trajectories,nullptr);
      truth_trajectories.m_hitIdOffset.reserve(n_truth_trajectories+1);
      truth_trajectories.m_hitIdOffset.push_back(offset);
      truth_trajectories.m_hitId.resize(n_hits_total,std::numeric_limits<unsigned int>::max());
      for (unsigned int &count : hit_counts) {
         if (count>=min_hits) {
            offset += count;
            truth_trajectories.m_hitIdOffset.push_back(offset);
         }
         else {
            count=0u;
         }
      }

      // fill measurement ids per truth trajectory
      unsigned int collection_id=0;
      for (auto [/*const MeasurementToTruthParticleAssociation * */measurement_to_truth_container, measurement_type_i] :
              Acts::zip(measurement_to_truth,
                        std::ranges::views::iota(0u,static_cast<unsigned int>(measurement_to_truth.size())))) {

         if (measurement_to_truth_container) {
            assert( collection_id< (1u<<6) );
            unsigned int measurement_id=TruthTrajectoryContainer::makeMeasurementId(collection_id,0u);
            const std::vector<bool> &the_mask = measurement_type_i < measurement_mask.size() ? measurement_mask[measurement_type_i] : empty_mask;
            for (auto [/*const ActsTrk::ParticleVector &*/ associated_truth, measurement_i] :
                    Acts::zip(*measurement_to_truth_container,
                              std::ranges::views::iota(0u,static_cast<unsigned int>(measurement_to_truth_container->size())))) {
               // skipped masked measurements.
               if (!the_mask.empty() && !the_mask.at(measurement_i)) continue;


               for (const xAOD::TruthParticle *truth_particle : associated_truth) {
                  if (truth_particle) {
                     const xAOD::TruthParticle *mother = elastic_decay.getMother(*truth_particle, max_energy_loss);
                     if (hit_counts.at(mother->index())>0u) {
                        if (mother != truth_particle) {
                           truth_to_truth_trajectory.at(truth_particle->index())=truth_to_truth_trajectory.at(mother->index());
                        }
                        unsigned int truth_trajectory_i = truth_to_truth_trajectory.at(mother->index());
                        if (--hit_counts[mother->index()] == 0) {
                           truth_trajectories.m_truthParticle.at(truth_trajectory_i)=mother;
                        }
                        assert( truth_trajectory_i < truth_trajectories.m_truthParticle.size()
                                && truth_trajectory_i+1 < truth_trajectories.m_hitIdOffset.size()
                                &&  truth_trajectories.m_hitIdOffset[truth_trajectory_i]
                                +hit_counts[mother->index()] < truth_trajectories.m_hitIdOffset[truth_trajectory_i+1]);
                        unsigned int hit_index = truth_trajectories.m_hitIdOffset[truth_trajectory_i]+hit_counts[mother->index()];
                        truth_trajectories.m_hitId.at(hit_index) = measurement_id;
                     }
                  }
               }
               ++measurement_id;
            }
         }
         ++collection_id;
      }
      truth_trajectories.m_truthTrajectoryIndex = std::move(truth_to_truth_trajectory);
      return truth_trajectories;
   }
}
