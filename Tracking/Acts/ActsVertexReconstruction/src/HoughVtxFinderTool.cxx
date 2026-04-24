/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "ActsVertexReconstruction/HoughVtxFinderTool.h"

#include <vector>


StatusCode ActsTrk::HoughVtxFinderTool::initialize() {
  ATH_MSG_DEBUG("Initializing ActsTrk::HoughVtxFinderTool");

  ATH_CHECK(m_beamSpotKey.initialize(m_useBeamSpot));

  // logger
  m_logger = makeActsAthenaLogger(this, "Acts");

  // check if vector sizes are compatible
  if (m_absEtaRanges.size() != m_absEtaFractions.size()) {
    ATH_MSG_ERROR("m_absEtaRanges.size() != m_absEtaFractions.size(), " 
                  << m_absEtaRanges.size() << " != " << m_absEtaFractions.size());
    return StatusCode::FAILURE;
  }
  if (m_rangeIterZ.size() != m_nBinsZIterZ.size() || m_rangeIterZ.size() != m_nBinsCotThetaIterZ.size()) {
    ATH_MSG_ERROR("sizes of m_rangeIterZ, m_nBinsZIterZ.size(), and m_nBinsCotThetaIterZ.size() are not equal, "
                  << m_rangeIterZ.size() << ", " << m_nBinsZIterZ.size() << ", " << m_nBinsCotThetaIterZ.size());
    return StatusCode::FAILURE;
  }

  // vertex finder configuration
  m_finderCfg.targetSPs = m_targetSPs;
  m_finderCfg.minAbsEta = m_minAbsEta;
  m_finderCfg.maxAbsEta = m_maxAbsEta;
  m_finderCfg.minHits = m_minHits;
  m_finderCfg.fillNeighbours = m_fillNeighbours;
  m_finderCfg.absEtaRanges = m_absEtaRanges;
  m_finderCfg.absEtaFractions = m_absEtaFractions;
  m_finderCfg.rangeIterZ = m_rangeIterZ;
  m_finderCfg.nBinsZIterZ = m_nBinsZIterZ;
  m_finderCfg.nBinsCotThetaIterZ = m_nBinsCotThetaIterZ;
  m_finderCfg.binsCotThetaDecrease = m_binsCotThetaDecrease;
  m_finderCfg.peakWidth = m_peakWidth;
  Acts::Vector3 defVtxPos{m_defVtxPosition[0], m_defVtxPosition[1], m_defVtxPosition[2]};
  m_finderCfg.defVtxPosition = defVtxPos;

  ATH_MSG_DEBUG("Successfully initialized ActsTrk::HoughVtxFinderTool");
  return StatusCode::SUCCESS;
}

std::pair<std::unique_ptr<xAOD::VertexContainer>, std::unique_ptr<xAOD::VertexAuxContainer>>
ActsTrk::HoughVtxFinderTool::findVertex(const EventContext &ctx,
                                        const xAOD::SpacePointContainer &spacePointContainer) const {
  // vertex finder configuration depending on the beamspot
  auto finderCfgBS = m_finderCfg;

  if (m_useBeamSpot) {
    // beamspot XY position for the default XY position of the vertex
    SG::ReadCondHandle<InDet::BeamSpotData> beamSpotHandle{m_beamSpotKey, ctx};
    const Trk::RecVertex &beamposition(beamSpotHandle->beamVtx());

    finderCfgBS.defVtxPosition[0] = beamposition.position().x();
    finderCfgBS.defVtxPosition[1] = beamposition.position().y();
    // default Z position is kept the same

    ATH_MSG_DEBUG("beamspot position: " << beamposition.position().x() << ", " << beamposition.position().y() << ", "
                                        << beamposition.position().z());
  }

  // the output vertex containers
  auto theVertexContainer = std::make_unique<xAOD::VertexContainer>();
  auto theVertexAuxContainer = std::make_unique<xAOD::VertexAuxContainer>();
  theVertexContainer->setStore(theVertexAuxContainer.get());

  if (spacePointContainer.size() < m_minSPs) {
    ATH_MSG_DEBUG("Not enough space points for vertex finding; " << spacePointContainer.size() << " < " << m_minSPs);
    // do not attempt vertex finding
    return std::make_pair(std::move(theVertexContainer), std::move(theVertexAuxContainer));
  }

  auto vertexFinder = std::make_unique<VertexFinder>(finderCfgBS, logger().cloneWithSuffix("Finder"));

  ATH_MSG_DEBUG("default vertex position: " << vertexFinder->config().defVtxPosition[0] << ", "
                                            << vertexFinder->config().defVtxPosition[1] << ", "
                                            << vertexFinder->config().defVtxPosition[2]);

  Acts::SpacePointContainer2 spacePoints(Acts::SpacePointColumns::X | 
                                          Acts::SpacePointColumns::Y |
                                          Acts::SpacePointColumns::Z);
  spacePoints.reserve(spacePointContainer.size(), 0);
  for (const auto sp : spacePointContainer) {
    auto newSp = spacePoints.createSpacePoint();
    newSp.x() = sp->x();
    newSp.y() = sp->y();
    newSp.z() = sp->z();
  }

  ATH_MSG_DEBUG("Number of input space points: " << spacePoints.size());
  auto vtx = vertexFinder->find(spacePoints);

  if (vtx.ok()) {
    ATH_MSG_DEBUG("Vertex position: " << (*vtx)[0] << ", " << (*vtx)[1] << ", " << (*vtx)[2]);
    auto xAODVertex = theVertexContainer->push_back(std::make_unique<xAOD::Vertex>());
    xAODVertex->setPosition(*vtx);
    xAODVertex->setVertexType(xAOD::VxType::PriVtx);
  } else {
    ATH_MSG_DEBUG("Vertex finding failed");
  }

  return std::make_pair(std::move(theVertexContainer), std::move(theVertexAuxContainer));
}
