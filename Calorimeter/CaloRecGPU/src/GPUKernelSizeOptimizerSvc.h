//
// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#ifndef CALORECGPU_GPUKERNELSIZEOPTIMIZERSVC_H
#define CALORECGPU_GPUKERNELSIZEOPTIMIZERSVC_H

#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

#include "AthenaBaseComps/AthService.h"

#include "CaloRecGPU/IGPUKernelSizeOptimizerSvc.h"
#include "CaloRecGPU/CaloGPUCUDAInitialization.h"

#include <nlohmann/json.hpp>

/**
 * @class GPUKernelSizeOptimizerSvc
 * @author Nuno Fernandes <nuno.dos.santos.fernandes@cern.ch>
 * @date 06 August 2023
 * @brief .
 */

class GPUKernelSizeOptimizerSvc : public extends <AthService, IGPUKernelSizeOptimizerSvc>, public CaloGPUCUDAInitialization
{
 public:

  GPUKernelSizeOptimizerSvc(const std::string & name, ISvcLocator * svc);

  /** @brief Register a set of kernels that can be referred back to with a name and a number.

      Uses C-style arrays for more immediate CUDA compatibility,
      assumes the size of @p kernels, @p blocksize_hints and @p gridsize_hints is @p number
      and starts the numbering with an optional @p offset.
  */
  virtual void register_kernels(const std::string & tool_name,
                                const int number,
                                void ** kernels,
                                const int * blocksize_hints,
                                const int * gridsize_hints,
                                const int * max_total_threads,
                                const int offset = 0) override;

  /** @brief Retrieve the (hopefully optimal) kernel launch configuration.*/
  virtual CUDAKernelLaunchConfiguration get_launch_configuration(const std::string & name,
                                                                 const int number = 0,
                                                                 const int dynamic_memory = 0) const override;

  /** @brief Whether the device + environment in use support cooperative groups. */
  virtual bool can_use_cooperative_groups() const override
  {
    return (!m_overrideCooperativeGroups && m_coopgroup_support);
  }

  /** @brief Whether the device + environment in use support dynamic parallelism. */
  virtual bool can_use_dynamic_parallelism() const override
  {
    return m_dynpar_support;
  }

  /** @brief Whether to avoid oversizing kernels and instead (if possible) launch kernels with the exact number of threads... */
  virtual bool should_use_minimal_kernel_sizes() const
  {
    //Testing shows that, at least on the devices we use,
    //we only lose performance by dyn-par'ing our way to do this.
    return false;
  }

  virtual StatusCode initialize() override
  {
    return CaloGPUCUDAInitialization::initialize();
  }
  
  virtual StatusCode initialize_CUDA() override;
  
  virtual StatusCode finalize() override;
  
 private:

  bool m_dynpar_support = false;
  bool m_coopgroup_support = false;

  struct KernelRecord
  {
    CUDAKernelLaunchConfiguration configs[101];
    void add_configuration(const CUDAKernelLaunchConfiguration & config,
                           const int usage_start = 0,
                           const int usage_end = 100,
                           const bool overwrite = false)
    {
      for (int u = usage_start; u <= usage_end && u <= 100; ++u)
        {
          CUDAKernelLaunchConfiguration & cfg = configs[u];
          if (overwrite || cfg.grid_x <= 0)
            {
              cfg = config;
            }
        }
    }
  };

  std::unordered_map<std::string, std::vector<KernelRecord>> m_kernel_map;

  /** @brief Get the GPU usage, in percentage, rounded to the nearest integer.

   *  @warning Getting GPU usage not yet supported in the current version of the code,
   *           it will default to considering the GPU 100% available. */
  int get_GPU_usage() const
  {
    return 0;
  }

  /** @brief List of JSON files from where to read (hopefully optimized) kernel sizes for different GPUs.
   */
  Gaudi::Property<std::vector<std::string>> m_kernelFiles {this, "KernelSizeInput", {}, "Kernel size input JSON files"};

  /** @brief If @p true, writes the (last used) kernel sizes to an output JSON file.
   *  Defaults to @p true.
   */
  Gaudi::Property<bool> m_outputSizes {this, "OutputSizes", true, "Write out last used kernel sizes"};

  /** @brief If @c m_outputSizes is @p true, the file to which the kernel sizes should be output.
   */
  Gaudi::Property<std::string> m_outputFile {this, "OutputFile", "sizes.json", "Kernel size output file"};
  
  /** @brief If @p true, forces @c can_use_cooperative_groups to always return @p false.
   *  Defaults to @p false.
   *  
   *  GPU algorithms may implement alternative code paths when cooperative groups are not supported.
   *  This allows easier testing of the performance impact of such a fallback.
   */
  Gaudi::Property<bool> m_overrideCooperativeGroups {this, "OverrideCooperativeGroups", false, "Disable cooperative group support (to force fallback to alternative iteration method)."};

  struct KernelsEntry
  {
    struct KernelInfo
    {
      int usage_start = 0;
      int usage_end = 100;
      int grid_x = 0, grid_y = 0, grid_z = 0, block_x = 0, block_y = 0, block_z = 0;

      NLOHMANN_DEFINE_TYPE_INTRUSIVE(KernelInfo, usage_start, usage_end,
                                     grid_x, grid_y, grid_z,
                                     block_x, block_y, block_z)
    };

    std::string device;
    std::string name;
    std::vector< std::vector<KernelInfo> > kernels;

    
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(KernelsEntry, device, name, kernels)
  };

};

#endif
