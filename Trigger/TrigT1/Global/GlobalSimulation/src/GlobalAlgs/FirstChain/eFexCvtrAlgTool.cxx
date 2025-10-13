/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "./eFexCvtrAlgTool.h"
#include "../../IO/eEmTOB.h"

namespace GlobalSim {
  
  eFexCvtrAlgTool::eFexCvtrAlgTool(const std::string& type,
				   const std::string& name,
				   const IInterface* parent):
    base_class(type, name, parent){
  }

  StatusCode eFexCvtrAlgTool::initialize() {
    CHECK(m_eEmRoIKey.initialize());
    CHECK( m_eEmTOBContainerKey.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode
  eFexCvtrAlgTool::run(const EventContext& ctx) const {

    SG::ReadHandle<xAOD::eFexEMRoIContainer> inContainer(m_eEmRoIKey, ctx);
    CHECK(inContainer.isValid());

    ATH_MSG_DEBUG("Number of eFexROIs read in " << inContainer->size());
    
    using OutContainer=GlobalSim::IOBitwise::IeEmTOBContainer;

    auto outContainer = std::make_unique<OutContainer>();
 
    outContainer->reserve(inContainer->size());


    using ConcTOB=GlobalSim::IOBitwise::eEmTOB;
    std::transform(std::cbegin(*inContainer),
		   std::cend(*inContainer),
		   std::back_inserter(*outContainer),
		   [](const auto& inTob){
		     return new ConcTOB(*inTob);});

    auto h_write =  SG::WriteHandle<OutContainer>(m_eEmTOBContainerKey,
						  ctx);
    CHECK(h_write.record(std::move(outContainer)));

    return StatusCode::SUCCESS;
  }

  
  StatusCode eFexCvtrAlgTool::updateTIP(std::bitset<s_nbits_TIP>& word,
					const EventContext& ctx) const {
    CHECK(IGlobalSimAlgTool::updateTIP(word, ctx));
    return StatusCode::SUCCESS;
  }

  std::string eFexCvtrAlgTool::toString() const {
    return "eFexRoI to eEmTOB converter";
  }
}

