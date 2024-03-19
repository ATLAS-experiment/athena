/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// MmDigit.h

#ifndef MmDigitUH
#define MmDigitUH

// Nektarios Chr. Benekos
// March 2013
// edit for micromegas needs: Karakostas Konstantinos   <Konstantinos.Karakostas@cern.ch>
//
// MicroMegas digitization.

#include <iosfwd>
#include "MuonDigitContainer/MuonDigit.h"
#include "MuonIdHelpers/MmIdHelper.h"

class MmDigit : public MuonDigit {

 private:  // data
    /** strip response info */
    float m_stripResponseTime{};
    float m_stripResponseCharge{};
 public:  // functions

  /** Default constructor */
  MmDigit() = default;

  /** Full constructor --- From Identifier. */ 
  MmDigit(const Identifier& id);

  /** Full constructor --- From Identifier and time */ 
 
  MmDigit(const Identifier& id,
          const float stripResponseTime,         
          const float stripResponseCharge);

  
   /** strip response info */
  /** Return the time */
  float stripResponseTime() const { return m_stripResponseTime; }
  /** Return the charge */
  float stripResponseCharge() const { return m_stripResponseCharge; }

};

#endif
