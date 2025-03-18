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
    ATH_CHECK( m_pixelDetEleCollKey.initialize() );
    ATH_CHECK(m_stripDetEleCollKey.initialize());
    ATH_CHECK(m_beamSpotKey.initialize());

    return StatusCode::SUCCESS;
}

StatusCode Gbts2ActsSeedingTool::finalize() {
  return SeedingToolBase::finalize();
}

StatusCode Gbts2ActsSeedingTool::createSeeds(const EventContext& ctx, const Acts::SpacePointContainer<ActsTrk::SpacePointCollector, Acts::detail::RefHolder>& spContainer, const Acts::Vector3&, const Acts::Vector3&, ActsTrk::SeedContainer& seedContainer) const {
  
    std::unique_ptr<GNN_DataStorage> storage = std::make_unique<GNN_DataStorage>(*m_geo);

    SG::ReadCondHandle<InDetDD::SiDetectorElementCollection> pixelDetEleHandle = SG::makeHandle(m_pixelDetEleCollKey, ctx);
    
    ATH_CHECK(pixelDetEleHandle.isValid());
    
    const InDetDD::SiDetectorElementCollection* pixelElements = pixelDetEleHandle.cptr();

    SG::ReadCondHandle<InDetDD::SiDetectorElementCollection> stripDetEleHandle = SG::makeHandle(m_stripDetEleCollKey, ctx);
    
    ATH_CHECK(stripDetEleHandle.isValid());
    
    const InDetDD::SiDetectorElementCollection* stripElements = stripDetEleHandle.cptr();

    SG::ReadCondHandle<InDet::BeamSpotData> beamSpotHandle { m_beamSpotKey, ctx };
    
    const Amg::Vector3D &vertex = beamSpotHandle->beamPos();
    
    float shift_x = vertex.x() - beamSpotHandle->beamTilt(0)*vertex.z();
    float shift_y = vertex.y() - beamSpotHandle->beamTilt(1)*vertex.z();

    //convert incoming xaod spacepoints into GNN nodes

    const std::vector<short>* sct_h2l = m_layerNumberTool->sctLayers();
    const std::vector<short>* pix_h2l = m_layerNumberTool->pixelLayers();

    std::map<short, std::vector<GNN_Node>> pix_nodes;
    std::map<short, std::vector<GNN_Node>> strip_nodes;

    std::vector<std::vector<GNN_Node>> pixel_vector;
    std::vector<std::vector<GNN_Node>> strip_vector;

    pixel_vector.resize(pix_h2l->size());
    strip_vector.resize(sct_h2l->size());

    unsigned int nPixelLoaded = 0;
    unsigned int nStripLoaded = 0;

    for(size_t idx=0; idx<spContainer.size(); idx++){
        const auto & sp = spContainer.at(idx);
        const auto & extSP = sp.externalSpacePoint();
        const std::vector<xAOD::DetectorIDHashType>& elementlist = extSP.elementIdList() ;

        bool isPixel(elementlist.size() == 1);

        const InDetDD::SiDetectorElement* detElement = (isPixel ? pixelElements : stripElements)->getDetectorElement(static_cast<int>(elementlist[0]));
        short layer = (isPixel ? pix_h2l : sct_h2l)->at(static_cast<int>(detElement->identifyHash()));

        GNN_Node node(layer);

        const auto& pos = extSP.globalPosition();

        node.m_x = pos.x() - shift_x;
        node.m_y = pos.y() - shift_y;
        node.m_z = pos.z();
        node.m_r = std::sqrt(node.m_x*node.m_x + node.m_y*node.m_y);
        node.m_phi = std::atan2(node.m_y,node.m_x);
	node.m_idx = idx;

        if(isPixel && m_useML){
	    const auto& lm = extSP.measurements();
            const xAOD::PixelCluster* pCL = dynamic_cast<const xAOD::PixelCluster*>(lm.front());
            if(pCL != nullptr){
                node.m_pcw = pCL->widthInEta();
            }
        }
        

        if(isPixel){
            pixel_vector.at(layer).push_back(node);
        }else{
            strip_vector.at(layer).push_back(node);
        }
    }

    for(size_t l = 0; l<pix_h2l->size(); l++){
        std::vector<GNN_Node>& nodes = pixel_vector.at(l);
        if(nodes.size() == 0) continue;
        nPixelLoaded += storage->loadPixelGraphNodes(l, std::move(nodes), m_useML);
    }

    for(size_t l = 0; l<sct_h2l->size(); l++){
        std::vector<GNN_Node>& nodes = strip_vector.at(l);
        if(nodes.size() == 0) continue;
        nStripLoaded += storage->loadStripGraphNodes(l, std::move(nodes));
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

    for(int edgeIndex=0;edgeIndex<graphStats.first;edgeIndex++) {
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
	
	const auto & sp1 = spContainer.at(vN[0]->sp_idx()).externalSpacePoint();
	const auto & sp2 = spContainer.at(vN[1]->sp_idx()).externalSpacePoint();
	const auto & sp3 = spContainer.at(vN[2]->sp_idx()).externalSpacePoint();
	
	//add seed to output

	std::unique_ptr<ActsTrk::Seed> to_add = std::make_unique<ActsTrk::Seed>(sp1, sp2, sp3);
	
        seedContainer.push_back(std::move(to_add));
    }

    ATH_MSG_DEBUG("GBTS created "<<seedContainer.size()<<" seeds");

    return StatusCode::SUCCESS;
}

