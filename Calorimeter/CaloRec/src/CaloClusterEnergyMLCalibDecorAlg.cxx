/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#include "CaloClusterEnergyMLCalibDecorAlg.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteDecorHandle.h"
#include "GaudiKernel/SystemOfUnits.h"

CaloClusterEnergyMLCalibDecorAlg::CaloClusterEnergyMLCalibDecorAlg(const std::string &name, ISvcLocator *pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator)
{
}

StatusCode CaloClusterEnergyMLCalibDecorAlg::initialize()
{
    ATH_MSG_INFO("Initializing " << name() << "...");

    ATH_CHECK(m_calibTool.retrieve());
    ATH_CHECK(m_clusterMLCalibDecorKey.initialize());
    ATH_CHECK(m_clusterMLCalibUncDecorKey.initialize());
    ATH_CHECK(m_eventInfoKey.initialize());
    ATH_CHECK(m_verticesKey.initialize());

    return StatusCode::SUCCESS;
}

StatusCode CaloClusterEnergyMLCalibDecorAlg::finalize()
{
    ATH_MSG_INFO("Finalizing " << name() << "...");
    return StatusCode::SUCCESS;
}

StatusCode CaloClusterEnergyMLCalibDecorAlg::execute(const EventContext &ctx) const
{
    ATH_MSG_DEBUG("Executing " << name() << "...");

    SG::WriteDecorHandle<xAOD::CaloClusterContainer, double> clusterMLCalibDecor(m_clusterMLCalibDecorKey, ctx);
    SG::WriteDecorHandle<xAOD::CaloClusterContainer, double> clusterMLCalibUncDecor(m_clusterMLCalibUncDecorKey, ctx);
    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_eventInfoKey, ctx);
    SG::ReadHandle<xAOD::VertexContainer> vertices(m_verticesKey, ctx);

    double nPrimVtx = 0;
    double avgMu = 0;
    avgMu = eventInfo->actualInteractionsPerCrossing();
    for (auto vtx : *vertices)
    {
        if (vtx->vertexType() == xAOD::VxType::PriVtx || vtx->vertexType() == xAOD::VxType::PileUp)
            ++nPrimVtx;
    }

    std::vector<double> clusterE_ML_vec;
    std::vector<double> clusterE_ML_Unc_vec;

    ATH_CHECK(m_calibTool->inference(*clusterMLCalibDecor, nPrimVtx, avgMu, clusterE_ML_vec, clusterE_ML_Unc_vec));

    int i = 0;
    for (const xAOD::CaloCluster *cluster : *clusterMLCalibDecor)
    {
        clusterMLCalibDecor(*cluster) = clusterE_ML_vec[i];
        clusterMLCalibUncDecor(*cluster) = clusterE_ML_Unc_vec[i];
        i++;
    }

    return StatusCode::SUCCESS;
}
