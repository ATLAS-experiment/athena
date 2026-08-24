//  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#include "GlobalSimulationAlg.h"
#include "TrigConfData/L1Menu.h"

#include "../Utilities/BasicDataCollector.h"
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

    // If we only run TOB creators, then don't write the TIP word
    // as this leads to SG clashes (or spurious stuff in SG)
    CHECK(m_tipWordKey.initialize(!m_TIPwriters.empty()));

    if (m_enableDumps) {
      std::stringstream ss;
      ss << "\nTOB creators (" << m_algTools.size() << ")\n";
      for (const auto& tool : m_algTools) {
        ss << tool->toString() << '\n';
        ss << "=========\n";
      }
      ss << "\nTIP writers (" << m_TIPwriters.size() << ")\n";
      for (const auto& tool : m_TIPwriters) {
        ss << tool->toString() << '\n';
        ss << "=========\n";
      }
 
      std::ofstream out(name() + "_init.log");
      out << ss.str();
      out.close();
    }
    
    // Test that TIPwriters don't clash
    TIPword tip_test;
    for (const auto& tool : m_TIPwriters) {
      ATH_MSG_VERBOSE("Current TIP word: " << tip_test);
      TIPword tip_tmp{tool->getFullTIPWord()};
      ATH_MSG_VERBOSE("TIP writer " << tool.name() << " produces: " << tip_tmp);
      if((tip_tmp & tip_test).any()) {
        ATH_MSG_ERROR("TIP word clash from " << tool->toString());
        return StatusCode::FAILURE;
      }
      tip_test |= tip_tmp;
    }

    return StatusCode::SUCCESS;
  }
      
  
 
  StatusCode GlobalSimulationAlg::execute(const EventContext& ctx) const {
    ATH_MSG_DEBUG("Executing ...");


    using TIPWord = std::bitset<ITIPWriterAlgTool::s_nbits_TIP>;

    auto dc = std::unique_ptr<IDataCollector>(nullptr);
    if(m_enableDumps){
      dc.reset(new BasicDataCollector());
    }

    for (const auto& tool : m_algTools) {
      ATH_MSG_DEBUG("Running Algtool " << tool.name());
      CHECK(tool -> run(dc, ctx));
    }

    if(m_TIPwriters.empty()) {return StatusCode::SUCCESS;}
      
    auto tipword = std::make_unique<TIPWord>(); // all zeros
    for (const auto& tool : m_TIPwriters) {
      ATH_MSG_DEBUG("Collecting TIP bits  " << tool.name());
      CHECK(tool -> updateTIP(*tipword, dc, ctx));
    }

    if (m_enableDumps) {
      
      {
	std::stringstream ss;
	ss << "\nRun " << ctx <<' ' << "TIP:\n" << *tipword << '\n';
	
	std::ofstream out(name() + "_tip.log", std::ios_base::app);
	out << ss.str();
	out.close();
      }

      {
	std::stringstream ss;
	ss << name() << "_datacol_"<< ctx.eventID().event_number()<<".log";
	std::ofstream out(ss.str());
	out << dc->to_string();
	out.close();
      }
    }
      
    ATH_MSG_DEBUG("TIP " << *tipword);
      
    // write out the selection result
    SG::WriteHandle<TIPWord> h_write(m_tipWordKey, ctx);
    CHECK(h_write.record(std::move(tipword)));
    
    return StatusCode::SUCCESS;
  }
}
