//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#ifndef CALORECGPU_TOPOAUTOMATONCLUSTERING_CUDA_H
#define CALORECGPU_TOPOAUTOMATONCLUSTERING_CUDA_H

#include "CaloRecGPU/CUDAFriendlyClasses.h"
#include "CaloRecGPU/DataHolders.h"
#include "CaloRecGPU/Helpers.h"
#include "ExtraTagDefinitions.h"

#include "CaloRecGPU/IGPUKernelSizeOptimizer.h"

namespace TAGrowing
{

  struct TACTag : public CaloRecGPU::Tag_1_1_12_32_18
  {
    using CaloRecGPU::Tag_1_1_12_32_18::Tag_1_1_12_32_18;

   protected:

    constexpr static carrier s_grow_tag = 0x7FFFFFFFFFFFFFFFULL;

    constexpr static carrier s_terminal_tag = 0x0000000000000001ULL;

    constexpr static carrier s_minimum_seed_tag = 0x8000000000000000ULL;

    constexpr static uint32_t s_start_counter = 0xFFFU;

    constexpr static carrier s_tag_propagation_delta = carrier(1) << s_12_bit_offset;

    constexpr static carrier s_no_can_merge_flag = s_second_flag_mask;

   public:

    [[nodiscard]] static constexpr carrier make_seed_tag(const uint32_t cell, const uint32_t SNR_pattern, const bool can_merge_with_others = true)
    {
      return make_generic_tag(cell, SNR_pattern, s_start_counter, !can_merge_with_others, true);
    }

    [[nodiscard]] static constexpr carrier make_grow_tag()
    {
      return s_grow_tag;
    }

    [[nodiscard]] static constexpr carrier make_terminal_tag()
    {
      return s_terminal_tag;
    }

    [[nodiscard]] constexpr bool is_part_of_cluster() const
    {
      return value > s_grow_tag;
    }

    [[nodiscard]] static constexpr bool is_part_of_cluster(const TACTag tag)
    {
      return tag.is_part_of_cluster();
    }

    [[nodiscard]] constexpr bool is_valid() const
    {
      return value >= s_terminal_tag;
    }

    [[nodiscard]] static constexpr bool is_valid(const TACTag tag)
    {
      return tag.is_valid();
    }

    [[nodiscard]] constexpr bool is_invalid() const
    {
      return !(this->is_valid());
    }

    [[nodiscard]] static constexpr bool is_invalid(const TACTag tag)
    {
      return tag.is_invalid();
    }

    [[nodiscard]] constexpr bool is_non_assigned_terminal() const
    {
      return value >= s_terminal_tag && value < s_grow_tag;
    }

    [[nodiscard]] static constexpr bool is_non_assigned_terminal(const TACTag tag)
    {
      return tag.is_non_assigned_terminal();
    }

    [[nodiscard]] constexpr bool is_non_assigned_grow() const
    {
      return value >= s_grow_tag && value < s_minimum_seed_tag;
    }

    [[nodiscard]] static constexpr bool is_non_assigned_grow(const TACTag tag)
    {
      return tag.is_non_assigned_grow();
    }

    [[nodiscard]] constexpr bool is_seed() const
    {
      return value >= s_minimum_seed_tag;
    }

    [[nodiscard]] static constexpr bool is_seed(const TACTag tag)
    {
      return tag.is_seed();
    }

    [[nodiscard]] constexpr bool is_grow_or_seed() const
    {
      return value >= s_grow_tag;
    }

    [[nodiscard]] static constexpr bool is_grow_or_seed(const TACTag tag)
    {
      return tag.is_grow_or_seed();
    }

    [[nodiscard]] constexpr carrier propagate() const
    {
      return (value - s_tag_propagation_delta) & (~s_no_can_merge_flag);
    }

    [[nodiscard]] static constexpr carrier propagate(const TACTag tag)
    {
      return tag.propagate();
    }

    [[nodiscard]] constexpr uint32_t index() const
    {
      return this->get_18_bits();
    }

    [[nodiscard]] static constexpr uint32_t index(const TACTag tag)
    {
      return tag.index();
    }

    [[nodiscard]] constexpr uint32_t SNR() const
    {
      return this->get_32_bits();
    }

    [[nodiscard]] static constexpr uint32_t SNR(const TACTag tag)
    {
      return tag.SNR();
    }

    [[nodiscard]] constexpr uint32_t counter() const
    {
      return this->get_12_bits();
    }

    [[nodiscard]] static constexpr uint32_t counter(const TACTag tag)
    {
      return tag.counter();
    }

    [[nodiscard]] constexpr carrier clear_counter_and_no_merge() const
    {
      return value & ~(s_second_flag_mask | s_12_bit_mask);
    }

    [[nodiscard]] static constexpr carrier clear_counter_and_no_merge(const TACTag tag)
    {
      return tag.clear_counter_and_no_merge();
    }

    [[nodiscard]] constexpr carrier set_index(const uint32_t new_index) const
    {
      return (value & (~s_18_bit_mask)) | (new_index & 0x3FFFFU);
    }

    [[nodiscard]] constexpr bool can_merge() const
    {
      return this->is_seed() && !this->get_second_flag();
    }

    [[nodiscard]] static constexpr bool can_merge(const TACTag tag)
    {
      return tag.can_merge();
    }

    [[nodiscard]] constexpr bool cannot_merge() const
    {
      return !this->can_merge();
    }

    [[nodiscard]] static constexpr bool cannot_merge(const TACTag tag)
    {
      return tag.cannot_merge();
    }

    [[nodiscard]] constexpr carrier clear_no_merge_flag() const
    {
      return this->unset_second_flag();
    }

    [[nodiscard]] static constexpr carrier clear_no_merge_flag(const TACTag tag)
    {
      return tag.clear_no_merge_flag();
    }
  };

  struct TopoAutomatonOptions
  {
    float        seed_threshold, grow_threshold, terminal_threshold;
    bool         abs_seed, abs_grow, abs_terminal;
    bool         use_two_gaussian;
    bool         treat_L1_predicted_as_good;
    bool         use_time_cut;
    bool         keep_significant_cells;
    bool         completely_exclude_cut_seeds;
    float        time_threshold;
    float        snr_threshold_for_keeping_cells;

    bool         limit_HECIW_and_FCal_neighs;
    bool         limit_PS_neighs;
    unsigned int neighbour_options;

    bool         use_crosstalk;
    float        crosstalk_delta;

    unsigned int valid_sampling_seed;
    //This is used to pack the bits that tell us whether a sample can be used to have seeds or not.
    unsigned int valid_calorimeter_by_sampling;
    //This is a way to express the selection of calorimeters to use for the algorithm
    //through the samplings (as the calorimeter <-> sampling relation is fixed...)

    constexpr bool uses_seed_sampling(const int sampling) const
    {
      return (valid_sampling_seed >> sampling) & 1;
    }

    /*! @brief Checks if the calorimeter is used through the sampling to which the cell belongs.

        We use the samplings to identify the calorimeters to be used since that's the information
        we have available on the GPU side of things (and the sampling-to-calorimeter map is well-defined).
    */
    constexpr bool uses_calorimeter_by_sampling(const int sampling) const
    {
      return (valid_calorimeter_by_sampling >> sampling) & 1;
    }

  };
  //Just a bundled way to pass the options for the Topo-Automaton Clustering.

  struct TACOptionsHolder
  {
    CaloRecGPU::Helpers::CPU_object<TopoAutomatonOptions> m_options;

    CaloRecGPU::Helpers::CUDA_object<TopoAutomatonOptions> m_options_dev;

    void allocate()
    {
      m_options.allocate();
    }

    void sendToGPU(const bool clear_CPU = false);
  };

  void register_kernels(IGPUKernelSizeOptimizer & optimizer);

  void signalToNoise(CaloRecGPU::EventDataHolder & holder,
                     const CaloRecGPU::ConstantDataHolder & instance_data,
                     const TACOptionsHolder & options,
                     const IGPUKernelSizeOptimizer & optimizer,
                     const bool synchronize = false,
                     CaloRecGPU::CUDA_Helpers::CUDAStreamPtrHolder stream_to_use = {});

  void cellPairs(CaloRecGPU::EventDataHolder & holder,
                 const CaloRecGPU::ConstantDataHolder & instance_data,
                 const TACOptionsHolder & options,
                 const IGPUKernelSizeOptimizer & optimizer,
                 const bool synchronize = false,
                 CaloRecGPU::CUDA_Helpers::CUDAStreamPtrHolder stream_to_use = {});

  void clusterGrowing(CaloRecGPU::EventDataHolder & holder,
                      const CaloRecGPU::ConstantDataHolder & instance_data,
                      const TACOptionsHolder & options,
                      const IGPUKernelSizeOptimizer & optimizer,
                      const bool synchronize = false,
                      CaloRecGPU::CUDA_Helpers::CUDAStreamPtrHolder stream_to_use = {});

}
#endif //CALORECGPU_TOPOAUTOMATONCLUSTERING_CUDA_H
