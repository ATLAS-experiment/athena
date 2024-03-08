//
// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#ifndef CALORECGPU_CALOGPUHYBRIDCLUSTERPROCESSOR_H
#define CALORECGPU_CALOGPUHYBRIDCLUSTERPROCESSOR_H


#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "AthenaMonitoringKernel/GenericMonitoringTool.h"

#include "CxxUtils/checker_macros.h"

#include "CaloUtils/CaloClusterCollectionProcessor.h"
#include "CaloRecGPU/CaloClusterGPUTransformers.h"
#include "CaloRecGPU/CaloClusterGPUProcessor.h"
#include "CaloRecGPU/CaloGPUTimed.h"
#include "CaloRecGPU/DataHolders.h"

#include "xAODCaloEvent/CaloClusterContainer.h"

#include "StoreGate/ReadDecorHandle.h"
#include "StoreGate/WriteDecorHandleKey.h"

#include "CaloConditions/CaloNoise.h"

#include <string>
#include <mutex>
#include <atomic>
#include <utility>

/**
 * @class CaloGPUHybridClusterProcessor
 * @author Nuno Fernandes <nuno.dos.santos.fernandes@cern.ch>
 * @date 27 May 2022
 * @brief Algorithm to reconstruct CaloCluster objects with GPU acceleration,
 * providing interoperability for calling standard CPU algorithms before and after
 * the GPU processing part.
 *
 * This class is meant as a replacement for @c CaloClusterMaker in that
 * it creates a CaloClusterCollection and runs several tools over it.
 * The main addition is the fact that, besides CPU-based tools,
 * GPU-accelerated versions of the standard tools can be run,
 * with adequate memory sharing between them to minimize transfers
 * and (re-)conversions from and to the GPU-friendly data representation.  */

class CaloGPUHybridClusterProcessor : public AthReentrantAlgorithm, public CaloGPUTimed
{
 public:

  CaloGPUHybridClusterProcessor(const std::string & name, ISvcLocator * pSvcLocator);
  virtual ~CaloGPUHybridClusterProcessor() override;
  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext & ctx) const override;
  virtual StatusCode finalize() override;

 private:


  /**
   * @brief The tool that will convert the constant data from the CPU to the GPU.
   *
   */
  ToolHandle<ICaloClusterGPUConstantTransformer> m_transformConstantData{this, "ConstantDataToGPUTool", "", "Tool for transforming the constant data and sending it to the GPU"};

  /**
   * @brief Tools to be applied to the clusters before being sent to the GPU for processing.
   *
   */
  ToolHandleArray<CaloClusterCollectionProcessor> m_preGPUoperations{this, "BeforeGPUTools", {}, "Tools to be applied to the clusters on the CPU before processing them on the GPU"};


  /**
   * @brief The tool that will actually convert the data from the CPU to the GPU.
   *
   */
  ToolHandle<ICaloClusterGPUInputTransformer> m_transformForGPU{this, "EventDataToGPUTool", "", "Tool for transforming the event data and sending it to the GPU"};

  /**
   * @brief Tools to be applied to the clusters on the GPU.
   *
   */
  ToolHandleArray<CaloClusterGPUProcessor> m_GPUoperations{this, "GPUTools", {}, "Tools to be applied to the clusters on the GPU"};

  /**
   * @brief The tool that will convert the data from the GPU back to the CPU.
   *
   */
  ToolHandle<ICaloClusterGPUOutputTransformer> m_transformBackToCPU{this, "GPUToEventDataTool", {}, "Tool for getting the data from the GPU back to the CPU Athena data structures"};

  /**
   * @brief Tools to be applied to the clusters after returning from the GPU.
   *
   */
  ToolHandleArray<CaloClusterCollectionProcessor> m_postGPUoperations{this, "AfterGPUTools", {}, "Tools to be applied to the clusters on the CPU after returning from the GPU"};


  /** @brief If @p true, calls the plotter specified by @p m_plotterTool at every tool execution.
             It should be the plotter's responsibility to only take data from the tools it wants to.
    */
  Gaudi::Property<bool> m_doPlots{this, "DoPlots", false, "Do plots based on the plotter tool optionally provided."};

  /**
   * @brief An optional plotter, for testing and/or debugging purposes.
   *
   */
  ToolHandle<ICaloClusterGPUPlotter> m_plotterTool{this, "PlotterTool", "", "An optional plotter, for testing and/or debugging purposes"};


  /** @brief If @p true, uses the monitoring tool specified by @p m_monitorTool.
    */
  Gaudi::Property<bool> m_doMonitoring{this, "DoMonitoring", false, "Do monitoring."};
  Gaudi::Property<bool> m_monitorCells{this, "MonitorCells", false, "Whether to monitor cells too."};
  Gaudi::Property<float> m_monitoring1thr { this, "Thr1", 2, "First Threshold to pass for cell monitoring" };
  Gaudi::Property<float> m_monitoring2thr { this, "Thr2", 4, "Second Threshold to pass for cell monitoring" };


  // adding noise handle for monitoring purposes
  SG::ReadCondHandleKey<CaloNoise> m_noiseCDOKey{this, "CaloNoiseKey", "totalNoise", "SG Key of CaloNoise data object"};

  /** @brief Monitoring tool.
    */
  ToolHandle<GenericMonitoringTool> m_moniTool { this, "MonitoringTool", "", "Monitoring tool" };

  ///Event input: To get <mu> from Event Info
  SG::ReadDecorHandleKey<xAOD::EventInfo> m_avgMuKey { this, "averageInteractionsPerCrossingKey", "EventInfo.averageInteractionsPerCrossing", "Decoration for Average Interaction Per Crossing" };

  /** @brief If @p true, writes some trigger-specific decorations.
    */
  Gaudi::Property<bool> m_writeTriggerSpecificInfo{this, "WriteTriggerSpecificInfo", false, "Write some trigger-specific decorations and use the trigger auxiliary container."};

  /** @brief Key to the handle for writing the number of cells as a decoration.
    */
  SG::WriteDecorHandleKey<xAOD::CaloClusterContainer> m_mDecor_ncells {this, "Decor_ncells", "nCells", "Decorator containing the number of cells associated to a cluster"};

  /**
   * @brief Number of events for which to pre-allocate space on GPU memory
   * (should ideally be set to the expected number of threads to be run with).
   *
   */
  Gaudi::Property<size_t> m_numPreAllocatedGPUData{this, "NumPreAllocatedDataHolders", 0, "Number of event data holders to pre-allocate on GPU memory"};

  /**
   * @brief vector of names of the cell containers to use as input.
   */
  SG::ReadHandleKey<CaloCellContainer> m_cellsKey {this, "CellsName", "", "Name(s) of Cell Containers"};

  /** @brief The name of the key in StoreGate for the output
      CaloClusterContainer */
  SG::WriteHandleKey<xAOD::CaloClusterContainer> m_clusterOutput {this, "ClustersOutputName", "", "The name of the key in StoreGate for the output CaloClusterContainer"};

  /** @brief The name of the key in StoreGate for the output
      CaloClusterCellLinkContainer */
  SG::WriteHandleKey<CaloClusterCellLinkContainer> m_clusterCellLinkOutput{this, "ClusterCellLinksOutputName", "", "The name of the key in StoreGate for the output CaloClusterCellLinkContainer"};

  /** @brief If @p true, the constant data is only converted and
      sent to the GPU on the first event, in case not all the necessary
      information is available during the @p initialize phase.
      */
  Gaudi::Property<bool> m_deferConstantDataToFirstEvent {this, "DeferConstantDataPreparationToFirstEvent", true, "Convert and send event data on first event instead of during initialize (needed for exporting geometry and noise properly)?"};


  /** @brief If @p true, both constant and event data conversion is skipped.
      */
  Gaudi::Property<bool> m_skipConversions {this, "SkipConversions", false, "If true, skip converting CPU to GPU data (useful if only instanting CPU tools)"};

  /** @brief A way to reduce allocations over multiple threads by keeping a cache
  *   of previously allocated objects that get assigned to the threads as they need them.
  *   It's all thread-safe due to an internal mutex ensuring no objects get assigned to different threads.
  */
  mutable CaloRecGPU::Helpers::separate_thread_holder<CaloRecGPU::EventDataHolder> m_eventDataThreadedHolder ATLAS_THREAD_SAFE;

  /** @class simple_GPU_pointer_holder
      @brief A simple RAII wrapper to ensure proper allocation and deallocation of GPU memory in a @p void *
             for the temporaries.
  */
  struct simple_GPU_pointer_holder
  {
   private:
    void * m_ptr;

   public:

    void allocate(const size_t size)
    {
      if (m_ptr == nullptr && size > 0)
        {
          m_ptr = CaloRecGPU::CUDA_Helpers::allocate(size);
        }
    }

    simple_GPU_pointer_holder():
      m_ptr(nullptr)
    {
    }
    simple_GPU_pointer_holder(const simple_GPU_pointer_holder &) = delete;
    simple_GPU_pointer_holder(simple_GPU_pointer_holder && other)
    {
      m_ptr = other.m_ptr;
      other.m_ptr = nullptr;
    }
    simple_GPU_pointer_holder & operator= (const simple_GPU_pointer_holder &) = delete;

    simple_GPU_pointer_holder & operator= (simple_GPU_pointer_holder && other)
    {
      std::swap(m_ptr, other.m_ptr);
      return (*this);
    }

    ~simple_GPU_pointer_holder()
    {
      if (m_ptr != nullptr)
        //This check might still be needed to ensure the code behaves on non-CUDA enabled platforms
        //where some destructors might still be called with nullptr.
        {
          CaloRecGPU::CUDA_Helpers::deallocate(m_ptr);
        }
    }

    void * operator* ()
    {
      return m_ptr;
    }

    void * get_pointer()
    {
      return m_ptr;
    }

  };

  /** @brief A way to reduce allocations over multiple threads by keeping a cache
  *   of previously allocated objects that get assigned to the threads as they need them.
  *   It's all thread-safe due to an internal mutex ensuring no objects get assigned to different threads.
  */
  mutable CaloRecGPU::Helpers::separate_thread_holder<simple_GPU_pointer_holder> m_temporariesThreadedHolder ATLAS_THREAD_SAFE;

  /** @brief The size of the temporary buffer to allocate for the GPU tools that will be called.

      Will correspond to the maximum of all the necessary sizes for all the GPU tools.

      @warning Every tool should consider the buffer as filled with uninitialized memory at their start.
   */
  size_t m_temporariesSize;

  /** @brief Constant data, common for all events and persisted throughout the run.
   *
   *  Is @p mutable to deal with the cases where the data preparation is deferred to the first event.
   */

  mutable CaloRecGPU::ConstantDataHolder m_constantData ATLAS_THREAD_SAFE;

  /** @brief A flag to signal that the constant data has been adequately sent to the GPU.
   *  This is required for everything to work properly in a multi-threaded context...
   */

  mutable std::atomic<bool> m_constantDataSent;


  /** @brief This mutex is locked when sending the constant data on the first event
    * to ensure thread safety. Otherwise, it's unused.
    */
  mutable std::mutex m_mutex;

};

#endif //CALORECGPU_CALOGPUHYBRIDCLUSTERPROCESSOR_H
