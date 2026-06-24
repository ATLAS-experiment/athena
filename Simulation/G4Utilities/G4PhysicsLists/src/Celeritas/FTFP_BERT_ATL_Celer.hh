// SPDX-FileCopyrightText: 2025 CERN
// SPDX-License-Identifier: Apache-2.0

#ifndef FTFP_BERT_ATL_Celer_h
#define FTFP_BERT_ATL_Celer_h 1

#include <CLHEP/Units/SystemOfUnits.h>

#include "globals.hh"
#include "G4VModularPhysicsList.hh"

class FTFP_BERT_ATL_Celer : public G4VModularPhysicsList {
public:
  FTFP_BERT_ATL_Celer(G4int ver = 1);
  virtual ~FTFP_BERT_ATL_Celer() = default;

  FTFP_BERT_ATL_Celer(const FTFP_BERT_ATL_Celer &)            = delete;
  FTFP_BERT_ATL_Celer &operator=(const FTFP_BERT_ATL_Celer &) = delete;
};

#endif
