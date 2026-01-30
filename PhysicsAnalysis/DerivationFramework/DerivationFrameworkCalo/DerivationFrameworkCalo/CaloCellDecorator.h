/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/*
 * @file DerivationFrameworkCalo/DerivationFrameworkCalo/CaloCellDecorator.h
 * @author Gabriel P. Matos <gpinheir@cern.ch>, adapted from MaxCellDecorator by Nikiforos K. Nikiforou and others.
 * @date Aug, 2025
 * @brief Adds cell-level features as decorations to e/gamma objects.
 */

#ifndef DERIVATIONFRAMEWORK_CALOCELLDECORATOR_H
#define DERIVATIONFRAMEWORK_CALOCELLDECORATOR_H

#include <string>
#include <vector>

#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"

#include "GaudiKernel/EventContext.h"
#include "LArCabling/LArOnOffIdMapping.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKeyArray.h"
#include "xAODEgamma/EgammaContainer.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#include "xAODCaloEvent/CaloCluster.h"

namespace DerivationFramework {

  class CaloCellDecorator : public extends<AthAlgTool, IAugmentationTool>
  {

  public:
    using base_class::base_class;

    ~CaloCellDecorator();
    virtual StatusCode initialize() override final;
    virtual StatusCode addBranches(const EventContext& ctx) const override final;

    struct cell_decorations
    {

      // Vector features, LAr only
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

      // Total number of cells, LAr + Tile
      int ncells = 0;

    };

  private:

    // LAr cabling
    SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKey{
      this,
        "CablingKey",
        "LArOnOffIdMap",
        "SG Key of LArOnOffIdMapping object"
        };

    // Photon containers
    SG::ReadHandleKey<xAOD::EgammaContainer> m_SGKey_photons{
      this,
      "SGKey_photons",
      "",
      "SG key of photon container"
    };

    SG::WriteDecorHandleKeyArray<xAOD::EgammaContainer>
    m_SGKey_photons_decorations{
      this,
      "SGKey_photons_decorations",
      m_SGKey_photons, {
        "cells_E", "cells_time", "cells_eta", "cells_phi",
        "cells_x", "cells_y", "cells_z", "cells_gain",
        "cells_layer", "cells_quality", "cells_onlId", "ncells"},
      "SG keys for photon decorations"
    };

    // Electron containers
    SG::ReadHandleKey<xAOD::EgammaContainer> m_SGKey_electrons{
      this,
      "SGKey_electrons",
      "",
      "SG key of electron container"
    };

    SG::WriteDecorHandleKeyArray<xAOD::EgammaContainer>
    m_SGKey_electrons_decorations{
      this,
      "SGKey_electrons_decorations",
      m_SGKey_electrons, {
        "cells_E", "cells_time", "cells_eta", "cells_phi",
        "cells_x", "cells_y", "cells_z", "cells_gain",
        "cells_layer", "cells_quality", "cells_onlId", "ncells"},
      "SG keys for electrons decorations"
    };

    /* @brief Decorates e/gamma objects with vector cell features
     * E, t, eta, phi, layer, x, y, z, gain, quality factor, and online ID
     * for LAr cells. Tabulates the total number of cells in the topo
     * cluster for both LAr and Tile.
     */
    StatusCode decorateCells(
                             const SG::ReadHandleKey<xAOD::EgammaContainer>& contKey,
                             const SG::WriteDecorHandleKeyArray<xAOD::EgammaContainer>& decorKeys,
                             const EventContext& ctx) const;

    /*
     * @brief Loops through cells in an e/gamma topo cluster and adds
     * them to vectors in output struct
     */
    cell_decorations getDecorations(
                                    const xAOD::CaloCluster* cluster,
                                    const EventContext& ctx) const;
  };

}
#endif // DERIVATIONFRAMEWORK_CALOCELLDECORATOR_H
