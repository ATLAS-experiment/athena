/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CALOREC_CALOCLUSTMLCALIBALG_H
#define CALOREC_CALOCLUSTMLCALIBALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#include "xAODEventInfo/EventInfo.h"
#include "xAODTracking/VertexContainer.h"
#include "CaloClusterCorrection/ICaloClusterMLCalibToolLite.h"

/**
 * @class CaloClusterMLCalibAlgLite
 * @brief An algorithm to apply ML-based energy calibration to calorimeter clusters.
 *
 * This is a placeholder for the algorithm implementation.
 */

class CaloClusterMLCalibAlgLite : public AthReentrantAlgorithm {
public:
  CaloClusterMLCalibAlgLite(const std::string& name, ISvcLocator* pSvcLocator);
  ~CaloClusterMLCalibAlgLite() override = default;

  StatusCode initialize() override;
  StatusCode execute(const EventContext& ctx) const override;
  StatusCode finalize() override;

private:

  ToolHandle<ICaloClusterMLCalibToolLite> m_calibTool{
         this, "CaloClusterMLCalibToolLite", "CaloClusterMLCalibToolLite"
  };


  SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey{this, "EventInfoContainer", "EventInfo", "Input EventInfo container"};
  SG::ReadHandleKey<xAOD::VertexContainer> m_verticesKey{this, "VertexContainer", "PrimaryVertices", "Input vertex container"};
  SG::ReadHandleKey<xAOD::CaloClusterContainer>    m_clusterInputContainerKey   { this, "InputClusterContainer"  , "CaloCalTopoClusters", "input cluster container key"  };
  SG::WriteHandleKey<xAOD::CaloClusterContainer>   m_clusterOutputContainerKey  { this, "OutputClusterContainer" , "CaloCalTopoClusters"       , "output cluster container key" };

  SG::WriteDecorHandleKey<xAOD::CaloClusterContainer> m_clusterMLCalibUncDecorKey{this,"ClusterMLCalibratedEnergyUncKeyName","","ML calibrated cluster energy uncertainty decoration"};

  Gaudi::Property<std::vector<float>> m_rapidityRange { this, "RapidityRange", { -2.5, 2.5}, "rapidity range of validity of the ML-based calibration" };
};

#endif // CALOREC_CALOCLUSTERMLCALIBALG_H
