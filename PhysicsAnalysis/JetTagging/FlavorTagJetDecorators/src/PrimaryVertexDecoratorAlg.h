/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FLAVOR_TAG_DISCRIMINANTS_PRIMARY_VERTEX_DECORATOR_ALG_H
#define FLAVOR_TAG_DISCRIMINANTS_PRIMARY_VERTEX_DECORATOR_ALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"

#include "xAODEventInfo/EventInfo.h"
#include "xAODTracking/VertexContainer.h"

namespace FlavorTagJetDecorators {

  /// Pre-compute primary vertex quantities as EventInfo decorations.
  ///
  /// TDD reads nPrimaryVertices (int) and primaryVertexDetectorZ (float)
  /// from EventInfo.  This algorithm computes them at derivation time
  /// so PrimaryVertices can be dropped from the DAOD output.

  class PrimaryVertexDecoratorAlg : public AthReentrantAlgorithm {
  public:
    PrimaryVertexDecoratorAlg(const std::string& name,
                              ISvcLocator* pSvcLocator);

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;

  private:
    SG::ReadHandleKey<xAOD::VertexContainer> m_vertexContainerKey {
      this, "VertexContainer", "PrimaryVertices",
      "Key for the input primary vertex collection"};

    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey {
      this, "EventInfoContainer", "EventInfo",
      "Input EventInfo"};

    SG::WriteDecorHandleKey<xAOD::EventInfo> m_dec_nPrimaryVertices {
      this, "ftag_nPrimaryVertices", "EventInfo.ftag_nPrimaryVertices",
      "Number of primary vertices"};

    SG::WriteDecorHandleKey<xAOD::EventInfo> m_dec_primaryVertexZ {
      this, "ftag_primaryVertexZ", "EventInfo.ftag_primaryVertexZ",
      "Primary vertex z position"};
  };

}  // namespace FlavorTagJetDecorators

#endif
