#ifndef VKalVrt_GNNVertexConstructorTool_H
#define VKalVrt_GNNVertexConstructorTool_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GNNVertexConstructor/IGNNVertexConstructorInterface.h"
#include "StoreGate/ReadDecorHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"

#include "AnalysisUtils/AnalysisMisc.h"
#include "GeoPrimitives/GeoPrimitivesHelpers.h"

#include "BeamSpotConditionsData/BeamSpotData.h"
#include "FlavorTagDiscriminants/GNNTool.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "TrkVKalVrtFitter/TrkVKalVrtFitter.h"
#include "TrkVKalVrtCore/TrkVKalVrtCore.h"
#include "VxSecVertex/VxSecVertexInfo.h"
#include "xAODEventInfo/EventInfo.h"
#include "xAODJet/JetContainer.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/Vertex.h"
#include "xAODTracking/VertexContainer.h"
#include "TrkToolInterfaces/ITrackSummaryTool.h"
#include "TMath.h"

#include "vector"
#include "iostream"
#include "iterator"
#include "map"

class TH2D;
class TH1F;
class TProfile;
class TTree;
class ITHistSvc;

namespace Trk {
  class TrkVKalVrtFitter;
  class IVertexFitter;
  class IVKalState;
  class VxSecVKalVertexInfo;
} // namespace Trk

#include "xAODTracking/TrackParticleContainer.h"

//Headers to use the GNN Tool
#include "FlavorTagDiscriminants/GNN.h"
#include "FlavorTagDiscriminants/GNNTool.h"
#include "FlavorTagDiscriminants/BTagTrackIpAccessor.h"
#include "FlavorTagDiscriminants/OnnxUtil.h"
#include "GaudiKernel/ToolHandle.h"
//Header for Data Containers
#include "xAODBTagging/BTagging.h"
#include "xAODJet/JetContainer.h"
#include "xAODJet/JetAuxContainer.h"
#include "xAODEventInfo/EventInfo.h"
#include "xAODTruth/TruthEventContainer.h"
#include "xAODCore/AuxContainerBase.h"
#include "xAODTracking/VertexContainer.h"
#include "xAODTracking/VertexAuxContainer.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/Vertex.h"

#include "TrkVKalVrtFitter/TrkVKalVrtFitter.h"
#include "TrkExInterfaces/IExtrapolator.h"
#include "lwtnn/parse_json.hh"
#include <vector>
#include "GaudiKernel/ServiceHandle.h"
//Remove in boost > 1.76 when the boost iterator issue
//is solved see ATLASRECTS-6358
#define BOOST_ALLOW_DEPRECATED_HEADERS
#include "boost/graph/adjacency_list.hpp"

#include "BeamSpotConditionsData/BeamSpotData.h"
#include "FlavorTagDiscriminants/GNNTool.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "TrkVKalVrtFitter/TrkVKalVrtFitter.h"
#include "TrkVKalVrtCore/TrkVKalVrtCore.h"
#include "VxSecVertex/VxSecVertexInfo.h"
#include "xAODEventInfo/EventInfo.h"
#include "xAODJet/JetContainer.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/Vertex.h"
#include "xAODTracking/VertexContainer.h"
#include "TrkToolInterfaces/ITrackSummaryTool.h"
#include "TMath.h"

#include "vector"
#include "iostream"
#include "iterator"
#include "map"

class TH2D;
class TH1F;
class TProfile;
class TTree;
class ITHistSvc;

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
  GNNVertexConstructorTool(const std::string &type, const std::string &name,
                           const IInterface *parent);
  virtual ~GNNVertexConstructorTool();

  StatusCode initialize();
  StatusCode finalize();

  virtual StatusCode decorateJets(const xAOD::JetContainer*) const;
  virtual StatusCode performVertexFit(const xAOD::JetContainer*, 
                                      xAOD::VertexContainer*, 
                                      const xAOD::Vertex & primaryVertex, 
                                      const EventContext&) const;

  // Tools
  ToolHandle<FlavorTagDiscriminants::GNNTool> m_gnn_Tool{this, "gnn_Tool", "",
                                                         "GNN Decorator tool"};
  ToolHandle<Trk::TrkVKalVrtFitter> m_vertexFitterTool;
    
  // Read handles
  SG::ReadDecorHandleKey<xAOD::JetContainer> m_trackLinksKey{
      this, "trackLinksKey", "", "Jet GNN Deco Read Key for track link"};
  SG::ReadDecorHandleKey<xAOD::JetContainer> m_vertexLinksKey{
      this, "vertexLinksKey", "", "Jet GNN Deco Read Key for vertex link"};
  SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey{this, "eventInfoKey", "EventInfo",
                                                    "EventInfo container to use"};

  // Write handles
  SG::WriteDecorHandleKey<xAOD::JetContainer> m_jetWriteDecorKeyVertexLink{this,"jetDecorKeyJetLink",
    "","WriteDecorHandleKey for adding VertexLink to Jets"};

  // Conditions
  SG::ReadCondHandleKey<InDet::BeamSpotData> m_beamSpotKey{this, "BeamSpotKey", "BeamSpotData",
                                                           "SG key for beam spot"};
  
  //Access the Primary Vertex Info
  const xAOD::Vertex* m_thePV;
  
private:
 
  std::string m_jetCollection;

  TLorentzVector TotalMom(const std::vector<const xAOD::TrackParticle*>& selTrk) const; 
  
  double vrtVrtDist(const xAOD::Vertex & primVrt, const Amg::Vector3D & secVrt, 
                                  const std::vector<double>& vrtErr,double& signif ) const;
 
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
    double projectedVrt=0.;
    int detachedTrack=-1;
    double BDT=1.1;
    };//end WrkVrt
    
    bool m_existIBL;
    double m_Xbeampipe;
    double m_Ybeampipe;
    double m_XlayerB;
    double m_YlayerB;
    double m_Xlayer1;
    double m_Ylayer1;
    double m_Xlayer2;
    double m_Ylayer2;
    double m_Rbeampipe;
    double m_RlayerB;
    double m_Rlayer1;
    double m_Rlayer2;
    double m_Rlayer3;
    double m_SVResolutionR;
    bool   m_MultiWithPrimary;
    double m_minD0;
    double m_massPi ;
    
};
} // namespace Rec

#endif
