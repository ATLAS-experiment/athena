/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
 
#include "TGCSimulation.h"

#include "xAODTrigger/MuonRoIAuxContainer.h"

namespace L0Muon {

StatusCode TGCSimulation::initialize() {
  ATH_MSG_DEBUG("Initializing " << name() << "...");

  ATH_CHECK(m_keyTgcDigit.initialize());

  /// container of output RoIs
  ATH_CHECK(m_outputMuonRoIKey.initialize());

  // TGC cabling service
  if (!m_cabling) {
    ATH_CHECK(m_cabling.retrieve());
  }
  
  /// retrieve the monitoring tool
  if (!m_monTool.empty()) ATH_CHECK(m_monTool.retrieve());

  return StatusCode::SUCCESS;
}


StatusCode TGCSimulation::execute(const EventContext& ctx) const {
  ATH_MSG_DEBUG ("Executing " << name() << "...");

  SG::ReadHandle<TgcDigitContainer> tgcDigit_handle(m_keyTgcDigit, ctx);
  if (!tgcDigit_handle.isValid()) {
    ATH_MSG_ERROR("Unable to retrieve input TGCDigitContainer: " << m_keyTgcDigit.key());
    return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG("Number of TGC Digits: " << tgcDigit_handle->size());

  // monitor some quantities
  auto nTgcDigits = Monitored::Scalar<unsigned int>("nTgcDigits", tgcDigit_handle->size());
  Monitored::Group(m_monTool, nTgcDigits);

  // Create an RoI (as an example)
  auto roiCont = std::make_unique<xAOD::MuonRoIContainer>();
  auto roiAuxCont = std::make_unique<xAOD::MuonRoIAuxContainer>();;
  roiCont->setStore(roiAuxCont.get());

  roiCont->push_back(std::make_unique<xAOD::MuonRoI>());
  uint32_t roiword = 0x0;
  float roi_eta = 0.0;
  float roi_phi = 0.0;
  std::string thrname = "L0_MUx";
  float thrvalue = 0.0;
  roiCont->back()->initialize(roiword, roi_eta, roi_phi, thrname, thrvalue, 0x1);  // TODO: roiExtraWord is 1 for the time being.

  // output RoIs container
  SG::WriteHandle<xAOD::MuonRoIContainer> outputRoI_handle(m_outputMuonRoIKey, ctx);
  ATH_CHECK(outputRoI_handle.record(std::move(roiCont), std::move(roiAuxCont)));

  return StatusCode::SUCCESS;
}

}   // end of namespace
