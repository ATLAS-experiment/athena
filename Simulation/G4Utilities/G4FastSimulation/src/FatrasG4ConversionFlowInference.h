/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

  C++ side of the FatrasG4 photon conversion normalizing flow.

  The whole sampling pipeline lives in the ONNX graph exported by the
  gammaConversion study's export_flow_onnx.py: the trunk, the three chained
  spline heads, the de-standardisation and the from_learned coordinate map. The
  only thing left here is the randomness.

  The random numbers are drawn from the Geant4 CLHEP engine and handed to the
  graph as an input, which makes the graph a deterministic function. ONNX does
  have a RandomNormal op, but it draws from the ONNX Runtime generator and its
  seed is a graph attribute fixed at export time, so it cannot be reseeded per
  event - and an Athena simulation job has to be reproducible from its random
  seed. Passing the draws in keeps that property.

  The noise column order and the output order below are the export's contract
  and must match export_flow_onnx.py.
*/

#ifndef G4FASTSIMULATION_FATRASG4CONVERSIONFLOWINFERENCE_H
#define G4FASTSIMULATION_FATRASG4CONVERSIONFLOWINFERENCE_H

// ONNX Runtime
#include <onnxruntime_cxx_api.h>

// CLHEP
#include "CLHEP/Random/RandomEngine.h"

#include <memory>
#include <string>

/** One sampled photon conversion final state. The two lepton energies are
kinetic, in MeV, and sum with eRecoil to the photon energy less 2*m_e. */
struct FatrasG4ConversionFlowSample
{
  double eLead = 0.;      //!< Energy of the more energetic lepton
  double eSub = 0.;       //!< Energy of the less energetic lepton
  double thetaLead = 0.;  //!< Polar angle of the leading lepton w.r.t. the photon
  double eRecoil = 0.;    //!< Energy taken by the recoiling target
  bool isTriplet = false; //!< Conversion on an electron rather than on the nucleus
};

class FatrasG4ConversionFlowInference
{
 public:
  /** Opens conversion_flow.onnx. Throws Ort::Exception if it cannot be loaded. **/
  explicit FatrasG4ConversionFlowInference(const std::string& modelFile);

  /** Samples one conversion, given the photon energy in MeV and the atomic
  number of the element it converts on. **/
  FatrasG4ConversionFlowSample sample(double eGamma, double targetZ,
                                      CLHEP::HepRandomEngine& generator);

 private:
  std::unique_ptr<Ort::Session> m_session;

  Ort::MemoryInfo m_memoryInfo =
      Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
};

#endif  // G4FASTSIMULATION_FATRASG4CONVERSIONFLOWINFERENCE_H
