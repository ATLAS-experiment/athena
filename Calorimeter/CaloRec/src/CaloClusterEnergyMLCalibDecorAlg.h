/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#ifndef CALOREC_CALOCLUSTERENERGYMLCALIBDECORALG_H
#define CALOREC_CALOCLUSTERENERGYMLCALIBDECORALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#include "xAODEventInfo/EventInfo.h"
#include "xAODTracking/VertexContainer.h"
#include "CaloClusterCorrection/ICaloClusterMLCalibToolLite.h"
// #include "TFile.h"
// #include "TTree.h"

/**
 * @class CaloClusterEnergyMLCalibDecorAlg
 * @brief An algorithm to apply ML-based energy calibration to calorimeter clusters.
 *
 * This is a placeholder for the algorithm implementation.
 */

class CaloClusterEnergyMLCalibDecorAlg : public AthReentrantAlgorithm
{
public:
    CaloClusterEnergyMLCalibDecorAlg(const std::string &name, ISvcLocator *pSvcLocator);
    ~CaloClusterEnergyMLCalibDecorAlg() override = default;

    StatusCode initialize() override;
    StatusCode execute(const EventContext &ctx) const override;
    StatusCode finalize() override;

private:
    ToolHandle<ICaloClusterMLCalibToolLite> m_calibTool{
        this, "CaloClusterMLCalibToolLite", "CaloClusterMLCalibToolLite"};

    SG::WriteDecorHandleKey<xAOD::CaloClusterContainer> m_clusterMLCalibDecorKey{this, "ClusterMLCalibratedEnergyKeyName", "CaloCalTopoClusters.clusterE_ML", "Name of the ML calib energy decoration"};
    SG::WriteDecorHandleKey<xAOD::CaloClusterContainer> m_clusterMLCalibUncDecorKey{this, "ClusterMLCalibratedEnergyUncKeyName", "CaloCalTopoClusters.clusterE_ML_unc", "Name of the ML calib energy uncertainty decoration"};
    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey{this, "EventInfoContainer", "EventInfo", "Input EventInfo container"};
    SG::ReadHandleKey<xAOD::VertexContainer> m_verticesKey{this, "VertexContainer", "PrimaryVertices", "Input vertex container"};
};

#endif // CALOREC_CALOCLUSTERENERGYMLCALIBDECORALG_H
