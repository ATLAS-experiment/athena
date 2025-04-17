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

//#include "CaloConditions/CaloNoise.h"
//For Two Gaussian Noise comparisons.

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

  /** @brief If @p true, do not delete the CPU version of the GPU-friendly data representation.
   *  Defaults to @p true.
   *
   */
  Gaudi::Property<bool> m_keepCPUData {this, "KeepCPUData", true, "Keep CPU version of GPU data format"};

  /**
   * @brief vector of names of the cell containers to use as input.
   */
  SG::ReadHandleKey<CaloCellContainer> m_cellsKey {this, "CellsName", "", "Name(s) of Cell Containers"};

  /** @brief If @p true, into account the possibility of a cell being shared between clusters.
   *  Hurts performance when not needed.
   *  Defaults to @p true.
   *
   */
  Gaudi::Property<bool> m_considerSharedCells {this, "ConsiderSharedCells", true, "Take into account the possibility of a cell being shared between clusters."};

  /** @brief Cell indices to fill as disabled cells (useful if the cell vector is always missing the same cells).
   */
  Gaudi::Property<std::vector<int>> m_missingCellsToFill {this, "MissingCellsToFill", {}, "Force fill these cells as disabled on empty containers."};

  /**
  * @brief Pointer to Calo ID Helper
  */
  const CaloCell_ID * m_calo_id {nullptr};

  // /** @brief Key of the CaloNoise Conditions data object. Typical values
  //     are '"electronicNoise', 'pileupNoise', or '"totalNoise' (default) */
  //SG::ReadCondHandleKey<CaloNoise> m_noiseCDOKey{this, "CaloNoiseKey", "totalNoise", "SG Key of CaloNoise data object"};
  //For Two Gaussian Noise comparisons.
};

#endif //CALORECGPU_BASICEVENTDATAGPUEXPORTER_H
