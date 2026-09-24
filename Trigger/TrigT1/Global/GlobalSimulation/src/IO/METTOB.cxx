/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "METTOB.h"
#include <sstream>

namespace GlobalSim::IOBitwise {

  METTOB::METTOB(const GlobalSim::IOBitwise::METTOB& tob) :
    CommonTOB(tob),
    m_ex_bits(tob.ex_bits()),
    m_ey_bits(tob.ey_bits()),
    m_sum_et_bits(tob.sum_et_bits()),
    m_ex_overflow_bit(tob.ex_overflow_bit()),
    m_ey_overflow_bit(tob.ey_overflow_bit()),
    m_sum_et_overflow_bit(tob.sum_et_overflow_bit()),
    m_met_overflow_bit(tob.met_overflow_bit()),
    m_upstream_overflow_bit(tob.upstream_overflow_bit()){}


  METTOB::METTOB(const GlobalSim::IOBitwise::CommonTOB& commonTOB,
		 const std::bitset<METTOB::s_ex_width>& ex_bits,
		 const std::bitset<METTOB::s_ey_width>& ey_bits,
		 const std::bitset<METTOB::s_sum_et_width>& sum_et_bits,
		 const std::bitset<METTOB::s_ex_overflow_width>& ex_overflow_bit,
		 const std::bitset<METTOB::s_ey_overflow_width>& ey_overflow_bit,
		 const std::bitset<METTOB::s_sum_et_overflow_width>& sum_et_overflow_bit,
		 const std::bitset<METTOB::s_met_overflow_width>& met_overflow_bit,
		 const std::bitset<METTOB::s_upstream_overflow_width>& upstream_overflow_bit
		 ) :
    CommonTOB(commonTOB),
    m_ex_bits(ex_bits),
    m_ey_bits(ey_bits),
    m_sum_et_bits(sum_et_bits),
    m_ex_overflow_bit(ex_overflow_bit),
    m_ey_overflow_bit(ey_overflow_bit),
    m_sum_et_overflow_bit(sum_et_overflow_bit),
    m_met_overflow_bit(met_overflow_bit),
    m_upstream_overflow_bit(upstream_overflow_bit){
    }

  const std::bitset<METTOB::s_ex_width>& METTOB::ex_bits() const {
    return m_ex_bits;
  }

  const std::bitset<METTOB::s_ey_width>& METTOB::ey_bits() const {
    return m_ey_bits;
  }

  const std::bitset<METTOB::s_sum_et_width>& METTOB::sum_et_bits() const {
    return m_sum_et_bits;
  }

  const std::bitset<METTOB::s_ex_overflow_width>& METTOB::ex_overflow_bit() const {
    return m_ex_overflow_bit;
  }

  const std::bitset<METTOB::s_ey_overflow_width>& METTOB::ey_overflow_bit() const {
    return m_ey_overflow_bit;
  }

  const std::bitset<METTOB::s_sum_et_overflow_width>& METTOB::sum_et_overflow_bit() const {
    return m_sum_et_overflow_bit;
  }

  const std::bitset<METTOB::s_met_overflow_width>& METTOB::met_overflow_bit() const {
    return m_met_overflow_bit;
  }

  const std::bitset<METTOB::s_upstream_overflow_width>& METTOB::upstream_overflow_bit() const {
    return m_upstream_overflow_bit;
  }

  std::string METTOB::to_string() const {

    std::stringstream ss;

    // Ex and Ey are printed as raw sign-magnitude bits and as the signed value they
    // stand for, since the bit pattern alone reads as a large positive number whenever
    // the sign bit is set.
    auto signed_value = [](const auto& bits, std::size_t width) {
      const unsigned long raw = bits.to_ulong();
      const unsigned long magnitude = raw & ((1UL << (width - 1)) - 1UL);
      const bool negative = (raw >> (width - 1)) & 1UL;
      return negative ? -static_cast<long>(magnitude) : static_cast<long>(magnitude);
    };

    ss << '\n'
       << CommonTOB::to_string()
       << " Ex " << ex_bits() << " (" << signed_value(ex_bits(), s_ex_width) << ")"
       << " Ey " << ey_bits() << " (" << signed_value(ey_bits(), s_ey_width) << ")"
       << " SumEt " << sum_et_bits() << " (" << sum_et_bits().to_ulong() << ")"
       << " Ex overflow " << ex_overflow_bit()
       << " Ey overflow " << ey_overflow_bit()
       << " SumEt overflow " << sum_et_overflow_bit()
       << " MET overflow " << met_overflow_bit()
       << " Upstream overflow " << upstream_overflow_bit();
    return ss.str();
  }

}
