/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGINDETPATTRECOTOOLS_SEEDINGTOOLBASE_H
#define TRIGINDETPATTRECOTOOLS_SEEDINGTOOLBASE_H

#include "GaudiKernel/ToolHandle.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "StoreGate/ReadHandleKey.h"

#include "IRegionSelector/IRegSelTool.h"
#include "TrigInDetToolInterfaces/ITrigL2LayerNumberTool.h"

#include "TrigInDetPattRecoTools/GNN_FasTrackConnector.h"
#include "TrigInDetPattRecoTools/GNN_Geometry.h"
#include "GNN_DataStorage.h"
#include <string>
#include <vector>
#include <utility> //for std::pair
#include <tuple>
#include <memory>
#include <array>

class AtlasDetectorID;
class SCT_ID;
class PixelID;

class SeedingToolBase: public AthAlgTool {
  public:
  SeedingToolBase(const std::string& t,const std::string& n,const IInterface* p): AthAlgTool(t,n,p){}
  
 protected:

  typedef TrigFTF_GNN_Node GNN_Node;
  typedef TrigFTF_GNN_DataStorage GNN_DataStorage;
  typedef TrigFTF_GNN_Edge GNN_Edge;

  virtual StatusCode initialize();
  virtual StatusCode finalize();

  std::pair<int, int> buildTheGraph(const IRoiDescriptor&, const std::unique_ptr<GNN_DataStorage>&, std::vector<GNN_Edge>&) const;

  int runCCA(int, std::vector<GNN_Edge>&) const;

  void extractSeedsFromTheGraph(int, int, int, std::vector<GNN_Edge>&, std::vector<std::tuple<float, int, std::vector<unsigned int> > >&) const;

  ToolHandle<ITrigL2LayerNumberTool> m_layerNumberTool {this, "layerNumberTool", "TrigL2LayerNumberToolITk"};

  const AtlasDetectorID* m_atlasId = nullptr;
  const SCT_ID*  m_sctId = nullptr;
  const PixelID* m_pixelId = nullptr;

  BooleanProperty m_filter_phi{this, "DoPhiFiltering", true};
  BooleanProperty m_useBeamTilt{this, "UseBeamTilt", false};
  BooleanProperty m_LRTmode{this, "LRTMode",false};
  BooleanProperty m_useML{this, "useML", true};

  UnsignedIntegerProperty m_nMaxPhiSlice{this, "nMaxPhiSlice", 53};
  BooleanProperty m_doubletFilterRZ{this, "Doublet_FilterRZ", true};
  BooleanProperty m_useEtaBinning{this, "UseEtaBinning", true};
  BooleanProperty m_matchBeforeCreate{this, "MatchBeforeCreate", true};
  FloatProperty m_minPt{this, "pTmin", 1000.0};
  FloatProperty m_etaBinOverride{this, "etaBin", 0.0f, "specify non-zero to override eta bin width from connection file (default 0.2 in createLinkingScheme.py)"};
  IntegerProperty m_nMaxEdges{this, "MaxGraphEdges", 2000000};
  IntegerProperty m_nBufferEdges{this, "BufferEdges", 200000};
  StringProperty  m_connectionFile{this, "ConnectionFileName", "binTables_ITK_RUN4.txt"};
  StringProperty  m_lutFile{this, "MLpredictorLutFileName", "gbts_ml_pixel_barrel_loose.lut"};

  BooleanProperty m_useGPUseedExtraction{this, "UseGPUseedExtraction", true};
  BooleanProperty m_useOldTunings{this, "UseOldTunings", false};

  FloatProperty m_tau_ratio_cut{this, "tau_ratio_cut", 0.007};

  float m_phiSliceWidth = 0.;

  std::unique_ptr<GNN_FasTrackConnector> m_connector = nullptr;
  std::vector<TrigInDetSiLayer> m_layerGeometry;
  std::unique_ptr<const TrigFTF_GNN_Geometry> m_geo = nullptr;
  std::vector<std::array<float, 5> > m_mlLUT;
};

#endif
