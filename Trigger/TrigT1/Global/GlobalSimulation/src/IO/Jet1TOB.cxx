/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "Jet1TOB.h"
#include <sstream>

namespace GlobalSim::IOBitwise {

  Jet1TOB::Jet1TOB(const GlobalSim::IOBitwise::Jet1TOB& tob) :
    CommonTOB(tob),
    m_ring_zero_ptt_bits(tob.ring_zero_ptt_bits()),
    m_ring_one_ptt_bits(tob.ring_one_ptt_bits()),
    m_ring_two_ptt_bits(tob.ring_two_ptt_bits()),
    m_ring_three_ptt_bits(tob.ring_three_ptt_bits()),
    m_ring_four_ptt_bits(tob.ring_four_ptt_bits()),
    m_total_tobs_bits(tob.total_tobs_bits()),
    m_ring_one_tobs_bits(tob.ring_one_tobs_bits()),
    m_ring_two_tobs_bits(tob.ring_two_tobs_bits()),
    m_ring_three_tobs_bits(tob.ring_three_tobs_bits()),
    m_ring_four_tobs_bits(tob.ring_four_tobs_bits()),
    m_truncate_bit(tob.truncate_bit()),
    m_next_tob_same_et_bit(tob.next_tob_same_et_bit()),
    m_error_bit(tob.error_bit()),
    m_et_overflow_bit(tob.et_overflow_bit()){}

  
  Jet1TOB::Jet1TOB(const GlobalSim::IOBitwise::CommonTOB& commonTOB,
		   const std::bitset<Jet1TOB::s_ring_zero_ptt_width>& ring_zero_ptt_bits,
		   const std::bitset<Jet1TOB::s_ring_one_ptt_width>& ring_one_ptt_bits,
		   const std::bitset<Jet1TOB::s_ring_two_ptt_width>& ring_two_ptt_bits,
		   const std::bitset<Jet1TOB::s_ring_three_ptt_width>& ring_three_ptt_bits,
		   const std::bitset<Jet1TOB::s_ring_four_ptt_width>& ring_four_ptt_bits,
		   const std::bitset<Jet1TOB::s_total_tobs_width>& total_tobs_bits,
		   const std::bitset<Jet1TOB::s_ring_one_tobs_width>& ring_one_tobs_bits,
		   const std::bitset<Jet1TOB::s_ring_two_tobs_width>&  ring_two_tobs_bits,
		   const std::bitset<Jet1TOB::s_ring_three_tobs_width>&  ring_three_tobs_bits,
		   const std::bitset<Jet1TOB::s_ring_four_tobs_width>&  ring_four_tobs_bits,
		   const std::bitset<Jet1TOB::s_truncate_width>&  truncate_bit,
		   const std::bitset<Jet1TOB::s_next_tob_same_et_width>&  next_tob_same_et_bit, 
		   const std::bitset<Jet1TOB::s_error_width>&  error_bit,
		   const std::bitset<Jet1TOB::s_et_overflow_width>& et_overflow_bit
		   ) :
    CommonTOB(commonTOB),
    m_ring_zero_ptt_bits(ring_zero_ptt_bits),
    m_ring_one_ptt_bits(ring_one_ptt_bits),
    m_ring_two_ptt_bits(ring_two_ptt_bits),
    m_ring_three_ptt_bits(ring_three_ptt_bits),
    m_ring_four_ptt_bits(ring_four_ptt_bits),
    m_total_tobs_bits(total_tobs_bits),
    m_ring_one_tobs_bits(ring_one_tobs_bits),
    m_ring_two_tobs_bits(ring_two_tobs_bits),
    m_ring_three_tobs_bits(ring_three_tobs_bits),
    m_ring_four_tobs_bits(ring_four_tobs_bits),
    m_truncate_bit(truncate_bit),
    m_next_tob_same_et_bit(next_tob_same_et_bit),
    m_error_bit(error_bit),
    m_et_overflow_bit(et_overflow_bit){
    }
 
  const std::bitset<Jet1TOB::s_ring_zero_ptt_width>& Jet1TOB::ring_zero_ptt_bits() const {
    return m_ring_zero_ptt_bits;
  }

  const std::bitset<Jet1TOB::s_ring_one_ptt_width>& Jet1TOB::ring_one_ptt_bits() const {
    return m_ring_one_ptt_bits;
  }

  const std::bitset<Jet1TOB::s_ring_two_ptt_width>& Jet1TOB::ring_two_ptt_bits() const {
    return m_ring_two_ptt_bits;
  }

  const std::bitset<Jet1TOB::s_ring_three_ptt_width>& Jet1TOB::ring_three_ptt_bits() const {
    return m_ring_three_ptt_bits;
  }

  const std::bitset<Jet1TOB::s_ring_four_ptt_width>& Jet1TOB::ring_four_ptt_bits() const {
    return m_ring_four_ptt_bits;
  }

  const std::bitset<Jet1TOB::s_total_tobs_width>& Jet1TOB::total_tobs_bits() const {
    return m_total_tobs_bits;
  }

  const std::bitset<Jet1TOB::s_ring_one_tobs_width>& Jet1TOB::ring_one_tobs_bits() const {
    return m_ring_one_tobs_bits;
  }

  const std::bitset<Jet1TOB::s_ring_two_tobs_width>& Jet1TOB::ring_two_tobs_bits() const {
    return m_ring_two_tobs_bits;
  }

  const std::bitset<Jet1TOB::s_ring_three_tobs_width>& Jet1TOB::ring_three_tobs_bits() const {
    return m_ring_three_tobs_bits;
  }

  const std::bitset<Jet1TOB::s_ring_four_tobs_width>& Jet1TOB::ring_four_tobs_bits() const {
    return m_ring_four_tobs_bits;
  }

  const std::bitset<Jet1TOB::s_truncate_width>& Jet1TOB::truncate_bit() const {
    return m_truncate_bit;
  }

  const std::bitset<Jet1TOB::s_next_tob_same_et_width>& Jet1TOB::next_tob_same_et_bit() const {
    return m_next_tob_same_et_bit;
  }

  const std::bitset<Jet1TOB::s_error_width>& Jet1TOB::error_bit() const {
    return m_error_bit;
  }

  const std::bitset<Jet1TOB::s_et_overflow_width>& Jet1TOB::et_overflow_bit() const {
    return m_et_overflow_bit;
  }

  std::string Jet1TOB::to_string() const {

    std::stringstream ss;
    
    ss << '\n'
       << CommonTOB::to_string()
       << " Ring zero ptt " << ring_zero_ptt_bits() << " (" <<   ring_zero_ptt_bits().to_ulong() << ")"
       << " Ring one ptt " << ring_one_ptt_bits() << " (" <<   ring_one_ptt_bits().to_ulong() << ")"
       << " Ring two ptt " << ring_two_ptt_bits() << " (" <<   ring_two_ptt_bits().to_ulong() << ")"       
       << " Ring three ptt " << ring_three_ptt_bits() << " (" <<   ring_three_ptt_bits().to_ulong() << ")"
       << " Ring four ptt " << ring_four_ptt_bits() << " (" <<   ring_four_ptt_bits().to_ulong() << ")"
       << " Total tobs " << total_tobs_bits() << " (" <<   total_tobs_bits().to_ulong() << ")"
       << " Ring one tobs " << ring_one_tobs_bits() << " (" <<   ring_one_tobs_bits().to_ulong() << ")"
       << " Ring two tobs " << ring_two_tobs_bits() << " (" <<   ring_two_tobs_bits().to_ulong() << ")"       
       << " Ring three tobs " << ring_three_tobs_bits() << " (" <<   ring_three_tobs_bits().to_ulong() << ")"
       << " Ring four tobs " << ring_four_tobs_bits() << " (" <<   ring_four_tobs_bits().to_ulong() << ")";
      return ss.str();
  }

}
