/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "./eEmEg1BDTMultAlgTool.h"
#include "./CommonSelector.h"
#include "./eEmSelector.h"
#include "./eEmEg1BDTSelector.h"

#include <fstream>

namespace GlobalSim {

  eEmEg1BDTMultAlgTool::eEmEg1BDTMultAlgTool(const std::string& type,
				 const std::string& name,
				 const IInterface* parent) :
    base_class(type, name, parent) {
  }
  
  
  // Initialize function running before first event
  StatusCode eEmEg1BDTMultAlgTool::initialize() {
    ATH_CHECK(TIPWriterAlgTool::initialize());

    CHECK(m_eEmEg1BDTTOBContainerKey.initialize());

    // create the necessary selector objects
    m_c_selector = std::make_unique<CommonSelector>(m_et_low_str,
						    m_et_high_str,
						    m_eta_low_str,
						    m_eta_high_str,
						    m_phi_low_str,
						    m_phi_high_str
						    );

    try {
      m_bdt_selector = std::make_unique<eEmEg1BDTSelector>(std::stoul(m_Eg1BDT_str),
							   m_Eg1BDT_op
							   );
    } catch (const std::exception& e) {
      
      ATH_MSG_ERROR("Error initialising eEmEg1BDTSelector " << e.what());
      return StatusCode::FAILURE;
    }

    return StatusCode::SUCCESS;
  }

  
  StatusCode eEmEg1BDTMultAlgTool::countPassingTOBs(const EventContext& ctx, unsigned int& N_pass_tobs) const {
    auto tobs =
      SG::ReadHandle<GlobalSim::IOBitwise::eEmEg1BDTTOBContainer>(m_eEmEg1BDTTOBContainerKey,
								  ctx);

    CHECK(tobs.isValid());

    // check if any of the incoming tobs is selected.

    std::vector<bool> tob_pass(tobs->size(), false);
    for (int tob_it = 0; const GlobalSim::IOBitwise::eEmEg1BDTTOB* t : *tobs){
      if (m_c_selector->select(*t) and m_bdt_selector->select(*t)) {
        tob_pass[tob_it] = true;
        if (++N_pass_tobs == m_maxtob){
          break;
        }
      }
      tob_it++;
    }

    ATH_MSG_DEBUG("no of passing TOBS" << N_pass_tobs);

    if (m_enableDump) {
      std::stringstream ss;
      ss << "\nRun " << ctx << '\n';
      std::size_t ind{0};
      for (const GlobalSim::IOBitwise::eEmEg1BDTTOB* tob : *tobs) {
	ss << tob->to_string()  << ' ' << std::boolalpha << " pass " << tob_pass[ind++] << '\n';
      }
      ss << "tob count " << N_pass_tobs << '\n';
 
      std::ofstream out(name() + ".log", std::ios_base::app);
      out << ss.str();
      out.close();
    }

    return StatusCode::SUCCESS;
  }

  std::string eEmEg1BDTMultAlgTool::toString() const {
    std::stringstream ss;
    ss <<name () << ": " <<m_menu_name << ' '
       << "eEmEg1BDTMultAlgTool read, select, count and report number of related eEmEg1BDTTOBS\n"
       << m_c_selector->to_string() << '\n'
       << m_bdt_selector->to_string() << '\n'
       << m_TIP_position << ' ' << m_TIP_width;

    return ss.str();
  }

}
