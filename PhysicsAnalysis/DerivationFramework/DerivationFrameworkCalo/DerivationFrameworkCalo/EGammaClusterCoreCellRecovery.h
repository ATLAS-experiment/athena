/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// EGammaClusterCoreCellRecovery.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
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
    EGammaClusterCoreCellRecovery(const std::string& t,
				  const std::string& n,
				  const IInterface* p);
    ~EGammaClusterCoreCellRecovery() = default;
    StatusCode initialize();
    StatusCode finalize() { return StatusCode::SUCCESS; }
    virtual StatusCode addBranches(const EventContext& ctx) const;
    
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
	"SGKey_photons_decorations_noConf",
	  {},
	"SG keys for photon decorations not really configurable"
	  };
    
    SG::WriteDecorHandleKeyArray<xAOD::EgammaContainer>
      m_SGKey_electrons_decorations{
      this,
	"SGKey_electrons_decorations_noConf",
	  {},
	"SG keys for electrons decorations not really configurable"
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
