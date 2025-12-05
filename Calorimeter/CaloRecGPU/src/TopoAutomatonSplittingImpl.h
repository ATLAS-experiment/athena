//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#ifndef CALORECGPU_TOPOAUTOMATONSPLITTING_CUDA_H
#define CALORECGPU_TOPOAUTOMATONSPLITTING_CUDA_H

#include "CaloRecGPU/CUDAFriendlyClasses.h"
#include "CaloRecGPU/DataHolders.h"
#include "CaloRecGPU/Helpers.h"
#include "ExtraTagDefinitions.h"

#include "FPHelpers.h"

#include "CaloRecGPU/IGPUKernelSizeOptimizer.h"

namespace TASplitting
{

  struct TASTag : public CaloRecGPU::Tag_1_1_12_32_18
//Any %?
  {
    using CaloRecGPU::Tag_1_1_12_32_18::Tag_1_1_12_32_18;

   protected:

    static constexpr uint32_t s_start_counter = 0xFFFU;
    //12 bits.

    static constexpr carrier s_tag_propagation_delta = carrier(1) << s_12_bit_offset;

    using EnergyFPFormat = FloatingPointHelpers::StandardFloat;

    [[nodiscard]] constexpr static uint32_t energy_to_storage(const uint32_t energy_pattern)
    {
      return EnergyFPFormat::template to_total_ordering<uint32_t>(energy_pattern);
    }

    [[nodiscard]] constexpr static uint32_t storage_to_energy(const uint32_t storage_pattern)
    {
      return EnergyFPFormat::template from_total_ordering<uint32_t>(storage_pattern);
    }

   public:

    [[nodiscard]] constexpr static carrier counter_delta()
    {
      return s_tag_propagation_delta;
    }

    [[nodiscard]] constexpr int32_t index() const
    {
      return this->get_18_bits();
    }

    [[nodiscard]] constexpr static int32_t index(const TASTag tag)
    {
      return tag.index();
    }

    [[nodiscard]] constexpr int32_t counter() const
    {
      return this->get_12_bits();
    }

    [[nodiscard]] static constexpr int32_t counter(const TASTag tag)
    {
      return tag.counter();
    }
    
    [[nodiscard]] constexpr carrier update_cell(const uint32_t new_index, const uint32_t new_energy) const
    {
      return make_generic_tag(new_index, energy_to_storage(new_energy), 0, 0, 0) | (value & ~(s_18_bit_mask | s_32_bit_mask));
    }

    [[nodiscard]] constexpr carrier update_energy(const uint32_t new_energy) const
    {
      return (value & ~s_32_bit_mask) | (energy_to_storage(new_energy) << s_32_bit_offset);
    }

    [[nodiscard]] constexpr carrier clear_energy() const
    {
      return (value & ~s_32_bit_mask);
    }

    [[nodiscard]] constexpr carrier update_index(const uint32_t new_index) const
    {
      return (value & (~s_18_bit_mask)) | ( (carrier(new_index) << s_18_bit_offset) & s_18_bit_mask);
    }

    [[nodiscard]] constexpr carrier update_counter(const uint32_t new_counter) const
    {
      return (value & (~s_12_bit_mask)) | (carrier(new_counter) << s_12_bit_offset);
    }

    /*! Expects @p maximum_energy_pattern to be the bit pattern of the float that represents the energy.
    */
    [[nodiscard]] static constexpr carrier make_maximum_tag(const int32_t index, const uint32_t energy_pattern, const bool is_primary)
    {
      return make_generic_tag(index, energy_to_storage(energy_pattern), s_start_counter, true, is_primary);
    }

   protected:

    static constexpr carrier s_unassigned_tag = 1;

   public:
   
    [[nodiscard]] static constexpr carrier make_non_split_cluster_tag()
    {
      return s_unassigned_tag;
    }

    [[nodiscard]] constexpr bool is_valid() const
    {
      return value >= s_unassigned_tag;
    }

    [[nodiscard]] static constexpr bool is_valid(const TASTag tag)
    {
      return tag.is_valid();
    }

    [[nodiscard]] constexpr bool is_invalid() const
    {
      return !this->is_valid();
    }

    [[nodiscard]] static constexpr bool is_invalid(const TASTag tag)
    {
      return tag.is_invalid();
    }

    [[nodiscard]] constexpr bool is_part_of_splitter_cluster() const
    {
      return value > s_unassigned_tag;
    }

    [[nodiscard]] static constexpr bool is_part_of_splitter_cluster(const TASTag tag)
    {
      return tag.is_part_of_splitter_cluster();
    }
    
    [[nodiscard]] constexpr bool is_not_shared() const
    {
      return this->get_second_flag();
    }

    [[nodiscard]] constexpr static bool is_not_shared(const TASTag tag)
    {
      return tag.is_not_shared();
    }

    [[nodiscard]] constexpr bool is_shared() const
    {
      return !this->is_not_shared();
    }

    [[nodiscard]] constexpr static bool is_shared(const TASTag tag)
    {
      return tag.is_shared();
    }

    [[nodiscard]] constexpr carrier set_shared() const
    {
      return this->unset_second_flag();
    }

    [[nodiscard]] static constexpr carrier set_shared(const TASTag tag)
    {
      return tag.set_shared();
    }

    [[nodiscard]] constexpr carrier clear_shared() const
    {
      return this->set_second_flag();
    }

    [[nodiscard]] static constexpr carrier clear_shared(const TASTag tag)
    {
      return tag.clear_shared();
    }

    [[nodiscard]] constexpr carrier prepare_for_sharing(const TASTag other_tag) const
    {
      return (other_tag & (~s_second_flag_mask)) | s_first_flag_mask | s_12_bit_mask;
    }
  
    [[nodiscard]] constexpr bool is_first() const
    {
      return this->get_first_flag();
    }
    
    [[nodiscard]] static constexpr bool is_first(const TASTag tag)
    {
      return tag.is_first();
    }
    
    [[nodiscard]] constexpr carrier propagate() const
    {
      return (value - s_tag_propagation_delta) & (~s_first_flag_mask);
    }

    [[nodiscard]] static constexpr carrier propagate(const TASTag tag)
    {
      return tag.propagate();
    }
    
   protected:

    static constexpr carrier s_secondary_maxima_eliminator_tag = 0xFFFFFFFFFFFFFFFFULL;

   public:

    [[nodiscard]] static constexpr carrier secondary_maxima_eliminator()
    {
      return s_secondary_maxima_eliminator_tag;
    }

    [[nodiscard]] constexpr bool is_secondary_maxima_eliminator() const
    {
      return value == s_secondary_maxima_eliminator_tag;
    }

    [[nodiscard]] static constexpr bool is_secondary_maxima_eliminator(const TASTag tag)
    {
      return tag.is_secondary_maxima_eliminator();
    }
    
    [[nodiscard]] constexpr bool is_primary_maximum() const
    {
      return this->get_first_flag();
    }
    
    [[nodiscard]] static constexpr bool is_primary_maximum(const TASTag tag)
    {
      return tag.is_primary_maximum();
    }
    
    [[nodiscard]] constexpr bool is_secondary_maximum() const
    {
      return !this->is_primary_maximum();
    }
    
    [[nodiscard]] static constexpr bool is_secondary_maximum(const TASTag tag)
    {
      return tag.is_secondary_maximum();
    }
    
    [[nodiscard]] constexpr carrier unset_secondary() const
    {
      return this->set_first_flag();
    }
    
    [[nodiscard]] static constexpr carrier unset_secondary(const TASTag tag)
    {
      return tag.unset_secondary();
    }
  };

  struct TopoAutomatonSplittingOptions
  {
    unsigned int valid_sampling_primary;
    unsigned int valid_sampling_secondary;

    int min_num_cells;
    float min_maximum_energy;

    float EM_shower_scale;

    bool share_border_cells;
    bool use_absolute_energy;
    bool treat_L1_predicted_as_good;

    bool limit_HECIW_and_FCal_neighs;
    bool limit_PS_neighs;
    //WARNING: the CPU version of the algorithm does not seem to have this option. Given the description,
    //         maybe it makes some sense to still allow this here? In our configuration we'll keep it disabled, but...

    unsigned int neighbour_options;
    
    static constexpr bool uses_this_sampling(const unsigned int pattern, const unsigned int sampling)
    {
      return (pattern >> sampling) & 1U;
    }

    constexpr bool uses_primary_sampling(const unsigned int sampling) const
    {
      return uses_this_sampling(valid_sampling_primary, sampling);
    }
    
    constexpr bool uses_secondary_sampling(const unsigned int sampling) const
    {
      return uses_this_sampling(valid_sampling_secondary, sampling);
    }
    
    constexpr bool uses_sampling(const unsigned int sampling) const
    {
      return uses_this_sampling(valid_sampling_primary | valid_sampling_secondary, sampling);
    }
  };

  struct TASOptionsHolder
  {
    CaloRecGPU::Helpers::CPU_object<TopoAutomatonSplittingOptions> m_options;

    CaloRecGPU::Helpers::CUDA_object<TopoAutomatonSplittingOptions> m_options_dev;

    void allocate()
    {
      m_options.allocate();
    }

    void sendToGPU(const bool clear_CPU = false);
  };

  void register_kernels(IGPUKernelSizeOptimizer & optimizer);

  void fillNeighbours(CaloRecGPU::EventDataHolder & holder,
                      const CaloRecGPU::ConstantDataHolder & instance_data,
                      const TASOptionsHolder & options,
                      const IGPUKernelSizeOptimizer & optimizer,
                      const bool synchronize = false,
                      CaloRecGPU::CUDA_Helpers::CUDAStreamPtrHolder stream_to_use = {});

  void findLocalMaxima(CaloRecGPU::EventDataHolder & holder,
                       const CaloRecGPU::ConstantDataHolder & instance_data,
                       const TASOptionsHolder & options,
                       const IGPUKernelSizeOptimizer & optimizer,
                       const bool synchronize = false,
                       CaloRecGPU::CUDA_Helpers::CUDAStreamPtrHolder stream_to_use = {});

  void excludeSecondaryMaxima(CaloRecGPU::EventDataHolder & holder,
                              const CaloRecGPU::ConstantDataHolder & instance_data,
                              const TASOptionsHolder & options,
                              const IGPUKernelSizeOptimizer & optimizer,
                              const bool synchronize = false,
                              CaloRecGPU::CUDA_Helpers::CUDAStreamPtrHolder stream_to_use = {});

  void splitClusterGrowing(CaloRecGPU::EventDataHolder & holder,
                           const CaloRecGPU::ConstantDataHolder & instance_data,
                           const TASOptionsHolder & options,
                           const IGPUKernelSizeOptimizer & optimizer,
                           const bool synchronize = false,
                           CaloRecGPU::CUDA_Helpers::CUDAStreamPtrHolder stream_to_use = {});

  void cellWeightingAndFinalization(CaloRecGPU::EventDataHolder & holder,
                                    const CaloRecGPU::ConstantDataHolder & instance_data,
                                    const TASOptionsHolder & options,
                                    const IGPUKernelSizeOptimizer & optimizer,
                                    const bool synchronize = false,
                                    CaloRecGPU::CUDA_Helpers::CUDAStreamPtrHolder stream_to_use = {});
}

#endif //CALORECGPU_TOPOAUTOMATONSPLITTING_CUDA_H
