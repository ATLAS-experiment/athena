/*
 *   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#include "EFTrackingFPGAUtility/EFTrackingDataStreamLoaderAlgorithm.h"
#include "EFTrackingFPGAUtility/FPGADataFormatUtilities.h"

#include <optional>
#include <vector>
#include <fstream>

EFTrackingDataStreamLoaderAlgorithm::EFTrackingDataStreamLoaderAlgorithm(
  const std::string& name,
  ISvcLocator* pSvcLocator
) : AthReentrantAlgorithm(name, pSvcLocator)
{}

StatusCode EFTrackingDataStreamLoaderAlgorithm::initialize() {
  ATH_CHECK(m_GHITZTxtInputKeys.initialize());
  ATH_CHECK(m_GHITZTxtOutputKeys.initialize());
  ATH_CHECK(m_GHITZTxtInputKeys.size() == m_GHITZTxtInputPaths.size());

  for (std::size_t index = 0; index < m_GHITZTxtInputKeys.size(); index++) {
    m_GHITZTxtInputEvents.push_back({});
    ATH_CHECK(readFile(
      m_GHITZTxtInputPaths.value().at(index), 
      [](std::ifstream& fileStream) {
        std::string line{};
        std::getline(fileStream, line);
        return std::stoul(line, nullptr, 16);
      },
      [](const uint64_t word) { return FPGADataFormatUtilities::get_bitfields_GHITZ_w1(word).last; },
      1,
      m_GHITZTxtInputEvents.back()
    ));
  }

  for (std::size_t index = 0; index < m_GHITZTxtOutputKeys.size(); index++) {
    m_GHITZTxtOutputEvents.push_back({});
  }

  ATH_CHECK(m_GHITZBinInputKeys.initialize());
  ATH_CHECK(m_GHITZBinOutputKeys.initialize());
  ATH_CHECK(m_GHITZBinInputKeys.size() == m_GHITZBinInputPaths.size());

  for (std::size_t index = 0; index < m_GHITZBinInputKeys.size(); index++) {
    m_GHITZBinInputEvents.push_back({});
    ATH_CHECK(readFile(
      m_GHITZBinInputPaths.value().at(index), 
      [](std::ifstream& fileStream) {
        std::array<char, 8> line;
        fileStream.read(line.data(), 8);
        return *std::bit_cast<uint64_t*>(line.data());
      },
      [](const uint64_t word) { return FPGADataFormatUtilities::get_bitfields_GHITZ_w1(word).last; },
      1,
      m_GHITZBinInputEvents.back()
    ));
  }

  for (std::size_t index = 0; index < m_GHITZBinOutputKeys.size(); index++) {
    m_GHITZBinOutputEvents.push_back({});
  }

  ATH_CHECK(m_CLUSTERTxtInputKeys.initialize());
  ATH_CHECK(m_CLUSTERTxtOutputKeys.initialize());
  ATH_CHECK(m_CLUSTERTxtInputKeys.size() == m_CLUSTERTxtInputPaths.size());

  for (std::size_t index = 0; index < m_CLUSTERTxtInputKeys.size(); index++) {
    m_CLUSTERTxtInputEvents.push_back({});
    ATH_CHECK(readFile(
      m_CLUSTERTxtInputPaths.value().at(index), 
      [](std::ifstream& fileStream) {
        std::string line{};
        std::getline(fileStream, line);
        return std::stoul(line, nullptr, 16);
      },
      [](const uint64_t word) { return FPGADataFormatUtilities::get_bitfields_STRIP_CLUSTER(word).last; },
      0,
      m_CLUSTERTxtInputEvents.back()
    ));
  }

  for (std::size_t index = 0; index < m_CLUSTERTxtOutputKeys.size(); index++) {
    m_CLUSTERTxtOutputEvents.push_back({});
  }

  ATH_CHECK(m_CLUSTERBinInputKeys.initialize());
  ATH_CHECK(m_CLUSTERBinOutputKeys.initialize());
  ATH_CHECK(m_CLUSTERBinInputKeys.size() == m_CLUSTERBinInputPaths.size());

  for (std::size_t index = 0; index < m_CLUSTERBinInputKeys.size(); index++) {
    m_CLUSTERBinInputEvents.push_back({});
    ATH_CHECK(readFile(
      m_CLUSTERBinInputPaths.value().at(index), 
      [](std::ifstream& fileStream) {
        std::array<char, 8> line;
        fileStream.read(line.data(), 8);
        return *std::bit_cast<uint64_t*>(line.data());
      },
      [](const uint64_t word) { return FPGADataFormatUtilities::get_bitfields_STRIP_CLUSTER(word).last; },
      0,
      m_CLUSTERBinInputEvents.back()
    ));
  }

  for (std::size_t index = 0; index < m_CLUSTERBinOutputKeys.size(); index++) {
    m_CLUSTERBinOutputEvents.push_back({});
  }

  return StatusCode::SUCCESS;
}

StatusCode EFTrackingDataStreamLoaderAlgorithm::execute(const EventContext& ctx) const {
  ATH_CHECK(m_GHITZTxtInputKeys.size() == m_GHITZTxtInputEvents.size());
  for (std::size_t index = 0; index < m_GHITZTxtInputKeys.size(); index++) {
    ATH_CHECK(ctx.evt() < m_GHITZTxtInputEvents.at(index).size());
    SG::WriteHandle<std::vector<uint64_t>> GHITZTxtInputHandle(m_GHITZTxtInputKeys[index], ctx);
    ATH_CHECK(GHITZTxtInputHandle.record(std::make_unique<std::vector<uint64_t>>(
      m_GHITZTxtInputEvents.at(index).at(ctx.evt())
    )));
  }

  ATH_CHECK(m_GHITZBinInputKeys.size() == m_GHITZBinInputEvents.size());
  for (std::size_t index = 0; index < m_GHITZBinInputKeys.size(); index++) {
    ATH_CHECK(ctx.evt() < m_GHITZBinInputEvents.at(index).size());
    SG::WriteHandle<std::vector<uint64_t>> GHITZBinInputHandle(m_GHITZBinInputKeys[index], ctx);
    ATH_CHECK(GHITZBinInputHandle.record(std::make_unique<std::vector<uint64_t>>(
      m_GHITZBinInputEvents.at(index).at(ctx.evt())
    )));
  }

  ATH_CHECK(m_CLUSTERTxtInputKeys.size() == m_CLUSTERTxtInputEvents.size());
  for (std::size_t index = 0; index < m_CLUSTERTxtInputKeys.size(); index++) {
    ATH_CHECK(ctx.evt() < m_CLUSTERTxtInputEvents.at(index).size());
    SG::WriteHandle<std::vector<uint64_t>> CLUSTERTxtInputHandle(m_CLUSTERTxtInputKeys[index], ctx);
    ATH_CHECK(CLUSTERTxtInputHandle.record(std::make_unique<std::vector<uint64_t>>(
      m_CLUSTERTxtInputEvents.at(index).at(ctx.evt())
    )));
  }

  ATH_CHECK(m_CLUSTERBinInputKeys.size() == m_CLUSTERBinInputEvents.size());
  for (std::size_t index = 0; index < m_CLUSTERBinInputKeys.size(); index++) {
    ATH_CHECK(ctx.evt() < m_CLUSTERBinInputEvents.at(index).size());
    SG::WriteHandle<std::vector<uint64_t>> CLUSTERBinInputHandle(m_CLUSTERBinInputKeys[index], ctx);
    ATH_CHECK(CLUSTERBinInputHandle.record(std::make_unique<std::vector<uint64_t>>(
      m_CLUSTERBinInputEvents.at(index).at(ctx.evt())
    )));
  }

  {
    std::lock_guard lock(m_mutex);
    for (std::size_t index = 0; index < m_GHITZTxtOutputKeys.size(); index++) {
      if (m_GHITZTxtOutputEvents.at(index).size() < (ctx.evt() + 1)) {
        m_GHITZTxtOutputEvents.at(index).resize(ctx.evt() + 1);
      }

      ATH_CHECK(ctx.evt() < m_GHITZTxtOutputEvents.at(index).size());
      SG::ReadHandle<std::vector<uint64_t>> GHITZTxtOutputHandle(m_GHITZTxtOutputKeys[index], ctx);
      m_GHITZTxtOutputEvents.at(index).at(ctx.evt()) = *GHITZTxtOutputHandle;
    }

    for (std::size_t index = 0; index < m_GHITZBinOutputKeys.size(); index++) {
      if (m_GHITZBinOutputEvents.at(index).size() < (ctx.evt() + 1)) {
        m_GHITZBinOutputEvents.at(index).resize(ctx.evt() + 1);
      }

      ATH_CHECK(ctx.evt() < m_GHITZBinOutputEvents.at(index).size());
      SG::ReadHandle<std::vector<uint64_t>> GHITZBinOutputHandle(m_GHITZBinOutputKeys[index], ctx);
      m_GHITZBinOutputEvents.at(index).at(ctx.evt()) = *GHITZBinOutputHandle;
    }

    for (std::size_t index = 0; index < m_CLUSTERTxtOutputKeys.size(); index++) {
      if (m_CLUSTERTxtOutputEvents.at(index).size() < (ctx.evt() + 1)) {
        m_CLUSTERTxtOutputEvents.at(index).resize(ctx.evt() + 1);
      }

      ATH_CHECK(ctx.evt() < m_CLUSTERTxtOutputEvents.at(index).size());
      SG::ReadHandle<std::vector<uint64_t>> CLUSTERTxtOutputHandle(m_CLUSTERTxtOutputKeys[index], ctx);
      m_CLUSTERTxtOutputEvents.at(index).at(ctx.evt()) = *CLUSTERTxtOutputHandle;
    }

    for (std::size_t index = 0; index < m_CLUSTERBinOutputKeys.size(); index++) {
      if (m_CLUSTERBinOutputEvents.at(index).size() < (ctx.evt() + 1)) {
        m_CLUSTERBinOutputEvents.at(index).resize(ctx.evt() + 1);
      }

      ATH_CHECK(ctx.evt() < m_CLUSTERBinOutputEvents.at(index).size());
      SG::ReadHandle<std::vector<uint64_t>> CLUSTERBinOutputHandle(m_CLUSTERBinOutputKeys[index], ctx);
      m_CLUSTERBinOutputEvents.at(index).at(ctx.evt()) = *CLUSTERBinOutputHandle;
    }
  }

  return StatusCode::SUCCESS;
}

StatusCode EFTrackingDataStreamLoaderAlgorithm::finalize() {
  ATH_CHECK(m_GHITZTxtOutputKeys.size() == m_GHITZTxtOutputPaths.size());

  for (std::size_t index = 0; index < m_GHITZTxtOutputKeys.size(); index++) {
    ATH_CHECK(writeFile(
      m_GHITZTxtOutputPaths.value().at(index), 
      [](
        std::ofstream& fileStream, 
        const uint64_t word
      ) { 
        fileStream << std::format("{:016x}\n", word);
      },
      [](const uint64_t word) { return FPGADataFormatUtilities::get_bitfields_GHITZ_w1(word).last; },
      1,
      m_GHITZTxtOutputEvents.at(index)
    ));
  }

  ATH_CHECK(m_GHITZBinOutputKeys.size() == m_GHITZBinOutputPaths.size());

  for (std::size_t index = 0; index < m_GHITZBinOutputKeys.size(); index++) {
    ATH_CHECK(writeFile(
      m_GHITZBinOutputPaths.value().at(index), 
      [](
        std::ofstream& fileStream, 
        const uint64_t word
      ) { 
        fileStream.write(std::bit_cast<char*>(&word), 8); 
      },
      [](const uint64_t word) { return FPGADataFormatUtilities::get_bitfields_GHITZ_w1(word).last; },
      1,
      m_GHITZBinOutputEvents.at(index)
    ));
  }

  ATH_CHECK(m_GHITZTxtOutputKeys.size() == m_GHITZTxtOutputPaths.size());

  for (std::size_t index = 0; index < m_GHITZTxtOutputKeys.size(); index++) {
    ATH_CHECK(writeFile(
      m_GHITZTxtOutputPaths.value().at(index), 
      [](
        std::ofstream& fileStream, 
        const uint64_t word
      ) { 
        fileStream << std::format("{:016x}\n", word);
      },
      [](const uint64_t word) { return FPGADataFormatUtilities::get_bitfields_GHITZ_w1(word).last; },
      1,
      m_GHITZTxtOutputEvents.at(index)
    ));
  }

  ATH_CHECK(m_GHITZBinOutputKeys.size() == m_GHITZBinOutputPaths.size());

  for (std::size_t index = 0; index < m_GHITZBinOutputKeys.size(); index++) {
    ATH_CHECK(writeFile(
      m_GHITZBinOutputPaths.value().at(index), 
      [](
        std::ofstream& fileStream, 
        const uint64_t word
      ) { 
        fileStream.write(std::bit_cast<char*>(&word), 8); 
      },
      [](const uint64_t word) { return FPGADataFormatUtilities::get_bitfields_GHITZ_w1(word).last; },
      1,
      m_GHITZBinOutputEvents.at(index)
    ));
  }

  ATH_CHECK(m_CLUSTERTxtOutputKeys.size() == m_CLUSTERTxtOutputPaths.size());

  for (std::size_t index = 0; index < m_CLUSTERTxtOutputKeys.size(); index++) {
    ATH_CHECK(writeFile(
      m_CLUSTERTxtOutputPaths.value().at(index), 
      [](
        std::ofstream& fileStream, 
        const uint64_t word
      ) { 
        fileStream << std::format("{:016x}\n", word);
      },
      [](const uint64_t word) { return FPGADataFormatUtilities::get_bitfields_STRIP_CLUSTER(word).last; },
      0,
      m_CLUSTERTxtOutputEvents.at(index)
    ));
  }

  ATH_CHECK(m_CLUSTERBinOutputKeys.size() == m_CLUSTERBinOutputPaths.size());

  for (std::size_t index = 0; index < m_CLUSTERBinOutputKeys.size(); index++) {
    ATH_CHECK(writeFile(
      m_CLUSTERBinOutputPaths.value().at(index), 
      [](
        std::ofstream& fileStream, 
        const uint64_t word
      ) { 
        fileStream.write(std::bit_cast<char*>(&word), 8); 
      },
      [](const uint64_t word) { return FPGADataFormatUtilities::get_bitfields_STRIP_CLUSTER(word).last; },
      0,
      m_CLUSTERBinOutputEvents.at(index)
    ));
  }

  return StatusCode::SUCCESS;
}

StatusCode EFTrackingDataStreamLoaderAlgorithm::readFile(
  const std::string& path,
  const auto& fileReadFunction,  
  const auto& endOfBlockCondition,
  const int32_t hitCountDown,
  std::vector<std::vector<uint64_t>>& events
) {
  ATH_MSG_DEBUG(
    "\nreadFile:\n" <<
    "\tpath: " << path << "\n"
  );

  std::ifstream fileStream(path, std::ios::binary);
  ATH_CHECK(fileStream.is_open());

  FileState fileState{};

  while (fileStream.peek() != EOF) {
    const uint64_t word = fileReadFunction(fileStream);
    DataFormatAction action = dataFormatStateMachine(
      word,
      endOfBlockCondition,
      hitCountDown,
      fileState
    );

    if (action == DataFormatAction::KEEP) {
      events.back().push_back(word);
    }
    else if (action == DataFormatAction::NEW_EVENT) {
      events.push_back({word});
    }
    else {
      ATH_CHECK(action != DataFormatAction::ERROR);
      ATH_CHECK(action == DataFormatAction::DISCARD);
    }
  }

  ATH_CHECK(fileState.dataFormatState == FileState::DataFormatState::FOOTER);
  ATH_CHECK(fileState.countDown == 0);

  return StatusCode::SUCCESS;
}

StatusCode EFTrackingDataStreamLoaderAlgorithm::writeFile(
  const std::string& path,
  const auto& fileWriteFunction,
  const auto& endOfBlockCondition,
  const int32_t hitCountDown,
  const std::vector<std::vector<uint64_t>>& events
) {
  ATH_MSG_DEBUG(
    "\nwriteFile:\n" <<
    "\tpath: " << path << "\n"
  );

  std::ofstream fileStream(path, std::ios::binary);
  ATH_CHECK(fileStream.is_open());
  FileState fileState{};

  for (const auto& event : events) {
    bool firstWord = true;

    for (const auto& word : event) {
      DataFormatAction action = dataFormatStateMachine(
        word,
        endOfBlockCondition,
        hitCountDown,
        fileState
      );

      if (
        action == DataFormatAction::NEW_EVENT &&
        !firstWord
      ) {
        fileWriteFunction(fileStream, word);
        break;
      }
      else if (
        action == DataFormatAction::NEW_EVENT ||
        action == DataFormatAction::KEEP
      ) {
        fileWriteFunction(fileStream, word);
      }
      else {
        ATH_CHECK(action != DataFormatAction::ERROR);
        ATH_CHECK(action == DataFormatAction::DISCARD);
      }
      
      firstWord = false;
    }
  }

  ATH_CHECK(fileState.dataFormatState == FileState::DataFormatState::FOOTER);
  ATH_CHECK(fileState.countDown == 0);

  return StatusCode::SUCCESS;
}

DataFormatAction EFTrackingDataStreamLoaderAlgorithm::dataFormatStateMachine(
  const uint64_t word,
  const auto& endOfBlockCondition,
  const int32_t hitCountDown,
  FileState& fileState
) {
  ATH_MSG_VERBOSE(
      "\ndataFormatStateMachine:\n" <<
      "\tword: " << std::hex << word << std::dec << "\n" <<
      "\tfileState.countDown: " << fileState.countDown << "\n" <<
      "\tfileState.dataFormatState: " << static_cast<int>(fileState.dataFormatState)
  );

  if (fileState.countDown != 0) {
    fileState = {
      .countDown = fileState.countDown - 1,
      .dataFormatState = fileState.dataFormatState,
    };

    return DataFormatAction::KEEP;
  }
  else if (
    fileState.dataFormatState == FileState::DataFormatState::FOOTER &&
    fileState.countDown == 0 &&
    FPGADataFormatUtilities::get_bitfields_EVT_HDR_w1(word).flag == FPGADataFormatUtilities::EVT_HDR_FLAG
  ) {
    fileState = {
      .countDown = 2,
      .dataFormatState = FileState::DataFormatState::HEADER,
    };

    return DataFormatAction::NEW_EVENT;
  }
  else if (
    fileState.dataFormatState == FileState::DataFormatState::FOOTER &&
    fileState.countDown == 0
  ) {
    fileState = {
      .countDown = 0,
      .dataFormatState = FileState::DataFormatState::FOOTER,
    };

    return DataFormatAction::DISCARD;
  }
  else if (
    fileState.dataFormatState == FileState::DataFormatState::HEADER &&
    fileState.countDown == 0 &&
    FPGADataFormatUtilities::get_bitfields_EVT_FTR_w1(word).flag == FPGADataFormatUtilities::EVT_FTR_FLAG
  ) {
    fileState = {
      .countDown = 2,
      .dataFormatState = FileState::DataFormatState::FOOTER,
    };

    return DataFormatAction::KEEP;
  }
  else if (
    fileState.dataFormatState == FileState::DataFormatState::HEADER &&
    fileState.countDown == 0 &&
    FPGADataFormatUtilities::get_bitfields_SLICE_HDR_w1(word).flag == FPGADataFormatUtilities::SLICE_HDR_FLAG
  ) {
    fileState = {
      .countDown = 0,
      .dataFormatState = FileState::DataFormatState::HEADER,
    };

    return DataFormatAction::KEEP;
  }
  else if (
    fileState.dataFormatState == FileState::DataFormatState::HEADER &&
    fileState.countDown == 0 &&
    FPGADataFormatUtilities::get_bitfields_M_HDR_w1(word).flag == FPGADataFormatUtilities::M_HDR_FLAG
  ) {
    fileState = {
      .countDown = 0,
      .dataFormatState = FileState::DataFormatState::HITS,
    };

    return DataFormatAction::KEEP;
  }
  else if (
    fileState.dataFormatState == FileState::DataFormatState::HEADER &&
    fileState.countDown == 0 &&
    FPGADataFormatUtilities::get_bitfields_GTRACK_HDR_w1(word).flag == FPGADataFormatUtilities::GTRACK_HDR_FLAG
  ) {
    fileState = {
      .countDown = 2,
      .dataFormatState = FileState::DataFormatState::HITS,
    };

    return DataFormatAction::KEEP;
  }
  else if (
    fileState.dataFormatState == FileState::DataFormatState::HEADER &&
    fileState.countDown == 0 &&
    FPGADataFormatUtilities::get_bitfields_RD_HDR_w1(word).flag == FPGADataFormatUtilities::RD_HDR_FLAG
  ) {
    fileState = {
      .countDown = 1,
      .dataFormatState = FileState::DataFormatState::HITS,
    };

    return DataFormatAction::KEEP;
  }
  else if (
    fileState.dataFormatState == FileState::DataFormatState::HEADER &&
    fileState.countDown == 0 &&
    FPGADataFormatUtilities::get_bitfields_EDM_STRIPCLUSTER_w1(word).flag == FPGADataFormatUtilities::EDM_STRIPCLUSTER_FLAG
  ) {
    fileState = {
      .countDown = 8,
      .dataFormatState = FileState::DataFormatState::HITS,
    };

    return DataFormatAction::KEEP;
  }
  else if (
    fileState.dataFormatState == FileState::DataFormatState::HEADER &&
    fileState.countDown == 0 &&
    FPGADataFormatUtilities::get_bitfields_EDM_PIXELCLUSTER_w1(word).flag == FPGADataFormatUtilities::EDM_PIXELCLUSTER_FLAG
  ) {
    fileState = {
      .countDown = 9,
      .dataFormatState = FileState::DataFormatState::HITS,
    };

    return DataFormatAction::KEEP;
  }
  else if (
    fileState.dataFormatState == FileState::DataFormatState::HITS &&
    fileState.countDown == 0 &&
    endOfBlockCondition(word)
  ) {
    fileState = {
      .countDown = hitCountDown,
      .dataFormatState = FileState::DataFormatState::HEADER,
    };

    return DataFormatAction::KEEP;
  }
  else if (
    fileState.dataFormatState == FileState::DataFormatState::HITS &&
    fileState.countDown == 0
  ) {
    fileState = {
      .countDown = hitCountDown,
      .dataFormatState = FileState::DataFormatState::HITS,
    };

    return DataFormatAction::KEEP;
  }

  return DataFormatAction::ERROR;
}

