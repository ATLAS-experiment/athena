/*
 *   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#ifndef TEST_VECTOR_GENERATOR_H
#define TEST_VECTOR_GENERATOR_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/WriteHandleKey.h"

namespace EFTrackingFPGAUtility {
class TestVectorGenerator : public AthReentrantAlgorithm
{
  SG::WriteHandleKey<std::vector<unsigned long>> m_inputDataStreamKey{
    this,
    "inputDataStream",
    "",
    "Key to access encoded 64bit words following the EFTracking specification, read as input."
  };

  Gaudi::Property<std::size_t> m_bufferSize {
    this,
    "bufferSize",
    8192,
    "Capacity of std::vector."
  };

 public:
  TestVectorGenerator(const std::string& name, ISvcLocator* pSvcLocator);
  StatusCode initialize() override final;
  StatusCode execute(const EventContext& ctx) const override final;
};
}

#endif

