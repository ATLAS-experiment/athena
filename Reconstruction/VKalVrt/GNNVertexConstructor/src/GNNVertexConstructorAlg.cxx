#include "GNNVertexConstructorAlg.h"
#include "xAODJet/JetContainer.h"
#include "xAODTracking/VertexContainer.h"
#include "xAODTracking/VertexAuxContainer.h"

namespace Rec {

GNNVertexConstructorAlg::GNNVertexConstructorAlg(const std::string &name, ISvcLocator *pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator),
      m_VtxTool("Rec::GNNVertexConstructorTool/VtxTool", this) {
  declareProperty("VtxTool", m_VtxTool, "The GNN Vtxing Tool");
}

StatusCode GNNVertexConstructorAlg::initialize() {

  // Retrieving the tool
  ATH_CHECK(m_VtxTool.retrieve());

  // Initializing Keys
  ATH_CHECK(m_inJetsKey.initialize());
  ATH_CHECK(m_outVertexKey.initialize());

  return StatusCode::SUCCESS;
}

StatusCode GNNVertexConstructorAlg::execute(const EventContext &ctx) const {

  ATH_MSG_DEBUG("In GNNVertexConstructorAlg::execute()");

  SG::ReadHandle<xAOD::JetContainer> inJetContainer(m_inJetsKey, ctx);
  if (!inJetContainer.isValid()) {
    ATH_MSG_WARNING("No xAOD::JetContainer named " << m_inJetsKey.key() << " found in StoreGate");
    return StatusCode::FAILURE;
  }

  SG::WriteHandle<xAOD::VertexContainer> outVertexContainer(m_outVertexKey, ctx);
  if (outVertexContainer
          .record(std::make_unique<xAOD::VertexContainer>(),
                  std::make_unique<xAOD::VertexAuxContainer>())
          .isFailure()) {
    ATH_MSG_ERROR("Storegate record of VertexContainer failed.");
    return StatusCode::FAILURE;
  }

  ATH_CHECK(m_VtxTool->decorateJets(inJetContainer.ptr()));
  ATH_CHECK(m_VtxTool->performVertexFit(inJetContainer.ptr(), outVertexContainer.ptr(), ctx));

  return StatusCode::SUCCESS;
}

StatusCode GNNVertexConstructorAlg::finalize(){
  ATH_MSG_DEBUG("devAlg: In devAlg::finalize()");
  return StatusCode::SUCCESS;
}

} // namespace Rec
