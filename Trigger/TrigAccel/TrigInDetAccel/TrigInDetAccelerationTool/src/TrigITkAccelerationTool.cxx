/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigITkAccelerationTool.h"
#include "AthenaBaseComps/AthMsgStreamMacros.h" 
#include "AthenaBaseComps/AthCheckMacros.h"
#include "TrigAccelEvent/TrigITkAccelEDM.h"
#include "InDetPrepRawData/PixelCluster.h"

TrigITkAccelerationTool::TrigITkAccelerationTool(const std::string& t, 
						     const std::string& n,
						     const IInterface*  p ): base_class(t,n,p),
									     m_accelSvc("TrigInDetAccelerationSvc",this->name()) {

  declareInterface< ITrigInDetAccelerationTool >( this );
}

StatusCode TrigITkAccelerationTool::initialize() {

  ATH_CHECK(m_accelSvc.retrieve());
  return StatusCode::SUCCESS;
}

size_t TrigITkAccelerationTool::exportSeedMakingJob(const TrigCombinatorialSettings& tcs, const IRoiDescriptor* roi, const std::vector<TrigSiSpacePointBase>& vsp, TrigAccel::DATA_EXPORT_BUFFER& output) const {

  typedef struct SpIndexPair {
  public:

    struct compareZ {
    public:
      bool operator()(const std::pair<int, const TrigSiSpacePointBase*>& p1, const std::pair<int, const TrigSiSpacePointBase*>& p2) {
        return p1.second->z() < p2.second->z();
      }
    };
    
    struct compareR {
    public:
      bool operator()(const std::pair<int, const TrigSiSpacePointBase*>& p1, const std::pair<int, const TrigSiSpacePointBase*>& p2) {
        return p1.second->r() < p2.second->r();
      }
    };

  } SP_INDEX_PAIR;
  

  //0. get InDet geometry information

  const std::vector<short>& pixelLayers = m_accelSvc->getLayerInformation(1);
  const std::vector<short>& layerTypes  = m_accelSvc->getLayerInformation(0);
  

  //1. check buffer size

  size_t dataTypeSize = sizeof(TrigAccel::ITk::SEED_MAKING_JOB);
  const size_t bufferOffset = 256;
  size_t totalSize = bufferOffset+dataTypeSize;//make room for the header
  if(!output.fit(totalSize)) output.reallocate(totalSize);

  TrigAccel::ITk::SEED_MAKING_JOB* pJ = reinterpret_cast<TrigAccel::ITk::SEED_MAKING_JOB*>(output.m_buffer+bufferOffset);

  // cppcheck-suppress memsetClassFloat; deliberate
  memset(pJ,0,dataTypeSize);

  TrigAccel::ITk::SEED_FINDER_SETTINGS& sfs = pJ->m_settings; 

  sfs.m_maxBarrelPix = tcs.m_maxBarrelPix;
  sfs.m_minEndcapPix = tcs.m_minEndcapPix;
  sfs.m_maxEndcapPix = tcs.m_maxEndcapPix;
  sfs.m_maxSiliconLayer = tcs.m_maxSiliconLayer;
  sfs.m_maxEta = roi->etaPlus();
  sfs.m_minDoubletLength = tcs.m_seedRadBinWidth;
  sfs.m_maxDoubletLength = tcs.m_doublet_dR_Max;
  
  sfs.m_magFieldZ = tcs.m_magFieldZ;

  sfs.m_tripletD0Max = tcs.m_tripletD0Max;
  sfs.m_tripletD0_PPS_Max = tcs.m_tripletD0_PPS_Max;
  sfs.m_tripletPtMin = tcs.m_tripletPtMin;
  sfs.m_tripletDoPSS = tcs.m_tripletDoPSS ? 1 : 0;
  sfs.m_tripletDoPPS = tcs.m_tripletDoPPS ? 1 : 0;
  sfs.m_doubletFilterRZ = tcs.m_doubletFilterRZ ? 1 : 0;
  sfs.m_nMaxPhiSlice = tcs.m_nMaxPhiSlice;
  sfs.m_maxTripletBufferLength = tcs.m_maxTripletBufferLength;
  sfs.m_isFullScan = 1;

  // roi->isFullscan() check cannot be currently used for standalone ITk ROI configuration
  bool isRoiFullscan = ((roi->etaPlus() >= 4) && (roi->etaMinus() <= -4));
  if(!(isRoiFullscan || roi->composite() )){
    //roi suitable for gpu
    //composite rois are not supported at this point
    sfs.m_isFullScan = 0;

    sfs.m_phiMinus = roi->phiMinus();
    sfs.m_phiPlus = roi->phiPlus();
  }
  
  sfs.m_zedMinus = roi->zedMinus();
  sfs.m_zedPlus =  roi->zedPlus();

  TrigAccel::ITk::SPACEPOINT_STORAGE& sps = pJ->m_data;

  unsigned int nSP = vsp.size();
  if(nSP >= TrigAccel::ITk::MAX_NUMBER_SPACEPOINTS) {
    nSP = TrigAccel::ITk::MAX_NUMBER_SPACEPOINTS-1;
    ATH_MSG_WARNING("MAX_NUMBER_SPACEPOINTS exceeded, exported data truncated ...");
  }

  //2. arrange spacepoints into phi/Layer bins

  double phiSliceWidth = 2*M_PI/tcs.m_nMaxPhiSlice;
  int nLayers = static_cast<int>(layerTypes.size());
  int nSlices = static_cast<int>(tcs.m_nMaxPhiSlice);
  
  std::vector<std::vector<std::pair<int, const TrigSiSpacePointBase*> > > phiLArray;
  phiLArray.resize(nLayers*nSlices);
  
  for(unsigned int i=0;i<nSP;i++) {
    const TrigSiSpacePointBase& sp  = vsp[i];
    const std::pair<IdentifierHash, IdentifierHash>& els = sp.offlineSpacePoint()->elementIdList();

    IdentifierHash hashId = els.first;
    short layerId = pixelLayers[hashId];

    int phiIdx = (sp.phi()+M_PI)/phiSliceWidth; 
    if (phiIdx >= tcs.m_nMaxPhiSlice) { 
      phiIdx %= tcs.m_nMaxPhiSlice; 
    } 
    else if (phiIdx < 0) { 
      phiIdx += tcs.m_nMaxPhiSlice; 
      phiIdx %= tcs.m_nMaxPhiSlice; 
    } 

    std::vector<std::pair<int, const TrigSiSpacePointBase*> >& v = phiLArray[layerId + phiIdx*nLayers];
    v.push_back(std::pair<int, const TrigSiSpacePointBase*>(i,&sp));//storing the original index of spacepoint
  }

  //sorting spacepoints in accordance with non-ref coordinate 
  int layerIdx=0;
  for(std::vector<short>::const_iterator it = layerTypes.begin();it!=layerTypes.end();++it, layerIdx++) {
    short barrel_ec = (*it);//0-barrel, !=0 - endcap
    for(int slice = 0;slice<nSlices;slice++) {
      std::vector<std::pair<int, const TrigSiSpacePointBase*> >& v = phiLArray[layerIdx + slice*nLayers];
      if(barrel_ec==0) std::sort(v.begin(), v.end(), SP_INDEX_PAIR::compareZ());
      else std::sort(v.begin(), v.end(), SP_INDEX_PAIR::compareR());
    }
  }

  sps.m_nSpacepoints = nSP;
  sps.m_nPhiSlices = nSlices;
  sps.m_nLayers = nLayers;
  sps.m_nMiddleLayers = m_accelSvc->getMiddleLayersSize();

  int spIdx=0;
  for(int slice = 0;slice<nSlices;slice++) {
    for(int layer = 0;layer<nLayers;layer++) {
      int layerStart = spIdx;
      bool isBarrel = (layerTypes[layer] == 0);  // barrel = 0, ec != 0
      std::vector<std::pair<int, const TrigSiSpacePointBase*> >& v = phiLArray[layer + slice*nLayers];
      for(std::vector<std::pair<int, const TrigSiSpacePointBase*> >::iterator it = v.begin();it!=v.end();++it) {
        const TrigSiSpacePointBase* sp  = (*it).second;
        const Trk::SpacePoint* osp = sp->offlineSpacePoint();
        const InDet::PixelCluster* pCL = dynamic_cast<const InDet::PixelCluster*>(osp->clusterList().first);
        float clusterWidth = pCL->width().widthPhiRZ().y();
        if (!isBarrel && clusterWidth > 0.2) continue;

        sps.m_index[spIdx] = (*it).first;
        sps.m_type[spIdx] = layerTypes[layer]; // store layer number
        sps.m_x[spIdx] = sp->x();
        sps.m_y[spIdx] = sp->y();
        sps.m_z[spIdx] = sp->z();
        sps.m_r[spIdx] = sp->r();
        sps.m_phi[spIdx] = sp->phi();
        sps.m_covR[spIdx] = getCovR(isBarrel);
        sps.m_covZ[spIdx] = getCovZ(isBarrel);
        sps.m_clusterWidth[spIdx] = clusterWidth;
        spIdx++;
      }
      int layerEnd = spIdx;
      sps.m_phiSlices[slice].m_layerBegin[layer] = layerStart;
      sps.m_phiSlices[slice].m_layerEnd[layer] = layerEnd;
    }
  }
  
  return totalSize;

}

int TrigITkAccelerationTool::extractTripletsFromOutput(std::shared_ptr<TrigAccel::OffloadBuffer> gpu_buffer, const std::vector<TrigSiSpacePointBase>& vsp, std::vector<TrigInDetTriplet>& output) const{
  TrigAccel::ITk::OUTPUT_SEED_STORAGE* pOutput = reinterpret_cast<TrigAccel::ITk::OUTPUT_SEED_STORAGE *>(gpu_buffer->m_rawBuffer);

  int nTriplets = pOutput->m_nSeeds;

  //copy seeds into the output buffer

  output.clear();

  for(int k=0;k<nTriplets;k++) {
    // Check if a valid triplet was returned
    if (pOutput->m_innerIndex[k] == pOutput->m_outerIndex[k]) continue;

    const TrigSiSpacePointBase& SPi = vsp[pOutput->m_innerIndex[k]];
    const TrigSiSpacePointBase& SPm = vsp[pOutput->m_middleIndex[k]];
    const TrigSiSpacePointBase& SPo = vsp[pOutput->m_outerIndex[k]];
    output.emplace_back(SPi, SPm, SPo, pOutput->m_Q[k]);
  }

  return nTriplets;
}

float TrigITkAccelerationTool::getCovR(bool isBarrel) const {
  if (isBarrel) return 0.01*0.01;
  else return 0.02*0.02;
}

float TrigITkAccelerationTool::getCovZ(bool isBarrel) const {
  if (isBarrel) return 0.02*0.02;
  else return 0.01*0.01;
}
