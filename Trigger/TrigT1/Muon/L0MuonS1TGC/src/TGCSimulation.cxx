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

  SG::ReadHandle  tgcDigitContainer(m_keyTgcDigit, ctx);
  ATH_CHECK(tgcDigitContainer.isPresent());
  ATH_MSG_DEBUG("Number of TGC Digits: " << tgcDigitContainer->size());

  // monitor some quantities
  auto nTgcDigits = Monitored::Scalar<unsigned int>("nTgcDigits", tgcDigitContainer->size());
  Monitored::Group(m_monTool, nTgcDigits);

  
    // output RoIs container
  SG::WriteHandle roiCont(m_outputMuonRoIKey, ctx);
  ATH_CHECK(roiCont.record(std::make_unique<xAOD::MuonRoIContainer>(), 
                           std::make_unique<xAOD::MuonRoIAuxContainer>()));

  xAOD::MuonRoI* roi = roiCont->push_back(std::make_unique<xAOD::MuonRoI>());
  uint32_t roiword = 0x0;
  float roi_eta = 0.0;
  float roi_phi = 0.0;
  std::string thrname = "L0_MUx";
  float thrvalue = 0.0;
  roi->initialize(roiword, roi_eta, roi_phi, thrname, thrvalue, 0x1);  // TODO: roiExtraWord is 1 for the time being.


  return StatusCode::SUCCESS;
}

}   // end of namespace
