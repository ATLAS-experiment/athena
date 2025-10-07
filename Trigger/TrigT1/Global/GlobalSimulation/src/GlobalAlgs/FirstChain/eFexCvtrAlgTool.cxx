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

    using OutContainer=GlobalSim::IOBitwise::IeEmTOBContainer;
    auto outContainer =  SG::WriteHandle<OutContainer>(m_eEmTOBContainerKey,
						      ctx);
    CHECK(outContainer.isValid());
    outContainer->reserve(inContainer->size());

    using ConcTOB=GlobalSim::IOBitwise::eEmTOB;
    std::transform(std::cbegin(*inContainer),
		   std::cend(*inContainer),
		   std::begin(*outContainer),
		   [](const auto& inTob){
		     return std::make_unique<ConcTOB>(*inTob);});

    return StatusCode::SUCCESS;
  }

  std::string eFexCvtrAlgTool::toString() const {
    return "eFexRoI to eEmTOB converter";
  }
}

