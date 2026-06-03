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

    CHECK(m_CommonTOBContainerKey.initialize());

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

    if (m_n_multbits == 0){
      m_maxtob = 0;
    } else {
      ulong maxtob = 1;
      for (ulong i = m_n_multbits; i != 0; --i) { maxtob *= 2;}
      m_maxtob = maxtob - 1;
    }

    return StatusCode::SUCCESS;
  }

  
  StatusCode CommonMultAlgTool::updateTIP(std::bitset<s_nbits_TIP>& word,
					     const EventContext& ctx) const {
    auto tobs =
      SG::ReadHandle<GlobalSim::IOBitwise::CommonTOBContainer>(m_CommonTOBContainerKey,
								  ctx);

    CHECK(tobs.isValid());

    // check if any of the incoming tobs is selected.

    ulong tob_count{0};
    std::vector<bool> tob_pass(tobs->size(), false);
    for (int tob_it = 0; const GlobalSim::IOBitwise::CommonTOB* t : *tobs){
      if (m_c_selector->select(*t)) {
	tob_pass[tob_it] = true;
	if (++tob_count == m_maxtob){
	  break;
	}
      }
      tob_it++;
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
      for (const GlobalSim::IOBitwise::CommonTOB* tob : *tobs) {
	ss << tob->to_string()  << ' ' << std::boolalpha << " pass " << tob_pass[ind++] << '\n';
      }
      ss << "tob count " << tob_count << '\n';
 
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
       << m_TIP_position << ' ' << m_n_multbits;

    return ss.str();
  }

}
