/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CALOCLUSTERCORRECTION_CALOCLUSTERTRITONTOOL_H
#define CALOCLUSTERCORRECTION_CALOCLUSTERTRITONTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "CaloInterface/ICaloClusterTritonTool.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#include "AthOnnxInterfaces/IAthInferenceTool.h"
#include "GaudiKernel/ToolHandle.h"

#include <cstddef>
#include <string>
#include <vector>

/**
 * @class CaloClusterTritonTool
 * @brief GNN cluster energy calibration. Each cluster is represented as a graph
 *        of its constituent cells and sent to a Triton server through
 *        AthInfer::TritonTool.
 *
 * Model signature (tensor names configurable):
 *   CELL_IDS        : int64 [nCells, 1]  compact 32-bit cell identifiers
 *   CELL_FEATURES   : float [nCells, 1]  weighted cell energy [GeV]
 *   GLOBAL_FEATURES : float [1]          cluster raw (EM-scale) energy [GeV]
 *   OUTPUT          : float [1, 1]       predicted cluster energy [GeV]
 *
 * Since the graph size varies per cluster, one request is issued per cluster.
 * Clusters with no cells above threshold, or with a non-finite / non-positive
 * prediction, fall back to the raw (EM-scale) energy.
 */
class CaloClusterTritonTool : public extends<AthAlgTool, ICaloClusterTritonTool>
{
public:
    CaloClusterTritonTool(const std::string &type, const std::string &name, const IInterface *parent);
    virtual ~CaloClusterTritonTool() = default;

    virtual StatusCode initialize() override;

    virtual StatusCode inference(const xAOD::CaloClusterContainer &clusters,
                                 std::vector<double> &clusterE_GNN_vec) const override;

private:
    /// Fill the input tensor map for one cluster; nCells is the number of
    /// cells retained after the energy threshold.
    StatusCode buildInputs(const xAOD::CaloCluster &cluster,
                           AthInfer::InputDataMap &inputData,
                           std::size_t &nCells) const;

    /// Run inference for one cluster. energyGeV is meaningful only if valid.
    StatusCode inferCluster(const xAOD::CaloCluster &cluster,
                            double &energyGeV,
                            bool &valid) const;

    Gaudi::Property<float> m_cellEThreshold{
        this, "CellEnergyThreshold", 0.005f,
        "Minimum weighted cell energy [GeV] for a cell to enter the graph"};

    Gaudi::Property<std::string> m_cellIdsName{
        this, "CellIdsInputName", "CELL_IDS", "Name of the cell-ID input tensor"};
    Gaudi::Property<std::string> m_cellFeaturesName{
        this, "CellFeaturesInputName", "CELL_FEATURES", "Name of the cell-feature input tensor"};
    Gaudi::Property<std::string> m_globalFeaturesName{
        this, "GlobalFeaturesInputName", "GLOBAL_FEATURES", "Name of the global-feature input tensor"};
    Gaudi::Property<std::string> m_outputName{
        this, "OutputName", "OUTPUT", "Name of the output tensor"};

    ToolHandle<AthInfer::IAthInferenceTool> m_tritonTool{
        this, "TritonInferenceTool", "AthInfer::TritonTool",
        "Inference tool forwarding requests to the Triton server"};
};

#endif // CALOCLUSTERCORRECTION_CALOCLUSTERTRITONTOOL_H