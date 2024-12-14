/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_AP_FIXED_H
#define GLOBALSIM_AP_FIXED_H

#include "CxxUtils/checker_macros.h"
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;  // due to statics used for debugging

#include <cstddef>
#include <sstream>

/*
 * representation of a fixed point number.
 * A fixed point number has a fiexed width and precision.
 * This implementation uses an C++ int type to store the bits.
 * so allowing fast integer arithmetic.
 */


namespace GlobalSim {

   
  struct Round{};
  struct Trunc{};
  struct XilDef{};
  
  template<std::size_t width, typename T>
  constexpr T max_to_overflow() {
    T t{0};
    static_assert(8*sizeof(t) >= width,  "ap_fixed underlying int to small");
    for (std::size_t i = 0; i <= sizeof(t)*8-width; ++i){
      T bit{1};
      t = t+(bit<<i);
    }

    t = t << (width-1);
    return t;
  }

  template <std::size_t width,
	    std::size_t dp,
	    typename S=XilDef,
	    typename T=int16_t,
	    typename WS=int32_t>
  struct ap_fixed  {
  
    T m_value = T{0};
    static constexpr T m_overflow_mask = max_to_overflow<width, T>();

    static inline bool s_check_overflow{false};
    static inline bool s_print_value{false};
    static inline bool s_debug{s_check_overflow or s_print_value};
    
    bool m_ovflw{false};
    friend std::ostream& operator<<(std::ostream& os,
				    const ap_fixed<width, dp, S,  T, WS> ap) {
      os << ap.m_value << ' ' << double(ap);
      return os;
    }

    ap_fixed() = default;

    ap_fixed(const double d) requires(std::is_same_v<S, Round>)  {
      m_value = T(d * double (1<< dp) + (d >= 0 ? 0.5 : -0.5));
      test_overflow();
    }

    ap_fixed(const double d) requires(std::is_same_v<S, XilDef>){
      m_value = T(d * double (1<< dp) + (d >= 0 ? 0. : -1.0));
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
      return form((WS(this->m_value) * WS(f.m_value)) >> dp);
    }
  
    const ap_fixed& operator *= (const ap_fixed& f) {
      this->m_value = (WS(this->m_value) * WS(f.m_value) >> dp);
      test_overflow();
      return *this;
    }

      
    ap_fixed operator / (const ap_fixed& f) const {
      return form((WS(this->m_value) << dp) / WS(f.m_value));
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

      if (m_value > 0) {
	if (m_value & m_overflow_mask) {
	  m_ovflw=true;
	}
      } else {
	if (-m_value & m_overflow_mask) {
	  m_ovflw=true;
	}
      }

      if (m_ovflw) {
	std::stringstream ss;
	T val = std::abs(m_value);
	
	ss << "ap_fixed overflow. val: " 
	   << m_value << " abs(val): " << val
	   << ' ' << std::hex << val
	   <<  " masked " << (val & m_overflow_mask);
	throw std::out_of_range(ss.str());
      }
    }
  };
}

#endif
