/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "xAODInDetMeasurement/ContainerAccessor.h"
#include "TrigSteeringEvent/TrigRoiDescriptor.h"
#include "xAODInDetMeasurement/PixelCluster.h"

#include "Gbts2ActsSeedingTool.h"

#include <optional>

Gbts2ActsSeedingTool::Gbts2ActsSeedingTool(const std::string& t,
					     const std::string& n,
					     const IInterface*  p ) : SeedingToolBase(t,n,p)
{
}

StatusCode Gbts2ActsSeedingTool::initialize(){
    ATH_CHECK(SeedingToolBase::initialize());
    ATH_CHECK(m_beamSpotKey.initialize());
    m_sct_h2l = m_layerNumberTool->sctLayers();
    m_pix_h2l = m_layerNumberTool->pixelLayers();
    m_are_pixels.resize(m_layerNumberTool->maxNumberOfUniqueLayers(), true);
    for(const auto& l : *m_sct_h2l) m_are_pixels[l] = false;
    
    return StatusCode::SUCCESS;
}

StatusCode Gbts2ActsSeedingTool::finalize() {
  return SeedingToolBase::finalize();
}

StatusCode Gbts2ActsSeedingTool::createSeeds(
      const EventContext& ctx,
      const std::vector<const xAOD::SpacePointContainer*>&
          spacePointCollections,
      const Eigen::Vector3f& beamSpotPos, float bFieldInZ,
      ActsTrk::SeedContainer& seedContainer) const {
 
    (void)ctx;
    (void)bFieldInZ;

    std::unique_ptr<GNN_DataStorage> storage = std::make_unique<GNN_DataStorage>(*m_geo, m_mlLUT);

    std::vector<std::vector<GNN_Node> > node_storage;//layer-based collections
    node_storage.resize(m_are_pixels.size());

    for(auto& v : node_storage) v.reserve(100000);
    
    unsigned int nPixelLoaded = 0;
    unsigned int nStripLoaded = 0;

    std::size_t totalSpacePoints = 0;
    for (const xAOD::SpacePointContainer* spacePoints : spacePointCollections) {
      totalSpacePoints += spacePoints->size();
    }

    std::vector<const xAOD::SpacePoint*> selectedXAODSpacePoints;
    selectedXAODSpacePoints.reserve(totalSpacePoints);

    for (const xAOD::SpacePointContainer* spacePoints : spacePointCollections) {
      for (const xAOD::SpacePoint* sp : *spacePoints) {
        const auto& pos = sp->globalPosition();
        const std::vector<xAOD::DetectorIDHashType>& elementlist = sp->elementIdList();
        const bool isPixel = (elementlist.size() == 1);
        const short layer = (isPixel ? m_pix_h2l : m_sct_h2l)->at(static_cast<int>(elementlist[0]));

        //convert incoming xaod spacepoints into GNN nodes

        GNN_Node& node = node_storage[layer].emplace_back(layer);

        node.m_x = pos.x() - beamSpotPos[0];
        node.m_y = pos.y() - beamSpotPos[1];
        node.m_z = pos.z();
        node.m_r = std::sqrt(std::pow(node.m_x, 2) + std::pow(node.m_y, 2));
        node.m_phi = std::atan2(node.m_y, node.m_x);
        node.m_idx = selectedXAODSpacePoints.size();

        if (isPixel && m_useML){
          //Check type in debug build otherwise assume it is correct
          assert(dynamic_cast<const xAOD::PixelCluster*>(sp->measurements().front())!=nullptr);
          const xAOD::PixelCluster* pCL = static_cast<const xAOD::PixelCluster*>(sp->measurements().front());
          node.m_pcw = pCL->widthInEta();
          node.m_locPosY = pCL->localPosition<2>().y();
        }

        selectedXAODSpacePoints.push_back(sp);
      }
    }

    for(size_t l = 0; l < node_storage.size(); l++) {

      const std::vector<GNN_Node>& nodes = node_storage[l];

      if(nodes.size() == 0) continue;
	
      if(m_are_pixels[l]) 
	nPixelLoaded += storage->loadPixelGraphNodes(l, nodes, m_useML);
      else
	nStripLoaded += storage->loadStripGraphNodes(l, nodes);
    }
    
    ATH_MSG_DEBUG("Loaded "<<nPixelLoaded<<" pixel spacepoints and "<<nStripLoaded<<" strip spacepoints");

    storage->sortByPhi();

    storage->initializeNodes(m_useML);

    storage->generatePhiIndexing(1.5f*m_phiSliceWidth);

    std::vector<GNN_Edge> edgeStorage;

    const TrigRoiDescriptor fakeRoi = TrigRoiDescriptor(0, -4.5, 4.5, 0, -M_PI, M_PI, 0, -150.0, 150.0);

    std::pair<int, int> graphStats = buildTheGraph(fakeRoi, storage, edgeStorage);

    ATH_MSG_DEBUG("Created graph with "<<graphStats.first<<" edges and "<<graphStats.second<< " edge links");

    if (graphStats.first == 0 || graphStats.second == 0) return StatusCode::SUCCESS;

    int maxLevel = runCCA(graphStats.first, edgeStorage);

    ATH_MSG_DEBUG("Reached Level "<<maxLevel<<" after GNN iterations");
    
    std::vector<std::pair<float, std::vector<unsigned int> > > vOutputSeeds;

    extractSeedsFromTheGraph(maxLevel, graphStats.first, totalSpacePoints, edgeStorage, vOutputSeeds);

    if (vOutputSeeds.empty()) return StatusCode::SUCCESS;

    seedContainer.reserve(vOutputSeeds.size(), 7.0f);  // 7 SP/seed to optimise allocations (average is 6.1 SP/seed)

    for (const auto& seed : vOutputSeeds) {
      // convert space points and add seed to output
      const float quality = seed.first;
      const float vertexZ = 0.0f;  // not used in GBTS seeding, set to 0
      seedContainer.push_back(
          seed.second,
          [&](const unsigned int spIndex) {
            return selectedXAODSpacePoints[spIndex];
          },
          quality, vertexZ);
    }

    ATH_MSG_DEBUG("GBTS created "<<seedContainer.size()<<" seeds");

    return StatusCode::SUCCESS;
}

