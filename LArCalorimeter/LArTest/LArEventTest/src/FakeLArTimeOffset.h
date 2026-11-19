/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FAKELARTIMEOFFSET
#define FAKELARTIMEOFFSET

#include "AthenaBaseComps/AthAlgorithm.h"
#include "LArRawConditions/LArFEBTimeOffset.h"
#include "LArRawConditions/LArGlobalTimeOffset.h"

class FakeLArTimeOffset : public AthAlgorithm {
  
 public:
  using AthAlgorithm::AthAlgorithm;
  ~FakeLArTimeOffset();

  StatusCode initialize();
  StatusCode execute(const EventContext& ctx);
  StatusCode finalize();
 private:
  Gaudi::Property<std::vector<unsigned>> m_FEBids{this, "FEBids", {}};
  Gaudi::Property<std::vector<float>> m_FEBtimeOffsets{this, "FEbTimeOffsets", {}};
  Gaudi::Property<float> m_globalTimeOffset{this, "GlobalTimeOffset", 0};

  Gaudi::Property<std::string> m_keyFebOffset{this, "FebTimeOffsetKey", "FebTimeOffset"};
  Gaudi::Property<std::string> m_keyGlobalOffset{this, "GlobalTimeOffsetKey", "GlobalTimeOffset"};
};

#endif
