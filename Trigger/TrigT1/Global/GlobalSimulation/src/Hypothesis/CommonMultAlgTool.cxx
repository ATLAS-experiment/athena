/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "./CommonMultAlgTool.h"
#include "./CommonSelector.h"

#include <fstream>

namespace GlobalSim {

  CommonMultAlgTool::CommonMultAlgTool(const std::string& type,
				 const std::string& name,
				 const IInterface* parent) :
    base_class(type, name, parent) {
  }
  
  
  // Initialize function running before first event
  StatusCode CommonMultAlgTool::initialize() {

    ATH_CHECK( TIPWriterAlgTool::initialize() );

    CHECK(m_CommonTOBContainerKey.initialize());

    // create the necessary selector objects
    m_c_selector = std::make_unique<CommonSelector>(m_et_low_str,
						    m_et_high_str,
						    m_eta_low_str,
						    m_eta_high_str,
						    m_phi_low_str,
						    m_phi_high_str
						    );

    return StatusCode::SUCCESS;
  }

  
  StatusCode CommonMultAlgTool::countPassingTOBs(const EventContext& ctx, unsigned int& N_pass_tobs) const {
    auto tobs =
      SG::ReadHandle<GlobalSim::IOBitwise::CommonTOBContainer>(m_CommonTOBContainerKey,
								  ctx);

    CHECK(tobs.isValid());

    // check if any of the incoming tobs is selected.
    N_pass_tobs = 0;
    std::vector<bool> tob_pass(tobs->size(), false);
    for (int tob_it = 0; const GlobalSim::IOBitwise::CommonTOB* t : *tobs){
      if (m_c_selector->select(*t)) {
        tob_pass[tob_it] = true;
        if (++N_pass_tobs == m_maxtob){
          break;
        }
      }
      tob_it++;
    }
    
    if (m_enableDump) {
      std::stringstream ss;
      ss << "\nRun " << ctx << '\n';
      std::size_t ind{0};
      for (const GlobalSim::IOBitwise::CommonTOB* tob : *tobs) {
	      ss << tob->to_string()  << ' ' << std::boolalpha << " pass " << tob_pass[ind++] << '\n';
      }
      ss << "tob count " << N_pass_tobs << '\n';
 
      std::ofstream out(name() + ".log", std::ios_base::app);
      out << ss.str();
      out.close();
    }

    return StatusCode::SUCCESS;
  }

  std::string CommonMultAlgTool::toString() const {
    std::stringstream ss;
    ss <<name () << ": " <<m_menu_name << ' '
       << "CommonMultAlgTool read, select, count and report number of related CommonTOBS\n"
       << m_c_selector->to_string() << '\n'
       << m_TIP_position << ' ' << m_TIP_width;

    return ss.str();
  }

}
