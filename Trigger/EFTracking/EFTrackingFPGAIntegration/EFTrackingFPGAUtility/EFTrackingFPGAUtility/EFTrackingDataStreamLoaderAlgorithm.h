/*
 *   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#ifndef EFTRACKING_DATA_STREAM_LOADER_ALGORITHM
#define EFTRACKING_DATA_STREAM_LOADER_ALGORITHM

#include <mutex>

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKeyArray.h"
#include "StoreGate/WriteHandleKeyArray.h"

namespace {
struct FileState {
  int32_t countDown{0};
  enum DataFormatState {
    HEADER = 0,
    HITS = 1,
    FOOTER = 2,
  } dataFormatState{FOOTER};
};

enum DataFormatAction {
  NEW_EVENT = 0,
  KEEP = 1,
  DISCARD = 2,
  ERROR = 3,
};
}

class EFTrackingDataStreamLoaderAlgorithm : public AthReentrantAlgorithm
{
  Gaudi::Property<std::size_t> m_bufferSize {
    this,
    "bufferSize",
    8192,
  };

  Gaudi::Property<std::vector<std::string>> m_GHITZTxtInputPaths{
    this,
    "GHITZTxtInputPaths", 
    {},
  };

  SG::WriteHandleKeyArray<std::vector<uint64_t>> m_GHITZTxtInputKeys{
    this, 
    "GHITZTxtInputKeys", 
    {},
  };

  Gaudi::Property<std::vector<std::string>> m_GHITZTxtOutputPaths{
    this,
    "GHITZTxtOutputPaths", 
    {},
  };

  SG::ReadHandleKeyArray<std::vector<uint64_t>> m_GHITZTxtOutputKeys{
    this, 
    "GHITZTxtOutputKeys", 
    {},
  };

  Gaudi::Property<std::vector<std::string>> m_GHITZBinInputPaths{
    this,
    "GHITZBinInputPaths", 
    {},
  };

  SG::WriteHandleKeyArray<std::vector<uint64_t>> m_GHITZBinInputKeys{
    this, 
    "GHITZBinInputKeys", 
    {},
  };

  Gaudi::Property<std::vector<std::string>> m_GHITZBinOutputPaths{
    this,
    "GHITZBinOutputPaths", 
    {},
  };

  SG::ReadHandleKeyArray<std::vector<uint64_t>> m_GHITZBinOutputKeys{
    this, 
    "GHITZBinOutputKeys", 
    {},
  };

  Gaudi::Property<std::vector<std::string>> m_CLUSTERTxtInputPaths{
    this,
    "CLUSTERTxtInputPaths", 
    {},
  };

  SG::WriteHandleKeyArray<std::vector<uint64_t>> m_CLUSTERTxtInputKeys{
    this, 
    "CLUSTERTxtInputKeys", 
    {},
  };

  Gaudi::Property<std::vector<std::string>> m_CLUSTERTxtOutputPaths{
    this,
    "CLUSTERTxtOutputPaths", 
    {},
  };

  SG::ReadHandleKeyArray<std::vector<uint64_t>> m_CLUSTERTxtOutputKeys{
    this, 
    "CLUSTERTxtOutputKeys", 
    {},
  };

  Gaudi::Property<std::vector<std::string>> m_CLUSTERBinInputPaths{
    this,
    "CLUSTERBinInputPaths", 
    {},
  };

  SG::WriteHandleKeyArray<std::vector<uint64_t>> m_CLUSTERBinInputKeys{
    this, 
    "CLUSTERBinInputKeys", 
    {},
  };

  Gaudi::Property<std::vector<std::string>> m_CLUSTERBinOutputPaths{
    this,
    "CLUSTERBinOutputPaths", 
    {},
  };

  SG::ReadHandleKeyArray<std::vector<uint64_t>> m_CLUSTERBinOutputKeys{
    this, 
    "CLUSTERBinOutputKeys", 
    {},
  };

  std::vector<std::vector<std::vector<uint64_t>>> m_GHITZTxtInputEvents{};
  mutable std::vector<std::vector<std::vector<uint64_t>>> m_GHITZTxtOutputEvents ATLAS_THREAD_SAFE {};

  std::vector<std::vector<std::vector<uint64_t>>> m_GHITZBinInputEvents{};
  mutable std::vector<std::vector<std::vector<uint64_t>>> m_GHITZBinOutputEvents ATLAS_THREAD_SAFE {};

  std::vector<std::vector<std::vector<uint64_t>>> m_CLUSTERTxtInputEvents{};
  mutable std::vector<std::vector<std::vector<uint64_t>>> m_CLUSTERTxtOutputEvents ATLAS_THREAD_SAFE {};

  std::vector<std::vector<std::vector<uint64_t>>> m_CLUSTERBinInputEvents{};
  mutable std::vector<std::vector<std::vector<uint64_t>>> m_CLUSTERBinOutputEvents ATLAS_THREAD_SAFE {};

  mutable std::mutex m_mutex ATLAS_THREAD_SAFE;

  StatusCode readFile(
    const std::string& path,
    const auto& fileReadFunction,  
    const auto& endOfBlockCondition,
    const int32_t hitCountDown,
    std::vector<std::vector<uint64_t>>& events
  );

  StatusCode writeFile(
    const std::string& path,
    const auto& fileWriteFunction,
    const auto& endOfBlockCondition,
    const int32_t hitCountDown,
    const std::vector<std::vector<uint64_t>>& events
  );

  DataFormatAction dataFormatStateMachine(
    const uint64_t word,
    const auto& endOfBlockCondition,
    const int32_t hitCountDown,
    FileState& fileState
  );

 public:
  EFTrackingDataStreamLoaderAlgorithm(const std::string& name, ISvcLocator* pSvcLocator);
  StatusCode initialize() override final;
  StatusCode execute(const EventContext& ctx) const override final;
  StatusCode finalize();
};

#endif

