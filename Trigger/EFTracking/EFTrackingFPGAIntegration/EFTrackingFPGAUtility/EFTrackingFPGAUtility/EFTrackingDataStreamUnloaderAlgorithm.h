/*
 *   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#ifndef EFTRACKING_DATA_STREAM_UNLOADER_ALGORITHM
#define EFTRACKING_DATA_STREAM_UNLOADER_ALGORITHM

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"

class EFTrackingDataStreamUnloaderAlgorithm : public AthReentrantAlgorithm
{
  Gaudi::Property<std::string> m_outputCsvPath{
    this,
    "outputCsvPath", 
    "", 
    "Path to output csv container."
  };

  SG::ReadHandleKey<std::vector<unsigned long>> m_outputDataStreamKey{
    this,
    "outputDataStream",
    "outputDataStream",
    "Key to access encoded 64bit words following the EFTracking specification, written as output."
  };
  
 public:
  EFTrackingDataStreamUnloaderAlgorithm(const std::string& name, ISvcLocator* pSvcLocator);
  StatusCode initialize() override final;
  StatusCode execute(const EventContext& ctx) const override final;
};

#endif

