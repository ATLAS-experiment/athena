//  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#include "GlobalSimulationAlg.h"
#include "TrigConfData/L1Menu.h"

#include "CxxUtils/checker_macros.h"

#include <fstream>

namespace GlobalSim {

  GlobalSimulationAlg::GlobalSimulationAlg(const std::string& name,
					   ISvcLocator *pSvcLocator):
    AthReentrantAlgorithm(name, pSvcLocator) {
  }
    
  StatusCode GlobalSimulationAlg::initialize () {

    ATH_MSG_INFO("number of TOB creators " << m_algTools.size());
    ATH_MSG_INFO("number of TIP writers " << m_TIPwriters.size());

    CHECK(m_tipWordKey.initialize());
    
    if (m_enableDumps) {
      std::stringstream ss;
      ss << "\nTOB creators\n";
      for (const auto& tool : m_algTools) {
	ss << tool->toString() << '\n';
	ss << "=========\n";
      }
      ss << "\nTIP writers\n";
      for (const auto& tool : m_TIPwriters) {
	ss << tool->toString() << '\n';
	ss << "=========\n";
      }
 
      std::ofstream out(name() + "_init.log");
      out << ss.str();
      out.close();
    }
    
    return StatusCode::SUCCESS;
  }
      
  
 
  StatusCode GlobalSimulationAlg::execute(const EventContext& ctx) const {
    ATH_MSG_DEBUG("Executing ...");


    using TipWord = std::bitset<ITIPwriterAlgTool::s_nbits_TIP>;

    for (const auto& tool : m_algTools) {
      ATH_MSG_DEBUG("Running Algtool " << tool.name());
      CHECK(tool -> run(ctx));
    }

    auto tipword = std::make_unique<TipWord>(); // all zeros
    for (const auto& tool : m_TIPwriters) {
      ATH_MSG_DEBUG("Collecting TIP bits  " << tool.name());
      CHECK(tool -> updateTIP(*tipword, ctx));
    }

    if (m_enableDumps) {
      std::stringstream ss;
      ss << "\nRun " << ctx <<' ' << "TIP:\n" << *tipword << '\n';
      
 
      std::ofstream out(name() + "_tip.log", std::ios_base::app);
      out << ss.str();
      out.close();
    }
    
    ATH_MSG_DEBUG("TIP " << *tipword);
    
    // write out the selection result
    SG::WriteHandle<TipWord> h_write(m_tipWordKey, ctx);
    CHECK(h_write.record(std::move(tipword)));
    
    return StatusCode::SUCCESS;
  }
}
