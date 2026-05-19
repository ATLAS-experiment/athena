/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */

#ifndef EGAMMAINTERFACES_IEGAMMALARGECLUSTERCELLRECOVERYTOOL_H
#define EGAMMAINTERFACES_IEGAMMALARGECLUSTERCELLRECOVERYTOOL_H

/// @class egammaLargeClusterCellRecoveryTool
/// Interface for the Reconstruction/egamma/egammaCaloTools/egammaLargeClusterCellRecoveryTool.cxx

// Gaudi
#include "GaudiKernel/IAlgTool.h"
#include <GaudiKernel/IInterface.h>

// xAOD includes
#include "xAODCaloEvent/CaloCluster.h"

#include <memory>
#include <vector>

class CaloCell;
class CaloCellContainer;
class CaloDetDescrManager;

static const InterfaceID IID_IegammaLargeClusterCellRecoveryTool("IegammaLargeClusterCellRecoveryTool", 1, 0);

class IegammaLargeClusterCellRecoveryTool : virtual public IAlgTool {

    public:
    /** @brief Virtual destructor */
    virtual ~IegammaLargeClusterCellRecoveryTool() = default;
    /** @brief AlgTool interface methods */
    static const InterfaceID& interfaceID();

    class Info {
        public:
            std::vector<const CaloCell*> cells711;
            std::unique_ptr<xAOD::CaloCluster> cluster;
    };

    /** @brief Method to recover large clusters */
    virtual StatusCode execute(const xAOD::CaloCluster* cluster,
                               const CaloDetDescrManager* cmgr,
                               const CaloCellContainer* cell_container,
                               Info& info) const = 0;

};

inline const InterfaceID& IegammaLargeClusterCellRecoveryTool::interfaceID()
{
  return IID_IegammaLargeClusterCellRecoveryTool;
}

#endif // EGAMMAINTERFACES_IEGAMMALARGECLUSTERCELLRECOVERYTOOL_H
