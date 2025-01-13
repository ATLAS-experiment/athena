/*
 *   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#ifndef EFTRACKING_DATA_STREAM_LOADER_ALGORITHM
#define EFTRACKING_DATA_STREAM_LOADER_ALGORITHM

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/WriteHandleKey.h"

class EFTrackingDataStreamLoaderAlgorithm : public AthReentrantAlgorithm
{
  Gaudi::Property<std::string> m_inputCsvPath{
    this,
    "inputCsvPath", 
    "", 
    "Path to input csv container."
  };

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

  Gaudi::Property<bool> m_ignoreIsolatedLineFeed {
    this,
    "ignoreIsolatedLineFeed",
    false,
    "Set std::vector entry to zero is csv line only contains line feed."
  };
  
 public:
  EFTrackingDataStreamLoaderAlgorithm(const std::string& name, ISvcLocator* pSvcLocator);
  StatusCode initialize() override final;
  StatusCode execute(const EventContext& ctx) const override final;
};

#endif

