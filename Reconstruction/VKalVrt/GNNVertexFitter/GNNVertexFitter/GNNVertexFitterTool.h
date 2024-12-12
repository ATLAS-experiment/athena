/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef VKalVrt_GNNVertexFitterTool_H
#define VKalVrt_GNNVertexFitterTool_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GNNVertexFitter/IGNNVertexFitterInterface.h"

#include "GaudiKernel/ToolHandle.h" //member
#include "StoreGate/ReadDecorHandleKey.h" //member
#include "StoreGate/WriteDecorHandleKey.h" //member
#include "StoreGate/ReadCondHandleKey.h" //member

#include "BeamSpotConditionsData/BeamSpotData.h" //ReadCondHandle template param

#include "GeoPrimitives/GeoPrimitives.h" //Amg::Vector3D
#include "TrkVKalVrtFitter/TrkVKalVrtFitter.h" //ToolHandle template param
#include "xAODEventInfo/EventInfo.h" //ReadHandle template param
#include "xAODJet/JetContainer.h" //ToolHandle template param
#include "xAODTracking/Vertex.h" //typedef for Vertex
#include "xAODTracking/VertexContainer.h" //typedef for Vertex
#include "AthContainers/AuxElement.h" //SG::AuxElement::Decorator
#include "TLorentzVector.h" //member
#include <deque>
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
  SG::ReadDecorHandleKey<xAOD::JetContainer> m_trackLinksKey{this, "trackLinksKey", "",
                                                             "Jet GNN Deco Read Key for track link"};
  SG::ReadDecorHandleKey<xAOD::JetContainer> m_trackOriginsKey{this, "trackOriginsKey", "",
                                                               "Jet GNN Deco Read Key for track origin"};
  SG::ReadDecorHandleKey<xAOD::JetContainer> m_vertexLinksKey{this, "vertexLinksKey", "",
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
  const xAOD::Vertex *m_thePV{};

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
  SG::AuxElement::Decorator<float>  m_deco_nHFTracks;
  
  StringProperty    m_gnnModel{this, "GNNModel", "GN2v01", "GNN model being used" };
  StringProperty    m_jetCollection{this, "JetCollection", "AntiKt4EMPFlowJets", "Jet Collection being used" };
  BooleanProperty   m_includePrimaryVertex {this, "includePrimaryVertex", false, "Include Primary Vertices"};
  BooleanProperty   m_removeNonHFVertices {this, "removeNonHFVertices", true, "Remove vertices with no heavy flavour tracks"};
  BooleanProperty   m_doInclusiveVertexing {this, "doInclusiveVertexing", false, "Merge all vertices so that there is at most one vertex per jet"};
  DoubleProperty    m_maxChi2{this, "maxChi2", 20, "Maximum Chi Squared"};
  BooleanProperty   m_applyCuts {this, "applyCuts", false, "Cut on vertex properties"};
  DoubleProperty    m_minLxy{this, "minLxy", 0.0, "Minimum radial distance from the PV"};
  DoubleProperty    m_maxLxy{this, "maxLxy", 1e5, "Maximum radial distance from the PV"};
  DoubleProperty    m_minSig3D{this, "minSig3D", 0, "Minimum 3D significance from the PV"};
  DoubleProperty    m_HFRatioThres{this, "HFRatio", 0.0, "The threshold for the Ratio between HF tracks and all track for a vertex"};
  DoubleProperty    m_minNTrack{this, "minNTrk", 2, "Minimum number of tracks in a vertex"};
  double m_massPi{};
};
} // namespace Rec

#endif

