/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "./gFexRhoCvtrAlgTool.h"

namespace GlobalSim {
  
  gFexRhoCvtrAlgTool::gFexRhoCvtrAlgTool(const std::string& type,
				   const std::string& name,
				   const IInterface* parent):
    base_class(type, name, parent){
  }

  StatusCode gFexRhoCvtrAlgTool::initialize() {
    CHECK(m_gFexJetRoIKey.initialize());
    CHECK(m_gFexRhoTOBContainerKey.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode
  gFexRhoCvtrAlgTool::run(const EventContext& ctx) const {

    SG::ReadHandle<xAOD::gFexJetRoIContainer> inContainer(m_gFexJetRoIKey, ctx);
    CHECK(inContainer.isValid());

    ATH_MSG_DEBUG("Number of gFexRhoROIs read in " << inContainer->size());
    if(inContainer->size() != 3){
      ATH_MSG_ERROR("Expected 3 input gFex Rho TOBs. I received " << inContainer->size());
    }
    
    using OutContainer=GlobalSim::IOBitwise::gFexRhoTOBContainer;
    auto outContainer = std::make_unique<OutContainer>();

    uint rho_bits = 0;
    uint rho_scale = 0;
    for(auto tob:*inContainer){
      ATH_MSG_DEBUG("tob->gFexTobEt() " << tob->gFexTobEt());
      rho_bits += tob->gFexTobEt();
      rho_scale = tob->tobEtScale();
    }

    outContainer->push_back(std::make_unique<IOBitwise::gFexRhoTOB>(rho_bits, rho_scale));

    for(auto tob:*outContainer){
      ATH_MSG_DEBUG("tob->gFexTobEt() " << tob->rho_bits());
    }
    
    auto h_write =  SG::WriteHandle<OutContainer>(m_gFexRhoTOBContainerKey,
						  ctx);
    CHECK(h_write.record(std::move(outContainer)));

    return StatusCode::SUCCESS;
  }

  std::string gFexRhoCvtrAlgTool::toString() const {
    return "gFexRhoRoI to gFexRhoTOB converter";
  }
}

