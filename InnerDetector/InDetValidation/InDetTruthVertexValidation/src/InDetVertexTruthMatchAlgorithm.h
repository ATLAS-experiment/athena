/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETVERTEXTRUTHMATCHALGORITHM_H
#define INDETVERTEXTRUTHMATCHALGORITHM_H

// Gaudi/Athena include(s):
#include "AthenaBaseComps/AthAlgorithm.h"
#include "AsgTools/ToolHandle.h"

// Local include(s):
#include "InDetTruthVertexValidation/IInDetVertexTruthMatchTool.h"

class InDetVertexTruthMatchAlgorithm : public AthAlgorithm {

 public:
  /// Regular Algorithm constructor
  using AthAlgorithm::AthAlgorithm;

  /// Function initialising the algorithm
  virtual StatusCode initialize() override final;
  /// Function executing the algorithm
  virtual StatusCode execute(const EventContext& ctx) override final;

 private:
  /// StoreGate key for the muon container to investigate
  Gaudi::Property<std::string> m_sgKey{this, "SGKey", "PrimaryVertices"};
  
  ToolHandle<IInDetVertexTruthMatchTool> m_matchTool{this, "VertexTruthMatchTool", "InDetVertexTruthMatchTool/InDetVertexTruthMatchTool"};

};

#endif
