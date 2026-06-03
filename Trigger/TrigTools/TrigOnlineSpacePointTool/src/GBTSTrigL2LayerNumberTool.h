/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGONLINESPACEPOINTTOOL_GBTS_TRIG_L2_LAYER_NUMBER_TOOL_H
#define TRIGONLINESPACEPOINTTOOL_GBTS_TRIG_L2_LAYER_NUMBER_TOOL_H

#include <vector>
#include <map>
#include <tuple>

#include "AthenaBaseComps/AthAlgTool.h"
#include "TrigInDetToolInterfaces/ITrigL2LayerNumberTool.h"

class SCT_ID;
class PixelID;

namespace InDetDD {
  class PixelDetectorManager;
  class SCT_DetectorManager;
}

struct GBTSPhiEtaHash {

  struct compare {
  public:
    bool operator()(const struct GBTSPhiEtaHash& p1, const struct GBTSPhiEtaHash& p2) {
      if(p1.m_phiIndex == p2.m_phiIndex) {
	return p1.m_etaIndex < p2.m_etaIndex;
      }
      else {
	return p1.m_phiIndex < p2.m_phiIndex;
      }
    }
  };

public:
  GBTSPhiEtaHash(short phi, short eta, int hash) : m_phiIndex(phi), m_etaIndex(eta), m_hash(hash) {};
  GBTSPhiEtaHash(const GBTSPhiEtaHash& p) : m_phiIndex(p.m_phiIndex), m_etaIndex(p.m_etaIndex), m_hash(p.m_hash) {}; 
  short m_phiIndex, m_etaIndex;
  int m_hash;
};


class GBTSTrigL2LayerNumberTool : virtual public ITrigL2LayerNumberTool, public AthAlgTool {
 public:

  // standard AlgTool methods
  GBTSTrigL2LayerNumberTool(const std::string&,const std::string&,const IInterface*);
  virtual ~GBTSTrigL2LayerNumberTool(){};
		
  // standard Athena methods
  StatusCode initialize();
  StatusCode finalize();

  //concrete implementations

  virtual int maxSiliconLayerNum() const {return m_MaxSiliconLayerNum;}
  virtual int offsetEndcapPixels() const {return m_OffsetEndcapPixels;}
  virtual int offsetBarrelSCT() const {return m_OffsetBarrelSCT;}
  virtual int offsetEndcapSCT() const {return m_OffsetEndcapSCT;}
  virtual void report() const;//prints out the above 

  virtual int maxNumberOfUniqueLayers() const {
    return (int) m_hashMap.size();
  }

  virtual const std::vector<short>* pixelLayers() const {
    return &m_pixelLayers;
  }

  virtual const std::vector<short>* sctLayers() const {
    return &m_sctLayers;
  }

  virtual const std::vector<TrigInDetSiLayer>* layerGeometry() const {
    return &m_layerGeometry;
  }

 protected:

  bool m_useNewScheme;

  //cached values
  int m_MaxSiliconLayerNum;
  int m_OffsetEndcapPixels;
  int m_OffsetBarrelSCT;
  int m_OffsetEndcapSCT;
  int m_LastBarrelLayer = 0;

  const SCT_ID*  m_sctId = nullptr;
  const PixelID* m_pixelId = nullptr;
  const InDetDD::PixelDetectorManager* m_pixelManager = nullptr;
  const InDetDD::SCT_DetectorManager* m_sctManager = nullptr;

  void createModuleHashMap(std::map<std::tuple<int, int, short, short>,std::vector<GBTSPhiEtaHash> >&);

  std::map<std::tuple<int, int, short, short>,std::vector<GBTSPhiEtaHash> > m_hashMap;
  std::vector<short> m_pixelLayers, m_sctLayers;//hashid addressable arrays of layer numbers
  std::vector<TrigInDetSiLayer> m_layerGeometry;
};
#endif
