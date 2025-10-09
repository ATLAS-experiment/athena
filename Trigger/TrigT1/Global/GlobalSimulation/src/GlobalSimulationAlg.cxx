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

    ATH_MSG_INFO("number of AlgTools " << m_algTools.size());

    CHECK(m_tipWordKey.initialize());
    
    if (m_enableDumps) {
      std::stringstream ss;
      for (const auto& tool : m_algTools) {
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


    using TipWord = std::bitset<IGlobalSimAlgTool::s_nbits_TIP>;

    for (const auto& tool : m_algTools) {
      ATH_MSG_DEBUG("Running Algtool " << tool.name());
      CHECK(tool -> run(ctx));
    }

    auto tipword = std::make_unique<TipWord>(); // all zeros
    for (const auto& tool : m_algTools) {
      ATH_MSG_DEBUG("Collecting TIP bits  " << tool.name());
      CHECK(tool -> updateTIP(*tipword, ctx));
    }

    // write out the selection result
    SG::WriteHandle<TipWord> h_write(m_tipWordKey, ctx);
    CHECK(h_write.record(std::move(tipword)));
    
    return StatusCode::SUCCESS;
  }
}
