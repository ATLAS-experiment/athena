/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// MdtDigit.cxx

#include "MuonDigitContainer/MdtDigit.h"

//**********************************************************************
// Local definitions
//**********************************************************************
namespace {

// Range of TDC.
constexpr int tdcmin = 0;
constexpr int tdcmax = 4096;

}
//**********************************************************************
// Member functions.
//**********************************************************************

// Full constructor from Identifier.
MdtDigit::MdtDigit(const Identifier& id, int tdc)
: MuonDigit(id), m_tdc(tdc), m_adc() { }

//**********************************************************************

// Full constructor from Identifier for the combined measurement mode

MdtDigit::MdtDigit(const Identifier& id, int tdc, int adc)
: MuonDigit(id), m_tdc(tdc), m_adc(adc) { }

//**********************************************************************

// Full constructor including masked flag

MdtDigit::MdtDigit(const Identifier& id, int tdc, int adc, bool isMasked)
: MuonDigit(id), m_tdc(tdc), m_adc(adc), m_isMasked(isMasked) { }

//**********************************************************************

// Validity check.

bool MdtDigit::is_valid(const MdtIdHelper * mdtHelper) const {
  return ( (m_tdc>=tdcmin && m_tdc<=tdcmax) && (mdtHelper->valid(m_muonId)));
}

