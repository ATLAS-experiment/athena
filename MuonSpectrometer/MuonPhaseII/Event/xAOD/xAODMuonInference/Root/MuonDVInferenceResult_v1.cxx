/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "xAODCore/AuxStoreAccessorMacros.h"

#include "xAODMuonInference/versions/MuonDVInferenceResult_v1.h"

namespace xAOD {

  MuonDVInferenceResult_v1::MuonDVInferenceResult_v1() : SG::AuxElement() {}

  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(MuonDVInferenceResult_v1, char, valid, setValid)
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(MuonDVInferenceResult_v1, char, pass, setPass)
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(MuonDVInferenceResult_v1, float, score, setScore)
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(MuonDVInferenceResult_v1, float, rawOutput, setRawOutput)
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(MuonDVInferenceResult_v1, float, decisionValue, setDecisionValue)
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(MuonDVInferenceResult_v1, float, cutValue, setCutValue)
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(MuonDVInferenceResult_v1, unsigned int, nNodes, setNNodes)
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(MuonDVInferenceResult_v1, unsigned int, nMuonNodes, setNMuonNodes)
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(MuonDVInferenceResult_v1, unsigned int, nCaloNodes, setNCaloNodes)
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(MuonDVInferenceResult_v1, unsigned int, nEdges, setNEdges)

}  // namespace xAOD
