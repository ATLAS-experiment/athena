/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "CaloClusterTritonAlg.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteDecorHandle.h"
#include "AthContainers/ConstDataVector.h"

CaloClusterTritonAlg::CaloClusterTritonAlg(const std::string &name, ISvcLocator *pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator)
{
}

StatusCode CaloClusterTritonAlg::initialize()
{
    ATH_MSG_INFO("Initializing " << name() << "...");

    ATH_CHECK(m_calibTool.retrieve());
    ATH_CHECK(m_clusterContainerKey.initialize());
    ATH_CHECK(m_cellContainerKey.initialize());
    ATH_CHECK(m_clusterGNNCalibEnergyDecorKey.initialize());

    if (m_rapidityRange.size() == 2) {
        ATH_MSG_INFO("GNN calibration will be applied for clusters within ["
                     << m_rapidityRange[0] << "," << m_rapidityRange[1] << "]");
    }
    ATH_MSG_INFO("GNN calibration will be applied to clusters with energy >= " << m_minClusterEnergy << " MeV");

    return StatusCode::SUCCESS;
}

StatusCode CaloClusterTritonAlg::finalize()
{
    ATH_MSG_INFO("Finalizing " << name() << "...");
    return StatusCode::SUCCESS;
}

StatusCode CaloClusterTritonAlg::execute(const EventContext &ctx) const
{
    ATH_MSG_DEBUG("Executing " << name() << "...");

    SG::WriteDecorHandle<xAOD::CaloClusterContainer, double> clusterGNNCalibEnergyDecor(m_clusterGNNCalibEnergyDecorKey, ctx);

    SG::ReadHandle<xAOD::CaloClusterContainer> clusterReadHandle(m_clusterContainerKey, ctx);
    if (!clusterReadHandle.isValid())
    {
        ATH_MSG_ERROR("cannot allocate the input cluster container with key <" << m_clusterContainerKey.key() << ">");
        return StatusCode::FAILURE;
    }

    SG::ReadHandle<CaloCellContainer> cellReadHandle(m_cellContainerKey, ctx);
    if (!cellReadHandle.isValid())
    {
        ATH_MSG_ERROR("cannot retrieve the cell container with key <" << m_cellContainerKey.key()
                      << ">; the GNN needs the cells referenced by the cluster cell links");
        return StatusCode::FAILURE;
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
        if (cluster->rawE() < m_minClusterEnergy) continue;

        selectedClusters.push_back(cluster);
        clusterMask.at(cluster->index()) = true;
    }

    std::vector<double> clusterE_GNN_vec;
    ATH_CHECK(m_calibTool->inference(*selectedClusters.asDataVector(), clusterE_GNN_vec));

    if (clusterE_GNN_vec.size() != selectedClusters.size()) {
        ATH_MSG_ERROR("Tool returned " << clusterE_GNN_vec.size() << " energies for "
                      << selectedClusters.size() << " selected clusters");
        return StatusCode::FAILURE;
    }

    size_t i = 0;
    for (const xAOD::CaloCluster *cluster : *clusterReadHandle)
    {
        if (clusterMask.at(cluster->index())) {
            clusterGNNCalibEnergyDecor(*cluster) = clusterE_GNN_vec[i];
            ++i;
        } else {
            clusterGNNCalibEnergyDecor(*cluster) = cluster->rawE();
        }
    }

    return StatusCode::SUCCESS;
}