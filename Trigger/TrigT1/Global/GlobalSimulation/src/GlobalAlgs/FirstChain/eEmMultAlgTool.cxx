/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "./eEmMultAlgTool.h"
#include "./CommonSelector.h"
#include "./eEmSelector.h"

namespace GlobalSim {

  eEmMultAlgTool::eEmMultAlgTool(const std::string& type,
				 const std::string& name,
				 const IInterface* parent) :
    base_class(type, name, parent) {
  }
  
  
  // Initialize function running before first event
  StatusCode eEmMultAlgTool::initialize() {

    CHECK(m_eEmTOBContainerKey.initialize());

    // to maintain const, use the event store as scratch space
    CHECK(m_multiplicity_in.initialize());
    CHECK(m_multiplicity_out.initialize());

    if (m_n_multbits < 0) {
      ATH_MSG_ERROR("number of bits to write to TIP is negative");
      return StatusCode::FAILURE;
    }
    
    int max_tip_pos = IGlobalSimAlgTool::s_nbits_TIP - m_n_multbits;

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

    
    m_e_selector = std::make_unique<eEmSelector>(m_rhad_low_str,
						 m_rhad_high_str,
						 m_reta_low_str,
						 m_reta_high_str,
						 m_wstot_low_str,
						 m_wstot_high_str
						 );

    return StatusCode::SUCCESS;
  }

  
  // Main functional block running for each event
  StatusCode eEmMultAlgTool::run(const EventContext& ctx) const {
    /*
     * write out the number of TOBS passing cuts, to be read in
     * in updateTIP()
     */
 
    auto tobs =
      SG::ReadHandle<GlobalSim::IOBitwise::IeEmTOBContainer>(m_eEmTOBContainerKey,
							     ctx);

    CHECK(tobs.isValid());

    // check if any of the incoming tobs is selected.

    auto  selected = std::make_unique<bool>(false);

    // m_n_multbits tested to be > 0 in initialize()
    ulong max_mult_unsigned = static_cast<ulong> (m_n_multbits);

    auto tob_count = std::make_unique<ulong>(0);
    for (const auto& t : *tobs){
      if (m_c_selector->select(*t) and m_e_selector->select(*t)) {
	if (++(*tob_count) == max_mult_unsigned){break;}
      }
    }

    
    auto h_write = SG::WriteHandle<ulong>(m_multiplicity_out, ctx);
    CHECK(h_write.record(std::move(tob_count)));
    
    return StatusCode::SUCCESS;
    
  }


  StatusCode eEmMultAlgTool::updateTIP(std::bitset<s_nbits_TIP>& word,
				       const EventContext& ctx) const {

    /*
     * Read in the multiplicity count (written out by run())
     * and set the appropriate buts in the TIP word.
     */
    
    auto tob_count =
      SG::ReadHandle<ulong>(m_multiplicity_in, ctx);
    
    auto count_bits = std::bitset<IGlobalSimAlgTool::s_nbits_TIP>(*tob_count);

    int p0{0};
    int p1{m_TIP_position};
    
    const int& mxb = m_n_multbits;

    for (; p0 != mxb; ++p0, ++p1) {
      if (count_bits.test(p0)) {word.set(p1);}
    }
	   
    return StatusCode::SUCCESS;
  }

}
