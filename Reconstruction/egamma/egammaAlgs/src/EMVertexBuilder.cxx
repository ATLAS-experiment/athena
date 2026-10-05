/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************

EMVertexBuilder.cxx  -  Description
-------------------
begin   : 11-03-2011
authors : Kerstin Tackmann, Bruno Lenzi
email   : kerstin.tackmann@cern.ch, Bruno.Lenzi@cern.ch
changes :
          Nov 2011 (KT) Initial version
          Mar 2014 (BL) xAOD migration, creates both double and single track vertices

***************************************************************************/

#include "EMVertexBuilder.h"

#include "GaudiKernel/EventContext.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/VertexContainer.h"
#include "xAODTracking/VertexAuxContainer.h"
#include "xAODEgamma/EgammaxAODHelpers.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"
#include "GeoPrimitives/GeoPrimitives.h" // Amg::Vector3D

EMVertexBuilder::EMVertexBuilder(const std::string& name,
                                 ISvcLocator* pSvcLocator)
  : AthReentrantAlgorithm(name, pSvcLocator)
{
}

StatusCode EMVertexBuilder::initialize() {

  ATH_MSG_DEBUG( "Initializing " << name() << "...");

  ATH_CHECK(m_inputTrackParticleContainerKey.initialize());
  ATH_CHECK(m_outputConversionContainerKey.initialize());

  // Get the ID VertexFinderTool
  ATH_CHECK( m_vertexFinderTool.retrieve());
  // Retrieve EMExtrapolationTool
  ATH_CHECK(m_EMExtrapolationTool.retrieve());

  return StatusCode::SUCCESS;
}

StatusCode EMVertexBuilder::finalize() {
  return StatusCode::SUCCESS;
}

StatusCode EMVertexBuilder::execute(const EventContext& ctx) const
{

  //retrieve TrackParticleContainer
  SG::ReadHandle<xAOD::TrackParticleContainer> TPCol(m_inputTrackParticleContainerKey,ctx);

  // check for serial running only, remove in MT
  ATH_CHECK(TPCol.isValid());

  std::pair<xAOD::VertexContainer*, xAOD::VertexAuxContainer*> verticesPair =
    m_vertexFinderTool->findVertex(ctx,TPCol.cptr());

  if (!verticesPair.first || !verticesPair.second){
    ATH_MSG_ERROR("Null pointer to conversion container");
    return StatusCode::SUCCESS;
  }

  SG::WriteHandle<xAOD::VertexContainer>vertices(m_outputConversionContainerKey,ctx);

  ATH_CHECK(vertices.record(std::unique_ptr<xAOD::VertexContainer>(verticesPair.first),
			    std::unique_ptr<xAOD::VertexAuxContainer>(verticesPair.second)));

  ATH_MSG_DEBUG("New conversion container size: " << vertices->size());

  // Remove vertices with radii above m_maxRadius
  xAOD::VertexContainer::iterator itVtx = vertices->begin();
  xAOD::VertexContainer::iterator itVtxEnd = vertices->end();
  static const SG::Accessor<float> accPx("px");
  static const SG::Accessor<float> accPy("py");
  static const SG::Accessor<float> accPz("pz");
  //
  while (itVtx != itVtxEnd){
    xAOD::Vertex* vertex = *itVtx;

    Amg::Vector3D momentum(0., 0., 0.);
    for (unsigned int i = 0; i < vertex->nTrackParticles(); ++i){
      momentum += m_EMExtrapolationTool->getMomentumAtVertex(ctx,*vertex, i);
    }

    accPx(*vertex) = momentum.x();
    accPy(*vertex) = momentum.y();
    accPz(*vertex) = momentum.z();

    xAOD::EgammaParameters::ConversionType convType(xAOD::EgammaHelpers::conversionType((*itVtx)));
    const bool vxDoubleTRT = (convType == xAOD::EgammaParameters::doubleTRT);
    const bool vxSingleTRT = (convType == xAOD::EgammaParameters::singleTRT);

    if ((vxDoubleTRT && momentum.perp() < m_minPtCut_DoubleTrack) ||
        (vxSingleTRT && momentum.perp() < m_minPtCut_SingleTrack) ||
        ((*itVtx)->position().perp() > m_maxRadius)) {

      itVtx = vertices->erase(itVtx);
      itVtxEnd = vertices->end();
    } else {
      // Decorate the vertices with the momentum at the conversion point and
      // etaAtCalo, phiAtCalo (extrapolate each vertex)
      float etaAtCalo = -9999.;
      float phiAtCalo = -9999.;

      if (!m_EMExtrapolationTool->getEtaPhiAtCalo(
            ctx, vertex, &etaAtCalo, &phiAtCalo)) {
        ATH_MSG_DEBUG("getEtaPhiAtCalo failed!");
      }

      static const SG::Accessor<float> accetaAtCalo("etaAtCalo");
      static const SG::Accessor<float> accphiAtCalo("phiAtCalo");
      accetaAtCalo(*vertex) = etaAtCalo;
      accphiAtCalo(*vertex) = phiAtCalo;

      ++itVtx;
    }
  }

  return StatusCode::SUCCESS;
}
