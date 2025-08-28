/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// EGammaCookieCutClusterTool.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef DERIVATIONFRAMEWORK_EGammaCookieCutClusterTool_H
#define DERIVATIONFRAMEWORK_EGammaCookieCutClusterTool_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKeyArray.h"
#include "xAODEgamma/EgammaContainer.h"
#include "CaloDetDescr/CaloDetDescrManager.h"
#include "egammaCaloUtils/egammaClusterCookieCut.h"

#include "CaloUtils/CaloClusterCollectionProcessor.h"

namespace DerivationFramework {

  class EGammaCookieCutClusterTool
    : public extends<AthAlgTool, IAugmentationTool>
  {
  public:
    EGammaCookieCutClusterTool(const std::string& t,
			       const std::string& n,
			       const IInterface* p);
    StatusCode initialize() override final;
    virtual StatusCode addBranches() const override final;
    
  private:

    /** @brief Output cluster container. */
    SG::WriteHandleKey<xAOD::CaloClusterContainer> m_outClusterContainerKey{
      this,
      "ClusterContainerName",
      "ForwardElectronCookieCutClusters",
      "Name of the output cookie cut cluster container"
    };

    /** @brief Output cluster container cell links: name taken from containter name. */
    SG::WriteHandleKey<CaloClusterCellLinkContainer> m_outClusterContainerCellLinkKey;

    /** @brief Calorimeter description. */
    SG::ReadCondHandleKey<CaloDetDescrManager> m_caloDetDescrMgrKey {
      this,
	"CaloDetDescrManager",
	"CaloDetDescrManager",
	"SG Key for CaloDetDescrManager in the Condition Store"
	};

    SG::ReadHandleKey<xAOD::EgammaContainer> m_SGKey_electrons{
      this,
	"SGKey_electrons",
	"ForwardElectrons",
	"SG key of electron container"
	};

    SG::WriteDecorHandleKeyArray<xAOD::EgammaContainer>
      m_SGKey_electrons_decorations{
      this,
	"SGKey_electrons_decorations_noConf",
	{},
	"SG keys for electrons decorations not really configurable"
        };
    
    /** @brief Size of maximum search window in eta. */
    Gaudi::Property<int> m_maxDelEtaCells{
      this,
	"MaxWindowDelEtaCells",
	3,
	"Size of maximum search window in eta"
	};

    /** @brief Size of maximum search window in phi. */
    Gaudi::Property<int> m_maxDelPhiCells{
      this,
	"MaxWindowDelPhiCells",
	3,
	"Size of maximum search window in phi"
	};

    /** @brief Size of cone to cookie cut on FCal. */
    Gaudi::Property<float> m_maxDelR{
      this,
	"MaxWindowDelR",
	0.3,
	"Cone size to collect cells around hottest-cell FCAL"
	};

    /** @brief Decide whether or not to store input cluster moments */
    Gaudi::Property<bool> m_storeOrigMom{
      this,
	"StoreInputMoments",
	false,
	"Decorate also with the moments from the original cluster"
	};

    /** @brief Decide whether or not to store cooked cluster moments */
    Gaudi::Property<bool> m_storeCookMom{
      this,
	"StoreCookedMoments",
	false,
	"Decorate also with the moments from the cookie-cut cluster"
	};

    egammaClusterCookieCut::CookieCutPars m_CookieCutPars{};

    mutable std::once_flag m_Seen;
    unsigned short m_nDecor = 0;
    const std::vector<xAOD::CaloCluster::MomentType> m_vecM{
      xAOD::CaloCluster::SECOND_LAMBDA,
      xAOD::CaloCluster::LATERAL,
      xAOD::CaloCluster::LONGITUDINAL,
      xAOD::CaloCluster::ENG_FRAC_MAX,
      xAOD::CaloCluster::SECOND_R,
      xAOD::CaloCluster::CENTER_LAMBDA,
      xAOD::CaloCluster::SECOND_ENG_DENS,
      xAOD::CaloCluster::SIGNIFICANCE
    };
    const std::vector<std::string> m_vecMName{
      "SECOND_LAMBDA",
      "LATERAL",
      "LONGITUDINAL",
      "ENG_FRAC_MAX",
      "SECOND_R",
      "CENTER_LAMBDA",
      "SECOND_ENG_DENS",
      "SIGNIFICANCE"
    };

    ToolHandleArray<CaloClusterCollectionProcessor> m_clusterCorrectionTools {
      this,
      "ClusterMomentMaker",
      {},
      "The moment maker"  
    };

  };

}

#endif // DERIVATIONFRAMEWORK_EGammaCookieCutClusterTool_H
