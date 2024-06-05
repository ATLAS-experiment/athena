/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// MuonDigit.h

#ifndef MuonDigitUH
#define MuonDigitUH

// Muon Digit holds an identifier.

#include <iosfwd>
#include "Identifier/Identifiable.h"
#include "Identifier/Identifier.h"

class MuonDigit : public Identifiable {

protected:  // data

  // ID.
  Identifier m_muonId{};

public:  // functions

  MuonDigit ()  = default;
  virtual ~MuonDigit() = default;
  MuonDigit(const Identifier& id)
    : m_muonId(id) {}
  void setID(const Identifier id) {m_muonId = id;}
  Identifier identify() const {return m_muonId;}

};

#endif


