/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "OnnxNNCondAlg.h"
#include "AthenaKernel/IOVInfiniteRange.h"
#include "PathResolver/PathResolver.h"

namespace InDet {


StatusCode OnnxNNCondAlg::initialize() {
  ATH_CHECK(m_onnxSvc.retrieve());
  ATH_CHECK(m_writeKey.initialize());

  // Validate that at least the number network path is provided
  if (m_numberNetworkPath.value().empty()) {
    ATH_MSG_FATAL("NumberNetworkPath must be set");
    return StatusCode::FAILURE;
  }

  ATH_MSG_INFO("OnnxNNCondAlg configured with:"
               << " NumberNetwork=" << m_numberNetworkPath.value()
               << " PosNetwork1=" << m_posNetwork1Path.value()
               << " PosNetwork2=" << m_posNetwork2Path.value()
               << " PosNetwork3=" << m_posNetwork3Path.value());

  return StatusCode::SUCCESS;
}

std::unique_ptr<Ort::Session> OnnxNNCondAlg::createSession(
    const std::string& modelPath) const {

  std::string resolvedPath = PathResolver::find_calib_file(modelPath);
  if (resolvedPath.empty()) {
    // Try as absolute/relative path directly
    resolvedPath = modelPath;
  }

  ATH_MSG_DEBUG("Loading ONNX model from: " << resolvedPath);

  Ort::SessionOptions sessionOptions;
  sessionOptions.SetIntraOpNumThreads(1);
  sessionOptions.SetGraphOptimizationLevel(ORT_ENABLE_ALL);
  sessionOptions.DisablePerSessionThreads();

  return std::make_unique<Ort::Session>(m_onnxSvc->env(), resolvedPath.c_str(),
                                         sessionOptions);
}

StatusCode OnnxNNCondAlg::execute(const EventContext& ctx) const {

  SG::WriteCondHandle<OnnxNNCollection> writeHandle(m_writeKey, ctx);
  if (writeHandle.isValid()) {
    ATH_MSG_DEBUG("OnnxNNCollection already valid, skipping");
    return StatusCode::SUCCESS;
  }

  auto collection = std::make_unique<OnnxNNCollection>();

  // Load number network
  try {
    collection->numberNetwork = createSession(m_numberNetworkPath.value());
  } catch (const Ort::Exception& e) {
    ATH_MSG_FATAL("Failed to load number network ONNX model: " << e.what());
    return StatusCode::FAILURE;
  }

  // Load position networks (paths are optional)
  const std::array<std::string, 3> posPaths = {
    m_posNetwork1Path.value(),
    m_posNetwork2Path.value(),
    m_posNetwork3Path.value()
  };
  std::unique_ptr<Ort::Session>* posMembers[3] = {
    &collection->positionNetwork1,
    &collection->positionNetwork2,
    &collection->positionNetwork3
  };

  for (int i = 0; i < 3; ++i) {
    if (posPaths[i].empty()) {
      ATH_MSG_DEBUG("Position network " << (i+1) << " path not set, skipping");
      continue;
    }
    try {
      *posMembers[i] = createSession(posPaths[i]);
    } catch (const Ort::Exception& e) {
      ATH_MSG_FATAL("Failed to load position network " << (i+1)
                    << " ONNX model: " << e.what());
      return StatusCode::FAILURE;
    }
  }

  // Set infinite IOV for file-based models
  writeHandle.addDependency(IOVInfiniteRange::infiniteRunLB());

  ATH_CHECK(writeHandle.record(std::move(collection)));
  ATH_MSG_DEBUG("Recorded OnnxNNCollection successfully");

  return StatusCode::SUCCESS;
}

} // namespace InDet
