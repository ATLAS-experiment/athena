//
// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#ifndef CALORECGPU_CALOCLUSTERGPUTRANSFORMERS_H
#define CALORECGPU_CALOCLUSTERGPUTRANSFORMERS_H


#include "AthenaBaseComps/AthAlgTool.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#include "CaloRecGPU/DataHolders.h"

/**
 * @class  ICaloClusterGPUConstantTransformer
 * @author Nuno Fernandes <nuno.dos.santos.fernandes@cern.ch>
 * @date 27 May 2022
 * @brief Base class for tools that convert constant information
 * to the GPU-friendly format used in @c CaloGPUHybridClusterProcessor.
 *
 * This class defines a @c convert method that takes as an argument
 * a @c CaloRecGPU::ConstantDataHolder.
 */

class ICaloClusterGPUConstantTransformer : virtual public IAlgTool
{
 public:

  /**
   * @brief Fill the @ConstantDataHolder with the relevant information.
   * @param constant_data Data held in GPU memory that is common to all events (cell noise and geometry).
   * @param override_keep_CPU_info If @p true, keep CPU info regardless of the default behaviour of the tool.
   */
  virtual StatusCode convert (CaloRecGPU::ConstantDataHolder & constant_data, const bool override_keep_CPU_info) const = 0;

  /**
   * @brief Fill the @CaloRecGPU::ConstantDataHolder with the relevant information at the first event.
   * @param ctx The event context.
   * @param constant_data Data held in GPU memory that is common to all events (cell noise and geometry).
   * @param override_keep_CPU_info If @p true, keep CPU info regardless of the default behaviour of the tool.
   *
   * Unless otherwise specified by the tool, this does exactly the same as @c convert(constant_data)
   */
  inline virtual StatusCode convert (const EventContext &, CaloRecGPU::ConstantDataHolder & constant_data, const bool override_keep_CPU_info) const
  {
    return this->convert(constant_data, override_keep_CPU_info);
  }

  DeclareInterfaceID( ICaloClusterGPUConstantTransformer, 1, 0);
};

/**
 * @class  ICaloClusterGPUInputTransformer
 * @author Nuno Fernandes <nuno.dos.santos.fernandes@cern.ch>
 * @date 27 May 2022
 * @brief Base class for tools that convert event information from the Athena structures
 * to the GPU-friendly format used in @c CaloGPUHybridClusterProcessor.
 *
 * This class defines a @c convert method that takes as an argument
 * a @c EventContext, a @c CaloRecGPU::ConstantDataHolder, a @c xAOD::CaloClusterContainer
 * and a @c CaloRecGPU::EventDataHolder.
 */

class ICaloClusterGPUInputTransformer : virtual public IAlgTool
{
 public:

  /**
   * @brief Fill the @p CaloRecGPU::EventDataHolder with the relevant information.
   * @param ctx The event context.
   * @param constant_data Data held in GPU memory that is common to all events (cell noise and geometry).
   * @param cluster_collection The cluster collection, in the standard Athena structures.
   * @param event_data Data held in GPU memory that is specific to this event (including the description of the clusters themselves).
   */
  virtual StatusCode convert (const EventContext & ctx, const CaloRecGPU::ConstantDataHolder & constant_data,
                              const xAOD::CaloClusterContainer * cluster_collection, CaloRecGPU::EventDataHolder & event_data) const = 0;

  DeclareInterfaceID( ICaloClusterGPUInputTransformer, 1, 0);
};

/**
 * @class  ICaloClusterGPUOutputTransformer
 * @author Nuno Fernandes <nuno.dos.santos.fernandes@cern.ch>
 * @date 27 May 2022
 * @brief Base class for tools that convert event information from the GPU-friendly format
 * used in @c CaloGPUHybridClusterProcessor to the Athena structures.
 *
 * This class defines a @c convert method that takes as an argument
 * a @c EventContext, a @c CaloRecGPU::ConstantDataHolder, a @c CaloRecGPU::EventDataHolder and a @xAOD::CaloClusterContainer.
 */

class ICaloClusterGPUOutputTransformer : virtual public IAlgTool
{
 public:

  /**
   * @brief Fill the @xAOD::CaloClusterContainer with the relevant information.
   * @param ctx The event context.
   * @param constant_data Data held in GPU memory that is common to all events (cell noise and geometry).
   * @param event_data Data held in GPU memory that is specific to this event (including the description of the clusters themselves).
   * @param cluster_collection The cluster collection, in the standard Athena structures.
   */
  virtual StatusCode convert (const EventContext & ctx, const CaloRecGPU::ConstantDataHolder & constant_data,
                              CaloRecGPU::EventDataHolder & event_data, xAOD::CaloClusterContainer * cluster_collection) const = 0;

  DeclareInterfaceID( ICaloClusterGPUOutputTransformer, 1, 0);
};


class CaloClusterCollectionProcessor;
class CaloClusterGPUProcessor;

/**
 * @class  ICaloClusterGPUPlotter
 * @author Nuno Fernandes <nuno.dos.santos.fernandes@cern.ch>
 * @date 09 November 2022
 * @brief Base class for tools that can be used to plot events within @c CaloGPUHybridClusterProcessor.
 *
 * This class should define several overloads of an @c update_plots function,
 * which is called after each tool has ran, an @c update_plots_start and @update_plots_end,
 * and a @c finalize_plots function, potentially to actually output the plots in the end.
 *
 */

class ICaloClusterGPUPlotter : virtual public IAlgTool
{
 public:
  virtual StatusCode update_plots_start(const EventContext & ctx,
                                        const CaloRecGPU::ConstantDataHolder & constant_data,
                                        const xAOD::CaloClusterContainer * cluster_collection_ptr) const = 0;

  virtual StatusCode update_plots_end(const EventContext & ctx,
                                      const CaloRecGPU::ConstantDataHolder & constant_data,
                                      const xAOD::CaloClusterContainer * cluster_collection_ptr) const = 0;

  virtual StatusCode update_plots(const EventContext & ctx,
                                  const CaloRecGPU::ConstantDataHolder & constant_data,
                                  const xAOD::CaloClusterContainer * cluster_collection_ptr,
                                  const CaloClusterCollectionProcessor * tool) const = 0;

  virtual StatusCode update_plots(const EventContext & ctx,
                                  const CaloRecGPU::ConstantDataHolder & constant_data,
                                  const xAOD::CaloClusterContainer * cluster_collection_ptr,
                                  const CaloRecGPU::EventDataHolder & event_data,
                                  const ICaloClusterGPUInputTransformer * tool) const = 0;

  virtual StatusCode update_plots(const EventContext & ctx,
                                  const CaloRecGPU::ConstantDataHolder & constant_data,
                                  const xAOD::CaloClusterContainer * cluster_collection_ptr,
                                  const CaloRecGPU::EventDataHolder & event_data,
                                  const CaloClusterGPUProcessor * tool) const = 0;

  virtual StatusCode update_plots(const EventContext & ctx,
                                  const CaloRecGPU::ConstantDataHolder & constant_data,
                                  const xAOD::CaloClusterContainer * cluster_collection_ptr,
                                  const CaloRecGPU::EventDataHolder & event_data,
                                  const ICaloClusterGPUOutputTransformer * tool) const = 0;

  virtual StatusCode finalize_plots() const = 0;
  
  DeclareInterfaceID( ICaloClusterGPUPlotter, 1, 0);
};

#endif //CALORECGPU_CALOCLUSTERGPUTRANSFORMERS_H
