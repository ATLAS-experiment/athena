/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
 
#include "NSWSimulation.h"

namespace L0Muon {

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

    SG::WriteHandle<xAOD::NSWCandDataContainer> output(m_outputKey, ctx);

    auto container = std::make_unique<xAOD::NSWCandDataContainer>();
    auto auxContainer = std::make_unique<xAOD::NSWCandDataAuxContainer>();
    container->setStore(auxContainer.get());

    /// Dummy candidate
    xAOD::NSWCandData* cand = container->push_back(std::make_unique<xAOD::NSWCandData>());
    cand->setBCID(ctx.eventID().bunch_crossing_id());
    cand->setNSegments(2);
    cand->setOverflow(false);
    cand->setBoardId(1);
    cand->setFiberId(0);

    // Push Segment #0
    cand->addSegment(100, 50, 2, 4);
    
    // Push Segment #1
    cand->addSegment(250, 120, 5, 3);

    // Read Segment #0 properties (Index 0)
    uint16_t eta0  = cand->segEtaIndex(0);
    uint16_t phi0  = cand->segPhiIndex(0);
    uint8_t  dth0  = cand->segDeltaThetaIndex(0);
    uint8_t  qual0 = cand->segQuality(0);

    // Read Segment #1 properties (Index 1)
    uint16_t eta1  = cand->segEtaIndex(1);
    uint16_t phi1  = cand->segPhiIndex(1);
    uint8_t  dth1  = cand->segDeltaThetaIndex(1);
    uint8_t  qual1 = cand->segQuality(1);

    ATH_MSG_DEBUG("Verification check:"
                  << "\n  Seg0 -> Eta: " << eta0 << ", Phi: " << phi0 
                  << ", DeltaTheta: " << static_cast<int>(dth0) << ", Quality: " << static_cast<int>(qual0)
                  << "\n  Seg1 -> Eta: " << eta1 << ", Phi: " << phi1 
                  << ", DeltaTheta: " << static_cast<int>(dth1) << ", Quality: " << static_cast<int>(qual1));

    ATH_MSG_DEBUG(*cand);

    cand->clearSegments();
    cand->addSegment(200, 100, 2, 3);
    cand->setNSegments(1);

    ATH_MSG_DEBUG(*cand);

    ATH_CHECK(output.record(std::move(container), std::move(auxContainer)));

    ATH_MSG_DEBUG( "Recorded NSW candidate container with " << output->size() << " candidate(s)");

    return StatusCode::SUCCESS;
  }

}   // end of namespace L0Muon
