/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGONLINESPACEPOINTTOOL_TRIG_L2_LAYER_NUMBER_TOOL_ITK_H
#define TRIGONLINESPACEPOINTTOOL_TRIG_L2_LAYER_NUMBER_TOOL_ITK_H

#include <Gaudi/Property.h>
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

struct PhiEtaHashITk {

  struct compare {
  public:
    bool operator()(const struct PhiEtaHashITk& p1, const struct PhiEtaHashITk& p2) {
      if(p1.m_phiIndex == p2.m_phiIndex) {
	return p1.m_etaIndex < p2.m_etaIndex;
      }
      else {
	return p1.m_phiIndex < p2.m_phiIndex;
      }
    }
  };

public:
  PhiEtaHashITk(short phi, short eta, int hash) : m_phiIndex(phi), m_etaIndex(eta), m_hash(hash) {};
  PhiEtaHashITk(const PhiEtaHashITk& p) : m_phiIndex(p.m_phiIndex), m_etaIndex(p.m_etaIndex), m_hash(p.m_hash) {}; 
  short m_phiIndex, m_etaIndex;
  int m_hash;
};


class TrigL2LayerNumberToolITk : virtual public ITrigL2LayerNumberTool, public AthAlgTool {
 public:

  // standard AlgTool methods
  TrigL2LayerNumberToolITk(const std::string&,const std::string&,const IInterface*);
  virtual ~TrigL2LayerNumberToolITk(){};
		
  // standard Athena methods
  virtual StatusCode initialize() override;

  //concrete implementations

  virtual int maxSiliconLayerNum() const override  {return m_MaxSiliconLayerNum;}
  virtual int offsetEndcapPixels() const override {return m_OffsetEndcapPixels;}
  virtual int offsetBarrelSCT() const override {return m_OffsetBarrelSCT;}
  virtual int offsetEndcapSCT() const override {return m_OffsetEndcapSCT;}
  virtual void report() const override;//prints out the above

  virtual int maxNumberOfUniqueLayers() const override {
    return static_cast<int>(m_hashMap.size());
  }

  virtual const std::vector<short>* pixelLayers() const override {
    return &m_pixelLayers;
  }

  virtual const std::vector<short>* sctLayers() const override {
    return &m_sctLayers;
  }

  virtual const std::vector<TrigInDetSiLayer>* layerGeometry() const override {
    return &m_layerGeometry;
  }

 protected:

  Gaudi::Property<bool> m_useNewScheme{this, "UseNewLayerScheme", false};
  Gaudi::Property<bool> m_dumpGeometry{this, "dumpGbtsGeometry", false};
  Gaudi::Property<std::string> m_geometryDumpDir{this, "geometryDump", ""};
  //cached values
  int m_MaxSiliconLayerNum{-1};
  int m_OffsetEndcapPixels{-1};
  int m_OffsetBarrelSCT{-1};
  int m_OffsetEndcapSCT{-1};
  int m_LastBarrelLayer{0};

  const SCT_ID*  m_sctId{nullptr};
  const PixelID* m_pixelId{nullptr};
  const InDetDD::PixelDetectorManager* m_pixelManager{nullptr};
  const InDetDD::SCT_DetectorManager* m_sctManager{nullptr};

  void createModuleHashMap(std::map<std::tuple<int, int, short, short>,std::vector<PhiEtaHashITk> >&);
  
  std::map<std::tuple<int, int, short, short>,std::vector<PhiEtaHashITk> > m_hashMap;
  std::vector<short> m_pixelLayers, m_sctLayers;//hashid addressable arrays of layer numbers
  std::vector<TrigInDetSiLayer> m_layerGeometry;
};
#endif
