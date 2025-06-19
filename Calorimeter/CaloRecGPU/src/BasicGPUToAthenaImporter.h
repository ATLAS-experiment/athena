//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

//Dear emacs, this is -*-c++-*-

#ifndef CALORECGPU_BASICGPUTOATHENAIMPORTER_H
#define CALORECGPU_BASICGPUTOATHENAIMPORTER_H

#include "AthenaBaseComps/AthAlgTool.h"

#include "CaloRecGPU/CaloClusterGPUTransformers.h"
#include "StoreGate/ReadHandleKey.h"
#include "CaloRecGPU/CaloGPUTimed.h"

class CaloCell_ID;

/**
 * @class BasicGPUToAthenaImporter
 * @author Nuno Fernandes <nuno.dos.santos.fernandes@cern.ch>
 * @date 30 May 2022
 * @brief Standard tool to convert the GPU data representation back to CPU.
 *
 *
 */

class BasicGPUToAthenaImporter :
  public extends<AthAlgTool, ICaloClusterGPUOutputTransformer>, public CaloGPUTimed
{
 public:

  BasicGPUToAthenaImporter(const std::string & type, const std::string & name, const IInterface * parent);

  virtual StatusCode initialize() override;

  virtual StatusCode convert (const EventContext & ctx, const CaloRecGPU::ConstantDataHolder & constant_data,
                              CaloRecGPU::EventDataHolder & event_data, xAOD::CaloClusterContainer * cluster_collection) const override;

  virtual StatusCode finalize() override;

  virtual ~BasicGPUToAthenaImporter() = default;

 private:

  /** @brief If @p true, do not delete the GPU data representation.
   *  Defaults to @p true.
   *
   */
  Gaudi::Property<bool> m_keepGPUData {this, "KeepGPUData", true, "Keep GPU allocated data"};

  /**
  * @brief if set to true, cluster properties are (re-)calculated using @p CaloClusterKineHelper::calculateKine.
  * Else, the GPU-calculated values are used. Default is @p false.
  */
  Gaudi::Property<bool> m_useCPUPropertiesCalculation {this, "UseCPUClusterPropertiesCalculation", false, "Use CaloClusterKineHelper::calculateKine instead of GPU-calculated cluster properties"};

  /**
   * @brief vector of names of the cell containers to use as input.
   */
  SG::ReadHandleKey<CaloCellContainer> m_cellsKey {this, "CellsName", "", "Name(s) of Cell Containers"};

  /// Cluster size. Should be set accordingly to the threshold.
  Gaudi::Property<std::string> m_clusterSizeString {this, "ClusterSize", "Topo_420", "The size/type of the clusters"};

  xAOD::CaloCluster::ClusterSize m_clusterSize;

  /** @brief Cell indices to fill as disabled cells (useful if the cell vector is always missing the same cells).
   */
  Gaudi::Property<std::vector<int>> m_missingCellsToFill {this, "MissingCellsToFill", {}, "Force fill these cells as disabled on empty containers."};

  /**
   * @brief Pointer to Calo ID Helper
   */
  const CaloCell_ID * m_calo_id {nullptr};

  /**
  * @brief if set to true, the uncalibrated state is saved when importing the clusters. Default is @p true.
  */
  Gaudi::Property<bool> m_saveUncalibrated {this, "SaveUncalibratedSignalState", true, "Use CaloClusterKineHelper::calculateKine instead of GPU-calculated cluster properties"};

};

#endif //CALORECGPU_BASICGPUTOATHENAIMPORTER_H
