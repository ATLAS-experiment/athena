/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_AP_INT_H
#define GLOBALSIM_AP_INT_H

#include <cstddef>

/*
 * class that represnts an int type of fixed width.
 * Implemented using a native C++ int type, but checks fopr overflow
 * is this occurs using the speeciefied bit width.
 *
 * Note: possibly ap_fixed with precision = 0 could be used instead of ap_int
 */

namespace GlobalSim {

  template <std::size_t n_dig, typename T=int16_t>
  struct ap_int  {
    T m_value{0};

    bool m_ovflw{false};

    ap_int() = default;

    ap_int(const double d) {
      m_value = T(d + (d >= 0 ? 0.5 : -0.5));
      test_overflow();
    }
  
    operator int() const{
      return m_value;
    }
  
  
    static ap_int form(T v) {
      ap_int k; k.m_value = v; k.test_overflow(); return k;
    }

    const ap_int operator + (const ap_int& f) const {
      return form(this->m_value + f.m_value);
    }


    const ap_int& operator += (const ap_int& f) {
      this->m_value +=f.m_value;
      test_overflow();
      return *this;
    }

    ap_int operator - (const ap_int& f) const {
      return form(this->m_value - f.m_value);
    }

  
    const ap_int& operator -= (const ap_int& f) {
      this->m_value -= f.m_value;
      test_overflow();
      return *this;
    }

    ap_int operator * (const ap_int& f) const {
      return form((WS(this->value) * WS(f.m_value)));
    }
  
    const ap_int& operator *= (const ap_int& f) {
      this->m_value -= (WS(this->m_value) * WS(f.m_value));
      test_overflow();
      return *this;
    }

      
    ap_int operator / (const ap_int& f) const {
      return form((WS(this->value)) / WS(f.m_value));
    }
  
    const ap_int& operator /= (const ap_int& f)  {
      this->m_value /= ((WS(this->m_value)) / WS(f.m_value));
      test_overflow();
      return *this;
    }

    // negation
    ap_int operator - () const {
      return form(-this->m_value);
    }
  
    void test_overflow() {
      auto val = m_value >= 0 ? m_value : -m_value;
    
      if (val > (1<< n_dig)) {
	m_ovflw=true;
	throw std::runtime_error("ap_int overflow " + std::to_string(m_value));
      }
    }
  };
}
#endif
