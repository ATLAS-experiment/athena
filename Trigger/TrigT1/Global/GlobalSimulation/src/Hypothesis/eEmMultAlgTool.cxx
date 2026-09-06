/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "./eEmMultAlgTool.h"
#include "./CommonSelector.h"
#include "./eEmSelector.h"

#include <fstream>
#include <sstream>

namespace GlobalSim {

  eEmMultAlgTool::eEmMultAlgTool(const std::string& type,
				 const std::string& name,
				 const IInterface* parent) :
    base_class(type, name, parent) {
  }
  
  
  // Initialize function running before first event
  StatusCode eEmMultAlgTool::initialize() {

    ATH_CHECK( TIPWriterAlgTool::initialize() );

    CHECK(m_eEmTOBContainerKey.initialize());

    // create the necessary selector objects
    m_c_selector = std::make_unique<CommonSelector>(m_et_low_str,
						    m_et_high_str,
						    m_eta_low_str,
						    m_eta_high_str,
						    m_phi_low_str,
						    m_phi_high_str
						    );

    try {
      m_e_selector = std::make_unique<eEmSelector>(std::stoul(m_rhad_str),
						   m_rhad_op,
						   std::stoul(m_reta_str),
						   m_reta_op,
						   std::stoul(m_wstot_str),
						   m_wstot_op
						   );
    } catch (const std::exception& e) {
      
      ATH_MSG_ERROR("Error initialising eEMSelector " << e.what());
      return StatusCode::FAILURE;
    }

    return StatusCode::SUCCESS;
  }

  
  StatusCode
  eEmMultAlgTool::countPassingTOBs(const EventContext& ctx,
				   const std::unique_ptr<IDataCollector>& dc, 
				   unsigned int& N_pass_tobs) const {

    auto tobs =
      SG::ReadHandle<GlobalSim::IOBitwise::eEmTOBContainer>(m_eEmTOBContainerKey,
							     ctx);

    CHECK(tobs.isValid());

    // check if any of the incoming tobs is selected.

    if(dc){
      std::stringstream ss;
      ss << "nummber of  tobs "<< tobs->size() << '\n';
      dc->collect(*this, ss.str());
      for (const GlobalSim::IOBitwise::eEmTOB* t : *tobs){
	dc->collect(*this, t->to_string());
      }
    }

    std::vector<bool> tob_pass(tobs->size(), false);
    for (uint tob_it = 0; const GlobalSim::IOBitwise::eEmTOB* t : *tobs){
      if (m_c_selector->select(*t) and m_e_selector->select(*t)) {
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
      for (const GlobalSim::IOBitwise::eEmTOB* tob : *tobs) {
	ss << tob->to_string()  << ' ' << std::boolalpha << " pass " << tob_pass[ind++] << '\n';
      }
      ss << "tob count " << N_pass_tobs << '\n';
 
      std::ofstream out(name() + ".log", std::ios_base::app);
      out << ss.str();
      out.close();
    }

    return StatusCode::SUCCESS;
  }

  std::string eEmMultAlgTool::toString() const {
    std::stringstream ss;
    ss <<name () << ": " <<m_menu_name << ' '
       << "eEmMultAlgTool read, select, count and report number of related eEmTOBS\n"
       << m_c_selector->to_string() << '\n'
       << m_e_selector->to_string() << '\n'
       << m_TIP_position << ' ' << m_TIP_width;

    return ss.str();
  }

  }
