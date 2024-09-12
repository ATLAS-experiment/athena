//  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

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

 
    for (const auto& tool : m_algTools) {
      CHECK(tool -> run(ctx));
    }
    
    return StatusCode::SUCCESS;
  }
}
