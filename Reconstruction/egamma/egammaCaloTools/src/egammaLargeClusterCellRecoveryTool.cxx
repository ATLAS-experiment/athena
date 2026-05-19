/*
    Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "egammaLargeClusterCellRecoveryTool.h"

// Calo includes
#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "CaloDetDescr/CaloDetDescrElement.h"
#include "CaloEvent/CaloCellContainer.h"
#include "CaloDetDescr/CaloDetDescrManager.h"
#include "CaloUtils/CaloClusterStoreHelper.h"
#include "CaloUtils/CaloLayerCalculator.h"
#include "CaloUtils/CaloCellList.h"
#include "egammaCaloUtils/egammaEnergyPositionAllSamples.h"

// xAOD includes
#include "xAODCaloEvent/CaloCluster.h"
#include <GaudiKernel/ThreadLocalContext.h>


egammaLargeClusterCellRecoveryTool::egammaLargeClusterCellRecoveryTool(const std::string& type,
                                                                 const std::string& name,
                                                                 const IInterface* parent)
    : AthAlgTool(type, name, parent) {
    declareInterface<IegammaLargeClusterCellRecoveryTool>(this);
}

StatusCode egammaLargeClusterCellRecoveryTool::initialize() {
    ATH_MSG_DEBUG("Initializing egammaLargeClusterCellRecoveryTool");

    if (!m_caloFillRectangularTool.empty()) {
      ATH_CHECK( m_caloFillRectangularTool.retrieve());
    }

    return StatusCode::SUCCESS;
}

StatusCode egammaLargeClusterCellRecoveryTool::execute(const xAOD::CaloCluster* cluster,
                                                    const CaloDetDescrManager* cmgr,
                                                    const CaloCellContainer* cell_container,
                                                    Info& info) const {
    ATH_MSG_DEBUG("Executing egammaLargeClusterRecoveryTool");

    if (cluster->et() < m_centEtThr) {
        ATH_MSG_DEBUG("Cluster Et below threshold for large cluster recovery: " << cluster->et());
        return StatusCode::SUCCESS;
    }

    if (!cluster->inBarrel() && !cluster->inEndcap()) {
        ATH_MSG_DEBUG("Cluster not in EMB or EMEC, skipping large cluster recovery.");
        return StatusCode::SUCCESS;
    }

    // Check if cluster is in barrel or endcap
    bool in_barrel = egammaEnergyPositionAllSamples::inBarrel(*cluster, 2);
    CaloSampling::CaloSample sam = CaloSampling::EMB2;
    if (!in_barrel) {
        sam = CaloSampling::EME2;
    }

    // (eta, phi) of the cluster in the 2nd sampling    
    auto eta = cluster->etaSample(sam);
    auto phi = cluster->phiSample(sam);

    if ((eta == 0. && phi == 0.) || std::abs(eta) > 100) {
        ATH_MSG_WARNING("Weird input cluster, eta = " << eta
                                                      << " phi = " << phi);
        return StatusCode::SUCCESS;
    }

    // Here decode_sample will overwrite these variables
    bool barrel = false;
    CaloCell_ID::SUBCALO subcalo = CaloCell_ID::LAREM;
    int sampling_or_module = 0;

    CaloDetDescrManager::decode_sample(
        subcalo, barrel, sampling_or_module, (CaloCell_ID::CaloSample)sam);

    // Get the corresponding granularities with CaloDetDescrElement
    const CaloDetDescrElement *dde = cmgr->get_element(
        CaloCell_ID::LAREM, sampling_or_module, barrel, eta, phi);

    // If the object does not exist then return
    if (!dde) {
        ATH_MSG_WARNING("Weird input cluster eta = " << eta
                                                     << " phi = " << phi);
        ATH_MSG_WARNING("No detetector element for seeding");
        return StatusCode::SUCCESS;
    }

    // Local granularity
    auto deta = dde->deta();
    auto dphi = dde->dphi();   

    // Search the hottest cell around the (eta,phi).
    // (eta,phi) are defined as etaSample() and phiSample().
    // Around this position a hot cell is searched for in a window
    // (m_neta*m_deta,m_nphi*m_dphi), by default (m_neta,m_nphi)=(7,7) 
    CaloLayerCalculator calc;
    StatusCode sc = 
        calc.fill(*cmgr, cell_container, cluster->etaSample(sam),
                  cluster->phiSample(sam), m_neta * deta, m_nphi * dphi,
                  (CaloSampling::CaloSample)sam);
                  
    if (sc.isFailure()) {
        ATH_MSG_WARNING("CaloLayerCalculator failed to fill");
        return StatusCode::SUCCESS;
    }
    double etamax = calc.etarmax();
    double phimax = calc.phirmax();

    // Create 7x11 cluster with hottest cell as seed
    std::unique_ptr<xAOD::CaloCluster> largeCluster = 
        CaloClusterStoreHelper::makeCluster(cell_container,
    etamax,
    phimax,
    xAOD::CaloCluster::SW_7_11);

    // Fill rectangular cluster
    if (!m_caloFillRectangularTool->execute(Gaudi::Hive::currentContext(), largeCluster.get()).isSuccess()) {
        ATH_MSG_WARNING("CaloFillRectangularCluster tool failed");
        return StatusCode::SUCCESS;
    }

    // Check that 7x11 cluster has cells
    if (largeCluster->size() == 0) {
        ATH_MSG_WARNING("Large cluster has no cells");
        return StatusCode::SUCCESS;
    }
    
    // Iterate through cells and store in info
    for (const CaloCell* cell : *largeCluster) {
        info.cells711.push_back(cell);
    }
    info.cluster = std::move(largeCluster);

    return StatusCode::SUCCESS;
}
