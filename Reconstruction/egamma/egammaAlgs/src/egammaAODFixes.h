/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EGAMMAALGS_EGAMMAAODFIXES_H
#define EGAMMAALGS_EGAMMAAODFIXES_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

//#include "xAODEgamma/EgammaContainer.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODEgamma/PhotonContainer.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#include "xAODEgamma/EgammaFwd.h"
#include <memory>

#include "CaloDetDescr/CaloDetDescrManager.h"
#include "IsolationCorrections/IIsolationCorrectionTool.h"
#include "EgammaAnalysisInterfaces/IegammaMVASvc.h"
#include "egammaInterfaces/IegammaCellRecoveryTool.h"
#include "egammaInterfaces/IegammaSwTool.h"
#include "CaloEvent/CaloCellContainer.h"
#include "CaloEvent/CaloCell.h"

/**
 * @class egammaAODFixes
 * @brief Algorithm to handle various AOD fixes : 
 *   - ambiguity relinking for electrons and photons
 *   - the consequence of the timing cut in egamma 
 *
 * This class is responsible for reading input electron and photon containers,
 * processing them to in order to link ambiguous candidates, and to correct 
 * the topoetcone isolation variables and the cluster layer 2 and 3 energies 
 * and writing to the output containers. It is used in the context of an AODFix 
 * to correct an ambiguityLink association bug in the Athena Rel24 up to 24.0.83,
 * and to mitigate the impact of the timing cut for topocluster building in egamma 
 * reconstruction, in Athena Rel24. 
 *
 * @details
 * - Input electron container: specified by ElectronInputName.
 * - Output electron container: specified by ElectronOutputName.
 * - Input photon container: specified by PhotonInputName.
 * - Output photon container: specified by PhotonOutputName.
 *
 */
class egammaAODFixes : public AthReentrantAlgorithm
{
public:

    egammaAODFixes(const std::string& name, ISvcLocator* pSvcLocator);

    StatusCode initialize() override final;
    StatusCode execute(const EventContext& ctx) const override final;

private:

    // Read/Write handlers

    /** @brief Name of the electron output collection*/
    SG::WriteHandleKey<xAOD::ElectronContainer> m_electronOutputKey {this,
        "ElectronOutputName", "Electrons",
        "Name of electron container to be created"};

    /** @brief Name of the electron input collection */
    SG::ReadHandleKey<xAOD::ElectronContainer> m_electronInputKey {this,
        "ElectronInputName", "old_Electrons",
        "Name of electron container to be read in"};
    
    /** @brief Name of the photon output collection*/
    SG::WriteHandleKey<xAOD::PhotonContainer> m_photonOutputKey {this,
        "PhotonOutputName", "Photons",
        "Name of photon container to be created"};

    /** @brief Name of the photon input collection */
    SG::ReadHandleKey<xAOD::PhotonContainer> m_photonInputKey {this,
        "PhotonInputName", "old_Photons",
        "Name of photon container to be read in"};

    /** @brief Name of the calo cell container */
    SG::ReadHandleKey<CaloCellContainer> m_CaloCellsKey {this,
	"CaloCellsName", "AllCalo", "Name of calo cell container"};

    /** @brief Name of the egamma cluster output collection*/
    SG::WriteHandleKey<xAOD::CaloClusterContainer> m_egClusOutputKey {this,
        "EGammaClustersOutputName", "",
        "Name of egamma cluster container to be created"};

    /** @brief Name of the egamma cluster input collection */
    SG::ReadHandleKey<xAOD::CaloClusterContainer> m_egClusInputKey {this,
        "EGammaClustersInputName", "",
        "Name of egamma cluster container to be read in"};

    /** @brief Key of the output cluster container cell links:
	name taken from containter name; only dummy configurable **/
    SG::WriteHandleKey<CaloClusterCellLinkContainer>
     m_egClusCellLinkOutputKey{this,
       "DoNotSet_OutputClusterContainerLinks",
       "",
       "Key of the output cluster container cell links; Do not set! Name taken "
       "from associated container"
     };

    /** @brief Key for calo detector description manager */
    SG::ReadCondHandleKey<CaloDetDescrManager> m_caloDetDescrMgrKey {this,
	"CaloDetDescrManager",
	"",
	"SG Key for CaloDetDescrManager in the Condition Store"
    };

    /** Pointer to the egammaCellRecoveryTool */
    ToolHandle<IegammaCellRecoveryTool> m_egammaCellRecoveryTool{
      this,
	"egammaCellRecoveryTool",
	"egammaCellRecoveryTool/egammaCellRecoveryTool",
	"Tool that adds cells in L2 or L3 "
	"that could have been rejected by timing cut"
    };

    /** @brief correct also layer energies ? */
    Gaudi::Property<bool> m_correctCluster {this,
	"CorrectCluster", false,
	"Whether or not to correct the cluster"};

    /** @brief correct ambiguity links ? */
    Gaudi::Property<bool> m_ambiguityFix {this,
	"FixAmbiguityLinks", true,
	"Whether or not to fix the ambiguity links"};

    /** @brief correct topoetcone isolation  ? */
    Gaudi::Property<bool> m_tpetcFix {this,
	"FixEGTopoetcone", true,
	"Whether or not to fix topoetcone variables"};

    /** @brief Tool to handle cluster corrections */
    ToolHandle<IegammaSwTool> m_clusterCorrectionTool{ this,
	"ClusterCorrectionTool",
	"",
	"tool that applies cluster corrections" };

    /** Handle to the MVA calibration service **/
    ServiceHandle<IegammaMVASvc> m_MVACalibSvc{ this,
	"MVACalibSvc",
	"",
	"calibration service" };

    /** Handle to the isolation leakage correction tool **/
    ToolHandle<CP::IIsolationCorrectionTool> m_IsoLeakCorrectionTool {this,
	"IsoLeakCorrectionTool", "",
	"Handle on the leakage correction tool"};

    void getLayerE(std::vector<const CaloCell*>& cells,
		   std::vector<float>& lE) const;
    StatusCode getCorrection(const xAOD::Egamma* eg,
			     float &aET,
			     std::vector<float>& lECl) const;
    StatusCode getCorrectionC(const xAOD::Egamma* eg,
			     float &aET,
			     IegammaCellRecoveryTool::Info &info) const;

    void rebuildLink(xAOD::EgammaContainer *egC,
		     xAOD::CaloClusterContainer *egclC,
		     const EventContext& ctx) const;
};

#endif
