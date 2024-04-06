/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef VKalVrt_GNNVertexConstructorTool_H
#define VKalVrt_GNNVertexConstructorTool_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GNNVertexConstructor/IGNNVertexConstructorInterface.h"
#include "StoreGate/ReadDecorHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"

#include "AnalysisUtils/AnalysisMisc.h"
#include "BeamSpotConditionsData/BeamSpotData.h"
#include "FlavorTagDiscriminants/GNNTool.h"
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

class GNNVertexConstructorTool : public AthAlgTool, virtual public IGNNVertexConstructorInterface {

public:
  GNNVertexConstructorTool(const std::string &type, const std::string &name, const IInterface *parent);
  virtual ~GNNVertexConstructorTool();

  StatusCode initialize();
  StatusCode finalize();

  virtual StatusCode performVertexFit(const xAOD::JetContainer *, xAOD::VertexContainer *,
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

  std::string m_jetCollection;

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

  std::unique_ptr< SG::AuxElement::Decorator<float> >  m_deco_mass;
  std::unique_ptr< SG::AuxElement::Decorator<float> >  m_deco_pt;
  std::unique_ptr< SG::AuxElement::Decorator<float> >  m_deco_charge;
  std::unique_ptr< SG::AuxElement::Decorator<float> >  m_deco_vPos;
  std::unique_ptr< SG::AuxElement::Decorator<float> >  m_deco_lxy;
  std::unique_ptr< SG::AuxElement::Decorator<float> >  m_deco_sig3D;
  std::unique_ptr< SG::AuxElement::Decorator<float> >  m_deco_deltaR;
  std::unique_ptr< SG::AuxElement::Decorator<float> >  m_deco_NGT;
  std::unique_ptr< SG::AuxElement::Decorator<float> >  m_deco_l3d;
  std::unique_ptr< SG::AuxElement::Decorator<float> >  m_deco_N2Tpair;
  std::unique_ptr< SG::AuxElement::Decorator<float> >  m_deco_minDst;
  std::unique_ptr< SG::AuxElement::Decorator<float> >  m_deco_eFrac;
  
  bool   m_multiWithPrimary;
  double m_minLxy;
  double m_maxLxy;
  double m_massPi;
  double m_minSig3D;
  double m_maxChi2;
  bool  m_HFTrackRatio;
  float m_HFRatioThres;
};
} // namespace Rec

#endif

