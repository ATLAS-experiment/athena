/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// Decorate egamma objects with the energies in L2 and L3 that are in cells
// not included in the original supercluser due to the timing cut in topocluster
// building. This is an AOD fix for data (and mc, but effect is small for mc)
// produced with rel23 (and 24 ?)

#ifndef DERIVATIONFRAMEWORK_EGammaClusterCoreCellRecovery_H
#define DERIVATIONFRAMEWORK_EGammaClusterCoreCellRecovery_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKeyArray.h"
#include "xAODEgamma/EgammaContainer.h"
#include "CaloEvent/CaloCellContainer.h"
#include "xAODCaloEvent/CaloCluster.h"
#include "egammaInterfaces/IegammaCellRecoveryTool.h"

namespace DerivationFramework {

  class EGammaClusterCoreCellRecovery : public extends<AthAlgTool, IAugmentationTool>
  {
  public:

    using base_class::base_class;

    virtual StatusCode initialize() override final;
    virtual StatusCode addBranches(const EventContext& ctx) const override final;

  private:
    SG::ReadHandleKey<xAOD::EgammaContainer> m_SGKey_photons
    { this, "SGKey_photons", "Photons", "SG key of photon container" };

    SG::ReadHandleKey<xAOD::EgammaContainer> m_SGKey_electrons
      { this, "SGKey_electrons", "Electrons", "SG key of electron container" };

    SG::ReadHandleKey<CaloCellContainer> m_SGKey_CaloCells
      { this, "SGKey_CaloCells", "AllCalo", "SG key of calo cell container" };

    SG::WriteDecorHandleKeyArray<xAOD::EgammaContainer>
    m_SGKey_photons_decorations{
      this,
      "SGKey_photons_decorations",
      m_SGKey_photons, {"nadded_Lr2", "Eadded_Lr2", "nadded_Lr3", "Eadded_Lr3"},
      "SG keys for photon decorations"
    };

    SG::WriteDecorHandleKeyArray<xAOD::EgammaContainer>
    m_SGKey_electrons_decorations{
      this,
      "SGKey_electrons_decorations",
      m_SGKey_electrons, {"nadded_Lr2", "Eadded_Lr2", "nadded_Lr3", "Eadded_Lr3"},
      "SG keys for electrons"
    };

    Gaudi::Property<bool> m_UseWeightForMaxCell{
      this,
      "UseWeightForMaxCell",
      false,
      "Use the cell weights when finding the L2 max energy cell"
    };


    IegammaCellRecoveryTool::Info decorateObject(const xAOD::Egamma*& egamma) const;
    StatusCode findMaxECell(const xAOD::CaloCluster *clus,
                            double &etamax, double &phimax) const;

    /** @brief Pointer to the egammaCellRecoveryTool*/
    ToolHandle<IegammaCellRecoveryTool> m_egammaCellRecoveryTool{
      this,
      "egammaCellRecoveryTool",
      "egammaCellRecoveryTool/egammaCellRecoveryTool",
      "Optional tool that adds cells in L2 or L3 "
      "that could have been rejected by timing cut"
    };

  };

}

#endif // DERIVATIONFRAMEWORK_EGammaClusterCoreCellRecovery_H
