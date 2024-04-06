/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "GNNVertexConstructorAlg.h"
#include "xAODJet/JetContainer.h"
#include "xAODTracking/VertexAuxContainer.h"
#include "xAODTracking/VertexContainer.h"

namespace Rec {

GNNVertexConstructorAlg::GNNVertexConstructorAlg(const std::string &name, ISvcLocator *pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator), m_VtxTool("Rec::GNNVertexConstructorTool/VtxTool", this) {
  declareProperty("VtxTool", m_VtxTool, "The GNN Vtxing Tool");
}

StatusCode GNNVertexConstructorAlg::initialize() {

  // Retrieving the tool
  ATH_CHECK(m_VtxTool.retrieve());

  // Initializing Keys
  ATH_CHECK(m_inJetsKey.initialize());
  ATH_CHECK(m_outVertexKey.initialize());
  ATH_CHECK(m_pvContainerKey.initialize());

  return StatusCode::SUCCESS;
}

StatusCode GNNVertexConstructorAlg::execute(const EventContext &ctx) const {

  ATH_MSG_DEBUG("In GNNVertexConstructorAlg::execute()");

  // Extract Jets
  SG::ReadHandle<xAOD::JetContainer> inJetContainer(m_inJetsKey, ctx);
  if (!inJetContainer.isValid()) {
    ATH_MSG_WARNING("No xAOD::JetContainer named " << m_inJetsKey.key() << " found in StoreGate");
    return StatusCode::FAILURE;
  }

  // Write new GNN Vertice Container
  SG::WriteHandle<xAOD::VertexContainer> outVertexContainer(m_outVertexKey, ctx);
  if (outVertexContainer.record(std::make_unique<xAOD::VertexContainer>(), std::make_unique<xAOD::VertexAuxContainer>())
          .isFailure()) {
    ATH_MSG_ERROR("Storegate record of VertexContainer failed.");
    return StatusCode::FAILURE;
  }

  const xAOD::Vertex *pv = nullptr;
  //-- Extract Primary Vertices
  SG::ReadHandle<xAOD::VertexContainer> pv_cont(m_pvContainerKey, ctx);
  if (!pv_cont.isValid()) {
    ATH_MSG_WARNING("No Primary Vertices container found in TDS");
  } else {
    //-- Extract PV itself
    for (auto v : *pv_cont) {
      if (v->vertexType() == xAOD::VxType::PriVtx) {
        pv = v;
        break;
      }
    }
  }

  // Perform a Vertex fit
  ATH_CHECK(m_VtxTool->performVertexFit(inJetContainer.ptr(), outVertexContainer.ptr(), *pv, ctx));

  return StatusCode::SUCCESS;
}

StatusCode GNNVertexConstructorAlg::finalize() {
  ATH_MSG_DEBUG("devAlg: In devAlg::finalize()");
  return StatusCode::SUCCESS;
}

} // namespace Rec
