/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGINDETPATTRECOTOOLS_GNNR3_GEOMETRY_H
#define TRIGINDETPATTRECOTOOLS_GNNR3_GEOMETRY_H

#include<vector>
#include<map>
#include<algorithm>
#include <memory>
#include "GNNR3_FasTrackConnector.h"
#include "TrigInDetPattRecoEvent/TrigInDetSiLayer.h"


class TrigFTF_GNNR3_Layer {
public:
  TrigFTF_GNNR3_Layer(const TrigInDetSiLayer&, float, int);
  ~TrigFTF_GNNR3_Layer();

  int getEtaBin(float, float) const;

  float getMinBinRadius(int) const;
  float getMaxBinRadius(int) const;

  int num_bins() const {return m_bins.size();} 

  bool verifyBin(const TrigFTF_GNNR3_Layer*, int, int, float, float) const;

  const TrigInDetSiLayer& m_layer;
  std::vector<int> m_bins;//eta-bin indices
  std::vector<float> m_minRadius;
  std::vector<float> m_maxRadius;
  std::vector<float> m_minBinCoord;
  std::vector<float> m_maxBinCoord;

  float m_minEta, m_maxEta, m_etaBin;

protected:

  float m_etaBinWidth;

  float m_r1, m_z1, m_r2, m_z2;
  int m_nBins; 

};

class TrigFTF_GNNR3_Geometry {
public:
  TrigFTF_GNNR3_Geometry(const std::vector<TrigInDetSiLayer>&, const std::unique_ptr<GNNR3_FasTrackConnector>&);
  ~TrigFTF_GNNR3_Geometry();
  
  const TrigFTF_GNNR3_Layer* getTrigFTF_GNNR3_LayerByKey(unsigned int) const;
  const TrigFTF_GNNR3_Layer* getTrigFTF_GNNR3_LayerByIndex(int) const;

  int num_bins() const {return m_nEtaBins;}
  unsigned int num_layers() const {return m_layArray.size();}
  const std::vector<std::pair<int, std::vector<int> > >& bin_groups() const {return m_binGroups;}
  
protected:

  const TrigFTF_GNNR3_Layer* addNewLayer(const TrigInDetSiLayer&, int);

  float m_etaBinWidth;

  std::map<unsigned int, TrigFTF_GNNR3_Layer*> m_layMap;
  std::vector<TrigFTF_GNNR3_Layer*> m_layArray;
  
  int m_nEtaBins;

  std::vector<std::pair<int, std::vector<int> > > m_binGroups;
  
};


#endif
