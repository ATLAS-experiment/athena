//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#ifndef CALORECGPU_BASICEVENTDATAGPUEXPORTER_H
#define CALORECGPU_BASICEVENTDATAGPUEXPORTER_H

#include "AthenaBaseComps/AthAlgTool.h"

#include "CaloRecGPU/CaloClusterGPUTransformers.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "CaloRecGPU/CaloGPUTimed.h"

class CaloCell_ID;

/**
 * @class BasicEventDataGPUExporter
 * @author Nuno Fernandes <nuno.dos.santos.fernandes@cern.ch>
 * @date 29 May 2022
 * @brief Standard tool to export cell energy and gain to the GPU.
 *
 */

class BasicEventDataGPUExporter :
  public extends<AthAlgTool, ICaloClusterGPUInputTransformer>, public CaloGPUTimed
{
 public:

  BasicEventDataGPUExporter(const std::string & type, const std::string & name, const IInterface * parent);

  virtual StatusCode initialize() override;

  virtual StatusCode convert (const EventContext & ctx, const CaloRecGPU::ConstantDataHolder & constant_data,
                              const xAOD::CaloClusterContainer * cluster_collection, CaloRecGPU::EventDataHolder & event_data) const override;

  virtual StatusCode finalize() override;

  virtual ~BasicEventDataGPUExporter() = default;

 private:

  /**
   * @brief vector of names of the cell containers to use as input.
   */
  SG::ReadHandleKey<CaloCellContainer> m_cellsKey {this, "CellsName", "", "Name(s) of Cell Containers"};

  /** @brief If @p true, also stores and sends the cluster moments.
   *  Hurts performance when not needed.
   */
  Gaudi::Property<bool> m_outputMoments {this, "OutputMoments", false, "Output cluster moments too."};
  
  /** @brief If @p true and `OutputMoments` is also @p true, send additional moments
   *  that are not calculated as part of topological clustering.
   */
  Gaudi::Property<bool> m_outputExtraMoments {this, "OutputExtraMoments", false,
                                              "Output cluster moments that are not calculated as part of topological clustering (e. g. local calibration probabilities)."};
  
  
  /** @brief If @p true, store cell assignment information in the (generic) tags instead of a list.
   *  Will skip storing cluster moments.
   *  Defaults to @p true.
   */
  Gaudi::Property<bool> m_outputCombinedTags {this, "OutputTags", true, "Whether to output the cell assignment as cluster tags instead of having the list. Also skips moments."};

  /** @brief If @p true and `OutputTags` is also @p true, into account the possibility of a cell being shared between clusters.
   *  Hurts performance when not needed.
   *  Defaults to @p true.
   */
  Gaudi::Property<bool> m_considerSharedCells {this, "ConsiderSharedCells", true, "Take into account the possibility of a cell being shared between clusters."};
  
  /** @brief Cell indices to fill as disabled cells (useful if the cell vector is always missing the same cells).
   */
  Gaudi::Property<std::vector<int>> m_missingCellsToFill {this, "MissingCellsToFill", {}, "Force fill these cells as disabled on empty containers."};

};

#endif //CALORECGPU_BASICEVENTDATAGPUEXPORTER_H
