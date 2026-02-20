//
// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#include "GPUKernelSizeOptimizerSvc.h"
#include "CaloRecGPU/Helpers.h"

#include <fstream>

GPUKernelSizeOptimizerSvc::GPUKernelSizeOptimizerSvc(const std::string & name, ISvcLocator * svc):
  base_class(name, svc)
{
}

void GPUKernelSizeOptimizerSvc::register_kernels(const std::string & tool_name,
                                                 const int number,
                                                 void ** kernels,
                                                 const int * /*blocksize_hints*/,
                                                 const int * gridsize_hints,
                                                 const int * max_total_threads,
                                                 const int offset)
{
  ATH_MSG_INFO("Registering " << number << " kernels under: " << tool_name);
  
  std::vector<KernelRecord> & vect = m_kernel_map[tool_name];

  if (int(vect.size()) < number + offset)
    {
      vect.resize(number + offset);
    }

  for (int i = 0; i < number; ++i)
    {      
      CUDAKernelLaunchConfiguration cfg{1, 1, 1, 1, 1, 1};
      if (gridsize_hints[i] == IGPUKernelSizeOptimizer::SpecialSizeHints::CooperativeLaunch)
        {
          CaloRecGPU::CUDA_Helpers::optimize_block_and_grid_size_for_cooperative_launch(kernels[i], cfg.block_x, cfg.grid_x);
        }
      else
        {
          CaloRecGPU::CUDA_Helpers::optimize_block_and_grid_size(kernels[i], cfg.block_x, cfg.grid_x);
        }
      cfg.grid_x = std::min(cfg.grid_x, CaloRecGPU::Helpers::int_ceil_div(max_total_threads[i], cfg.block_x));
      vect[i + offset].add_configuration(cfg);
    }
}


CUDAKernelLaunchConfiguration GPUKernelSizeOptimizerSvc::get_launch_configuration(const std::string & name, int number, const int /*dynamic_memory*/) const
{
  auto it = m_kernel_map.find(name);
  if (it != m_kernel_map.end() && int(it->second.size()) > number)
    {
      const int usage = get_GPU_usage();
      return it->second[number].configs[usage];
    }
  else
    {
      return {};
    }
}


StatusCode GPUKernelSizeOptimizerSvc::initialize_CUDA()
{
  m_dynpar_support = CaloRecGPU::CUDA_Helpers::supports_dynamic_parallelism();
  m_coopgroup_support = CaloRecGPU::CUDA_Helpers::supports_cooperative_launches();

  const std::string device_name = CaloRecGPU::CUDA_Helpers::GPU_name();

  for (const auto & file : m_kernelFiles)
    {
      std::ifstream in(file);

      if (!in.is_open())
        {
          ATH_MSG_WARNING("Cannot open '" << m_outputFile << "' for kernel size input.");
          continue;
        }

      nlohmann::json j;
      in >> j;

      for (const auto & entry : j)
        {
          if (entry.at("device") != device_name)
            {
              continue;
            }
          const KernelsEntry ke = entry.template get<KernelsEntry>();

          std::vector<KernelRecord> & vect = m_kernel_map[ke.name];
          vect.resize(ke.kernels.size());

          for (size_t i = 0; i < vect.size(); ++i)
            {
              for (const auto & ki : ke.kernels[i])
                {
                  CUDAKernelLaunchConfiguration config;
                  config.grid_x = ki.grid_x;
                  config.grid_y = ki.grid_y;
                  config.grid_z = ki.grid_z;
                  config.block_x = ki.block_x;
                  config.block_y = ki.block_y;
                  config.block_z = ki.block_z;

                  vect[i].add_configuration(config, ki.usage_start, ki.usage_end, true);
                }
            }
        }

    }

  return StatusCode::SUCCESS;
}

StatusCode GPUKernelSizeOptimizerSvc::finalize()
{
  if (m_outputSizes && m_kernel_map.size() > 0)
    {
      std::ofstream output(m_outputFile);

      auto delta_configs = [](const CUDAKernelLaunchConfiguration & a, const KernelsEntry::KernelInfo & b) -> bool
      {
        return ( a.grid_x  != b.grid_x  ) ||
        ( a.grid_y  != b.grid_y  ) ||
        ( a.grid_z  != b.grid_z  ) ||
        ( a.block_x != b.block_x ) ||
        ( a.block_y != b.block_y ) ||
        ( a.block_z != b.block_z );
      };

      if (output.is_open())
        {
          output << "[\n";
          const std::string device_name = CaloRecGPU::CUDA_Helpers::GPU_name();
          bool first = true;
          for (const auto & pair : m_kernel_map)
            {
              if (first)
                {
                  first = false;
                }
              else
                {
                  output << ",\n";
                }

              KernelsEntry ke;
              ke.device = device_name;
              ke.name = pair.first;
              ke.kernels.resize(pair.second.size());

              for (size_t i = 0; i < ke.kernels.size(); ++i)
                {
                  const KernelRecord & kr = pair.second[i];

                  KernelsEntry::KernelInfo ki;
                  for (int u = 0; u <= 100; ++u)
                    {
                      const CUDAKernelLaunchConfiguration & cfg = kr.configs[u];
                      if (delta_configs(cfg, ki))
                        {
                          if (ki.grid_x > 0)
                            {
                              ki.usage_end = u - 1;
                              ke.kernels[i].push_back(ki);
                            }
                          ki.usage_start = u;
                          ki.grid_x = cfg.grid_x;
                          ki.grid_y = cfg.grid_y;
                          ki.grid_z = cfg.grid_z;
                          ki.block_x = cfg.block_x;
                          ki.block_y = cfg.block_y;
                          ki.block_z = cfg.block_z;
                        }
                    }
                  if (ki.grid_x > 0)
                    {
                      ki.usage_end = 100;
                      ke.kernels[i].push_back(ki);
                    }

                }

              nlohmann::json j = ke;

              output << j.dump(2);

            }
          output << "\n]" << std::endl;
        }
      else
        {
          ATH_MSG_WARNING("Cannot open '" << m_outputFile << "' for kernel size output.");
        }
    }
  return StatusCode::SUCCESS;
}
