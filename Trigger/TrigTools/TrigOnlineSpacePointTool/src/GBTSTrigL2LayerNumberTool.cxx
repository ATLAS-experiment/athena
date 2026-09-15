/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "InDetIdentifier/SCT_ID.h"
#include "InDetIdentifier/PixelID.h" 
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "PixelReadoutGeometry/PixelDetectorManager.h"
#include "SCT_ReadoutGeometry/SCT_DetectorManager.h"

#include "GBTSTrigL2LayerNumberTool.h"
#include "InDetReadoutGeometry/SiNumerology.h"
#include <stdexcept>

GBTSTrigL2LayerNumberTool::GBTSTrigL2LayerNumberTool(const std::string& t, 
					     const std::string& n,
					     const IInterface*  p ): AthAlgTool(t,n,p),
								     m_useNewScheme(false),
								     m_MaxSiliconLayerNum(-1),
								     m_OffsetEndcapPixels(-1),
								     m_OffsetBarrelSCT(-1),
								     m_OffsetEndcapSCT(-1)
{
  declareInterface< ITrigL2LayerNumberTool >( this );
  declareProperty( "UseNewLayerScheme",      m_useNewScheme = false );
}

StatusCode GBTSTrigL2LayerNumberTool::initialize() {

  StatusCode sc = AthAlgTool::initialize();

  ATH_MSG_DEBUG("In initialize...");

  sc = detStore()->retrieve(m_pixelId, "PixelID");
  if (sc.isFailure()) {
    ATH_MSG_FATAL("Could not get Pixel ID helper"); 
    return sc;
  } 

  sc = detStore()->retrieve(m_sctId, "SCT_ID");
  if (sc.isFailure()) {
    ATH_MSG_FATAL("Could not get SCT ID helper"); 
    return sc;
  } 
 
  sc = detStore()->retrieve(m_pixelManager);  
  if( sc.isFailure() ) {
    ATH_MSG_ERROR("Could not retrieve Pixel DetectorManager from detStore."); 
    return sc;
  } 

  sc = detStore()->retrieve(m_sctManager);
  if( sc.isFailure() ) {
    ATH_MSG_ERROR("Could not retrieve SCT DetectorManager from detStore.");
    return sc;
  } 

  //calculate the numbers

  if(!m_useNewScheme) {

    const InDetDD::SiNumerology& pixSiNum = m_pixelManager->numerology(); 
    const InDetDD::SiNumerology& sctSiNum = m_sctManager->numerology(); 

    m_MaxSiliconLayerNum = pixSiNum.numLayers()+pixSiNum.numDisks()+sctSiNum.numLayers()+sctSiNum.numDisks();
    m_OffsetBarrelSCT = pixSiNum.numLayers();
    m_OffsetEndcapPixels = pixSiNum.numLayers()+sctSiNum.numLayers();
    m_LastBarrelLayer = m_OffsetEndcapPixels - 1;
    m_OffsetEndcapSCT = pixSiNum.numLayers()+sctSiNum.numLayers()+pixSiNum.numDisks();
  }
  else {
    createModuleHashMap(m_hashMap);

    ATH_MSG_DEBUG("Total number of unique silicon layers = "<<m_hashMap.size());

    m_MaxSiliconLayerNum = (int)m_hashMap.size();
    m_OffsetEndcapPixels = m_LastBarrelLayer;
  }

  
  ATH_MSG_DEBUG("GBTSTrigL2LayerNumberTool initialized ");

  report();

  return sc;
}

StatusCode GBTSTrigL2LayerNumberTool::finalize()
{
  StatusCode sc = AthAlgTool::finalize(); 
  return sc;
}

void GBTSTrigL2LayerNumberTool::report() const {

  ATH_MSG_DEBUG("TrigL2 Layer numbering scheme:");
  ATH_MSG_DEBUG("Total number of layers = "<<maxSiliconLayerNum());
  ATH_MSG_DEBUG("OffsetEndcapPixels     = "<<offsetEndcapPixels());
  ATH_MSG_DEBUG("OffsetBarrelSCT        = "<<offsetBarrelSCT());
  ATH_MSG_DEBUG("OffsetEndcapSCT        = "<<offsetEndcapSCT());
}

void GBTSTrigL2LayerNumberTool::createModuleHashMap(std::map<std::tuple<int,int,short,short>,std::vector<GBTSPhiEtaHash> >& hashMap) {


  short subdetid = 1;
  
  for(int hash = 0; hash<static_cast<int>(m_pixelId->wafer_hash_max()); hash++) {

    Identifier offlineId = m_pixelId->wafer_id(hash);

    if(offlineId==0) continue;
 
    short barrel_ec = m_pixelId->barrel_ec(offlineId);
    if(std::abs(barrel_ec)>2) {std::cout << "barrel " << " " << hash << std::endl; continue;}//no DBM needed
    
    short phi_index = m_pixelId->phi_module(offlineId);
    short eta_index = m_pixelId->eta_module(offlineId);
    int lay_id = m_pixelId->layer_disk(offlineId);
    int eta_mod = m_pixelId->eta_module(offlineId);

    int vol_id = -1;
    if(barrel_ec== 0) vol_id = 8;
    if(barrel_ec==-2) vol_id = 7;
    if(barrel_ec== 2) vol_id = 9;
    
    int new_vol=0, new_lay=0;

    if(vol_id == 7 || vol_id == 9) {
      new_vol = 10*vol_id + lay_id;
      new_lay = eta_mod;
    }
    else if(vol_id == 8) {
      new_lay = 0;
      new_vol = 10*vol_id + lay_id;
    }

    auto t = std::make_tuple(barrel_ec==0 ? -100 : barrel_ec, subdetid, new_vol, new_lay);

    std::map<std::tuple<int, int, short, short>,std::vector<GBTSPhiEtaHash> >::iterator it = hashMap.find(t);
    if(it==hashMap.end())
      hashMap.insert(std::pair<std::tuple<int, int, short, short>,std::vector<GBTSPhiEtaHash> >(t,std::vector<GBTSPhiEtaHash>(1, GBTSPhiEtaHash(phi_index, eta_index, hash) )));
    else (*it).second.push_back(GBTSPhiEtaHash(phi_index, eta_index, hash));
  }

  if (false) {
    subdetid = 2; //SCT

  for(int hash = 0; hash<static_cast<int>(m_sctId->wafer_hash_max()); hash++) {

    Identifier offlineId = m_sctId->wafer_id(hash);

    if(offlineId==0) continue;
 
    short barrel_ec = m_sctId->barrel_ec(offlineId);
    short phi_index = m_sctId->phi_module(offlineId);
    short eta_index = m_sctId->eta_module(offlineId);

    int vol_id = 13;
    
    if(barrel_ec) vol_id = 12;
    if(barrel_ec>0) vol_id = 14;

    int lay_id = m_sctId->layer_disk(offlineId);

    auto t = std::make_tuple(barrel_ec==0 ? -100 : barrel_ec, subdetid, vol_id, lay_id); 

    std::map<std::tuple<int, int, short, short>,std::vector<GBTSPhiEtaHash> >::iterator it = hashMap.find(t);
    if(it==hashMap.end())
      hashMap.insert(std::pair<std::tuple<int, int, short, short>,std::vector<GBTSPhiEtaHash> >(t,std::vector<GBTSPhiEtaHash>(1, GBTSPhiEtaHash(phi_index, eta_index, hash))));
    else (*it).second.push_back(GBTSPhiEtaHash(phi_index, eta_index, hash));
  }
  }


  m_pixelLayers.clear();
  m_sctLayers.clear();

  m_pixelLayers.resize(m_pixelId->wafer_hash_max(), -100);
  m_sctLayers.resize(m_sctId->wafer_hash_max(), -100);
  m_layerGeometry.resize(hashMap.size());

  int layerId=0;
  m_LastBarrelLayer=0;
  for(std::map<std::tuple<int,int,short,short>,std::vector<GBTSPhiEtaHash> >::iterator it = hashMap.begin();it!=hashMap.end();++it, layerId++) {

    short vol_id = std::get<2>((*it).first);
    short lay_id = std::get<3>((*it).first);

    int combinedId = static_cast<int>(vol_id)*1000 + lay_id;
    
    short subdetId = std::get<1>((*it).first);
    short barrel_ec = std::get<0>((*it).first);

    if(barrel_ec==-100) barrel_ec = 0;

    if(barrel_ec == 0) m_LastBarrelLayer++;

    m_layerGeometry[layerId].m_type = barrel_ec;
    m_layerGeometry[layerId].m_subdet = combinedId;


    float rc=0.0;
    float minBound = 100000.0;
    float maxBound =-100000.0;
    int nModules = 0;

    for(std::vector<GBTSPhiEtaHash>::iterator hIt = (*it).second.begin();hIt != (*it).second.end();++hIt) {
   
      const InDetDD::SiDetectorElement *p{nullptr};

      if(subdetId == 1) {//pixel
	      m_pixelLayers[(*hIt).m_hash] = layerId;
	      p = m_pixelManager->getDetectorElement((*hIt).m_hash);
      }
      if(subdetId == 2) {//SCT
	      m_sctLayers[(*hIt).m_hash] = layerId;
	      p = m_sctManager->getDetectorElement((*hIt).m_hash);
      }
    
      if (p==nullptr) {
        ATH_MSG_ERROR("nullptr SiDetectorElement with idHash " << (*hIt).m_hash);
        continue;
      }
      const Amg::Vector3D& C = p->center();
      if(barrel_ec == 0) {
	      rc += sqrt(C(0)*C(0)+C(1)*C(1));
	      if(p->zMin() < minBound) minBound = p->zMin();
	      if(p->zMax() > maxBound) maxBound = p->zMax();
      }
      else {
	      rc += C(2);
	      if(p->rMin() < minBound) minBound = p->rMin();
	      if(p->rMax() > maxBound) maxBound = p->rMax();	
      }
      nModules++;
    }
    if (nModules == 0)[[unlikely]]{
      throw std::runtime_error("GBTSTrigL2LayerNumberTool::createModuleHashMap: nModules is zero.");
    }
    m_layerGeometry[layerId].m_refCoord = rc/nModules;
    m_layerGeometry[layerId].m_minBound = minBound;
    m_layerGeometry[layerId].m_maxBound = maxBound;
  }

  int M = (layerId-m_LastBarrelLayer)/2;

  ATH_MSG_DEBUG("List of unique layers in Pixel and SCT :");
  for(int l=0;l<layerId;l++) {

    int oldL = l;

    if(l>m_LastBarrelLayer-1) {
      oldL = l-m_LastBarrelLayer;
      oldL = oldL < M ? oldL : oldL - M;
      oldL += m_LastBarrelLayer;
    }

    if(m_layerGeometry[l].m_subdet==1) {
      ATH_MSG_DEBUG("Layer "<<l<<" ("<<oldL<<") : PIX, reference coordinate ="<< m_layerGeometry[l].m_refCoord<<" boundaries: "<<m_layerGeometry[l].m_minBound<<
		   " "<<m_layerGeometry[l].m_maxBound);
    }
    if(m_layerGeometry[l].m_subdet==2) {
      ATH_MSG_DEBUG("Layer "<<l<<" ("<<oldL<<") : SCT, reference coordinate ="<< m_layerGeometry[l].m_refCoord<<" boundaries: "<<m_layerGeometry[l].m_minBound<<
		   " "<<m_layerGeometry[l].m_maxBound);
    }
  }
}
