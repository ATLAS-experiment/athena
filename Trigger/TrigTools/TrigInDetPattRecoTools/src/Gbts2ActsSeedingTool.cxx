/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "xAODInDetMeasurement/ContainerAccessor.h"
#include "TrigSteeringEvent/TrigRoiDescriptor.h"
#include "xAODInDetMeasurement/PixelCluster.h"

#include "Gbts2ActsSeedingTool.h"
#include "GNN_TrackingFilter.h"

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

StatusCode Gbts2ActsSeedingTool::createSeeds(const EventContext& ctx, const Acts::SpacePointContainer<ActsTrk::SpacePointCollector, Acts::detail::RefHolder>& spContainer, const Acts::Vector3&, const Acts::Vector3&, ActsTrk::SeedContainer& seedContainer) const {
  
  std::unique_ptr<GNN_DataStorage> storage = std::make_unique<GNN_DataStorage>(*m_geo, m_mlLUT);

    SG::ReadCondHandle<InDet::BeamSpotData> beamSpotHandle { m_beamSpotKey, ctx };
    
    const Amg::Vector3D &vertex = beamSpotHandle->beamPos();
    
    float shift_x = vertex.x() - beamSpotHandle->beamTilt(0)*vertex.z();
    float shift_y = vertex.y() - beamSpotHandle->beamTilt(1)*vertex.z();

    std::vector<std::vector<GNN_Node> > node_storage;//layer-based collections
    node_storage.resize(m_are_pixels.size());

    for(auto& v : node_storage) v.reserve(100000);
    
    unsigned int nPixelLoaded = 0;
    unsigned int nStripLoaded = 0;

    for(size_t idx=0; idx<spContainer.size(); idx++){
        const auto & sp = spContainer.at(idx);
        const auto & extSP = sp.externalSpacePoint();
        const std::vector<xAOD::DetectorIDHashType>& elementlist = extSP.elementIdList() ;

        bool isPixel(elementlist.size() == 1);

	short layer = (isPixel ? m_pix_h2l : m_sct_h2l)->at(static_cast<int>(elementlist[0]));
 
	//convert incoming xaod spacepoints into GNN nodes
	
        GNN_Node& node = node_storage[layer].emplace_back(layer);

        const auto& pos = extSP.globalPosition();	

	node.m_x = pos.x() - shift_x;
	node.m_y = pos.y() - shift_y;
	node.m_z = pos.z();
	node.m_r = std::sqrt(std::pow(node.m_x, 2) + std::pow(node.m_y, 2));
	node.m_phi = std::atan2(node.m_y, node.m_x);
	node.m_idx = idx;

        if(isPixel && m_useML){
            const xAOD::PixelCluster* pCL = dynamic_cast<const xAOD::PixelCluster*>(extSP.measurements().front());
            if(pCL != nullptr){
                node.m_pcw = pCL->widthInEta();
		node.m_locPosY = pCL->localPosition<2>().y();
            }
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

    storage->generatePhiIndexing(1.5*m_phiSliceWidth);

    std::vector<GNN_Edge> edgeStorage;

    const TrigRoiDescriptor fakeRoi = TrigRoiDescriptor(0, -4.5, 4.5, 0, -M_PI, M_PI, 0, -150.0, 150.0);

    std::pair<int, int> graphStats = buildTheGraph(fakeRoi, storage, edgeStorage);

    ATH_MSG_DEBUG("Created graph with "<<graphStats.first<<" edges and "<<graphStats.second<< " edge links");

    int maxLevel = runCCA(graphStats.first, edgeStorage);

    ATH_MSG_DEBUG("Reached Level "<<maxLevel<<" after GNN iterations");

    int minLevel = 3;//a triplet + 2 confirmation

    if(m_LRTmode) {
        minLevel = 2;//a triplet + 1 confirmation
    }

    if(maxLevel < minLevel) return StatusCode::SUCCESS;

    std::vector<GNN_Edge*> vSeeds;

    vSeeds.reserve(graphStats.first/2);

    for(int edgeIndex = 0; edgeIndex < graphStats.first; edgeIndex++) {
      
      GNN_Edge* pS = &(edgeStorage.at(edgeIndex));

      if(pS->m_level < minLevel) continue;

      vSeeds.push_back(pS);
    }

    if(vSeeds.empty()) return StatusCode::SUCCESS;
 
    std::sort(vSeeds.begin(), vSeeds.end(), GNN_Edge::CompareLevel());

    //backtracking

    TrigFTF_GNN_TrackingFilter tFilter(m_layerGeometry, edgeStorage);

    for(auto pS : vSeeds) {
        if(pS->m_level == -1) continue;

        TrigFTF_GNN_EdgeState rs(false);

        tFilter.followTrack(pS, rs);

        if(!rs.m_initialized) {
            continue;
        }

        if(static_cast<int>(rs.m_vs.size()) < minLevel) continue;

        std::vector<const GNN_Node*> vN;

        for(std::vector<GNN_Edge*>::reverse_iterator sIt=rs.m_vs.rbegin();sIt!=rs.m_vs.rend();++sIt) {
            (*sIt)->m_level = -1;//mark as collected
            
            if(sIt == rs.m_vs.rbegin()) {
                vN.push_back((*sIt)->m_n1);
            }

            vN.push_back((*sIt)->m_n2);
	    
        }

        if(vN.size()<3) continue;
	
        std::vector<const xAOD::SpacePoint*> sps;
        sps.reserve(vN.size());
        for (const auto* vNptr : vN) {
          sps.push_back(&spContainer.at(vNptr->sp_idx()).externalSpacePoint());
        }
	
	//add seed to output

	std::unique_ptr<ActsTrk::Seed> to_add = std::make_unique<ActsTrk::Seed>(std::move(sps));
	
        seedContainer.push_back(std::move(to_add));
    }

    ATH_MSG_DEBUG("GBTS created "<<seedContainer.size()<<" seeds");

    return StatusCode::SUCCESS;
}

