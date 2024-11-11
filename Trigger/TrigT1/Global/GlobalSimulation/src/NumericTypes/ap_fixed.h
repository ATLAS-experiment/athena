/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_AP_FIXED_H
#define GLOBALSIM_AP_FIXED_H

#include <cstddef>
#include <sstream>

/*
 * representation of a fixed point floating point number
 */

namespace GlobalSim {

  template <std::size_t m_dig,
	    std::size_t dp,
	    typename T=int16_t,
	    typename WS=int32_t>
  struct ap_fixed  {
  
    T m_value = T{0};
    bool m_ovflw{false};
    friend std::ostream& operator<<(std::ostream& os,
				    const ap_fixed<m_dig, dp, T, WS> ap) {
      os << ap.m_value << ' ' << double(ap);
      return os;
    }

    ap_fixed() = default;

    ap_fixed(const double d) {
      m_value = T(d * double (1<< dp) + (d >= 0 ? 0.5 : -0.5));
      test_overflow();
    }
      
    operator double() const{
      return double(this->m_value) / double(1 << dp);
    }


    static ap_fixed form(T v) {
      ap_fixed k; k.m_value = v; k.test_overflow();return k;
    }

    const ap_fixed operator + (const ap_fixed& f) const {
      return form(this->m_value + f.m_value);
    }


    const ap_fixed& operator += (const ap_fixed& f)  {
      this->m_value += f.m_value;
      test_overflow();
      return *this;
    }

    ap_fixed operator - (const ap_fixed& f) const {
      return form(this->m_value - f.m_value);
    }

  
    const ap_fixed& operator -= (const ap_fixed& f) {
      this->m_value -= f.m_value;
      test_overflow();
      return *this;
    }

    ap_fixed operator * (const ap_fixed& f) const {
      return form((WS(this->value) * WS(f.m_value)) >> dp);
    }
  
    const ap_fixed& operator *= (const ap_fixed& f) {
      this->m_value = (WS(this->m_value) * WS(f.m_value) >> dp);
      test_overflow();
      return *this;
    }

      
    ap_fixed operator / (const ap_fixed& f) const {
      return form((WS(this->value) << dp) / WS(f.m_value));
    }
  
    const ap_fixed& operator /= (const ap_fixed& f) {
      this->m_value = ((WS(this->m_value) << dp) / WS(f.m_value));
      test_overflow();
      return *this;
    }

    // negation
    ap_fixed operator - () const {
      return form(-this->m_value);
    }

    void test_overflow() {
      // FIXME very hack, assumes int16_t
      if (m_value > 0) {
	if ((m_value & 0xFC00) > 0) {
	  m_ovflw=true;
	}
      } else {
	if ((-m_value & 0xFC00) > 0) {
	  m_ovflw=true;
	}
      }

      if (m_ovflw) {
	std::stringstream ss;
	T val = std::abs(m_value);
	
	ss << "ap_fixed overflow. val: " 
	   << m_value << " abs(val): " << val
	   << ' ' << std::hex << val
	   <<  " masked " << (val &  0xFC00);
	throw std::runtime_error(ss.str());
      }
    }
  };
}

#endif
