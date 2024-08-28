/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// MdtDigit.cxx

#include "MuonDigitContainer/MdtDigit.h"

MdtDigit::MdtDigit(const Identifier& id, int16_t tdc, int16_t adc, bool isMasked): 
      MuonDigit{id}, m_tdc{tdc}, m_adc{adc}, m_isMasked{isMasked} { }
    
void MdtDigit::setTdc(const int16_t tdc) { m_tdc = tdc; }
void MdtDigit::setAdc(const int16_t adc) { m_adc = adc; }