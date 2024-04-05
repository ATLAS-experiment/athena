#ifndef VKalVrt_GNNVertexConstructorAlg_H
#define VKalVrt_GNNVertexConstructorAlg_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GNNVertexConstructor/GNNVertexConstructorTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "xAODJet/JetContainer.h"

namespace Rec {

class GNNVertexConstructorAlg : public AthReentrantAlgorithm {
public:
  GNNVertexConstructorAlg(const std::string &name, ISvcLocator *pSvcLocator);
  StatusCode initialize() override;
  StatusCode execute(const EventContext &ctx) const override;
  StatusCode finalize() override;

private:
  // Tools
  ToolHandle<Rec::GNNVertexConstructorTool> m_VtxTool;
  // Input jets
  SG::ReadHandleKey<xAOD::JetContainer> m_inJetsKey{this, "inputJetContainer", "", "Input jet container"};
  // Output vertices
  SG::WriteHandleKey<xAOD::VertexContainer> m_outVertexKey{this, "outputVertexContainer", "GNNVertices",
                                                           "Output vertex container"};

  // Input Primary Vertices
  SG::ReadHandleKey<xAOD::VertexContainer> m_pvContainerKey{this, "PrimaryVertexContainer", "PrimaryVertices",
                                                            "Read PrimaryVertices container"};
};

} // namespace Rec

#endif
