/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGINDETTRACKFITTER_TRIGINDETROADPREDICTORTOOL_H
#define TRIGINDETTRACKFITTER_TRIGINDETROADPREDICTORTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"

#include "TrigInDetToolInterfaces/ITrigInDetRoadPredictorTool.h"
#include "TrigInDetToolInterfaces/ITrigL2LayerNumberTool.h"

// MagField cache
#include "MagFieldConditions/AtlasFieldCacheCondObj.h"
#include "MagFieldElements/AtlasFieldCache.h"

#include <fstream>
#include <array>
#include <map>

namespace InDetDD {
  class PixelDetectorManager;
  class SCT_DetectorManager;
}

namespace Trk {	
  class SpacePoint;
}

class PixelID;
class SCT_ID;

class TrigInDetRoadPredictorTool: public AthAlgTool, virtual public ITrigInDetRoadPredictorTool
{
 public:
  TrigInDetRoadPredictorTool( const std::string&, const std::string&, const IInterface* );

  virtual StatusCode initialize() override;

  virtual int getRoad(const std::vector<const Trk::SpacePoint*>&, std::vector<const InDetDD::SiDetectorElement*>&,
		      const EventContext&) const override;

private:

  struct DetectorElementDescription {
    DetectorElementDescription(unsigned int h) : m_hash(h) {};
    unsigned int m_hash;
    short m_index;
    float m_ref;//z or Phi
    float m_c[4][3];
    float m_minBound, m_maxBound;
  };
  
  struct DetectorElementsCollection {
    DetectorElementsCollection(short idx, float min, float max) : m_index(idx), m_minCoord(min), m_maxCoord(max) {};
    short m_index;
    float m_minCoord, m_maxCoord; //phi or R
    std::vector<DetectorElementDescription> m_vDE;
  };

  struct LayerDescription {
    LayerDescription(unsigned int id, int nSL, unsigned int mType) : m_id(id), m_nSubLayers(nSL), m_mappingType(mType) {};
    unsigned int m_id;
    int m_nSubLayers;
    unsigned int m_mappingType;
    std::map<short, DetectorElementsCollection> m_colls[2];
  };

  
  struct SearchInterval {
    SearchInterval(float z, float r) : m_r(0.0), m_z(0.0), m_phi(0.0) {
      m_vZx.push_back(z);
      m_vRx.push_back(r);
    }

    void addPoint(float z, float r) {
      m_vZx.push_back(z);
      m_vRx.push_back(r);
    }
    
    float getAverageRadius() {
      return std::accumulate(m_vRx.begin(), m_vRx.end(), 0)/m_vRx.size();
    }

    float getAverageZ() {
      return std::accumulate(m_vZx.begin(), m_vZx.end(), 0)/m_vZx.size();
    }
    
    float getMinZ() const {
      return *std::min_element( m_vZx.begin(), m_vZx.end());
    }
    
    float getMaxZ() const {
      return *std::max_element( m_vZx.begin(), m_vZx.end());
    }

    float getMinR() const {
      return *std::min_element( m_vRx.begin(), m_vRx.end());
    }
    
    float getMaxR() const {
      return *std::max_element( m_vRx.begin(), m_vRx.end());
    }

    std::vector<float> m_vZx;
    std::vector<float> m_vRx;
    float m_r;
    float m_z;
    float m_phi;
  };
  
  struct VolumeBoundary {
    int m_index;
    float m_zr[4];
    int m_vol_id;
    std::vector<int> m_layers;
  };

  struct LayerBoundary {
    int m_index, m_lay_id, m_nVertices;
    std::vector<float> m_z;
    std::vector<float> m_r;
  };
  
  void buildDetectorDescription();
  void createHitBoxes();
  
  void addNewElement(unsigned int, short, short, const InDetDD::SiDetectorElement*);
  void findDetectorElements(unsigned int, const SearchInterval&, std::vector<unsigned int>&, bool) const;
  
  ToolHandle<ITrigL2LayerNumberTool> m_layerNumberTool {this, "LayerNumberTool", "TrigL2LayerNumberToolITk"};
  
  Gaudi::Property<float> m_min_rz_rw {this, "MinRzRoadWidth", 3.0,  "Minimum rz road width"};
  Gaudi::Property<float> m_max_rz_rw {this, "MaxRzRoadWidth", 15.0, "Maximum rz road width"};
  Gaudi::Property<float> m_min_rphi_rw {this, "MinRPhiRoadWidth", 3.0,  "Minimum rphi road width"};
  Gaudi::Property<float> m_max_rphi_rw {this, "MaxRPhiRoadWidth", 15.0, "Maximum rphi road width"};
  Gaudi::Property<float> m_min_phi_rw {this, "MinPhiRoadWidth", 0.05,  "Minimum phi road width"};
  Gaudi::Property<float> m_max_phi_rw {this, "MaxPhiRoadWidth", 0.1,   "Maximum phi road width"};
  
  SG::ReadCondHandleKey<AtlasFieldCacheCondObj> m_fieldCondObjInputKey{
    this, "AtlasFieldCacheCondObj", "fieldCondObj",
    "Name of the Magnetic Field conditions object key"};

  std::vector<VolumeBoundary> m_vBoundaries;
  std::vector<LayerBoundary>  m_lBoundaries;  
  std::map<unsigned int, LayerDescription> m_layerMap;
  
  const InDetDD::PixelDetectorManager* m_pixelManager{nullptr};
  const InDetDD::SCT_DetectorManager*  m_stripManager{nullptr};
  const PixelID* m_pixelId{nullptr};
  const SCT_ID*  m_stripId{nullptr};
};

#endif
