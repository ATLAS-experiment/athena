/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// MdtDigit.h

#ifndef MdtDigitUH
#define MdtDigitUH

// David Adams
// November 2001
//
// MDT digitization. Holds a channel ID and a TDC value.

#include <iosfwd>
#include "MuonDigitContainer/MuonDigit.h"
#include "MuonIdHelpers/MdtIdHelper.h"

class MdtDigit : public MuonDigit {
    public:
        // Default constructor.
        MdtDigit() = default;
        // Full constructor for combined measurement mode tdc+adc
        MdtDigit(const Identifier& id, int16_t tdc, int16_t adc, bool isMasked);
        // Return the TDC.
        int16_t tdc() const { return m_tdc; }
        // Return the ADC
        int16_t adc() const { return m_adc; } 
        /// Return whether the digit is masked or not
        bool isMasked() const { return m_isMasked; }
        /// Overwrites a new tdc value
        void setTdc(const int16_t tdc);
        /// Overwrites a new adc value
        void setAdc(const int16_t adc);

    private:
        // TDC value.
        int16_t m_tdc{0};
        // ADC value for combined measurement mode
        int16_t m_adc{0};
        /// Masked lag
        bool m_isMasked{false};


};

#endif
