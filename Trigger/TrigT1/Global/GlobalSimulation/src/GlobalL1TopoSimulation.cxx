//  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#include "GlobalL1TopoSimulation.h"
#include "TrigConfData/L1Menu.h"

#include "CxxUtils/checker_macros.h"

#include <fstream>

namespace GlobalSim {

  GlobalL1TopoSimulation::GlobalL1TopoSimulation(const std::string& name,
						 ISvcLocator *pSvcLocator):
    AthReentrantAlgorithm(name, pSvcLocator) {
  }
    
  StatusCode GlobalL1TopoSimulation::initialize () {

    ATH_MSG_INFO("number of L1TopoAlgTools " << m_topoAlgs.size());

    if (m_enableDumps) {
      std::stringstream ss;
      for (const auto& tool : m_topoAlgs) {
	ss << tool->toString() << '\n';
	ss << "=========\n";
      }
      std::ofstream out(name() + "_init.log");
      out << ss.str();
      out.close();
    }
    
    return StatusCode::SUCCESS;
  }
      
  
 
  StatusCode GlobalL1TopoSimulation::execute(const EventContext& ctx) const {
    ATH_MSG_DEBUG("Executing ...");

 
    for (const auto& tool : m_topoAlgs) {
      CHECK(tool -> run(ctx));
    }
    
    return StatusCode::SUCCESS;
  }
}
