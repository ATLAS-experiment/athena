//
// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
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

    [[nodiscard]] constexpr uint32_t energy_bits() const
    {
      return this->get_32_bits();
    }

    [[nodiscard]] constexpr static uint32_t energy_bits(const TASTag tag)
    {
      return tag.energy_bits();
    }

    [[nodiscard]] constexpr int32_t counter() const
    {
      return this->get_12_bits();
    }

    [[nodiscard]] static constexpr int32_t counter(const TASTag tag)
    {
      return tag.counter();
    }

    [[nodiscard]] static constexpr int32_t max_counter()
    {
      return s_start_counter;
    }

    [[nodiscard]] constexpr bool is_primary() const
    {
      return this->get_first_flag();
    }

    [[nodiscard]] constexpr static bool is_primary(const TASTag tag)
    {
      return tag.is_primary();
    }

    [[nodiscard]] constexpr bool is_secondary() const
    {
      return !this->is_primary();
    }

    [[nodiscard]] constexpr static bool is_secondary(const TASTag tag)
    {
      return tag.is_secondary();
    }

    /*! Expects @p maximum_energy_pattern to be the bit pattern of the float that represents the energy.
    */
    [[nodiscard]] static constexpr carrier make_maximum_tag(const int32_t index, const uint32_t maximum_energy_pattern, const bool is_primary)
    {
      return make_generic_tag(index, energy_to_storage(maximum_energy_pattern), s_start_counter, true, is_primary);
    }

    [[nodiscard]] static constexpr carrier make_original_cluster_tag(const uint16_t original_cluster_index)
    {
      return make_generic_tag(original_cluster_index, 0xFFFFFFFFU, s_start_counter, true, true);
    }

    /*! Expects @p energy_pattern to be the bit pattern of the float that represents the energy.
    */
    [[nodiscard]] static constexpr carrier make_shared_tag(const int32_t index, const uint32_t energy_pattern, const int32_t counter)
    {
      return make_generic_tag(index, energy_to_storage(energy_pattern), counter, false, true);
    }

    [[nodiscard]] static constexpr carrier make_cluster_cell_tag(const uint16_t original_cluster_index)
    {
      return make_generic_tag(original_cluster_index, 1, 0, 0, 0);
    }

   protected:

    static constexpr carrier s_last_non_assigned_tag = (s_18_bit_mask << 1) | s_18_bit_mask;

   public:

    [[nodiscard]] constexpr carrier set_primary() const
    {
      return this->set_first_flag();
    }

    [[nodiscard]] static constexpr carrier set_primary(const TASTag tag)
    {
      return tag.set_primary();
    }

    [[nodiscard]] constexpr carrier clear_primary() const
    {
      return this->unset_first_flag();
    }

    [[nodiscard]] static constexpr carrier clear_primary(const TASTag tag)
    {
      return tag.clear_primary();
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

    [[nodiscard]] constexpr bool is_valid() const
    {
      return value > 0ULL;
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

    [[nodiscard]] constexpr bool is_part_of_original_cluster() const
    {
      return (value & s_32_bit_mask) == s_32_bit_mask;
      //As original cluster cells have all bits of the energy set to 1
      //(which would be a NaN for normal energy)
    }

    [[nodiscard]] static constexpr bool is_part_of_original_cluster(const TASTag tag)
    {
      return tag.is_part_of_original_cluster();
    }

    [[nodiscard]] constexpr bool is_part_of_splitter_cluster() const
    {
      return value > s_last_non_assigned_tag && !this->is_part_of_original_cluster();
    }

    [[nodiscard]] static constexpr bool is_part_of_splitter_cluster(const TASTag tag)
    {
      return tag.is_part_of_splitter_cluster();
    }

    [[nodiscard]] constexpr bool is_shared() const
    {
      return !(this->get_second_flag());
    }

    [[nodiscard]] constexpr static bool is_shared(const TASTag tag)
    {
      return tag.is_shared();
    }

    [[nodiscard]] constexpr bool is_non_assigned_part_of_split_cluster() const
    {
      return value > 0 && value <= s_last_non_assigned_tag;
    }

    [[nodiscard]] static constexpr bool is_non_assigned_part_of_split_cluster(const TASTag tag)
    {
      return tag.is_non_assigned_part_of_split_cluster();
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

    [[nodiscard]] constexpr carrier ordering_bits() const
    {
      return value & (s_32_bit_mask | s_18_bit_mask);
    }

    [[nodiscard]] static constexpr carrier ordering_bits(const TASTag tag)
    {
      return tag.ordering_bits();
    }

    [[nodiscard]] constexpr carrier prepare_for_sharing(const TASTag other_tag) const
    {
      return (other_tag & (~s_second_flag_mask)) | s_first_flag_mask | s_12_bit_mask;
    }

   protected:

    static constexpr carrier s_secondary_maxima_eliminator_tag = 0xFFFFFFFFFFFFFFFFULL;

   public:

    /*! Just to be semantically clearer for the maxima exclusion.
    */
    [[nodiscard]] constexpr bool is_secondary_maximum_seed() const
    {
      return this->is_part_of_splitter_cluster() && this->is_secondary();
    }

    [[nodiscard]] constexpr static bool is_secondary_maximum_seed(const TASTag tag)
    {
      return tag.is_secondary_maximum_seed();
    }

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

    [[nodiscard]] constexpr carrier propagate() const
    {
          return (value - s_tag_propagation_delta) & (~s_first_flag_mask);
    }

    [[nodiscard]] static constexpr carrier propagate(const TASTag tag)
    {
      return tag.propagate();
    }

  };

  struct TopoAutomatonSplittingTemporaries
  {
    CaloRecGPU::tag_type secondary_array[CaloRecGPU::NCaloCells];

    CaloRecGPU::tag_type tertiary_array[CaloRecGPU::NCaloCells];

    int original_cluster_map[CaloRecGPU::NMaxClusters];

    int cell_to_cluster_map[CaloRecGPU::NCaloCells];

    struct PairsArr
    {
      constexpr static int s_size = CaloRecGPU::NMaxPairs + CaloRecGPU::NMaxPairs / 2;
      constexpr static int s_intermediate_mark = CaloRecGPU::NMaxPairs;

      // I think we would be safe even with just NMaxPairs,
      // but this way we can store all the four pairs consecutively:
      //
      // 0                                                  NMaxPairs             NMaxPairs + NMaxPairs/2
      // ^                                                      ^                           ^
      // |                                                      |                           |
      // +------------------------------------------------------+---------------------------+
      // |                                                      |                           |
      // |normal pairs>                              <next pairs|prev pairs>    <extra pairs|
      //
      //
      // We are far from pressed for memory,
      // so we can even increase this,
      // but even NMaxPairs already has a margin
      // (a factor of a bit less than 2...)

      int cellID[s_size];
      int neighbourID[s_size];
      int number_normal;
      int number_next;
      int number_prev;
      int number_extra;
    };

    PairsArr pairs;

    int reset_counters[2];

    int continue_flag;

    int stop_flag;


    //While somewhat distasteful,
    //all of this casting is safe
    //in the sense that everything
    //is really a POD and CUDA already
    //gives us just a region of memory...

    template <size_t i = 0, class Type = float>
    constexpr const Type * get_cells_extra_array() const
    {
      static_assert((i + 1) * sizeof(Type) * CaloRecGPU::NCaloCells <= sizeof(TopoAutomatonSplittingTemporaries),
                    "Cannot access outside of temporary bounds...");

      return ((Type *) ((void *) secondary_array)) + i * CaloRecGPU::NCaloCells;
    }

    template <size_t i = 0, class Type = float>
    constexpr Type * get_cells_extra_array()
    {
      static_assert((i + 1) * sizeof(Type) * CaloRecGPU::NCaloCells <= sizeof(TopoAutomatonSplittingTemporaries),
                    "Cannot access outside of temporary bounds...");

      return ((Type *) ((void *) secondary_array)) + i * CaloRecGPU::NCaloCells;
    }

    template <size_t i = 0, class Type = float>
    constexpr const Type & get_cells_extra_array(const int j) const
    {
      return this->get_cells_extra_array<i, Type>()[j];
    }

    template <size_t i = 0, class Type = float>
    constexpr Type & get_cells_extra_array(const int j)
    {
      return this->get_cells_extra_array<i, Type>()[j];
    }

    template <size_t i = 0, class Type = float>
    constexpr const Type * get_cluster_extra_array() const
    {
      static_assert((i + 1) * sizeof(Type) * CaloRecGPU::NMaxClusters <= sizeof(TopoAutomatonSplittingTemporaries),
                    "Cannot access outside of temporary bounds...");

      return ((Type *) ((void *) secondary_array)) + i * CaloRecGPU::NMaxClusters;
    }

    template <size_t i = 0, class Type = float>
    constexpr Type * get_cluster_extra_array()
    {
      static_assert((i + 1) * sizeof(Type) * CaloRecGPU::NMaxClusters <= sizeof(TopoAutomatonSplittingTemporaries),
                    "Cannot access outside of temporary bounds...");

      return ((Type *) ((void *) secondary_array)) + i * CaloRecGPU::NMaxClusters;
    }

    template <size_t i = 0, class Type = float>
    constexpr const Type & get_cluster_extra_array(const int j) const
    {
      return this->get_cluster_extra_array<i, Type>()[j];
    }

    template <size_t i = 0, class Type = float>
    constexpr Type & get_cluster_extra_array(const int j)
    {
      return this->get_cluster_extra_array<i, Type>()[j];
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

    constexpr bool uses_primary_sampling(const unsigned int sampling) const
    {
      return (valid_sampling_primary >> sampling) & 1U;
    }
    constexpr bool uses_secondary_sampling(const unsigned int sampling) const
    {
      return (valid_sampling_secondary >> sampling) & 1U;
    }
    constexpr bool uses_sampling(const unsigned int sampling) const
    {
      return uses_primary_sampling(sampling) || uses_secondary_sampling(sampling);
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