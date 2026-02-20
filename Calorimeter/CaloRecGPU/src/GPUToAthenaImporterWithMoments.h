//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

//Dear emacs, this is -*-c++-*-

#ifndef CALORECGPU_GPUTOATHENAIMPORTERWITHMOMENTS_H
#define CALORECGPU_GPUTOATHENAIMPORTERWITHMOMENTS_H

#include "AthenaBaseComps/AthAlgTool.h"

#include "CaloRecGPU/CaloClusterGPUTransformers.h"
#include "CaloRecGPU/CaloGPUTimed.h"
#include "CaloRecGPU/DataHolders.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadCondHandleKey.h"

#include "CaloDetDescr/CaloDetDescrManager.h"
#include "LArCabling/LArOnOffIdMapping.h"
#include "LArElecCalib/ILArHVScaleCorr.h"

class CaloCell_ID;

/**
 * @class GPUToAthenaImporterWithMoments
 * @author Nuno Fernandes <nuno.dos.santos.fernandes@cern.ch>
 * @date 30 May 2022
 * @brief Tool to convert the GPU data representation back to CPU, with selected moments too.
 *
 */

class GPUToAthenaImporterWithMoments :
  public extends<AthAlgTool, ICaloClusterGPUOutputTransformer>, public CaloGPUTimed
{
 public:

  GPUToAthenaImporterWithMoments(const std::string & type, const std::string & name, const IInterface * parent);

  virtual StatusCode initialize() override;

  virtual StatusCode convert (const EventContext & ctx, const CaloRecGPU::ConstantDataHolder & constant_data,
                              CaloRecGPU::EventDataHolder & event_data, xAOD::CaloClusterContainer * cluster_collection) const override;

  virtual StatusCode finalize() override;

  virtual ~GPUToAthenaImporterWithMoments() = default;

 private:

  /** @brief If @p true, do not delete the GPU data representation.
   *  Defaults to @p true.
   *
   */
  Gaudi::Property<bool> m_keepGPUData {this, "KeepGPUData", true, "Keep GPU allocated data"};

  /**
   * @brief vector of names of the cell containers to use as input.
   */
  SG::ReadHandleKey<CaloCellContainer> m_cellsKey {this, "CellsName", "", "Name(s) of Cell Containers"};

  /// Cluster size. Should be set accordingly to the threshold.
  Gaudi::Property<std::string> m_clusterSizeString {this, "ClusterSize", "Topo_420", "The size/type of the clusters"};

  xAOD::CaloCluster::ClusterSize m_clusterSize;

  /**
   * @brief Pointer to Calo ID Helper
   */
  const CaloCell_ID * m_calo_id {nullptr};

  /**
   * @brief Key for the CaloDetDescrManager in the Condition Store
   */
  SG::ReadCondHandleKey<CaloDetDescrManager> m_caloMgrKey{this, "CaloDetDescrManager", "CaloDetDescrManager",
    "SG Key for CaloDetDescrManager in the Condition Store"};

  //Handles for things we can't (yet) do on the GPU.

  /**
  * @brief if set to true, fill the HV-related moments using the respective tools.
  */
  Gaudi::Property<bool> m_fillHVMoments {this, "FillHVMoments", false, "Fill the HV-related moments using the respective tools."};
  
  ///@brief Cabling for the CPU-based HV moments calculation.
   SG::ReadCondHandleKey<LArOnOffIdMapping> m_HVCablingKey{this, "LArCablingKey","LArOnOffIdMap","SG Key of LAr Cabling object"};
 
 ///@brief HV corrections for the CPU-based HV moments.
   SG::ReadCondHandleKey<ILArHVScaleCorr> m_HVScaleKey{this,"HVScaleCorrKey","LArHVScaleCorr","SG key of HVScaleCorr conditions object"};
 
  ///@brief Threshold above which a cell contributes to the HV moments.
   Gaudi::Property<float> m_HVthreshold{this,"HVThreshold",0.2,"Threshold to consider a cell 'affected' by HV issues"};

  /** @brief Cell indices to fill as disabled cells (useful if the cell vector is always missing the same cells).
   */
  Gaudi::Property<std::vector<int>> m_missingCellsToFill {this, "MissingCellsToFill", {}, "Force fill these cells as disabled on empty containers."};

  /**
  * @brief if set to true, the uncalibrated state is saved when importing the clusters. Default is @p true.
  */
  Gaudi::Property<bool> m_saveUncalibrated {this, "SaveUncalibratedSignalState", true, "Use CaloClusterKineHelper::calculateKine instead of GPU-calculated cluster properties"};


  /**
   * @brief vector holding the input list of names of moments to
   * calculate.
   *
   * This is the list of desired names of moments given in the
   * jobOptions.*/
  Gaudi::Property<std::vector<std::string>> m_momentsNames{this, "MomentsNames", {}, "List of names of moments to calculate"};

  /** @brief Holds (in a linearized way) the moments and whether to add them to the clusters.
             (on the GPU side, they are unconditionally calculated).
  */
  CaloRecGPU::MomentsOptionsArray m_momentsToDo;


  ///@brief To abbreviate checks of @p m_momentsToDo...
  bool m_doHVMoments;

};

#endif //CALORECGPU_GPUTOATHENAIMPORTERWITHMOMENTS_H
