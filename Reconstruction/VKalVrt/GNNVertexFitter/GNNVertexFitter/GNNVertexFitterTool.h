/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef VKalVrt_GNNVertexFitterTool_H
#define VKalVrt_GNNVertexFitterTool_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GNNVertexFitter/IGNNVertexFitterInterface.h"
#include "StoreGate/ReadDecorHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"

#include "AnalysisUtils/AnalysisMisc.h"
#include "BeamSpotConditionsData/BeamSpotData.h"
#include "GaudiKernel/ITHistSvc.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "GeoPrimitives/GeoPrimitivesHelpers.h"
#include "InDetTrackSystematicsTools/InDetTrackTruthOriginDefs.h"
#include "TMath.h"
#include "TrkToolInterfaces/ITrackSummaryTool.h"
#include "TrkVKalVrtCore/TrkVKalVrtCore.h"
#include "TrkVKalVrtFitter/TrkVKalVrtFitter.h"
#include "VxSecVertex/VxSecVertexInfo.h"
#include "xAODEventInfo/EventInfo.h"
#include "xAODJet/JetContainer.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/Vertex.h"
#include "xAODTracking/VertexContainer.h"
#include "xAODBTagging/BTaggingContainer.h"

#include "algorithm"
#include "iostream"
#include "iterator"
#include "map"
#include "ranges"
#include "set"
#include "vector"
#include <numeric>
#include <vector>

namespace Trk {
class TrkVKalVrtFitter;
class IVertexFitter;
class IVKalState;
class VxSecVKalVertexInfo;
} // namespace Trk

namespace Rec {

struct workVectorArrxAOD {
  std::vector<const xAOD::TrackParticle *> listSelTracks; // Selected tracks after quality cuts
  double beamX = 0.;
  double beamY = 0.;
  double beamZ = 0.;
  double tanBeamTiltX = 0.;
  double tanBeamTiltY = 0.;
};

class GNNVertexFitterTool : public AthAlgTool, virtual public IGNNVertexFitterInterface {

public:
  GNNVertexFitterTool(const std::string &type, const std::string &name, const IInterface *parent);
  virtual ~GNNVertexFitterTool();

  StatusCode initialize();
  StatusCode finalize();

  virtual StatusCode fitAllVertices(const xAOD::JetContainer *, xAOD::VertexContainer *,
                                      const xAOD::Vertex &primaryVertex, const EventContext &) const;

  // Tools
  ToolHandle<Trk::TrkVKalVrtFitter> m_vertexFitterTool;

  // Read handles
  SG::ReadDecorHandleKey<xAOD::BTaggingContainer> m_trackLinksKey{this, "trackLinksKey", "",
                                                             "Jet GNN Deco Read Key for track link"};
  SG::ReadDecorHandleKey<xAOD::BTaggingContainer> m_trackOriginsKey{this, "trackLinksKey", "",
                                                               "Jet GNN Deco Read Key for track origin"};
  SG::ReadDecorHandleKey<xAOD::BTaggingContainer> m_vertexLinksKey{this, "vertexLinksKey", "",
                                                              "Jet GNN Deco Read Key for vertex link"};
  SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey{this, "eventInfoKey", "EventInfo", "EventInfo container to use"};

  // Write handles
  SG::WriteDecorHandleKey<xAOD::JetContainer> m_jetWriteDecorKeyVertexLink{
      this, "jetDecorKeyJetLink", "", "WriteDecorHandleKey for adding VertexLink to Jets"};
  SG::WriteDecorHandleKey<xAOD::JetContainer> m_jetWriteDecorKeyVertexNumber{
      this, "jetDecorKeyVertexNumber", "", "WriteDecorHandleKey for adding number of vertices within a Jet"};

  // Conditions
  SG::ReadCondHandleKey<InDet::BeamSpotData> m_beamSpotKey{this, "BeamSpotKey", "BeamSpotData", "SG key for beam spot"};

  // Access the Primary Vertex Info
  const xAOD::Vertex *m_thePV;

private:

  TLorentzVector TotalMom(const std::vector<const xAOD::TrackParticle *> &selTrk) const;

  double vrtVrtDist(const xAOD::Vertex &primVrt, const Amg::Vector3D &secVrt, const std::vector<double> &vrtErr,
                    double &signif) const;

  struct WrkVrt {
    bool Good = true;
    std::deque<long int> selTrk;
    Amg::Vector3D vertex;
    TLorentzVector vertexMom;
    long int vertexCharge{};
    std::vector<double> vertexCov;
    std::vector<double> chi2PerTrk;
    std::vector<std::vector<double>> trkAtVrt;
    double chi2{};
  }; // end WrkVrt

  SG::AuxElement::Decorator<float>  m_deco_mass;
  SG::AuxElement::Decorator<float>  m_deco_pt;
  SG::AuxElement::Decorator<float>  m_deco_charge;
  SG::AuxElement::Decorator<float>  m_deco_vPos;
  SG::AuxElement::Decorator<float>  m_deco_lxy;
  SG::AuxElement::Decorator<float>  m_deco_sig3D;
  SG::AuxElement::Decorator<float>  m_deco_deltaR;
  SG::AuxElement::Decorator<float>  m_deco_ntrk;
  SG::AuxElement::Decorator<float>  m_deco_lxyz;
  SG::AuxElement::Decorator<float>  m_deco_eFrac;
  
  StringProperty    m_gnnModel{this, "GNNModel", "GN2v01", "GNN model being used" };
  StringProperty    m_jetCollection{this, "JetCollection", "AntiKt4EMPFlowJets", "Jet Collection being used" };
  BooleanProperty   m_multiWithPrimary {this, "multiPrimary", false, " Include Primary Vertices"};
  DoubleProperty    m_minLxy{this, "minLxy", 2.0, "Minimum radial distance from the PV"};
  DoubleProperty    m_minPerp{this, "minPerp", 2.0, "Minimum distance from the PV"};
  DoubleProperty    m_maxLxy{this, "maxLxy", 300, "Maximum radial distance from the PV"};
  DoubleProperty    m_minSig3D{this, "minSig3D", 20, "Minimum 3D significance from the PV"};
  DoubleProperty    m_maxChi2{this, "maxChi2", 20, "Maximum Chi Squared"};
  DoubleProperty    m_HFRatioThres{this, "HFRatio", 0.3, "The threshold for the Ratio between HF tracks and all track for a vertex"};
  DoubleProperty    m_minNTrack{this, "minNTrk", 2, "Minimum number of tracks in a vertex"};
  double m_massPi;
};
} // namespace Rec

#endif

