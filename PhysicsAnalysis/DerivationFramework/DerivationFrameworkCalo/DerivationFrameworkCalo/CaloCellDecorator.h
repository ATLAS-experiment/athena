/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file CaloCellDecorator.h
 * @author Gabriel P. Matos <gpinheir@cern.ch>
 *         Adapted from MaxCellDecorator by Nikiforos K. Nikiforou and others.
 * @date Nov 2025
 * @brief Adds cell-level features as decorations to e/gamma objects. The cells
 *        included are those in the object's supercluster, those recovered from
 *        the timing cut by the egammaCellRecoveryTool, and those in the large
 *        7x11 cluster, recovered by the egammaLargeClusterCellRecoveryTool.
 */

#ifndef DERIVATIONFRAMEWORK_CALOCELLDECORATOR_H
#define DERIVATIONFRAMEWORK_CALOCELLDECORATOR_H

#include <cstdint>
#include <string>
#include <vector>

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/EventContext.h"

#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKeyArray.h"

#include "LArCabling/LArOnOffIdMapping.h"
#include "CaloDetDescr/CaloDetDescrManager.h"

#include "xAODEgamma/EgammaContainer.h"
#include "xAODCaloEvent/CaloCluster.h"

// Cell timing recovery tool
#include "egammaInterfaces/IegammaCellRecoveryTool.h"
#include "egammaInterfaces/IegammaLargeClusterCellRecoveryTool.h"

namespace DerivationFramework {

  class CaloCellDecorator : public AthReentrantAlgorithm
  {

  public:
    using AthReentrantAlgorithm::AthReentrantAlgorithm;

    ~CaloCellDecorator();
    virtual StatusCode initialize() override final;
    virtual StatusCode execute(const EventContext& ctx) const override final;

  private:

    // LAr cabling
    SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKey{
      this,
        "CablingKey",
        "LArOnOffIdMap",
        "SG Key of LArOnOffIdMapping object"
        };

    // Photon container
    SG::ReadHandleKey<xAOD::EgammaContainer> m_SGKey_photons
    { this, "SGKey_photons", "", "SG key of photon container" };

    // Electron container
    SG::ReadHandleKey<xAOD::EgammaContainer> m_SGKey_electrons
      { this, "SGKey_electrons", "", "SG key of electron container" };

    // Calo cell container
    SG::ReadHandleKey<CaloCellContainer> m_SGKey_CaloCells
      { this, "SGKey_CaloCells", "AllCalo", "SG key of calo cell container" };

    // Calo detector description manager
    SG::ReadCondHandleKey<CaloDetDescrManager> m_caloDetDescrMgrKey{
      this,
      "CaloDetDescrManager",
      "CaloDetDescrManager",
      "SG Key for CaloDetDescrManager in the Condition Store"
    };

    // Photon decorators
    SG::WriteDecorHandleKeyArray<xAOD::EgammaContainer>
    m_SGKey_photons_decorations{
      this,
      "SGKey_photons_decorations",
      m_SGKey_photons, { "cells_E", "cells_time", "cells_eta", "cells_phi",
      "cells_x", "cells_y", "cells_z", "cells_gain",
      "cells_layer", "cells_quality", "cells_onlId", "cells_clusterOriginInfo" } ,
      "SG keys for photon decorations"
    };

    // Electron decorators
    SG::WriteDecorHandleKeyArray<xAOD::EgammaContainer>
    m_SGKey_electrons_decorations{
      this,
      "SGKey_electrons_decorations",
      m_SGKey_electrons, { "cells_E", "cells_time", "cells_eta", "cells_phi",
      "cells_x", "cells_y", "cells_z", "cells_gain",
      "cells_layer", "cells_quality", "cells_onlId", "cells_clusterOriginInfo" } ,
      "SG keys for electrons decorations"
    };

    /** @brief Pointer to the egammaCellRecoveryTool*/
    ToolHandle<IegammaCellRecoveryTool> m_egammaCellRecoveryTool{
      this,
      "egammaCellRecoveryTool",
      "egammaCellRecoveryTool/egammaCellRecoveryTool",
      "Optional tool that adds cells in L2 or L3 "
      "that could have been rejected by timing cut"
    };

    /** @brief Pointer to egammaLargeClusterCellRecoveryTool */
    ToolHandle<IegammaLargeClusterCellRecoveryTool> m_egammaLargeClusterCellRecoveryTool{
      this,
      "egammaLargeClusterCellRecoveryTool",
      "",
      "Optional tool that collects cells in a 7x11 cluster around the hottest cell"
    };

    /** @brief Struct to hold cell decoration data */
    struct CellDecorationData
    {
      std::vector<float> cells_E{};
      std::vector<float> cells_time{};
      std::vector<float> cells_eta{};
      std::vector<float> cells_phi{};
      std::vector<float> cells_x{};
      std::vector<float> cells_y{};
      std::vector<float> cells_z{};
      std::vector<int> cells_gain{};
      std::vector<int> cells_layer{};
      std::vector<int> cells_quality{};
      std::vector<uint64_t> cells_onlId{};
      std::vector<uint8_t> cells_clusterOriginInfo{};
    };

    /** @brief Struct to keep track of where cell came from
     e.g. supercluster, timing cut recovery, 7x11 cluster 
     
     * supercluster cells           |= 0x01
     * timing cut recovered cells   |= 0x02
     * 7x11 cells                   |= 0x04

     */
    struct CellClusterInfo {
      uint8_t mask = 0x0;
      size_t index = -1;
    };

    /** @brief Decorates e/gamma objects with vector cell features
     E, t, eta, phi, layer, x, y, z, gain, quality factor, online ID,
     and cluster origin info for LAr cells. The cells included are 
     those in the associated supercluster to the object, those recovered
     by the timing cut recovery in a 5x7 (3x7) window in EMB (EMEC), and
     those in the 7x11 cluster around the hottest cell.
     */
    StatusCode decorateCells(
                             const SG::ReadHandleKey<xAOD::EgammaContainer>& contKey,
                             const SG::WriteDecorHandleKeyArray<xAOD::EgammaContainer>& decorKeys,
                             const EventContext& ctx) const;

    /** @brief Loops through cells and adds to decoration struct */
    CellDecorationData getDecorations(
                                    const xAOD::CaloCluster* cluster,
                                    const CaloCellContainer* caloCells,
                                    const CaloDetDescrManager* cmgr,
                                    const EventContext& ctx) const;

    /** @brief Inline to get calorimeter sampling */
    inline int layerFromSampling(int s) const {

      // LAr EMB and EMEC
      if (s == CaloCell_ID::PreSamplerB || s == CaloCell_ID::PreSamplerE) return 0;
      if (s == CaloCell_ID::EMB1        || s == CaloCell_ID::EME1)        return 1;
      if (s == CaloCell_ID::EMB2        || s == CaloCell_ID::EME2)        return 2;
      if (s == CaloCell_ID::EMB3        || s == CaloCell_ID::EME3)        return 3;

      // Anything else
      return -1;
    }
  };

}
#endif // DERIVATIONFRAMEWORK_CALOCELLDECORATOR_H
