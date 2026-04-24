/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigTauCaloRoiUpdater.h"

#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/IToolSvc.h"
#include "GaudiKernel/StatusCode.h"

#include "TrigSteeringEvent/TrigRoiDescriptor.h"
#include "CxxUtils/phihelper.h"

#include "TLorentzVector.h"


TrigTauCaloRoiUpdater::TrigTauCaloRoiUpdater(const std::string & name, ISvcLocator* pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator)
{

}


StatusCode TrigTauCaloRoiUpdater::initialize()
{
    ATH_MSG_DEBUG("Initializing " << name());
    ATH_MSG_DEBUG("dRForCenter: " << m_dRForCenter);

    ATH_MSG_DEBUG("Initialising HandleKeys");
    ATH_CHECK(m_roIInputKey.initialize());
    ATH_CHECK(m_clustersKey.initialize());
    ATH_CHECK(m_roIOutputKey.initialize());  

    return StatusCode::SUCCESS;
}


StatusCode TrigTauCaloRoiUpdater::execute(const EventContext& ctx) const
{
    ATH_MSG_DEBUG("Running " << name());

    //---------------------------------------------------------------
    // Prepare I/O
    //---------------------------------------------------------------
    
    // Prepare output RoI container
    std::unique_ptr<TrigRoiDescriptorCollection> roiCollection = std::make_unique<TrigRoiDescriptorCollection>();
    SG::WriteHandle<TrigRoiDescriptorCollection> outputRoIHandle = SG::makeHandle(m_roIOutputKey, ctx);
    ATH_CHECK(outputRoIHandle.record(std::move(roiCollection)));


    // Retrieve input RoI descriptor
    SG::ReadHandle<TrigRoiDescriptorCollection> roisHandle = SG::makeHandle(m_roIInputKey, ctx);
    ATH_MSG_DEBUG("Size of roisHandle: " << roisHandle->size());
    const TrigRoiDescriptor* roiDescriptor = roisHandle->at(0); // We only have one RoI in the handle


    // Fill local variables for RoI reference position
    float eta = roiDescriptor->eta();
    float phi = roiDescriptor->phi();
    
    const float dEta = (roiDescriptor->etaPlus() - roiDescriptor->etaMinus()) / 2;
    const float dPhi = CxxUtils::deltaPhi(roiDescriptor->phiPlus(), roiDescriptor->phiMinus()) / 2;

    ATH_MSG_DEBUG("RoI ID: " << roiDescriptor->roiId() << ", eta: " << eta << ", phi: " << phi);


    
    //---------------------------------------------------------------
    // Find detector tau axis
    //---------------------------------------------------------------
        
    // Retrieve Input CaloClusterContainer
    SG::ReadHandle<xAOD::CaloClusterContainer> CCContainerHandle = SG::makeHandle(m_clustersKey, ctx);
    ATH_CHECK(CCContainerHandle.isValid());
    const xAOD::CaloClusterContainer *RoICaloClusterContainer = CCContainerHandle.get();

    if(!RoICaloClusterContainer) {
        ATH_MSG_ERROR("No CaloCluster container found");
        return StatusCode::FAILURE;
    }

    ATH_MSG_DEBUG("Size of vector CaloCluster container is: " << RoICaloClusterContainer->size());

    // We first need to get the barycenter of the LCTopo jet, including all clusters
    TLorentzVector tau_barycenter;
    for(const xAOD::CaloCluster* cluster : *RoICaloClusterContainer) {
        // Skip clusters with negative energy
        if(cluster->e() < 0) continue;

        tau_barycenter += cluster->p4();
    }

    // Determine the LCTopo jet pT at the detector axis
    TLorentzVector tau_detector_axis;
    for(const xAOD::CaloCluster* cluster : *RoICaloClusterContainer) {
        // Skip clusters with negative energy
        if(cluster->e() < 0) continue;

        // Skip clusters further than a maximum Delta R
        if(tau_barycenter.DeltaR(cluster->p4()) > m_dRForCenter) continue;

        tau_detector_axis += cluster->p4();
    }



    //---------------------------------------------------------------
    // Update the RoI
    //---------------------------------------------------------------
    
    // Only update the roi if tau_detector_axis.Pt() > 0, i.e. if the calo cluster sum makes sense
    if(tau_detector_axis.Pt() > 0) {
        eta = tau_detector_axis.Eta();
        phi = tau_detector_axis.Phi();
    }

    // Create the new RoI
    outputRoIHandle->push_back(std::make_unique<TrigRoiDescriptor>(
        roiDescriptor->roiWord(), roiDescriptor->l1Id(), roiDescriptor->roiId(),
        eta, eta-dEta, eta+dEta,
        phi, CxxUtils::wrapToPi(phi-dPhi), CxxUtils::wrapToPi(phi+dPhi),
        roiDescriptor->zed(), roiDescriptor->zedMinus(), roiDescriptor->zedPlus()
    ));


    ATH_MSG_DEBUG("Input RoI: " << *roiDescriptor);
    ATH_MSG_DEBUG("Output RoI: " << *outputRoIHandle->back());

    return StatusCode::SUCCESS;
}
