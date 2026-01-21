/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "StringVector.h"

StringVector::StringVector(const G4ThreeVector& p, G4double m) : m_p(p), m_m(m) {}
StringVector::StringVector(G4double x, G4double y, G4double z, G4double m) : m_p(x,y,z), m_m(m) {}
