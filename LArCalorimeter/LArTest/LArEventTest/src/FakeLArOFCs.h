/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FAKELAROFS
#define FAKELAROFS

#include "AthenaBaseComps/AthAlgorithm.h"

class FakeLArOFCs : public AthAlgorithm {
  
 public:
  using AthAlgorithm::AthAlgorithm;
  ~FakeLArOFCs();

  StatusCode initialize();
  StatusCode execute(const EventContext& ctx);
  StatusCode finalize();
 private:
  Gaudi::Property<short> m_maxSample{this, "MaxSample", 2};
  Gaudi::Property<short> m_NSample{this, "NSample", 5};
  Gaudi::Property<std::string> m_keyOFC{this, "OFCKey", "LArOFC"};
};
#endif
