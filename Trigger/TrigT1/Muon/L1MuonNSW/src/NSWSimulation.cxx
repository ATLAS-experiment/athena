/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
 
#include "NSWSimulation.h"

namespace L1Muon {

  StatusCode NSWSimulation::initialize() {
    ATH_MSG_DEBUG("Initializing " << name() << "...");

    ATH_CHECK(m_keyMmDigit.initialize());
    ATH_CHECK(m_keySTgcDigit.initialize());

    /// Container of output NSW trigger candidates
    ATH_CHECK(m_outputKey.initialize());

    /// retrieve the monitoring tool
    if (!m_monTool.empty()) ATH_CHECK(m_monTool.retrieve());

    return StatusCode::SUCCESS;
  }

  StatusCode NSWSimulation::execute(const EventContext& ctx) const {
    ATH_MSG_DEBUG ("Executing " << name() << "...");

    SG::ReadHandle<MmDigitContainer> mmDigits(m_keyMmDigit, ctx);
    if( !mmDigits.isValid() ){
      ATH_MSG_ERROR("Cannot retrieve MmDigitContainer");
      return StatusCode::FAILURE;
    }

    SG::ReadHandle<sTgcDigitContainer> stgcDigits(m_keySTgcDigit, ctx);
    if(!stgcDigits.isValid()) {
      ATH_MSG_ERROR("Failed to retrieve the sTGC Digit container");
      return StatusCode::FAILURE;
    }
  
    ATH_MSG_DEBUG("Number of MM Digits: " << mmDigits->size());
    ATH_MSG_DEBUG("Number of sTGC Digits: " << stgcDigits->size());
    ATH_MSG_DEBUG("Number of NSW Digits: " << (mmDigits->size() + stgcDigits->size()));

    /// Monitor
    auto nMMDigits = Monitored::Scalar<unsigned int>("nMMDigits", mmDigits->size() );
    auto nsTGCDigits = Monitored::Scalar<unsigned int>("nsTGCDigits", stgcDigits->size() );
    Monitored::Group(m_monTool, nMMDigits, nsTGCDigits);

    SG::WriteHandle<xAOD::L1NSWCandDataContainer> output(m_outputKey, ctx);

    auto container = std::make_unique<xAOD::L1NSWCandDataContainer>();
    auto auxContainer = std::make_unique<xAOD::L1NSWCandDataAuxContainer>();
    container->setStore(auxContainer.get());

    /// Dummy candidates
    xAOD::L1NSWCandData* cand0 = container->push_back(std::make_unique<xAOD::L1NSWCandData>());
    cand0->setL1Bcid(ctx.eventID().bunch_crossing_id());
    cand0->setL1NSegments(2);
    cand0->setL1Overflow(false);
    cand0->setBoardID(1);
    cand0->setFiberID(0);
    cand0->setSegment(100, 50, 2, 4);

    xAOD::L1NSWCandData* cand1 = container->push_back(std::make_unique<xAOD::L1NSWCandData>());
    cand1->setL1Bcid(ctx.eventID().bunch_crossing_id());
    cand1->setL1NSegments(2);
    cand1->setL1Overflow(false);
    cand1->setBoardID(1);
    cand1->setFiberID(0);
    cand1->setSegment(250, 120, 5, 3);

    uint16_t eta0  = cand0->segEtaIndex();
    uint16_t phi0  = cand0->segPhiIndex();
    uint8_t  dth0  = cand0->segDeltaThetaIndex();
    uint8_t  qual0 = cand0->segQuality();

    uint16_t eta1  = cand1->segEtaIndex();
    uint16_t phi1  = cand1->segPhiIndex();
    uint8_t  dth1  = cand1->segDeltaThetaIndex();
    uint8_t  qual1 = cand1->segQuality();

    ATH_MSG_DEBUG("Verification check:"
                  << "\n  Cand0 -> Eta: " << eta0 << ", Phi: " << phi0
                  << ", DeltaTheta: " << static_cast<int>(dth0) << ", Quality: " << static_cast<int>(qual0)
                  << "\n  Cand1 -> Eta: " << eta1 << ", Phi: " << phi1
                  << ", DeltaTheta: " << static_cast<int>(dth1) << ", Quality: " << static_cast<int>(qual1));

    ATH_MSG_DEBUG(*cand0);
    ATH_MSG_DEBUG(*cand1);

    ATH_CHECK(output.record(std::move(container), std::move(auxContainer)));

    ATH_MSG_DEBUG( "Recorded NSW candidate container with " << output->size() << " candidate(s)");

    return StatusCode::SUCCESS;
  }

}   // end of namespace L1Muon
