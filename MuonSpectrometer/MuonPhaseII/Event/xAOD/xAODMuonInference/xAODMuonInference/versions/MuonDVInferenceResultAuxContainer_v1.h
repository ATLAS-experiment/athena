/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONINFERENCE_VERSIONS_MUONDVINFERENCERESULTAUXCONTAINER_V1_H
#define XAODMUONINFERENCE_VERSIONS_MUONDVINFERENCERESULTAUXCONTAINER_V1_H

#include <vector>

#include "xAODCore/AuxContainerBase.h"

namespace xAOD {

  /**
   * @brief Auxiliary store for xAOD::MuonDVInferenceResult_v1.
   */
  class MuonDVInferenceResultAuxContainer_v1 : public AuxContainerBase {
  public:
    MuonDVInferenceResultAuxContainer_v1();

  private:
    std::vector<char> valid;
    std::vector<char> pass;
    std::vector<float> score;
    std::vector<float> rawOutput;
    std::vector<float> decisionValue;
    std::vector<float> cutValue;
    std::vector<unsigned int> nNodes;
    std::vector<unsigned int> nMuonNodes;
    std::vector<unsigned int> nCaloNodes;
    std::vector<unsigned int> nEdges;
  };

}  // namespace xAOD

#include "xAODCore/BaseInfo.h"
SG_BASE(xAOD::MuonDVInferenceResultAuxContainer_v1, xAOD::AuxContainerBase);

#endif
