// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#include "FPGATrackSimGNNRootOutputTool.h"

///////////////////////////////////////////////////////////////////////
// AthAlgTool

FPGATrackSimGNNRootOutputTool::FPGATrackSimGNNRootOutputTool(const std::string& algname, const std::string& name, const IInterface *ifc) :
  AthAlgTool(algname, name, ifc)
{
}

StatusCode FPGATrackSimGNNRootOutputTool::initialize()
{
  ATH_CHECK(m_tHistSvc.retrieve());
  ATH_CHECK(bookTree());
  return StatusCode::SUCCESS;
}

///////////////////////////////////////////////////////////////////////
// Functions

StatusCode FPGATrackSimGNNRootOutputTool::bookTree()
{
  std::string hittree_str = "FPGATrackSimHit_reg" + m_region.value();
  m_hit_tree = new TTree(hittree_str.c_str(), hittree_str.c_str());
  m_hit_tree->Branch("hit_id",&m_hit_id);
  m_hit_tree->Branch("hit_module_id",&m_hit_module_id);
  m_hit_tree->Branch("hit_x",&m_hit_x);
  m_hit_tree->Branch("hit_y",&m_hit_y);
  m_hit_tree->Branch("hit_z",&m_hit_z);
  m_hit_tree->Branch("hit_r",&m_hit_r);
  m_hit_tree->Branch("hit_phi",&m_hit_phi);
  m_hit_tree->Branch("hit_isPixel",&m_hit_isPixel);
  m_hit_tree->Branch("hit_isStrip",&m_hit_isStrip);
  m_hit_tree->Branch("hit_hitType",&m_hit_hitType);
  m_hit_tree->Branch("hit_uniqueID",&m_hit_uniqueID);
  m_hit_tree->Branch("hit_eventIndex",&m_hit_eventIndex);
  m_hit_tree->Branch("hit_cluster_x",&m_hit_cluster_x);
  m_hit_tree->Branch("hit_cluster_y",&m_hit_cluster_y);
  m_hit_tree->Branch("hit_cluster_z",&m_hit_cluster_z);

  std::string GNNhittree_str = "FPGATrackSimGNNHit_reg" + m_region.value();
  m_GNNHit_tree = new TTree(GNNhittree_str.c_str(), GNNhittree_str.c_str());
  m_GNNHit_tree->Branch("hit_id",&m_GNNHit_id);
  m_GNNHit_tree->Branch("hit_module_id",&m_GNNHit_module_id);
  m_GNNHit_tree->Branch("hit_road_id",&m_GNNHit_road_id);
  m_GNNHit_tree->Branch("hit_x",&m_GNNHit_x);
  m_GNNHit_tree->Branch("hit_y",&m_GNNHit_y);
  m_GNNHit_tree->Branch("hit_z",&m_GNNHit_z);
  m_GNNHit_tree->Branch("hit_r",&m_GNNHit_r);
  m_GNNHit_tree->Branch("hit_phi",&m_GNNHit_phi);
  m_GNNHit_tree->Branch("hit_eta",&m_GNNHit_eta);
  m_GNNHit_tree->Branch("hit_cluster_x_1",&m_GNNHit_cluster_x_1);
  m_GNNHit_tree->Branch("hit_cluster_y_1",&m_GNNHit_cluster_y_1);
  m_GNNHit_tree->Branch("hit_cluster_z_1",&m_GNNHit_cluster_z_1);
  m_GNNHit_tree->Branch("hit_cluster_r_1",&m_GNNHit_cluster_r_1);
  m_GNNHit_tree->Branch("hit_cluster_phi_1",&m_GNNHit_cluster_phi_1);
  m_GNNHit_tree->Branch("hit_cluster_eta_1",&m_GNNHit_cluster_eta_1);
  m_GNNHit_tree->Branch("hit_cluster_x_2",&m_GNNHit_cluster_x_2);
  m_GNNHit_tree->Branch("hit_cluster_y_2",&m_GNNHit_cluster_y_2);
  m_GNNHit_tree->Branch("hit_cluster_z_2",&m_GNNHit_cluster_z_2);
  m_GNNHit_tree->Branch("hit_cluster_r_2",&m_GNNHit_cluster_r_2);
  m_GNNHit_tree->Branch("hit_cluster_phi_2",&m_GNNHit_cluster_phi_2);
  m_GNNHit_tree->Branch("hit_cluster_eta_2",&m_GNNHit_cluster_eta_2);

  std::string GNNedgetree_str = "FPGATrackSimGNNEdge_reg" + m_region.value();
  m_GNNEdge_tree = new TTree(GNNedgetree_str.c_str(), GNNedgetree_str.c_str());
  m_GNNEdge_tree->Branch("edge_index_1",&m_GNNEdge_index_1);
  m_GNNEdge_tree->Branch("edge_index_2",&m_GNNEdge_index_2);
  m_GNNEdge_tree->Branch("edge_dr",&m_GNNEdge_dR);
  m_GNNEdge_tree->Branch("edge_dphi",&m_GNNEdge_dPhi);
  m_GNNEdge_tree->Branch("edge_dz",&m_GNNEdge_dZ);
  m_GNNEdge_tree->Branch("edge_deta",&m_GNNEdge_dEta);
  m_GNNEdge_tree->Branch("edge_phislope",&m_GNNEdge_phiSlope);
  m_GNNEdge_tree->Branch("edge_rphislope",&m_GNNEdge_rPhiSlope);
  m_GNNEdge_tree->Branch("edge_score",&m_GNNEdge_score);

  std::string roadtree_str = "FPGATrackSimRoad_reg" + m_region.value();
  m_road_tree = new TTree(roadtree_str.c_str(), roadtree_str.c_str());
  m_road_tree->Branch("road_id",&m_road_id);
  m_road_tree->Branch("road_nHits",&m_road_nHits);
  m_road_tree->Branch("road_nHits_layer",&m_road_nHits_layer);
  m_road_tree->Branch("road_nLayers",&m_road_nLayers);
  m_road_tree->Branch("road_hit_uniqueID",&m_road_hit_uniqueID);
  m_road_tree->Branch("road_hit_barcode",&m_road_hit_barcode);
  m_road_tree->Branch("road_hit_eventIndex",&m_road_hit_eventIndex);
  m_road_tree->Branch("road_hit_z",&m_road_hit_z);
  m_road_tree->Branch("road_hit_r",&m_road_hit_r);

  ATH_CHECK(m_tHistSvc->regTree(Form("/TRIGFPGATrackSimGNNOUTPUT/%s",m_hit_tree->GetName()), m_hit_tree));
  ATH_CHECK(m_tHistSvc->regTree(Form("/TRIGFPGATrackSimGNNOUTPUT/%s",m_GNNHit_tree->GetName()), m_GNNHit_tree));
  ATH_CHECK(m_tHistSvc->regTree(Form("/TRIGFPGATrackSimGNNOUTPUT/%s",m_GNNEdge_tree->GetName()), m_GNNEdge_tree));
  ATH_CHECK(m_tHistSvc->regTree(Form("/TRIGFPGATrackSimGNNOUTPUT/%s",m_road_tree->GetName()), m_road_tree));

  return StatusCode::SUCCESS;
}

StatusCode FPGATrackSimGNNRootOutputTool::fillTree(const std::vector<std::shared_ptr<const FPGATrackSimHit>> & hits,
                                                   const std::vector<std::shared_ptr<FPGATrackSimGNNHit>> & gnn_hits,
                                                   const std::vector<std::shared_ptr<FPGATrackSimGNNEdge>> & edges,
                                                   const std::vector<FPGATrackSimRoad> & roads)
{ 
  // fill the FPGATrackSimHits
  int hit_count = 0;
  for (const auto& hit : hits) {
    m_hit_id.push_back(hit_count);
    m_hit_module_id.push_back(hit->getIdentifier());
    m_hit_x.push_back(hit->getX());
    m_hit_y.push_back(hit->getY());
    m_hit_z.push_back(hit->getZ());
    m_hit_r.push_back(hit->getR());
    m_hit_phi.push_back(hit->getGPhi());
    m_hit_isPixel.push_back(hit->isPixel());
    m_hit_isStrip.push_back(hit->isStrip());
    m_hit_hitType.push_back(static_cast<int>(hit->getHitType()));
    m_hit_uniqueID.push_back(hit->getUniqueID());
    m_hit_eventIndex.push_back(hit->getEventIndex());
    m_hit_cluster_x.push_back(hit->getOriginalHit().getX());
    m_hit_cluster_y.push_back(hit->getOriginalHit().getY());
    m_hit_cluster_z.push_back(hit->getOriginalHit().getZ());
    hit_count++;
  }
  m_hit_tree->Fill();

  // fill the FPGATrackSimGNNHits
  for (const auto& hit : gnn_hits) {
    m_GNNHit_id.push_back(hit->getHitID());
    m_GNNHit_module_id.push_back(hit->getIdentifier());
    m_GNNHit_road_id.push_back(hit->getRoadID());
    m_GNNHit_x.push_back(hit->getX());
    m_GNNHit_y.push_back(hit->getY());
    m_GNNHit_z.push_back(hit->getZ());
    m_GNNHit_r.push_back(hit->getR());
    m_GNNHit_phi.push_back(hit->getPhi());
    m_GNNHit_eta.push_back(hit->getEta());
    m_GNNHit_cluster_x_1.push_back(hit->getCluster1X());
    m_GNNHit_cluster_y_1.push_back(hit->getCluster1Y());
    m_GNNHit_cluster_z_1.push_back(hit->getCluster1Z());
    m_GNNHit_cluster_r_1.push_back(hit->getCluster1R());
    m_GNNHit_cluster_phi_1.push_back(hit->getCluster1Phi());
    m_GNNHit_cluster_eta_1.push_back(hit->getCluster1Eta());
    m_GNNHit_cluster_x_2.push_back(hit->getCluster2X());
    m_GNNHit_cluster_y_2.push_back(hit->getCluster2Y());
    m_GNNHit_cluster_z_2.push_back(hit->getCluster2Z());
    m_GNNHit_cluster_r_2.push_back(hit->getCluster2R());
    m_GNNHit_cluster_phi_2.push_back(hit->getCluster2Phi());
    m_GNNHit_cluster_eta_2.push_back(hit->getCluster2Eta());
  }
  m_GNNHit_tree->Fill();

  // fill the FPGATrackSimGNNEdges
  for (const auto& edge : edges) {
    m_GNNEdge_index_1.push_back(edge->getEdgeIndex1());
    m_GNNEdge_index_2.push_back(edge->getEdgeIndex2());
    m_GNNEdge_dR.push_back(edge->getEdgeDR());
    m_GNNEdge_dPhi.push_back(edge->getEdgeDPhi());
    m_GNNEdge_dZ.push_back(edge->getEdgeDZ());
    m_GNNEdge_dEta.push_back(edge->getEdgeDEta());
    m_GNNEdge_phiSlope.push_back(edge->getEdgePhiSlope());
    m_GNNEdge_rPhiSlope.push_back(edge->getEdgeRPhiSlope());
    m_GNNEdge_score.push_back(edge->getEdgeScore());
  }
  m_GNNEdge_tree->Fill();

  for (const auto& road : roads) {
    m_road_id.push_back(road.getRoadID());
    m_road_nHits.push_back(road.getNHits());
    m_road_nHits_layer.push_back(road.getNHits_layer());
    m_road_nLayers.push_back(road.getNLayers());

    std::vector<std::vector<HepMcParticleLink::barcode_type>> road_hit_uniqueID(road.getNLayers());
    std::vector<std::vector<HepMcParticleLink::barcode_type>> road_hit_barcode(road.getNLayers());
    std::vector<std::vector<long>> road_hit_eventIndex(road.getNLayers());
    std::vector<std::vector<float>> road_hit_z(road.getNLayers());
    std::vector<std::vector<float>> road_hit_r(road.getNLayers());
    for (size_t l = 0; l < road.getNLayers(); ++l) {
      for (const auto &layerH : road.getHits(l)) {
        road_hit_uniqueID[l].push_back((*layerH).getUniqueID());
        road_hit_barcode[l].push_back((*layerH).getBarcode());
        road_hit_eventIndex[l].push_back((*layerH).getEventIndex());
        road_hit_z[l].push_back((*layerH).getOriginalHit().getZ());
        road_hit_r[l].push_back((*layerH).getOriginalHit().getR());
      }
    }

    m_road_hit_uniqueID.push_back(std::move(road_hit_uniqueID));
    m_road_hit_barcode.push_back(std::move(road_hit_barcode));
    m_road_hit_eventIndex.push_back(std::move(road_hit_eventIndex));
    m_road_hit_z.push_back(std::move(road_hit_z));
    m_road_hit_r.push_back(std::move(road_hit_r));

  }
  m_road_tree->Fill();

  resetVectors();

  return StatusCode::SUCCESS;
}

void FPGATrackSimGNNRootOutputTool::resetVectors() 
{
  m_hit_id.clear();
  m_hit_module_id.clear();
  m_hit_x.clear();
  m_hit_y.clear();
  m_hit_z.clear();
  m_hit_r.clear();
  m_hit_phi.clear();
  m_hit_isPixel.clear();
  m_hit_isStrip.clear();
  m_hit_hitType.clear();
  m_hit_uniqueID.clear();
  m_hit_eventIndex.clear();
  m_hit_cluster_x.clear();
  m_hit_cluster_y.clear();
  m_hit_cluster_z.clear();
  m_GNNHit_id.clear();
  m_GNNHit_module_id.clear();
  m_GNNHit_road_id.clear();
  m_GNNHit_x.clear();
  m_GNNHit_y.clear();
  m_GNNHit_z.clear();
  m_GNNHit_r.clear();
  m_GNNHit_phi.clear();
  m_GNNHit_eta.clear();
  m_GNNHit_cluster_x_1.clear();
  m_GNNHit_cluster_y_1.clear();
  m_GNNHit_cluster_z_1.clear();
  m_GNNHit_cluster_r_1.clear();
  m_GNNHit_cluster_phi_1.clear();
  m_GNNHit_cluster_eta_1.clear();
  m_GNNHit_cluster_x_2.clear();
  m_GNNHit_cluster_y_2.clear();
  m_GNNHit_cluster_z_2.clear();
  m_GNNHit_cluster_r_2.clear();
  m_GNNHit_cluster_phi_2.clear();
  m_GNNHit_cluster_eta_2.clear();
  m_GNNEdge_index_1.clear();
  m_GNNEdge_index_2.clear();
  m_GNNEdge_dR.clear();
  m_GNNEdge_dPhi.clear();
  m_GNNEdge_dZ.clear();
  m_GNNEdge_dEta.clear();
  m_GNNEdge_phiSlope.clear();
  m_GNNEdge_rPhiSlope.clear();
  m_GNNEdge_score.clear();
  m_road_id.clear();
  m_road_nHits.clear();
  m_road_nHits_layer.clear();
  m_road_nLayers.clear();
  m_road_hit_uniqueID.clear();
  m_road_hit_barcode.clear();
  m_road_hit_eventIndex.clear();
  m_road_hit_z.clear();
  m_road_hit_r.clear();
}