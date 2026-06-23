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
    
    using OutContainer=GlobalSim::IOBitwise::gFexRhoTOBContainer;

    auto outContainer = std::make_unique<OutContainer>();
 
    outContainer->reserve(inContainer->size());

    using ConcTOB=GlobalSim::IOBitwise::gFexRhoTOB;
    std::transform(std::cbegin(*inContainer),
		   std::cend(*inContainer),
		   std::back_inserter(*outContainer),
		   [](const auto& inTob){
		     return new ConcTOB(*inTob);});

    auto h_write =  SG::WriteHandle<OutContainer>(m_gFexRhoTOBContainerKey,
						  ctx);
    CHECK(h_write.record(std::move(outContainer)));

    return StatusCode::SUCCESS;
  }

  std::string gFexRhoCvtrAlgTool::toString() const {
    return "gFexRhoRoI to gFexRhoTOB converter";
  }
}

