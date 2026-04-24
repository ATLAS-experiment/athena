/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef SICLUSTERIZATIONTOOL_ONNXNNCONDALG_H
#define SICLUSTERIZATIONTOOL_ONNXNNCONDALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ServiceHandle.h"
#include "StoreGate/WriteCondHandleKey.h"
#include "SiClusterizationTool/OnnxNNCollection.h"
#include "AthOnnxInterfaces/IOnnxRuntimeSvc.h"
#include <onnxruntime_cxx_api.h>

namespace InDet {

class OnnxNNCondAlg : public AthReentrantAlgorithm {
public:
  using AthReentrantAlgorithm ::AthReentrantAlgorithm;
  virtual ~OnnxNNCondAlg() = default;

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;
  virtual bool isReEntrant() const override { return false; }

private:
  std::unique_ptr<Ort::Session> createSession(const std::string& modelPath) const;

  ServiceHandle<AthOnnx::IOnnxRuntimeSvc> m_onnxSvc{
    this, "OnnxRuntimeSvc", "AthOnnx::OnnxRuntimeSvc",
    "Handle to the ONNX Runtime service"};

  SG::WriteCondHandleKey<OnnxNNCollection> m_writeKey{
    this, "WriteKey", "PixelClusterNNONNX",
    "Output conditions key for ONNX NN collection"};

  Gaudi::Property<std::string> m_numberNetworkPath{
    this, "NumberNetworkPath", "",
    "Path to ONNX model for number network"};

  Gaudi::Property<std::string> m_posNetwork1Path{
    this, "PositionNetwork1Path", "",
    "Path to ONNX model for 1-particle position network"};

  Gaudi::Property<std::string> m_posNetwork2Path{
    this, "PositionNetwork2Path", "",
    "Path to ONNX model for 2-particle position network"};

  Gaudi::Property<std::string> m_posNetwork3Path{
    this, "PositionNetwork3Path", "",
    "Path to ONNX model for 3-particle position network"};

};

} // namespace InDet

#endif
