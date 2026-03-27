/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include <cmath>

#include "TrigTauHitZRoiUpdater.h"

#include "GaudiKernel/IToolSvc.h"
#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/SystemOfUnits.h"

#include "StoreGate/ReadDecorHandle.h"

#include "TrigSteeringEvent/TrigRoiDescriptor.h"
#include "CxxUtils/phihelper.h"

TrigTauHitZRoiUpdater::TrigTauHitZRoiUpdater(const std::string& name, ISvcLocator* pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator)
{

}


StatusCode TrigTauHitZRoiUpdater::initialize()
{
    ATH_MSG_DEBUG("Initializing " << name() );
    ATH_MSG_DEBUG("z0HalfWidth: " << m_z0HalfWidth.value() );
    ATH_MSG_DEBUG("etaHalfWidth: " << m_etaHalfWidth.value() );
    ATH_MSG_DEBUG("phiHalfWidth: " << m_phiHalfWidth.value() );
    ATH_MSG_DEBUG("maxSigma: " << m_maxSigma.value() );
    ATH_MSG_DEBUG("maxPt: " << m_maxPt.value() );
    
    if(m_z0HalfWidth < 0 || m_etaHalfWidth < 0 || m_phiHalfWidth < 0) {
        ATH_MSG_ERROR("Incorrect parameters");
        return StatusCode::FAILURE;
    }
    
    ATH_MSG_DEBUG("Initialising HandleKeys");
    ATH_CHECK(m_roIInputKey.initialize());
    ATH_CHECK(m_roIOutputKey.initialize());

    ATH_CHECK(m_tauKey.initialize());
    m_zDecorKey = m_tauKey.key() + "." + m_zDecorKey.key();
    ATH_CHECK(m_zDecorKey.initialize());
    m_sigmaDecorKey = m_tauKey.key() + "." + m_sigmaDecorKey.key();
    ATH_CHECK(m_sigmaDecorKey.initialize());

    return StatusCode::SUCCESS;
}


StatusCode TrigTauHitZRoiUpdater::execute(const EventContext& ctx) const
{
    ATH_MSG_DEBUG("Running " << name());
 
    //---------------------------------------------------------------
    // Prepare I/O
    //---------------------------------------------------------------

    // Prepare output RoI container
    std::unique_ptr<TrigRoiDescriptorCollection> roiCollection = std::make_unique<TrigRoiDescriptorCollection>();
    SG::WriteHandle<TrigRoiDescriptorCollection> outputRoIHandle(m_roIOutputKey, ctx);
    ATH_CHECK(outputRoIHandle.record(std::move(roiCollection)));


    // Retrieve Input Tau container
    SG::ReadHandle<xAOD::TauJetContainer> tauHandle(m_tauKey, ctx);
    ATH_CHECK(tauHandle.isValid());
    const xAOD::TauJetContainer* tauContainer = tauHandle.get();
    if(!tauContainer) {
      ATH_MSG_ERROR("No tau container found, the Tau RoI updater should not be scheduled");
      return StatusCode::FAILURE;
    }
    ATH_MSG_DEBUG("Found " << tauContainer->size() << " taus, updating the RoI");
    const xAOD::TauJet *tau = tauContainer->at(0); // We only have one tau in the container

    
    // Retrieve input RoI descriptor
    SG::ReadHandle<TrigRoiDescriptorCollection> roisHandle(m_roIInputKey, ctx);
    ATH_CHECK(roisHandle.isValid());
    ATH_MSG_DEBUG("Size of roisHandle: " << roisHandle->size());
    if(roisHandle->size() != 1) {
      ATH_MSG_ERROR("Expected exactly one RoI!");
      return StatusCode::FAILURE;
    }
    const TrigRoiDescriptor* roiDescriptor = roisHandle->at(0); // We only have one RoI in the handle


    // Fill local variables for RoI reference position
    const float eta = roiDescriptor->eta();
    const float phi = roiDescriptor->phi();  
    float zed = roiDescriptor->zed();
    float zedMinus = roiDescriptor->zedMinus();
    float zedPlus = roiDescriptor->zedPlus();


    //---------------------------------------------------------------
    // Get HitZ regression results
    //---------------------------------------------------------------

    SG::ReadDecorHandle<xAOD::TauJetContainer, float> zDecor(m_zDecorKey, ctx);
    ATH_CHECK(zDecor.isAvailable());
    const float hitz_z0 = zDecor(*tau);

    SG::ReadDecorHandle<xAOD::TauJetContainer, float> sigmaDecor(m_sigmaDecorKey, ctx);
    ATH_CHECK(sigmaDecor.isAvailable());
    const float hitz_sigma = sigmaDecor(*tau);

    ATH_MSG_DEBUG("HitZ Tau pt: " << tau->pt() << ", eta: " << tau->eta() << ", phi: " << tau->phi() << ", HitZ z0: " << hitz_z0 << ", HitZ sigma: " << hitz_sigma);

    //---------------------------------------------------------------
    // Update the RoI
    //---------------------------------------------------------------
    // If the HitZ regression is valid (z0 != -1111) and the regression sigma is below the threshold, 
    // update the RoI z position and width:
    constexpr float invalid_z0 = -1111;
    if(hitz_z0 != invalid_z0 && tau->pt() < m_maxPt * Gaudi::Units::GeV && hitz_sigma < m_maxSigma) {
        zed = hitz_z0;
        zedMinus = zed - m_z0HalfWidth;
        zedPlus = zed + m_z0HalfWidth;
    }

    const float etaMinus = eta - m_etaHalfWidth;
    const float etaPlus  = eta + m_etaHalfWidth;
    const float phiMinus = CxxUtils::wrapToPi(phi - m_phiHalfWidth);
    const float phiPlus  = CxxUtils::wrapToPi(phi + m_phiHalfWidth);

    // Create the new RoI
    outputRoIHandle->push_back(std::make_unique<TrigRoiDescriptor>(
        roiDescriptor->roiWord(), roiDescriptor->l1Id(), roiDescriptor->roiId(),
      	eta, etaMinus, etaPlus,
      	phi, phiMinus, phiPlus,
      	zed, zedMinus, zedPlus
    ));
    

    ATH_MSG_DEBUG("Input RoI: " << *roiDescriptor);
    ATH_MSG_DEBUG("Output RoI: " << *outputRoIHandle->back());

    return StatusCode::SUCCESS;
}

