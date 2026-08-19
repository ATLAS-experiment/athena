/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

  See the header for what the graph does and what is left here.
*/

#include "FatrasG4ConversionFlowInference.h"

// CLHEP
#include "CLHEP/Random/RandGauss.h"

// Thread safety checker macros
#include "CxxUtils/checker_macros.h"

#include <array>
#include <cstdint>

namespace {

  // ONNX Runtime expects a single environment per process, shared by every
  // session. Ort::Env is internally thread safe, hence the checker annotation.
  Ort::Env& onnxEnv()
  {
    static Ort::Env env ATLAS_THREAD_SAFE (ORT_LOGGING_LEVEL_WARNING, "FatrasG4");
    return env;
  }

  // Graph inputs: (eGamma [MeV], Z) and the random draws the graph consumes.
  // The noise column order is the export's contract.
  constexpr std::size_t s_inputColumns = 2;
  constexpr std::size_t s_noiseColumns = 4;
  constexpr std::size_t s_noiseTriplet = 0;  //!< Uniform(0,1), picks nuclear or triplet
  constexpr std::size_t s_noiseRecoil = 1;   //!< Normal(0,1), recoil spline
  constexpr std::size_t s_noiseLead = 2;     //!< Normal(0,1), lead spline
  constexpr std::size_t s_noiseTheta = 3;    //!< Normal(0,1), theta spline

  const char* const s_inputNames[] = {"inputs", "noise"};
  const char* const s_outputNames[] = {"eLead", "eSub", "thetaLead", "eRecoil", "isTriplet"};
  constexpr std::size_t s_outputCount = 5;

}  // namespace


FatrasG4ConversionFlowInference::FatrasG4ConversionFlowInference(const std::string& modelFile)
{
  // Athena owns the threading: the ONNX Runtime default is a thread pool per
  // session sized to the core count, which would oversubscribe the machine
  // many times over once every Geant4 worker has its own session.
  Ort::SessionOptions sessionOptions;
  sessionOptions.SetIntraOpNumThreads(1);
  sessionOptions.SetInterOpNumThreads(1);
  sessionOptions.SetExecutionMode(ORT_SEQUENTIAL);
  sessionOptions.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);

  m_session = std::make_unique<Ort::Session>(onnxEnv(), modelFile.c_str(), sessionOptions);
}

FatrasG4ConversionFlowSample FatrasG4ConversionFlowInference::sample(
    double eGamma, double targetZ, CLHEP::HepRandomEngine& generator)
{
  // The photon energy and the atomic number go in raw: the log10 and min-max
  // normalisation are baked into the graph.
  std::array<float, s_inputColumns> inputValues{static_cast<float>(eGamma),
                                                static_cast<float>(targetZ)};

  // Every random number the graph needs, drawn from the Geant4 engine so the
  // job stays reproducible from its random seed.
  std::array<float, s_noiseColumns> noiseValues{};
  noiseValues[s_noiseTriplet] = static_cast<float>(generator.flat());
  noiseValues[s_noiseRecoil] = static_cast<float>(CLHEP::RandGauss::shoot(&generator));
  noiseValues[s_noiseLead] = static_cast<float>(CLHEP::RandGauss::shoot(&generator));
  noiseValues[s_noiseTheta] = static_cast<float>(CLHEP::RandGauss::shoot(&generator));

  // The graph carries a dynamic batch axis; one conversion is one row.
  const std::array<int64_t, 2> inputShape{1, s_inputColumns};
  const std::array<int64_t, 2> noiseShape{1, s_noiseColumns};

  std::array<Ort::Value, 2> inputs{
      Ort::Value::CreateTensor<float>(m_memoryInfo, inputValues.data(), inputValues.size(),
                                      inputShape.data(), inputShape.size()),
      Ort::Value::CreateTensor<float>(m_memoryInfo, noiseValues.data(), noiseValues.size(),
                                      noiseShape.data(), noiseShape.size())};

  auto outputs = m_session->Run(Ort::RunOptions{nullptr}, s_inputNames, inputs.data(),
                                inputs.size(), s_outputNames, s_outputCount);

  FatrasG4ConversionFlowSample sampled;
  sampled.eLead = outputs[0].GetTensorData<float>()[0];
  sampled.eSub = outputs[1].GetTensorData<float>()[0];
  sampled.thetaLead = outputs[2].GetTensorData<float>()[0];
  sampled.eRecoil = outputs[3].GetTensorData<float>()[0];
  sampled.isTriplet = outputs[4].GetTensorData<float>()[0] > 0.5f;

  return sampled;
}
