/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "./eEmMultAlgTool.h"
#include "./CommonSelector.h"
#include "./eEmSelector.h"

#include <fstream>

namespace GlobalSim {

  eEmMultAlgTool::eEmMultAlgTool(const std::string& type,
				 const std::string& name,
				 const IInterface* parent) :
    base_class(type, name, parent) {
  }
  
  
  // Initialize function running before first event
  StatusCode eEmMultAlgTool::initialize() {

    CHECK(m_eEmTOBContainerKey.initialize());

    if (m_n_multbits < 0) {
      ATH_MSG_ERROR("number of bits to write to TIP is negative");
      return StatusCode::FAILURE;
    }
    
    int max_tip_pos = s_nbits_TIP - m_n_multbits;

    if (m_TIP_position < 0 or m_TIP_position > max_tip_pos) {
      ATH_MSG_ERROR("TIP word out of bounds " << m_TIP_position);
      return StatusCode::FAILURE;
    }

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
      
      

    if (m_n_multbits == 0){
      m_maxtob = 0;
    } else {
      ulong maxtob = 1;
      for (ulong i = m_n_multbits; i != 0; --i) { maxtob *= 2;}
      m_maxtob = maxtob - 1;
    }

    return StatusCode::SUCCESS;
  }

  
  StatusCode eEmMultAlgTool::updateTIP(std::bitset<s_nbits_TIP>& word,
				       const EventContext& ctx) const {
    auto tobs =
      SG::ReadHandle<GlobalSim::IOBitwise::IeEmTOBContainer>(m_eEmTOBContainerKey,
							     ctx);

    CHECK(tobs.isValid());

    // check if any of the incoming tobs is selected.

    auto  selected = std::make_unique<bool>(false);

    ulong tob_count{0};
    std::vector<bool> tob_pass(tobs->size(), false);
    for (const GlobalSim::IOBitwise::IeEmTOB* t : *tobs){
      if (m_c_selector->select(*t) and m_e_selector->select(*t)) {
	tob_pass[tob_count] = true;
	if (++tob_count == m_maxtob){
	  tob_pass[tob_count] = true;
	  break;
	}
      }
    }

    

    ATH_MSG_DEBUG("no of passing TOBS" << tob_count);

    auto count_bits = std::bitset<s_nbits_TIP>(tob_count);

    int p0{0};
    int p1{m_TIP_position};
    
    const int& mxb = m_n_multbits;
    
    for (; p0 != mxb; ++p0, ++p1) {
      if (count_bits.test(p0)) {word.set(p1);}
    }

    ATH_MSG_DEBUG("TIP word " << word);

    
    if (m_enableDump) {
      std::stringstream ss;
      ss << "\nRun " << ctx <<' ' << "TIP:\n" << word << '\n';
      std::size_t ind{0};
      for (const GlobalSim::IOBitwise::IeEmTOB* tob : *tobs) {
	ss << *tob  << ' ' << std::boolalpha << " pass " << tob_pass[ind++] << '\n';
      }
      ss << "tob count " << tob_count << '\n';
 
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
       << m_TIP_position << ' ' << m_n_multbits;

    return ss.str();
  }

}
