/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "./TIPWriterAlgTool.h"

#include <cmath>
#include <fstream>

namespace GlobalSim {

  TIPWriterAlgTool::TIPWriterAlgTool(const std::string& type,
				 const std::string& name,
				 const IInterface* parent) :
    base_class(type, name, parent) {
  }
  
  
  // Initialize function running before first event
  StatusCode TIPWriterAlgTool::initialize() {

    unsigned int max_tip_pos = s_nbits_TIP - m_TIP_width;

    if (m_TIP_position > max_tip_pos) {
      ATH_MSG_ERROR("TIP word out of bounds " << m_TIP_position);
      return StatusCode::FAILURE;
    }

    m_maxtob = std::pow(2,m_TIP_width.value()) - 1;
    ATH_MSG_VERBOSE(
      "TIP position " << m_TIP_position <<
      ", width " << m_TIP_width <<
      ", max TOBs " << m_maxtob);

    return StatusCode::SUCCESS;
  }

  
  StatusCode TIPWriterAlgTool::updateTIP(
    std::bitset<s_nbits_TIP>& word,
    const std::unique_ptr<IDataCollector>& dc, 
    const EventContext& ctx) const {

    if (dc){dc->collect(*this, "start");}
    
    unsigned int N_pass_tobs{0};
    ATH_CHECK( countPassingTOBs(ctx, N_pass_tobs) );
    ATH_MSG_DEBUG("no of passing TOBS" << N_pass_tobs);

    auto count_bits = std::bitset<s_nbits_TIP>(N_pass_tobs);
    word |= (count_bits << m_TIP_position);
    
    ATH_MSG_DEBUG("TIP word " << word);
    if (dc){dc->collect(*this, "end");}

    return StatusCode::SUCCESS;
  }


  TIPword TIPWriterAlgTool::getFullTIPWord() const {
    TIPword word(m_maxtob);
    word <<= m_TIP_position;
    return word;
  }

}
