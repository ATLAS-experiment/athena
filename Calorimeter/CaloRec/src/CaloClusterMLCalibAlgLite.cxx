/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "CaloClusterMLCalibAlgLite.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteDecorHandle.h"
#include "CaloUtils/CaloClusterStoreHelper.h"

CaloClusterMLCalibAlgLite::CaloClusterMLCalibAlgLite(const std::string &name, ISvcLocator *pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator)
{
}

StatusCode CaloClusterMLCalibAlgLite::initialize()
{
    ATH_MSG_INFO("Initializing " << name() << "...");

    ATH_CHECK(m_calibTool.retrieve());
    ATH_CHECK(m_eventInfoKey.initialize());
    ATH_CHECK(m_verticesKey.initialize());
    ATH_CHECK(m_clusterInputContainerKey.initialize());
    ATH_CHECK(m_clusterOutputContainerKey.initialize());
    ATH_CHECK(m_clusterMLCalibUncDecorKey.initialize());

    ATH_MSG_INFO("ML calibration will be applied for clusters within [" << m_rapidityRange[0] << "," << m_rapidityRange[1] << "]");

    return StatusCode::SUCCESS;
}

StatusCode CaloClusterMLCalibAlgLite::finalize()
{
    ATH_MSG_INFO("Finalizing " << name() << "...");
    return StatusCode::SUCCESS;
}

StatusCode CaloClusterMLCalibAlgLite::execute(const EventContext &ctx) const
{
    ATH_MSG_DEBUG("Executing " << name() << "...");

    SG::WriteDecorHandle<xAOD::CaloClusterContainer, double> clusterMLCalibUncDecor(m_clusterMLCalibUncDecorKey, ctx);

    // -- get the input
    SG::ReadHandle<xAOD::CaloClusterContainer> clusterIn(m_clusterInputContainerKey, ctx);
    if (!clusterIn.isValid())
    {
        ATH_MSG_ERROR("cannot allocate the input cluster container with key <" << m_clusterInputContainerKey.key() << ">");
        return StatusCode::FAILURE;
    }
    // -- prepare the output
    SG::WriteHandle<xAOD::CaloClusterContainer> clusterOut(m_clusterOutputContainerKey, ctx);

    // AddContainerWriteHandle will attempt to record/create the container
    ATH_CHECK(CaloClusterStoreHelper::AddContainerWriteHandle(clusterOut));

    // Sanity check: after successful AddContainerWriteHandle the handle should be valid
    if (!clusterOut.isValid())
    {
        ATH_MSG_ERROR("cannot allocate the output cluster container with key <" << m_clusterOutputContainerKey.key() << "> after recording");
        return StatusCode::FAILURE;
    }
    // -- copy from the input
    CaloClusterStoreHelper::copyContainer(clusterIn.cptr(), clusterOut.ptr());

    double nPrimVtx = 0;
    double avgMu = 0;

    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_eventInfoKey, ctx);
    SG::ReadHandle<xAOD::VertexContainer> vertices(m_verticesKey, ctx);

    avgMu = eventInfo->actualInteractionsPerCrossing();
    for (auto vtx : *vertices)
    {
        if (vtx->vertexType() == xAOD::VxType::PriVtx || vtx->vertexType() == xAOD::VxType::PileUp)
            ++nPrimVtx;
    }

    std::vector<double> clusterE_ML_vec;
    std::vector<double> clusterE_ML_Unc_vec;

    ATH_CHECK(m_calibTool->inference(*clusterOut, nPrimVtx, avgMu, clusterE_ML_vec, clusterE_ML_Unc_vec));
    
    int i = 0;
    for (xAOD::CaloCluster *cluster : *clusterOut)
    {
        bool inAcc = false;
        if (m_rapidityRange.size() == 2)
        {
            const double eta = cluster->eta(xAOD::CaloCluster::UNCALIBRATED);
            inAcc = (eta >= m_rapidityRange[0] && eta <= m_rapidityRange[1]);
        }

        if (inAcc)
        {
            cluster->setAltE(clusterE_ML_vec[i]);
            clusterMLCalibUncDecor(*cluster) = clusterE_ML_Unc_vec[i];
        }
        else
        {
            cluster->setAltE(cluster->rawE());
            clusterMLCalibUncDecor(*cluster) = 0.0;
        }
        
        cluster->setAltEta(cluster->rawEta());
        cluster->setAltPhi(cluster->rawPhi());
        cluster->setAltM(cluster->rawM());

        i++;
    }

    return StatusCode::SUCCESS;
}
