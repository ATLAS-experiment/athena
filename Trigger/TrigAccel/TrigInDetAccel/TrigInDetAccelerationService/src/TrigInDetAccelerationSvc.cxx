/*   
     Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration 
*/

#include "InDetIdentifier/SCT_ID.h"
#include "InDetIdentifier/PixelID.h" 

#include "SCT_ReadoutGeometry/SCT_DetectorManager.h"
#include "PixelReadoutGeometry/PixelDetectorManager.h"
#include "InDetReadoutGeometry/SiNumerology.h" 
#include "InDetReadoutGeometry/SiDetectorElement.h"

#include "TrigInDetAccelerationSvc.h"
#include "TrigAccelEvent/TrigInDetAccelCodes.h"
#include "TrigAccelEvent/TrigInDetAccelEDM.h"
#include "TrigAccelEvent/TrigITkAccelEDM.h"
#include "CxxUtils/checker_macros.h"
#include "AthenaInterprocess/Incidents.h"
#include "GaudiKernel/ConcurrencyFlags.h"

#include <dlfcn.h>

////////// 
/// Constructor 
///// 

TrigInDetAccelerationSvc::TrigInDetAccelerationSvc( const std::string& name, ISvcLocator* pSvcLocator) : 
  base_class( name, pSvcLocator ),
  m_nDCs(12),
  m_moduleName("libTrigInDetCUDA.so"),
  m_useITkGeometry(false),
  m_libHandle(0),
  m_pWF(0),
  m_module(0),
  m_detStore("DetectorStore", name),
  m_evtStore("StoreGateSvc",name), 
  m_factoryConfigured(false) {

  declareProperty( "NumberOfDCs", m_nDCs = 8 );
  declareProperty( "ModuleName", m_moduleName = "libTrigInDetCUDA.so");
  declareProperty( "useITkGeometry", m_useITkGeometry = false);
  declareProperty( "MiddleSpacePointLayers", m_middleSpacePointLayers = std::vector<int>(), "Global IDs of layers that can contain middle spacepoints of track seeds" );
} 


////////// 
/// Initialize 
///// 

StatusCode TrigInDetAccelerationSvc::initialize() {   

  ATH_MSG_INFO("TrigInDetAccelerationSvc: initialize");
  
  ATH_CHECK (m_evtStore.retrieve() );
  ATH_CHECK (m_detStore.retrieve() );

  //load the OffloadFactory library

  m_libHandle = dlopen(m_moduleName.c_str(), RTLD_LAZY);
  
  if(!m_libHandle) {
    ATH_MSG_INFO("TrigInDetAccelerationSvc: cannot load the factory library, error:"<<dlerror());
    return StatusCode::SUCCESS;
  }

  dlerror();

  //declare library interface methods 
  TrigAccel::Module* (*getModule)();

  getModule = (TrigAccel::Module* (*)()) dlsym(m_libHandle, "getModule");

  m_module = getModule();

  int factory_id_to_load = 0;

  if(m_useITkGeometry){
    factory_id_to_load = TrigAccel::TrigITkModuleID_CUDA;
  }else{
    factory_id_to_load = TrigAccel::TrigInDetModuleID_CUDA;
  }

  m_pWF = m_module->getFactoryById(factory_id_to_load);

  if(m_pWF == nullptr){
    ATH_MSG_INFO("OffloadFactory with id "<<std::hex<<factory_id_to_load<<" not available from the module");
    m_factoryConfigured = false;
    return StatusCode::SUCCESS;
  }
  
  bool cfgResult = m_pWF->configure();

  if(!cfgResult) {
    ATH_MSG_INFO("OffloadFactory config failed");
    m_factoryConfigured = false;
    return StatusCode::SUCCESS;
  }

  m_factoryConfigured = true;
  
  for(int i=0;i<3;i++) m_layerInfo[i].clear();

  ATH_MSG_INFO("TrigInDetAccelerationSvc: created OffloadFactory, factory id = "<<std::hex<<m_pWF->getFactoryId()<<std::dec);
  
  /*    
   * Ask to be informed at the beginning of a new run so that we    
   * can collect geometry, conditions, etc. and copy them to on-GPU data structures   
   * For athenaHLT this should be UpdateAfterFork
   */   

  SmartIF<IIncidentSvc> incsvc{service("IncidentSvc")};
  const int priority = 100;
  if( incsvc ) {
    const bool is_multiprocess = (Gaudi::Concurrency::ConcurrencyFlags::numProcs() > 0);
    if (is_multiprocess) {
      incsvc->addListener( this, AthenaInterprocess::UpdateAfterFork::type(), priority);
    }
    else {
      incsvc->addListener( this, "BeginRun", priority);
    }
  }
  
  return StatusCode::SUCCESS;
} 

///////// 
/// Finalize 
///// 
StatusCode TrigInDetAccelerationSvc::finalize() {

  delete m_pWF;

  dlclose(m_libHandle);

  return StatusCode::SUCCESS; 
}


/////////  
/// OnBeginRun data gathering and export 
/////

void TrigInDetAccelerationSvc::handle(const Incident&) {    

  ATH_MSG_INFO("OnBeginRun ");

  if (m_useITkGeometry) {
    // barrel ec, subdetector id, itk volume id, itk layer disk id
    std::map<std::tuple<short,short, int, int>,std::vector<PhiEtaHash> > hashMap;
    if(!extractITkGeometryInformation(hashMap)) {
      ATH_MSG_INFO("ITk Geometry extraction failed");
      return;
    }
    
    if(!exportITkGeometryInformation(hashMap)) {
      ATH_MSG_INFO("ITk Geometry export failed");
    } 

  } else {
    // subdetector id, barrel ec, layer disk
    std::map<std::tuple<short,short,short>,std::vector<PhiEtaHash> > hashMap;
    
    if(!extractGeometryInformation(hashMap)) {
      ATH_MSG_INFO("Geometry extraction failed");
      return;
    }
    
    if(!exportGeometryInformation(hashMap)) {
      ATH_MSG_INFO("Geometry export failed");
    }

  }

  return; 
}

TrigAccel::Work* TrigInDetAccelerationSvc::createWork(unsigned int jobCode, std::shared_ptr<TrigAccel::OffloadBuffer> pB) const {
  
  if(!m_factoryConfigured) return 0;

  // To remove this, WorkFactory::createWork() should be made const
  // (and thread-safe).
  std::scoped_lock lock (m_workMutex);
  TrigAccel::WorkFactory* wf ATLAS_THREAD_SAFE = m_pWF;
  return wf->createWork(jobCode, pB);

}


const std::vector<short>& TrigInDetAccelerationSvc::getLayerInformation(int i) const {
  if (i<0 || i>2) i = 0;
  return m_layerInfo[i];
}

bool TrigInDetAccelerationSvc::exportITkGeometryInformation(const std::map<std::tuple<short,short, int, int>,std::vector<PhiEtaHash> >& hashMap) const {
  
  const InDetDD::PixelDetectorManager * pix_mgr = nullptr;

  if (m_detStore->retrieve(pix_mgr,"ITkPixel").isFailure()) {
    ATH_MSG_WARNING("failed to get ITkPixel Manager");
    return false;
  } 

  //export layer structure

  size_t dataTypeSize = sizeof(TrigAccel::ITk::DETECTOR_MODEL);
  const size_t bufferOffset = 256; 
  size_t totalSize = bufferOffset + dataTypeSize;

  TrigAccel::DATA_EXPORT_BUFFER* pBG = new TrigAccel::DATA_EXPORT_BUFFER(5000);
  
  if(!pBG->fit(totalSize)) pBG->reallocate(totalSize);

  TrigAccel::ITk::DETECTOR_MODEL* pArray = reinterpret_cast<TrigAccel::ITk::DETECTOR_MODEL*>(pBG->m_buffer + bufferOffset);

  // cppcheck-suppress memsetClassFloat; deliberate
  memset(pArray,0,dataTypeSize);
  
  int nLayers = (int)hashMap.size();
  pArray->m_nLayers = nLayers;
  pArray->m_nModules=0;
  
  int layerIdx=0;
  int middleLayerIdx=0;

  for(std::map<std::tuple<short,short, int, int>,std::vector<PhiEtaHash> >::const_iterator it = hashMap.begin();it!=hashMap.end();++it, layerIdx++) {
    
    short barrel_ec = std::get<0>((*it).first);
    int vol_id = std::get<2>((*it).first);
    int lay_id = std::get<3>((*it).first);
    if(barrel_ec == -100) barrel_ec = 0;
    int globalLayerId = vol_id*1000 + lay_id;

    pArray->m_layers[layerIdx].m_nElements = 0;
    pArray->m_layers[layerIdx].m_subdet = globalLayerId;
    pArray->m_layers[layerIdx].m_type = barrel_ec;

    // Fill in a table of layer ids that can contain the middle SPs
    if (std::find(m_middleSpacePointLayers.begin(), m_middleSpacePointLayers.end(), globalLayerId) != m_middleSpacePointLayers.end()){
      pArray->m_middleSpacePointLayers[middleLayerIdx] = layerIdx;
      ++middleLayerIdx;
    }
    
    std::vector<std::vector<PhiEtaHash>::const_iterator> vStops;
    vStops.push_back((*it).second.begin());
    std::vector<PhiEtaHash>::const_iterator firstIt = (*it).second.begin();
    std::vector<PhiEtaHash>::const_iterator nextIt = (*it).second.begin();
    ++nextIt;

    int nPhiSlices=0;

    for(; nextIt!=(*it).second.end();++nextIt, ++firstIt) {
      if((*nextIt).m_phiIndex!=(*firstIt).m_phiIndex) {
        vStops.push_back(nextIt);
        nPhiSlices++;
      }
    }
    
    nPhiSlices++;
    
    vStops.push_back((*it).second.end());

    float rc=0.0;
    float minBound = 100000.0;
    float maxBound =-100000.0;

    pArray->m_layers[layerIdx].m_nPhiSlices = nPhiSlices;
    
    //3. iterating over phi sectors

    for(unsigned int iStops = 1; iStops<vStops.size();iStops++) {
      int nPhiModules = 0;
      bool first = true;

      for(std::vector<PhiEtaHash>::const_iterator hIt = vStops[iStops-1];hIt!=vStops[iStops];++hIt, nPhiModules++) {

        pArray->m_hashArray[pArray->m_nModules] = (*hIt).m_hash;
        const InDetDD::SiDetectorElement *p = pix_mgr->getDetectorElement((*hIt).m_hash);

        if (first) {
          first = false;
        }

        pArray->m_layers[layerIdx].m_nElements++;

        const Amg::Vector3D& C = p->center();
        if (barrel_ec == 0) {
          rc += sqrt(C(0)*C(0)+C(1)*C(1));
          if(p->zMin() < minBound) minBound = p->zMin();
          if(p->zMax() > maxBound) maxBound = p->zMax();
          pArray->m_minRZ[pArray->m_nModules] = p->zMin();
          pArray->m_maxRZ[pArray->m_nModules] = p->zMax();
        } else {
          rc += C(2);
          if(p->rMin() < minBound) minBound = p->rMin();
          if(p->rMax() > maxBound) maxBound = p->rMax();        
          pArray->m_minRZ[pArray->m_nModules] = p->rMin();
          pArray->m_maxRZ[pArray->m_nModules] = p->rMax();
        }

	      pArray->m_nModules++;
      }

    }
    pArray->m_layers[layerIdx].m_refCoord = rc/pArray->m_layers[layerIdx].m_nElements;
    pArray->m_layers[layerIdx].m_minBound = minBound;
    pArray->m_layers[layerIdx].m_maxBound = maxBound;
  }

  std::shared_ptr<TrigAccel::OffloadBuffer> pDMBuff = std::make_shared<TrigAccel::OffloadBuffer>(pBG);

  delete pBG;

  ATH_MSG_INFO("Creating Work item for task "<<TrigAccel::InDetJobControlCode::SIL_LAYERS_EXPORT);

  TrigAccel::Work* pW = createWork(TrigAccel::InDetJobControlCode::SIL_LAYERS_EXPORT, pDMBuff);

  return pW == 0;//request is actioned immediately, no actual WorkItem is created

}


bool TrigInDetAccelerationSvc::extractITkGeometryInformation(std::map<std::tuple<short,short,int,int>, std::vector<PhiEtaHash> >& hashMap) {

  const PixelID* pixelId = nullptr;

  if (m_detStore->retrieve(pixelId, "PixelID").isFailure()) {
    ATH_MSG_WARNING("Could not get Pixel ID helper");
    return false;
  }

  // Read Pixel layer details
  short subdetid = 1;
  for(int hash = 0; hash<(int)pixelId->wafer_hash_max(); hash++) {

    Identifier offlineId = pixelId->wafer_id(hash);

    if(offlineId==0) continue;
 
    short barrel_ec = pixelId->barrel_ec(offlineId);
    if(abs(barrel_ec)>2) continue;//no DBM needed

    short phi_index = pixelId->phi_module(offlineId);
    short eta_index = pixelId->eta_module(offlineId);
    short layer_disk = pixelId->layer_disk(offlineId);

    // Calculate new volume and layer id for ITk
    int vol_id = -1;
    if (barrel_ec== 0) vol_id = 8;
    if (barrel_ec==-2) vol_id = 7;
    if (barrel_ec== 2) vol_id = 9;
    
    int new_vol=0, new_lay=0;

    if (vol_id == 7 || vol_id == 9) {
      new_vol = 10*vol_id + layer_disk;
      new_lay = eta_index;
    } else if (vol_id == 8) {
      new_lay = 0;
      new_vol = 10*vol_id + layer_disk;
    }
    
    auto t = std::make_tuple(barrel_ec==0 ? -100 : barrel_ec, subdetid, new_vol, new_lay);
    std::map<std::tuple<short,short,int,int>,std::vector<PhiEtaHash> >::iterator it = hashMap.find(t);
    if(it==hashMap.end())
      hashMap.insert(std::pair<std::tuple<short,short,int,int>,std::vector<PhiEtaHash> >(t,std::vector<PhiEtaHash>(1, PhiEtaHash(phi_index, eta_index, hash) )));
    else (*it).second.push_back(PhiEtaHash(phi_index, eta_index, hash));
  }

  m_layerInfo[0].clear();
  m_layerInfo[1].clear();

  m_layerInfo[0].resize(hashMap.size()); // Mapping from layerId to barrel_ec
  m_layerInfo[1].resize(pixelId->wafer_hash_max(), -1); // Mapping from layerId to the module hash

  // Map layer geometry details to module hash id
  int layerId=0;
  for(std::map<std::tuple<short,short,int,int>,std::vector<PhiEtaHash> >::iterator it = hashMap.begin();it!=hashMap.end();++it, layerId++) {

    short barrel_ec = std::get<0>((*it).first);
    short subdetId = std::get<1>((*it).first);
    if(barrel_ec == -100) barrel_ec = 0;

    m_layerInfo[0][layerId] = barrel_ec;

    if(subdetId == 1) {//pixel
      for(std::vector<PhiEtaHash>::iterator hIt = (*it).second.begin();hIt != (*it).second.end();++hIt) {
        m_layerInfo[subdetId][(*hIt).m_hash] = layerId;
      }
    }
  }
  
  for(std::map<std::tuple<short,short,int,int>,std::vector<PhiEtaHash> >::iterator it = hashMap.begin();it!=hashMap.end();++it) {

    //sorting along phi first, then along eta
    std::sort((*it).second.begin(), (*it).second.end(), PhiEtaHash::compare());

  }

  return true;
}


bool TrigInDetAccelerationSvc::exportGeometryInformation(const std::map<std::tuple<short,short,short>,std::vector<PhiEtaHash> >& hashMap) const {
  
  const InDetDD::SCT_DetectorManager *  sct_mgr = nullptr;
  const InDetDD::PixelDetectorManager * pix_mgr = nullptr;

  if (m_detStore->retrieve(sct_mgr, "SCT").isFailure()) {
    ATH_MSG_WARNING("failed to get SCT Manager");
    return false;
    
  } 
  if (m_detStore->retrieve(pix_mgr,"Pixel").isFailure()) {
    ATH_MSG_WARNING("failed to get SCT Manager");
    return false;
  } 

  //export layer structure

  size_t dataTypeSize = sizeof(TrigAccel::DETECTOR_MODEL);
  const size_t bufferOffset = 256; 
  size_t totalSize = bufferOffset + dataTypeSize;

  TrigAccel::DATA_EXPORT_BUFFER* pBG = new TrigAccel::DATA_EXPORT_BUFFER(5000);
  
  if(!pBG->fit(totalSize)) pBG->reallocate(totalSize);

  TrigAccel::DETECTOR_MODEL* pArray = reinterpret_cast<TrigAccel::DETECTOR_MODEL*>(pBG->m_buffer + bufferOffset);

  // cppcheck-suppress memsetClassFloat; deliberate
  memset(pArray,0,dataTypeSize);
  
  int nLayers = (int)hashMap.size();
  pArray->m_nLayers = nLayers;
  pArray->m_nModules=0;
  
  int layerIdx=0;

  for(std::map<std::tuple<short,short,short>,std::vector<PhiEtaHash> >::const_iterator it = hashMap.begin();it!=hashMap.end();++it, layerIdx++) {
    short subdetid = std::get<0>((*it).first);
    short barrel_ec = std::get<1>((*it).first);
    
    pArray->m_layers[layerIdx].m_nElements = 0;
    pArray->m_layers[layerIdx].m_subdet = subdetid;
    pArray->m_layers[layerIdx].m_type = barrel_ec;
    
    std::vector<std::vector<PhiEtaHash>::const_iterator> vStops;
    vStops.push_back((*it).second.begin());
    std::vector<PhiEtaHash>::const_iterator firstIt = (*it).second.begin();
    std::vector<PhiEtaHash>::const_iterator nextIt = (*it).second.begin();
    ++nextIt;

    int nPhiSlices=0;

    for(; nextIt!=(*it).second.end();++nextIt, ++firstIt) {
      if((*nextIt).m_phiIndex!=(*firstIt).m_phiIndex) {
        vStops.push_back(nextIt);
        nPhiSlices++;
      }
    }
    
    nPhiSlices++;
    
    vStops.push_back((*it).second.end());

    float rc=0.0;
    float minBound = 100000.0;
    float maxBound =-100000.0;

    pArray->m_layers[layerIdx].m_nPhiSlices = nPhiSlices;
    
    //3. iterating over phi sectors

    for(unsigned int iStops = 1; iStops<vStops.size();iStops++) {

      int nPhiModules = 0;

      bool first = true;

      for(std::vector<PhiEtaHash>::const_iterator hIt = vStops[iStops-1];hIt!=vStops[iStops];++hIt, nPhiModules++) {

        pArray->m_hashArray[pArray->m_nModules] = (*hIt).m_hash;

        const InDetDD::SiDetectorElement *p = (subdetid==1) ? pix_mgr->getDetectorElement((*hIt).m_hash) : sct_mgr->getDetectorElement((*hIt).m_hash);

        if(first) {
          first = false;
        }

        pArray->m_layers[layerIdx].m_nElements++;

        const Amg::Vector3D& C = p->center();
        if(barrel_ec == 0) {
          rc += sqrt(C(0)*C(0)+C(1)*C(1));
          if(p->zMin() < minBound) minBound = p->zMin();
          if(p->zMax() > maxBound) maxBound = p->zMax();
          pArray->m_minRZ[pArray->m_nModules] = p->zMin();
          pArray->m_maxRZ[pArray->m_nModules] = p->zMax();
        }
        else {
          rc += C(2);
          if(p->rMin() < minBound) minBound = p->rMin();
          if(p->rMax() > maxBound) maxBound = p->rMax();        
          pArray->m_minRZ[pArray->m_nModules] = p->rMin();
          pArray->m_maxRZ[pArray->m_nModules] = p->rMax();
        }

	pArray->m_nModules++;
      }

    }
    pArray->m_layers[layerIdx].m_refCoord = rc/pArray->m_layers[layerIdx].m_nElements;
    pArray->m_layers[layerIdx].m_minBound = minBound;
    pArray->m_layers[layerIdx].m_maxBound = maxBound;
  }

  std::shared_ptr<TrigAccel::OffloadBuffer> pDMBuff = std::make_shared<TrigAccel::OffloadBuffer>(pBG);

  delete pBG;

  ATH_MSG_INFO("Creating Work item for task "<<TrigAccel::InDetJobControlCode::SIL_LAYERS_EXPORT);

  TrigAccel::Work* pW = createWork(TrigAccel::InDetJobControlCode::SIL_LAYERS_EXPORT, pDMBuff);

  return pW == 0;//request is actioned immediately, no actual WorkItem is created

}


bool TrigInDetAccelerationSvc::extractGeometryInformation(std::map<std::tuple<short,short,short>, std::vector<PhiEtaHash> >& hashMap) {

  const PixelID* pixelId = nullptr;
  const SCT_ID* sctId = nullptr;

  if (m_detStore->retrieve(pixelId, "PixelID").isFailure()) {
    ATH_MSG_WARNING("Could not get Pixel ID helper");
    return false;
  }
  
  if (m_detStore->retrieve(sctId, "SCT_ID").isFailure()) { 
    ATH_MSG_WARNING("Could not get Pixel ID helper");
    return false;
  }
  
  short subdetid = 1;

  for(int hash = 0; hash<(int)pixelId->wafer_hash_max(); hash++) {

    Identifier offlineId = pixelId->wafer_id(hash);

    if(offlineId==0) continue;
 
    short barrel_ec = pixelId->barrel_ec(offlineId);
    if(abs(barrel_ec)>2) continue;//no DBM needed
    short layer_disk = pixelId->layer_disk(offlineId);
    short phi_index = pixelId->phi_module(offlineId);
    short eta_index = pixelId->eta_module(offlineId);
    auto t = std::make_tuple(subdetid, barrel_ec, layer_disk); 
    std::map<std::tuple<short,short,short>,std::vector<PhiEtaHash> >::iterator it = hashMap.find(t);
    if(it==hashMap.end())
      hashMap.insert(std::pair<std::tuple<short,short,short>,std::vector<PhiEtaHash> >(t,std::vector<PhiEtaHash>(1, PhiEtaHash(phi_index, eta_index, hash) )));
    else (*it).second.push_back(PhiEtaHash(phi_index, eta_index, hash));
  }
  subdetid = 2;
  for(int hash = 0; hash<(int)sctId->wafer_hash_max(); hash++) {

    Identifier offlineId = sctId->wafer_id(hash);

    if(offlineId==0) continue;
 
    short barrel_ec = sctId->barrel_ec(offlineId);
    short layer_disk = sctId->layer_disk(offlineId);
    short phi_index = sctId->phi_module(offlineId);
    short eta_index = sctId->eta_module(offlineId);

    auto t = std::make_tuple(subdetid, barrel_ec, layer_disk); 
    std::map<std::tuple<short,short,short>,std::vector<PhiEtaHash> >::iterator it = hashMap.find(t);
    if(it==hashMap.end())
      hashMap.insert(std::pair<std::tuple<short,short,short>,std::vector<PhiEtaHash> >(t,std::vector<PhiEtaHash>(1, PhiEtaHash(phi_index, eta_index, hash))));
    else (*it).second.push_back(PhiEtaHash(phi_index, eta_index, hash));
  }

  m_layerInfo[1].resize(pixelId->wafer_hash_max(), -1);
  m_layerInfo[2].resize(sctId->wafer_hash_max(), -1);

  int layerId=0;

  for(std::map<std::tuple<short,short,short>,std::vector<PhiEtaHash> >::iterator it = hashMap.begin();it!=hashMap.end();++it, layerId++) {

    short subdetId = std::get<0>((*it).first);
    short barrel_ec = std::get<1>((*it).first);

    m_layerInfo[0].push_back(barrel_ec);

    if(subdetId == 1) {//pixel
      for(std::vector<PhiEtaHash>::iterator hIt = (*it).second.begin();hIt != (*it).second.end();++hIt) {
        m_layerInfo[subdetId][(*hIt).m_hash] = layerId;
      }
    }
    if(subdetId == 2) {//SCT
      for(std::vector<PhiEtaHash>::iterator hIt = (*it).second.begin();hIt != (*it).second.end();++hIt) {
        m_layerInfo[subdetId][(*hIt).m_hash] = layerId;
      }
    }
  }
  
  
  for(std::map<std::tuple<short,short,short>,std::vector<PhiEtaHash> >::iterator it = hashMap.begin();it!=hashMap.end();++it) {

    //sorting along phi first, then along eta

    std::sort((*it).second.begin(), (*it).second.end(), PhiEtaHash::compare());

  }
  return true;
}
