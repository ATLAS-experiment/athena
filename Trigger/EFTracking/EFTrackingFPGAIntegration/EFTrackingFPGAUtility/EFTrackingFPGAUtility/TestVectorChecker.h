/*
 *   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#ifndef TEST_VECTOR_CHECKER_H
#define TEST_VECTOR_CHECKER_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"

namespace EFTrackingFPGAUtility {
class TestVectorChecker : public AthReentrantAlgorithm
{
  SG::ReadHandleKey<std::vector<unsigned long>> m_outputDataStreamAKey{this, "outputDataStreamA"};
  SG::ReadHandleKey<std::vector<unsigned long>> m_outputDataStreamBKey{this, "outputDataStreamB"};
  Gaudi::Property<std::size_t> m_bufferSize {
    this,
    "bufferSize",
    8192,
    "Capacity of std::vector."
  };

 public:
  TestVectorChecker(const std::string& name, ISvcLocator* pSvcLocator);
  StatusCode initialize() override final;
  StatusCode execute(const EventContext& ctx) const override final;
};
}

#endif

