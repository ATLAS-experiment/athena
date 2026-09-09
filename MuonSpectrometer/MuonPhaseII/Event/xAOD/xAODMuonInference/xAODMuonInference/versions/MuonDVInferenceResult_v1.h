/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONINFERENCE_VERSIONS_MUONDVINFERENCERESULT_V1_H
#define XAODMUONINFERENCE_VERSIONS_MUONDVINFERENCERESULT_V1_H

#include "AthContainers/AuxElement.h"

namespace xAOD {

  /**
   * @brief Event-level output of the Muon Phase-II displaced-vertex ONNX classifier.
   *
   * The object is expected to be written as a one-element container per event.
   */
  class MuonDVInferenceResult_v1 : public SG::AuxElement {
  public:
    MuonDVInferenceResult_v1();

    char valid() const;
    void setValid(char value);

    char pass() const;
    void setPass(char value);

    float score() const;
    void setScore(float value);

    float rawOutput() const;
    void setRawOutput(float value);

    float decisionValue() const;
    void setDecisionValue(float value);

    float cutValue() const;
    void setCutValue(float value);

    unsigned int nNodes() const;
    void setNNodes(unsigned int value);

    unsigned int nMuonNodes() const;
    void setNMuonNodes(unsigned int value);

    unsigned int nCaloNodes() const;
    void setNCaloNodes(unsigned int value);

    unsigned int nEdges() const;
    void setNEdges(unsigned int value);
  };

}  // namespace xAOD

#endif
