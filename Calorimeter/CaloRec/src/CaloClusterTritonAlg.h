/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CALOREC_CALOCLUSTERTRITONALG_H
#define CALOREC_CALOCLUSTERTRITONALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#include "CaloEvent/CaloCellContainer.h"
#include "CaloInterface/ICaloClusterTritonTool.h"

#include <vector>

/**
 * @class CaloClusterTritonAlg
 * @brief Applies GNN-based (Triton-served) energy calibration to calorimeter
 *        clusters and stores the result as a decoration [MeV].
 *
 * Clusters outside the rapidity range or below the minimum raw energy are
 * decorated with their raw (EM-scale) energy.
 */
class CaloClusterTritonAlg : public AthReentrantAlgorithm {
public:
  CaloClusterTritonAlg(const std::string& name, ISvcLocator* pSvcLocator);
  ~CaloClusterTritonAlg() override = default;

  StatusCode initialize() override;
  StatusCode execute(const EventContext& ctx) const override;
  StatusCode finalize() override;

private:
  ToolHandle<ICaloClusterTritonTool> m_calibTool{
      this, "CaloClusterTritonTool", "CaloClusterTritonTool"};

  SG::ReadHandleKey<xAOD::CaloClusterContainer> m_clusterContainerKey{
      this, "ClusterContainer", "CaloCalTopoClusters", "Cluster container key"};

  // Cell container referenced by the cluster cell links. Declared so the
  // scheduler makes it available before this algorithm runs.
  SG::ReadHandleKey<CaloCellContainer> m_cellContainerKey{
      this, "CellContainer", "AllCalo", "Cell container referenced by the cluster cell links"};

  SG::WriteDecorHandleKey<xAOD::CaloClusterContainer> m_clusterGNNCalibEnergyDecorKey{
      this, "ClusterGNNCalibratedEnergyKeyName", "CaloCalTopoClusters.GNNCalibratedEnergy",
      "GNN calibrated cluster energy decoration"};

  Gaudi::Property<std::vector<float>> m_rapidityRange{
      this, "RapidityRange", {-2.5, 2.5}, "Rapidity range of validity of the GNN calibration"};

  // Minimum cluster energy in MeV above which GNN calibration will be applied.
  Gaudi::Property<double> m_minClusterEnergy{
      this, "MinClusterEnergy", 300.0, "Minimum cluster energy (MeV) to apply GNN calibration"};
};

#endif // CALOREC_CALOCLUSTERTRITONALG_H