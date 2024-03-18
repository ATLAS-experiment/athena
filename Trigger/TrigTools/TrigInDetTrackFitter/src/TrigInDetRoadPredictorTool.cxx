/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include <cmath>
#include <iostream>
#include <memory>
#include "GaudiKernel/SystemOfUnits.h"

#include "TrkParameters/TrackParameters.h"
#include "TrkPrepRawData/PrepRawData.h"
#include "TrkSpacePoint/SpacePoint.h"
#include "PixelReadoutGeometry/PixelDetectorManager.h"
#include "SCT_ReadoutGeometry/SCT_DetectorManager.h"
#include "InDetIdentifier/PixelID.h"


#include "TrigInDetRoadPredictorTool.h"

#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "AthenaBaseComps/AthCheckMacros.h"

#include "TrkSurfaces/Surface.h"
#include "InDetPrepRawData/PixelCluster.h"



TrigInDetRoadPredictorTool::TrigInDetRoadPredictorTool(const std::string& t,
						       const std::string& n,
						       const IInterface*  p ): AthAlgTool(t,n,p)
{
  declareInterface< ITrigInDetRoadPredictorTool >( this );
}

StatusCode TrigInDetRoadPredictorTool::initialize() {

  StatusCode sc = m_layerNumberTool.retrieve();
  if(sc.isFailure()) {
    ATH_MSG_ERROR("Could not retrieve "<<m_layerNumberTool);
    return sc;
  } else {
    ATH_MSG_INFO("Detector layer structure has "<<m_layerNumberTool->maxNumberOfUniqueLayers()<<" unique layers");
  }

  
  ATH_CHECK( m_fieldCondObjInputKey.initialize());
  ATH_CHECK( detStore()->retrieve(m_pixelManager, "ITkPixel") );
  ATH_CHECK( detStore()->retrieve(m_stripManager, "ITkStrip") );
  ATH_CHECK( detStore()->retrieve(m_pixelId, "PixelID") );
  ATH_CHECK( detStore()->retrieve(m_stripId, "SCT_ID") );

  buildDetectorDescription();
  
  
  return StatusCode::SUCCESS;
}

void TrigInDetRoadPredictorTool::addNewElement(unsigned int layerID, short phi_idx, short eta_idx, const InDetDD::SiDetectorElement* p) {

  unsigned int hash = p->identifyHash();
  
  DetectorElementDescription ded(hash);

  //find corners in the global c.s.
  
  float de_len  = 0.5*p->design().length();
  float de_wmax = 0.5*p->design().maxWidth();
  float de_wmin = 0.5*p->design().minWidth();
  
  float dPhi[4] = {de_wmax, de_wmin,  -de_wmin, -de_wmax};//locX
  float dEta[4] = {de_len, -de_len,   -de_len,   de_len}; //locY

  const Amg::Vector3D& C  = p->center();
  const Amg::Vector3D& PhiAx = p->phiAxis();
  const Amg::Vector3D& EtaAx = p->etaAxis();
  
  for(int ic=0; ic<4; ic++) {
    float x = C.x() + PhiAx.x()*dPhi[ic] + EtaAx.x()*dEta[ic];
    float y = C.y() + PhiAx.y()*dPhi[ic] + EtaAx.y()*dEta[ic];
    float z = C.z() + PhiAx.z()*dPhi[ic] + EtaAx.z()*dEta[ic];
    ded.m_c[ic][0] = std::sqrt(x*x+y*y);//r
    ded.m_c[ic][1] = z;
    ded.m_c[ic][2] = std::atan2(y,x);//phi
  }

  auto& L = (*m_layerMap.find(layerID)).second;
  
  short prim_idx = L.m_mappingType != 2 ? phi_idx : eta_idx;
  short sec_idx  = L.m_mappingType != 2 ? eta_idx : phi_idx;
  
  if(L.m_mappingType == 0) {//barrel: primary index is phi, eta/z is secondary
    ded.m_ref   = C.z();
    ded.m_index = sec_idx;
    ded.m_minBound = ded.m_c[0][1];
    ded.m_maxBound = ded.m_c[0][1];
    for(int ic=1;ic<4;ic++) {
      if(ded.m_minBound > ded.m_c[ic][1]) ded.m_minBound = ded.m_c[ic][1];
      if(ded.m_maxBound < ded.m_c[ic][1]) ded.m_maxBound = ded.m_c[ic][1];
    }
  }

  if(L.m_mappingType == 1) {//pixel endcap layers : primary index is phi, eta/R is secondary
    ded.m_ref   = C.perp();
    ded.m_index = sec_idx;
    ded.m_minBound = ded.m_c[0][0];
    ded.m_maxBound = ded.m_c[0][0];
    for(int ic=1;ic<4;ic++) {
      if(ded.m_minBound > ded.m_c[ic][0]) ded.m_minBound = ded.m_c[ic][0];
      if(ded.m_maxBound < ded.m_c[ic][0]) ded.m_maxBound = ded.m_c[ic][0];
    }
  }

  if(L.m_mappingType == 2) {//strip endcaps: primary index is R, phi is secondary, layers are double
    ded.m_ref   = std::atan2(C.y(),C.x());
    ded.m_index = sec_idx;
    ded.m_minBound = ded.m_c[0][2];
    ded.m_maxBound = ded.m_c[0][2];
    for(int ic=1;ic<4;ic++) {
      if(ded.m_minBound > ded.m_c[ic][2]) ded.m_minBound = ded.m_c[ic][2];
      if(ded.m_maxBound < ded.m_c[ic][2]) ded.m_maxBound = ded.m_c[ic][2];
    }
  }

  //put DetEl description into the corresponding collection
  
  int detColIdx = 0;
  
  if(L.m_nSubLayers == 2 && (hash % 2) != 0) {//double layer, odd detector elements
    detColIdx = 1;
  }
  
  if(L.m_colls[ detColIdx].find(prim_idx) == L.m_colls[ detColIdx].end()) {

    float primBounds[2] = {0,0};
    if(L.m_mappingType == 2) {
      primBounds[0] = p->rMin();
      primBounds[1] = p->rMax();
    }
    else {
      primBounds[0] = p->phiMin();
      primBounds[1] = p->phiMax();
    }
    
    DetectorElementsCollection dc(prim_idx, primBounds[0], primBounds[1]);
    L.m_colls[ detColIdx].insert(std::make_pair(prim_idx, dc));
  }

  (*L.m_colls[detColIdx].find(prim_idx)).second.m_vDE.push_back(ded);

}


void TrigInDetRoadPredictorTool::buildDetectorDescription() {

  const std::vector<short>& vPixelL =  *(m_layerNumberTool->pixelLayers());
  const std::vector<TrigInDetSiLayer>& SiL = *(m_layerNumberTool->layerGeometry());
  
  for(int hash = 0; hash<static_cast<int>(m_pixelId->wafer_hash_max()); hash++) {

    Identifier offlineId = m_pixelId->wafer_id(hash);
    short barrel_ec = m_pixelId->barrel_ec(offlineId);
    if(std::abs(barrel_ec)>2) continue;//no DBM needed
    
    unsigned int layerID  = SiL.at(vPixelL.at(hash)).m_subdet;
    unsigned int volumeID = layerID / 1000;

    if(m_layerMap.find(layerID) == m_layerMap.end()) {
      
      int nSubLayers = 1;
      unsigned int mappingType = 1;
      
      if(barrel_ec == 0) mappingType = 0;
      else {
	if(volumeID == 12 || volumeID == 14) mappingType = 2;
      }
      m_layerMap.try_emplace(layerID, layerID, nSubLayers, mappingType);
    }
    
    short phi_index = m_pixelId->phi_module(offlineId);
    short eta_index = m_pixelId->eta_module(offlineId);
    
    addNewElement(layerID, phi_index, eta_index, m_pixelManager->getDetectorElement(hash)); 
  }

  const std::vector<short>& vStripL =  *(m_layerNumberTool->sctLayers());
 
  for(int hash = 0; hash<static_cast<int>(m_stripId->wafer_hash_max()); hash++) {

    Identifier offlineId = m_stripId->wafer_id(hash);
    short barrel_ec = m_stripId->barrel_ec(offlineId);
    if(std::abs(barrel_ec)>2) continue;
    
    unsigned int layerID  = SiL.at(vStripL.at(hash)).m_subdet;
    unsigned int volumeID = layerID / 1000;
    
    if(m_layerMap.find(layerID) == m_layerMap.end()) {
      int nSubLayers = 2;//strip layers are double
      unsigned int mappingType = 1;

      if(barrel_ec == 0) mappingType = 0;
      else {
	if(volumeID == 12 || volumeID == 14) mappingType = 2;
      }
      m_layerMap.try_emplace(layerID, layerID, nSubLayers, mappingType);
    }

    short phi_index = m_stripId->phi_module(offlineId);
    short eta_index = m_stripId->eta_module(offlineId);
    
    addNewElement(layerID, phi_index, eta_index, m_stripManager->getDetectorElement(hash)); 
  }

  //building hit boxes

  createHitBoxes();
   
}

void TrigInDetRoadPredictorTool::createHitBoxes() {

  const float margin_r = 3.0;
  const float margin_z = 3.0;

  for(const auto& l : m_layerMap) {

    unsigned int layerID = l.first;
    unsigned int volumeID = layerID / 1000;

    const auto& L = l.second;

    float minR =  1e8;// vert0
    float minZ =  1e8;// vert1
    float maxR = -1e8;// vert2
    float maxZ = -1e8;// vert3

    float vert[4][2]; //z,r

    memset(&vert[0][0],0,sizeof(vert));
  
    for(int iSL = 0; iSL < L.m_nSubLayers;iSL++) {
      for(const auto& deColl : L.m_colls[iSL]) {
	for(const auto& de : deColl.second.m_vDE) {
	  for(int ic=0;ic<4;ic++) {
	    float r = de.m_c[ic][0];
	    float z = de.m_c[ic][1];
	    if(r < minR) {
	      minR = r;
	      vert[0][0] = z;
	      vert[0][1] = r;
	    }
	    if(z < minZ) {
	      minZ = z;
	      vert[1][0] = z;
	      vert[1][1] = r;
	    }
	    if(r > maxR) {
	      maxR = r;
	      vert[2][0] = z;
	      vert[2][1] = r;
	    }
	    if(z > maxZ) {
	      maxZ = z;
	      vert[3][0] = z;
	      vert[3][1] = r;
	    }
	  }
	}
      }
    }
    minR -= margin_r;
    vert[0][1] -= margin_r;
    minZ -= margin_z;
    vert[1][0] -= margin_z;
    maxR += margin_r;
    vert[2][1] += margin_r;
    maxZ += margin_z;
    vert[3][0] += margin_z;

    int new_layer_index = -1;
    LayerBoundary lb;
    new_layer_index = m_lBoundaries.size();
    lb.m_index = new_layer_index;
    lb.m_lay_id = layerID;
    lb.m_nVertices = 5;
    
    if(volumeID == 73 || volumeID == 75 || volumeID == 77) {//negative inclined
      lb.m_z = {vert[3][0], vert[2][0], vert[1][0], vert[0][0], vert[3][0]};
      lb.m_r = {vert[3][1], vert[2][1], vert[1][1], vert[0][1], vert[3][1]}; 
    }
    else if(volumeID == 93 || volumeID == 95 || volumeID == 97) {//positive inclined
      lb.m_z = {vert[2][0], vert[1][0], vert[0][0], vert[3][0], vert[2][0]};
      lb.m_r = {vert[2][1], vert[1][1], vert[0][1], vert[3][1], vert[2][1]}; 
    }
    else {//all other layers
      lb.m_z = {maxZ, minZ, minZ, maxZ, maxZ};
      lb.m_r = {maxR, maxR, minR, minR, maxR};  
    }

    m_lBoundaries.push_back(lb);
  
    bool volExists = false;
  
    for(auto& v : m_vBoundaries) {
      if(v.m_vol_id == (int)volumeID) {
	volExists = true;//update corners
	if(v.m_zr[0] > minZ) v.m_zr[0] = minZ;
	if(v.m_zr[1] < maxZ) v.m_zr[1] = maxZ;
	if(v.m_zr[2] > minR) v.m_zr[2] = minR;
	if(v.m_zr[3] < maxR) v.m_zr[3] = maxR;
	v.m_layers.push_back(new_layer_index);
	break;
      }
    }
    if(!volExists) {//add a new volume with 4 corners
      VolumeBoundary vb;
      vb.m_layers.push_back(new_layer_index);
      vb.m_index = m_vBoundaries.size();
      vb.m_vol_id = volumeID;
      vb.m_zr[0] = minZ;
      vb.m_zr[1] = maxZ;
      vb.m_zr[2] = minR;
      vb.m_zr[3] = maxR;
      m_vBoundaries.push_back(vb);
    }
  }
}

void TrigInDetRoadPredictorTool::findDetectorElements(unsigned int layerID, const SearchInterval& searchArea,
						      std::vector<unsigned int>& vIDs, bool hasHit) const {

  const float road_width_rz   = hasHit ? m_min_rz_rw   : m_max_rz_rw;
  const float road_width_rphi = hasHit ? m_min_rphi_rw : m_max_rphi_rw;

  float phi_test = searchArea.m_phi;
  float phi_res  = road_width_rphi/searchArea.m_r;
    
  if(phi_res > m_max_phi_rw) phi_res = m_max_phi_rw; 
  if(phi_res < m_min_phi_rw) phi_res = m_min_phi_rw;
  
  float phi_min = phi_test - phi_res;
  float phi_max = phi_test + phi_res;
    
  const auto& L = (*m_layerMap.find(layerID)).second;

  if(L.m_mappingType != 2) { // primary index is Phi, secondary is Z or R

    float rz_test_m = searchArea.getMinR();
    float rz_test_p = searchArea.getMaxR();
    
    if(L.m_mappingType == 0) {      
      rz_test_m = searchArea.getMinZ();
      rz_test_p = searchArea.getMaxZ();
    }

    rz_test_m -= road_width_rz;
    rz_test_p += road_width_rz;
    
    for(int iSubL = 0; iSubL < L.m_nSubLayers;iSubL++) {
      
      for(const auto& prim : L.m_colls[iSubL]) {

	const auto& slice = prim.second;

	float f1 = slice.m_minCoord;
	float f2 = slice.m_maxCoord;
	
	if(std::abs(f2 - f1) < M_PI) {
	  if(phi_max < f1) continue;
	  if(phi_min > f2) continue;
	}
	else {// +/- pi boundary
	  std::swap(f2, f1);
	  if(phi_test < 0 && phi_min > f1) continue;
	  if(phi_test > 0 && phi_max < f2) continue;
	}
	
	for(const auto& de : slice.m_vDE) {
	  
	  float p1 = de.m_minBound;
	  float p2 = de.m_maxBound;

	  if(rz_test_p < p1) break;
	  if(rz_test_m > p2) continue;

	  if (! (rz_test_p < p1 || rz_test_m > p2)) vIDs.push_back(de.m_hash);
	}
      }
    }
  }
  else { //Strip endcaps: primary R, secondary Phi
    
    float r_test_m = searchArea.getMinR() - road_width_rz;
    float r_test_p = searchArea.getMaxR() + road_width_rz;
      
    for(int iSubL = 0; iSubL < L.m_nSubLayers;iSubL++) {
      
      for(const auto& prim : L.m_colls[iSubL]) {

	const auto& slice = prim.second;

	float r1 = slice.m_minCoord;
	float r2 = slice.m_maxCoord;
	
	if(r_test_p < r1) break;
	if(r_test_m > r2) continue;

	if (! (r_test_p < r1 || r_test_m > r2)) {
	  
	  for(const auto& de : slice.m_vDE) {

	    float f1 = de.m_minBound;
	    float f2 = de.m_maxBound;
	    
	    if(f2 - f1 < M_PI) {
	      if(phi_max < f1) continue;
	      if(phi_min > f2) continue;
	    }
	    else {// +/- pi boundary
	      if(phi_test < 0 && phi_min > f1) continue;
	      if(phi_test > 0 && phi_max < f2) continue;
	    }
	    vIDs.push_back(de.m_hash);
	  }
	}
      }      
    }
  }
  
}



    
int TrigInDetRoadPredictorTool::getRoad(const std::vector<const Trk::SpacePoint*>& seed,
					std::vector<const InDetDD::SiDetectorElement*>& road,
					const EventContext& ctx) const {

  
  const float MAX_R         = 1030.0;//detector envelope
  const float MAX_Z         = 3000.0;//detector envelope
  const float maxCornerDist = 15.0;
  
  //1. get magnetic field

  MagField::AtlasFieldCache fieldCache;

  SG::ReadCondHandle<AtlasFieldCacheCondObj> fieldCondObj{m_fieldCondObjInputKey, ctx};
  if (!fieldCondObj.isValid()) {
    ATH_MSG_ERROR("Failed to retrieve AtlasFieldCacheCondObj with key " << m_fieldCondObjInputKey.key());
    return -1;
  }

  fieldCondObj->getInitializedCache (fieldCache);

  road.clear();

  unsigned int nSP = seed.size();
 
  if(nSP < 3) return -2;

  std::vector<unsigned int> seedHashes;
  
  for(unsigned int spIdx=0;spIdx<nSP;spIdx++) {
    const auto& sp = seed.at(spIdx);
    const Trk::PrepRawData* prd  = sp->clusterList().first;
    const InDet::PixelCluster* pPixelHit = dynamic_cast<const InDet::PixelCluster*>(prd);
    unsigned int hash = pPixelHit->detectorElement()->identifyHash();
    seedHashes.push_back(hash);
  }
  
  std::vector<std::array<float,2> > zr;
  zr.resize(nSP+2);

  for(unsigned int spIdx=0;spIdx<nSP;spIdx++) {
    const auto& sp = seed.at(spIdx);
    zr[spIdx+1][0] = sp->globalPosition().z();
    zr[spIdx+1][1] = sp->globalPosition().perp();
  }

  //adding the first point at beamline

  zr[0][0] = zr[1][0] - zr[1][1]*(zr[2][0]-zr[1][0])/(zr[2][1]-zr[1][1]);
  zr[0][1] = 0;

  //adding the last point at detector exit
  float zlast = zr[nSP-1][0] + (MAX_R - zr[nSP-1][1])*(zr[nSP][0]-zr[nSP-1][0])/(zr[nSP][1]-zr[nSP-1][1]);
  float rlast = MAX_R;

  if (std::fabs(zlast) > MAX_Z) {
    if(zlast > 0) zlast = MAX_Z;
    else zlast = -MAX_Z;
    rlast = zr[nSP-1][1] + (zlast - zr[nSP-1][0])*(zr[nSP][1]-zr[nSP-1][1])/(zr[nSP][0]-zr[nSP-1][0]);
  }
  zr[nSP+1][0] = zlast;
  zr[nSP+1][1] = rlast;
  
  std::map<unsigned int, SearchInterval> rzIntervals;

  for(unsigned int k1 = 0;k1<zr.size()-1;k1++) {//loop over trajectory segments

    unsigned int k2 = k1+1;

    float z1 = zr[k1][0];
    float r1 = zr[k1][1];
    float z2 = zr[k2][0];
    float r2 = zr[k2][1];
    float dz21 = z2-z1;
    float dr21 = r2-r1;
    float L = std::sqrt(dz21*dz21 + dr21*dr21);
    float invL = 1.0/L;
    float sinF = dr21*invL;
    float cosF = dz21*invL;
    
    for(const auto& vb : m_vBoundaries) {

      if (z1 < vb.m_zr[0] && z2 < vb.m_zr[0]) continue;
      if (z1 > vb.m_zr[1] && z2 > vb.m_zr[1]) continue;
      if (r1 < vb.m_zr[2] && r2 < vb.m_zr[2]) continue;
      if (r1 > vb.m_zr[3] && r2 > vb.m_zr[3]) continue;

      //corners:
      
      float zc[4] = {vb.m_zr[0], vb.m_zr[1], vb.m_zr[1], vb.m_zr[0]};
      float rc[4] = {vb.m_zr[2], vb.m_zr[2], vb.m_zr[3], vb.m_zr[3]};

      int nUp(0), nDn(0);

      float minDistances[4];
      
      for (int ic=0;ic<4;ic++) {
	minDistances[ic] = (rc[ic] - r1)*cosF - (zc[ic] - z1)*sinF;
      }

      float minH = std::abs(minDistances[0]);
      
      for (int ic=0;ic<4;ic++) {
	float h = minDistances[ic];
	if (h <=0) nDn += 1;
	else nUp += 1;
	if(std::abs(h) < minH) minH = std::abs(h);
      }

      if (nUp == 4 || nDn == 4) {
	if(minH > maxCornerDist) {//the closest corner is still too far
	  continue;
	}
      }
      
      //search through layers inside the volume

      for(auto lIdx : vb.m_layers) {
	const auto& lb = m_lBoundaries.at(lIdx);
	for(int s1=0;s1<lb.m_nVertices-1;s1++) {
	  int s2 = s1 + 1;
	  
	  float h1 = (lb.m_r[s1] - r1)*cosF - (lb.m_z[s1] - z1)*sinF;
	  float h2 = (lb.m_r[s2] - r1)*cosF - (lb.m_z[s2] - z1)*sinF;
	  if (h1*h2 > 0) continue;
	  float l1 = (lb.m_z[s1] - z1)*cosF + (lb.m_r[s1] - r1)*sinF;
	  float l2 = (lb.m_z[s2] - z1)*cosF + (lb.m_r[s2] - r1)*sinF;
	  if (l1 < 0 && l2 < 0) continue;
	  if (l1 > L && l2 > L) continue;
	  float lx = (l1 + (l2-l1)*h1/(h1-h2))*invL;
	  
	  if (lx < 0 || lx > 1) continue;
	  float zx = z2*lx + (1-lx)*z1;
	  float rx = r2*lx + (1-lx)*r1;
	  
	  auto radItr = rzIntervals.find(lb.m_lay_id);
	  
	  if(radItr == rzIntervals.end()) {
	    rzIntervals.insert(std::make_pair(lb.m_lay_id, SearchInterval(zx, rx)));
	  }
	  else {
	    (*radItr).second.addPoint(zx, rx);
	  }
	}
      }
    }
  }

  //3. parabolic extrapolation in r-phi

  unsigned int sp1Idx = 0;
  unsigned int sp3Idx = nSP-1;
  unsigned int sp2Idx = (sp3Idx+sp1Idx)/2;
  
  //3a. Converting XY coords to UV coords

  float uv_coords[2][2];

  float dx = seed.at(sp3Idx)->globalPosition().x() - seed.at(sp2Idx)->globalPosition().x();
  float dy = seed.at(sp3Idx)->globalPosition().y() - seed.at(sp2Idx)->globalPosition().y();
    
  uv_coords[1][0] = -std::sqrt(dx*dx + dy*dy);
  uv_coords[1][1] = 0.0;

  float cos_theta = dx/(-uv_coords[1][0]);
  float sin_theta = dy/(-uv_coords[1][0]);

  float rot_matrix[2][2];
  float inv_rot_matrix[2][2];

  rot_matrix[0][0] = cos_theta;
  rot_matrix[0][1] = sin_theta;
  rot_matrix[1][0] = -sin_theta;
  rot_matrix[1][1] = cos_theta;

  inv_rot_matrix[0][0] = cos_theta;
  inv_rot_matrix[0][1] = -sin_theta;
  inv_rot_matrix[1][0] = sin_theta;
  inv_rot_matrix[1][1] = cos_theta;

  float sp3_coords[3];
  sp3_coords[0] = seed.at(sp3Idx)->globalPosition().x();
  sp3_coords[1] = seed.at(sp3Idx)->globalPosition().y();
  sp3_coords[2] = seed.at(sp3Idx)->globalPosition().z();

  //UV-coordinates of the XY origin (0,0)

  float u_c = rot_matrix[0][0]*(0-sp3_coords[0]) + rot_matrix[0][1]*(0-sp3_coords[1]);
  float v_c = rot_matrix[1][0]*(0-sp3_coords[0]) + rot_matrix[1][1]*(0-sp3_coords[1]);

  int sign_up = u_c < 0 ? 1 : -1;
    
  //transforming SP1

  float dR1[2];

  dR1[0] = seed.at(sp1Idx)->globalPosition().x() - sp3_coords[0];
  dR1[1] = seed.at(sp1Idx)->globalPosition().y() - sp3_coords[1];

  uv_coords[0][0] = rot_matrix[0][0]*dR1[0] + rot_matrix[0][1]*dR1[1];
  uv_coords[0][1] = rot_matrix[1][0]*dR1[0] + rot_matrix[1][1]*dR1[1];

  //3b. parameters of the parabola in the u-v c.s.

  float a = uv_coords[0][1]/(uv_coords[0][0]*(uv_coords[0][0]-uv_coords[1][0]));
  float b = -a*uv_coords[1][0]; // b = -a*x2

  //3c. calculate impact point radii

  std::vector<unsigned int> pixelHashIds;
  std::vector<unsigned int> stripHashIds;
  
  for(auto& ip : rzIntervals) {

    float R = ip.second.getAverageRadius();

    float R2 = R*R;

    float dRv = R2-v_c*v_c;

    if(dRv < 0) continue;//no intersection with the circle

    float u_p = u_c + sign_up*std::sqrt(dRv);

    float v_p = b*u_p + a*u_p*u_p;
    float dv2 = (v_p-v_c)*(v_p-v_c);

    if (R2-dv2<0) continue; // check for intersection between v=v_p and the circle
    
    float u_star = u_c + sign_up*std::sqrt(R2-dv2);
    float v_star = b*u_star + a*u_star*u_star;

    float x_star = inv_rot_matrix[0][0]*u_star + inv_rot_matrix[0][1]*v_star + sp3_coords[0];
    float y_star = inv_rot_matrix[1][0]*u_star + inv_rot_matrix[1][1]*v_star + sp3_coords[1];
    
    ip.second.m_r   = std::sqrt(x_star*x_star + y_star*y_star);
    ip.second.m_phi = std::atan2(y_star, x_star);
    
    if(ip.first > 15000) {//assuming Pixel-only seeds

      bool hasHit = false;

      for(unsigned int i=1;i<nSP-1;i++) {

	float dpr = ip.second.m_r - zr[i][1];
	float dpz = ip.second.m_z - zr[i][0];
	float dist = std::sqrt(dpr*dpr + dpz*dpz);
	if(dist < maxCornerDist) {
	  hasHit = true;
	  break;
	}
      }
      findDetectorElements(ip.first, ip.second, pixelHashIds, hasHit);      
    }
    else {
      findDetectorElements(ip.first, ip.second, stripHashIds, false);
    }
  }

  std::set<unsigned int> pixelHashSet(pixelHashIds.begin(), pixelHashIds.end());

  for(auto id : seedHashes) {
    if(pixelHashSet.find(id) == pixelHashSet.end()) pixelHashIds.push_back(id);
  }

  std::vector<std::pair<float, const InDetDD::SiDetectorElement*> > theRoad;

  for(auto hash_id : pixelHashIds) {
    const InDetDD::SiDetectorElement *p = m_pixelManager->getDetectorElement(hash_id);
    if(p == nullptr) continue;
    const Amg::Vector3D& C = p->center();
    float dist = std::sqrt(C(0)*C(0) + C(1)*C(1) + C(2)*C(2));
    theRoad.push_back(std::make_pair(dist,p));
  }
  
  for(auto hash_id : stripHashIds) {
    const InDetDD::SiDetectorElement *p = m_stripManager->getDetectorElement(hash_id);
    if(p == nullptr) continue;
    const Amg::Vector3D& C = p->center();
    float dist = std::sqrt(C(0)*C(0) + C(1)*C(1) + C(2)*C(2));
    theRoad.push_back(std::make_pair(dist,p));
  }

  std::sort(theRoad.begin(), theRoad.end());

  for(const auto & dp : theRoad) {
    road.push_back(dp.second);
  }
  
  return (int)theRoad.size();
}
