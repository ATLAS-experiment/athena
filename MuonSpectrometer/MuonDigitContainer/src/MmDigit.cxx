/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// MmDigit.cxx

#include "MuonDigitContainer/MmDigit.h"

//**********************************************************************/
// Full constructor from Identifier.
MmDigit::MmDigit(const Identifier& id)
  : MuonDigit(id) {}
//**********************************************************************/
  
// Full constructor, with trigger Info

MmDigit::MmDigit(const Identifier& id,
                 const float stripResponseTime,                 
                 const float stripResponseCharge)  :
    MuonDigit(id),
    m_stripResponseTime(stripResponseTime),
    m_stripResponseCharge(stripResponseCharge){}
//**********************************************************************/
