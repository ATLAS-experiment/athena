/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRUTHRESETALG_H
#define TRUTHRESETALG_H

// Base class include
#include "AthenaBaseComps/AthReentrantAlgorithm.h"

// Athena includes
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "GeneratorObjects/McEventCollection.h"

class TruthResetAlg : public AthReentrantAlgorithm {

public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;
  virtual StatusCode initialize() override final;
  virtual StatusCode execute(const EventContext& ctx) const override final;

private:

  SG::ReadHandleKey<McEventCollection> m_inputMcEventCollection{this, "InputMcEventCollection" , "TruthEvent"};
  SG::WriteHandleKey<McEventCollection> m_outputMcEventCollection{this, "OutputMcEventCollection" , "NewTruthEvent"};

};
#endif
