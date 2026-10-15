/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "CaloClusterTritonTool.h"

#include "xAODCaloEvent/CaloCluster.h"
#include "CaloEvent/CaloCell.h"
#include "CaloEvent/CaloClusterCellLink.h"
#include "GaudiKernel/SystemOfUnits.h"

#include <cmath>
#include <cstdint>
#include <utility>
#include <variant>

CaloClusterTritonTool::CaloClusterTritonTool(const std::string &type, const std::string &name, const IInterface *parent)
    : base_class(type, name, parent) {}

StatusCode CaloClusterTritonTool::initialize()
{
    ATH_MSG_DEBUG("Initializing " << name() << "...");
    ATH_CHECK(m_tritonTool.retrieve());
    ATH_MSG_INFO("Cell energy threshold for graph construction: " << m_cellEThreshold.value() << " GeV");
    return StatusCode::SUCCESS;
}

StatusCode CaloClusterTritonTool::buildInputs(const xAOD::CaloCluster &cluster,
                                              AthInfer::InputDataMap &inputData,
                                              std::size_t &nCells) const
{
    nCells = 0;

    const CaloClusterCellLink *links = cluster.getCellLinks();
    if (!links) {
        ATH_MSG_DEBUG("Cluster " << cluster.index() << " has no cell links.");
        return StatusCode::SUCCESS;
    }
    // The links store indices into a CaloCellContainer (normally AllCalo).
    // If that container is not available, dereferencing a link is a null access.
    if (!links->getCellContainer()) {
        ATH_MSG_ERROR("Cell links of cluster " << cluster.index()
                      << " point to cell container '" << links->getCellContainerLink().dataID()
                      << "', which is not available in the event store.");
        return StatusCode::FAILURE;
    }

    std::vector<int64_t> cellIds;
    std::vector<float> cellEnergies;
    cellIds.reserve(cluster.size());
    cellEnergies.reserve(cluster.size());

    for (auto itCell = cluster.cell_begin(); itCell != cluster.cell_end(); ++itCell) {
        const CaloCell *cell = *itCell;
        if (!cell) continue;

        const float cellE = cell->e() * itCell.weight() / Gaudi::Units::GeV;
        if (cellE < m_cellEThreshold) continue;

        cellIds.push_back(static_cast<int64_t>(cell->ID().get_identifier32().get_compact()));
        cellEnergies.push_back(cellE);
    }

    nCells = cellIds.size();
    const int64_t n = static_cast<int64_t>(nCells);

    std::vector<float> globalFeatures{static_cast<float>(cluster.rawE() / Gaudi::Units::GeV)};

    inputData[m_cellIdsName.value()] =
        std::make_pair(std::vector<int64_t>{n, 1}, std::move(cellIds));
    inputData[m_cellFeaturesName.value()] =
        std::make_pair(std::vector<int64_t>{n, 1}, std::move(cellEnergies));
    inputData[m_globalFeaturesName.value()] =
        std::make_pair(std::vector<int64_t>{1}, std::move(globalFeatures));

    return StatusCode::SUCCESS;
}

StatusCode CaloClusterTritonTool::inferCluster(const xAOD::CaloCluster &cluster,
                                               double &energyGeV,
                                               bool &valid) const
{
    energyGeV = 0.0;
    valid = false;

    AthInfer::InputDataMap inputData;
    std::size_t nCells = 0;
    ATH_CHECK(buildInputs(cluster, inputData, nCells));
    if (nCells == 0) return StatusCode::SUCCESS;

    AthInfer::OutputDataMap outputData;
    outputData[m_outputName.value()] =
        std::make_pair(std::vector<int64_t>{1, 1}, std::vector<float>{});

    ATH_CHECK(m_tritonTool->inference(inputData, outputData));

    const auto itOut = outputData.find(m_outputName.value());
    if (itOut == outputData.end()) {
        ATH_MSG_ERROR("Output tensor '" << m_outputName.value() << "' not returned by server.");
        return StatusCode::FAILURE;
    }

    const auto &data = itOut->second.second;
    if (const auto *vf = std::get_if<std::vector<float>>(&data); vf && !vf->empty()) {
        energyGeV = vf->front();
        valid = true;
    } else if (const auto *vi = std::get_if<std::vector<int64_t>>(&data); vi && !vi->empty()) {
        energyGeV = static_cast<double>(vi->front());
        valid = true;
    }

    return StatusCode::SUCCESS;
}

StatusCode CaloClusterTritonTool::inference(const xAOD::CaloClusterContainer &clusters,
                                            std::vector<double> &clusterE_GNN_vec) const
{
    ATH_MSG_DEBUG("Executing " << name() << "...");

    clusterE_GNN_vec.clear();
    clusterE_GNN_vec.reserve(clusters.size());

    std::size_t nFallback = 0;

    for (const xAOD::CaloCluster *cluster : clusters) {
        const double rawE = cluster->rawE(); // MeV

        double energyGeV = 0.0;
        bool valid = false;
        ATH_CHECK(inferCluster(*cluster, energyGeV, valid));

        if (valid && std::isfinite(energyGeV) && energyGeV > 0.0) {
            clusterE_GNN_vec.push_back(energyGeV * Gaudi::Units::GeV);
        } else {
            clusterE_GNN_vec.push_back(rawE);
            ++nFallback;
        }
    }

    if (nFallback > 0) {
        ATH_MSG_DEBUG(nFallback << " of " << clusters.size()
                      << " clusters fell back to raw energy (no cells above threshold or invalid prediction).");
    }

    return StatusCode::SUCCESS;
}