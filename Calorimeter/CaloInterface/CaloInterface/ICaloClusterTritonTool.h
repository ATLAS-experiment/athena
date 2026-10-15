/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CALOINTERFACE_ICALOCLUSTERTRITONTOOL_H
#define CALOINTERFACE_ICALOCLUSTERTRITONTOOL_H

#include "GaudiKernel/IAlgTool.h"
#include "xAODCaloEvent/CaloClusterContainer.h"

#include <vector>

/**
 * @class ICaloClusterTritonTool
 * @brief Interface for GNN-based cluster energy calibration served via Triton.
 */
class ICaloClusterTritonTool : virtual public IAlgTool
{
public:
    DeclareInterfaceID(ICaloClusterTritonTool, 1, 0);

    virtual ~ICaloClusterTritonTool() = default;

    /// Compute the GNN-calibrated energy [MeV] for every cluster in the container.
    /// The output vector is index-aligned with the input container.
    virtual StatusCode inference(const xAOD::CaloClusterContainer &clusters,
                                 std::vector<double> &clusterE_GNN_vec) const = 0;
};

#endif // CALOINTERFACE_ICALOCLUSTERTRITONTOOL_H