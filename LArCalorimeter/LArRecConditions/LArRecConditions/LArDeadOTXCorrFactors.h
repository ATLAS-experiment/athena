/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#ifndef LARRECCONDITIONS_LARDEADOTXCORRFACTORS
#define LARRECCONDITIONS_LARDEADOTXCORRFACTORS

#include <map>
#include <vector>
#include "Identifier/HWIdentifier.h"
#include "Identifier/IdentifierHash.h"

class LArDeadOTXCorrFactors {

 public:
  LArDeadOTXCorrFactors();

  typedef std::map<HWIdentifier, std::vector<std::pair<IdentifierHash, float> > > payload_t;
  //map key: SuperCell HWIdentifer, value.first: dead feb-cell hash, value.second: conversion factor from SC-ET integer to cell E in MeV

  payload_t& get() { return m_scToDeadCellMap; }             // non-const
  const payload_t &get() const { return m_scToDeadCellMap; }  // const

 private:
  payload_t m_scToDeadCellMap;
};

#include "AthenaKernel/CLASS_DEF.h" 
CLASS_DEF(LArDeadOTXCorrFactors,177250913,1)
#include "AthenaKernel/CondCont.h"
CONDCONT_MIXED_DEF( LArDeadOTXCorrFactors , 231229673);

#endif
