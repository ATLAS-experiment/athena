/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "./eEmMultAlgTool.h"
#include "./CommonSelector.h"
#include "./eEmSelector.h"

namespace GlobalSim {

  eEmMultAlgTool::eEmMultAlgTool(const std::string& type,
				 const std::string& name,
				 const IInterface* parent) :
    base_class(type, name, parent) {
  }
  
  
  // Initialize function running before first event
  StatusCode eEmMultAlgTool::initialize() {

    CHECK(m_eEmTOBContainerKey.initialize());
    CHECK(m_resultKey.initialize());

    // create the necessary selector objects
    m_c_selector = std::make_unique<CommonSelector>(m_et_low_str,
						    m_et_high_str,
						    m_eta_low_str,
						    m_eta_high_str,
						    m_phi_low_str,
						    m_phi_high_str
						    );

    
    m_e_selector = std::make_unique<eEmSelector>(m_rhad_low_str,
						 m_rhad_high_str,
						 m_reta_low_str,
						 m_reta_high_str,
						 m_wstot_low_str,
						 m_wstot_high_str
						 );

    return StatusCode::SUCCESS;
  }

  
  // Main functional block running for each event
  StatusCode eEmMultAlgTool::run(const EventContext& ctx) const {
    auto tobs =
      SG::ReadHandle<GlobalSim::IOBitwise::IeEmTOBContainer>(m_eEmTOBContainerKey,
							     ctx);

    CHECK(tobs.isValid());

    // check if any of the incoming tobs is selected.
    auto  selected = std::make_unique<bool>(false);
    for (const auto& t : *tobs){
      if (m_c_selector->select(*t) and m_e_selector->select(*t)) {
	*selected=true;
	break;
      }
    }

    // write out the selection result
    SG::WriteHandle<bool> h_write(m_resultKey, ctx);
    CHECK(h_write.record(std::move(selected)));

    return StatusCode::SUCCESS;

  }

  StatusCode eEmMultAlgTool::updateTIP(std::bitset<s_nbits_TIP>& word,
				       const EventContext& ctx) const {
    CHECK(IGlobalSimAlgTool::updateTIP(word, ctx));
    return StatusCode::SUCCESS;
  }
}
