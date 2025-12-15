/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "CaloClusterMLCalibAlgLite.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteDecorHandle.h"
#include "AthContainers/ConstDataVector.h"

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
    ATH_CHECK(m_clusterContainerKey.initialize());
    ATH_CHECK(m_clusterMLCalibEnergyDecorKey.initialize());
    ATH_CHECK(m_clusterMLCalibEnergyUncDecorKey.initialize());

    ATH_MSG_INFO("ML calibration will be applied for clusters within [" << m_rapidityRange[0] << "," << m_rapidityRange[1] << "]");
    ATH_MSG_INFO("ML calibration will be applied for clusters with energy >= " << m_minClusterEnergy << " MeV");

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

    SG::WriteDecorHandle<xAOD::CaloClusterContainer, double> clusterMLCalibEnergyDecor(m_clusterMLCalibEnergyDecorKey, ctx);
    SG::WriteDecorHandle<xAOD::CaloClusterContainer, double> clusterMLCalibEnergyUncDecor(m_clusterMLCalibEnergyUncDecorKey, ctx);

    // -- get the input
    SG::ReadHandle<xAOD::CaloClusterContainer> clusterReadHandle(m_clusterContainerKey, ctx);
    if (!clusterReadHandle.isValid())
    {
        ATH_MSG_ERROR("cannot allocate the input cluster container with key <" << m_clusterContainerKey.key() << ">");
        return StatusCode::FAILURE;
    }

    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_eventInfoKey, ctx);
    SG::ReadHandle<xAOD::VertexContainer> vertices(m_verticesKey, ctx);

    float avgMu = eventInfo->actualInteractionsPerCrossing();
    int nPrimVtx = 0;
    for (auto vtx : *vertices)
    {
        if (vtx->vertexType() == xAOD::VxType::PriVtx || vtx->vertexType() == xAOD::VxType::PileUp)
            ++nPrimVtx;
    }

    // only run inference for clusters passing cuts
    ConstDataVector<xAOD::CaloClusterContainer> selectedClusters(SG::VIEW_ELEMENTS);
    std::vector<bool> clusterMask(clusterReadHandle->size(), false);
    for (const xAOD::CaloCluster *cluster : *clusterReadHandle) {
      if (m_rapidityRange.size() == 2) {
	const double eta = cluster->eta(xAOD::CaloCluster::UNCALIBRATED);
	if (eta < m_rapidityRange[0] || eta > m_rapidityRange[1]) continue;
      }
      // minimum cluster energy cut; in MeV
      const double energy = cluster->rawE();
      if (energy < m_minClusterEnergy) continue;

      selectedClusters.push_back(cluster);
      clusterMask.at(cluster->index()) = true;
    }

    std::vector<double> clusterE_ML_vec;
    std::vector<double> clusterE_ML_Unc_vec;

    ATH_CHECK(m_calibTool->inference(*selectedClusters.asDataVector(), nPrimVtx, avgMu, clusterE_ML_vec, clusterE_ML_Unc_vec));
    
    size_t i = 0;
    for (const xAOD::CaloCluster *cluster : *clusterReadHandle)
    {
      if (clusterMask.at(cluster->index()))
        {
	  clusterMLCalibEnergyDecor(*cluster) = clusterE_ML_vec[i];
	  clusterMLCalibEnergyUncDecor(*cluster) = clusterE_ML_Unc_vec[i];
	  ++i;
        }
        else
        {
            clusterMLCalibEnergyDecor(*cluster) = cluster->rawE();
            clusterMLCalibEnergyUncDecor(*cluster) = 0.0;
        }
    }

    return StatusCode::SUCCESS;
}
