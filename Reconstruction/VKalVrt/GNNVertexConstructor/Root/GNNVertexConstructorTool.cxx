// Headers
#include "GNNVertexConstructor/GNNVertexConstructorTool.h"
// Headers to Read & Write Decorations
#include "AnalysisUtils/AnalysisMisc.h"
#include "GeoPrimitives/GeoPrimitivesHelpers.h"
#include "StoreGate/ReadDecorHandle.h"
#include "StoreGate/WriteDecorHandle.h"
#include "TrkVKalVrtFitter/TrkVKalVrtFitter.h"
#include "VxSecVertex/VxSecVertexInfo.h"
#include "iostream"
#include "iterator"
#include "map"
#include "vector"

namespace Rec {

GNNVertexConstructorTool::GNNVertexConstructorTool(const std::string &type, const std::string &name,
                                                   const IInterface *parent)
    : AthAlgTool(type, name, parent),
      m_vertexFitterTool("Trk::TrkVKalVrtFitter/VertexFitterTool", this),
      m_jetCollection("AntiKt4EMPFlowJets") {
  declareInterface<IGNNVertexConstructorInterface>(this);

  declareProperty("JetTrackLinks", m_trackLinksKey = "BTagging_AntiKt4EMPFlowAuxDyn.TrackLinks");
  declareProperty("JetVertexLinks",
                  m_vertexLinksKey = "BTagging_AntiKt4EMPFlowAuxDyn.track_vertexing");

  declareProperty("GNNTool", m_gnn_Tool, "GNN Tool");
  declareProperty("VertexFitterTool", m_vertexFitterTool, "Vertex fitting tool");
}

/* Destructor */

GNNVertexConstructorTool::~GNNVertexConstructorTool() {
  ATH_MSG_DEBUG("GNNVertexConstructorTool destructor called");
}

StatusCode GNNVertexConstructorTool::initialize() {

  ATH_MSG_DEBUG("GNNVertexConstructor Tool in initialize()");

  // Initialize keys
  ATH_CHECK(m_trackLinksKey.initialize());
  ATH_CHECK(m_vertexLinksKey.initialize());

  m_jetWriteDecorKeyVertexLink = m_jetCollection + ".GNNVerticesLink";
  ATH_CHECK( m_jetWriteDecorKeyVertexLink.initialize()); 

  // Retrieve tools
  ATH_CHECK(m_gnn_Tool.retrieve());
  ATH_CHECK(m_vertexFitterTool.retrieve());

  ATH_CHECK(m_beamSpotKey.initialize());

  ATH_CHECK(m_eventInfoKey.initialize());

  // @Luke TODO: what is this parameter?
  m_w_1 = 1.;

  return StatusCode::SUCCESS;
}

StatusCode GNNVertexConstructorTool::decorateJets(const xAOD::JetContainer *jetCont) const {

  for (auto jet : *jetCont) {
    m_gnn_Tool->decorate(*jet);
  }
  return StatusCode::SUCCESS;
}

StatusCode GNNVertexConstructorTool::performVertexFit(const xAOD::JetContainer *inJetContainer,
                            xAOD::VertexContainer *outVertexContainer,
                            const EventContext &ctx) const {

  using TLC = std::vector<ElementLink<DataVector<xAOD::TrackParticle_v1>>>;
  using TL  = ElementLink<DataVector<xAOD::TrackParticle_v1>>;

  SG::ReadDecorHandle<xAOD::JetContainer, TLC> trackLinksHandle(m_trackLinksKey, ctx);
  SG::ReadDecorHandle<xAOD::JetContainer, std::vector<char, std::allocator<char>>>
      vertexLinksHandle(m_vertexLinksKey, ctx);
  SG::WriteDecorHandle< xAOD::JetContainer, std::vector<ElementLink<xAOD::VertexContainer>>> 
      jetWriteDecorHandleVertexLink (m_jetWriteDecorKeyVertexLink, ctx);

  // Create a map of track links and track vertexing values (Using mutlimap)
  std::multimap<int, TL> vertexMap;

  // Loop over the jets
  for (const auto &jet : *inJetContainer) {


    vertexMap.clear();

    auto vertexCollection = vertexLinksHandle(*jet);
    auto trackCollection = trackLinksHandle(*jet);

    int i = 0;

    for (auto v : vertexCollection) {
      vertexMap.insert(std::pair<int, TL>(v, (trackCollection[i])));
      i++;
    }
    std::multimap<int, TL>::iterator itr;

    workVectorArrxAOD *xAODwrk = new workVectorArrxAOD();
    SG::ReadCondHandle<InDet::BeamSpotData> beamSpotHandle{m_beamSpotKey, ctx};

    xAODwrk->beamX = beamSpotHandle->beamPos().x();
    xAODwrk->beamY = beamSpotHandle->beamPos().y();
    xAODwrk->beamZ = beamSpotHandle->beamPos().z();

    xAODwrk->tanBeamTiltX = tan(beamSpotHandle->beamTilt(0));
    xAODwrk->tanBeamTiltY = tan(beamSpotHandle->beamTilt(1));

    std::vector<xAOD::Vertex *> finalVertices(0);

    std::unique_ptr<std::vector<WrkVrt>> wrkVrtSet = std::make_unique<std::vector<WrkVrt>>();
    WrkVrt newvrt;
    newvrt.Good = true;
    std::unique_ptr<Trk::IVKalState> state = m_vertexFitterTool->makeState();
    std::vector<const xAOD::NeutralParticle *> neutralPartDummy(0);

    xAODwrk->tmpListTracks.clear();

    // returns next value after last key - easier for "for loop"
    auto lastKey = (vertexMap.end())->first;

    for (int k = 0; k < lastKey; ++k) {
      if (vertexMap.count(k) >= 2) {
        // Need at least 2 tracks to perform a fit
        auto elements = vertexMap.equal_range(k);

        for (auto i = elements.first; i != elements.second; ++i) {
          xAODwrk->listSelTracks.push_back(*(i->second));
        }

        StatusCode sc = (m_vertexFitterTool->VKalVrtFit(
            xAODwrk->listSelTracks, neutralPartDummy, newvrt.vertex, newvrt.vertexMom,
            newvrt.vertexCharge, newvrt.vertexCov, newvrt.chi2PerTrk, newvrt.trkAtVrt, newvrt.chi2,
            *state, false));
        if (sc.isFailure())
          continue;

        ATH_MSG_DEBUG("Found IniVertex=" << newvrt.vertex[0] << ", " << newvrt.vertex[1] << ", "
                                         << newvrt.vertex[2]);

        Amg::Vector3D vDist = newvrt.vertex; // - m_thePV->position();

        double vPos = (vDist.x() * newvrt.vertexMom.Px() + vDist.y() * newvrt.vertexMom.Py() +
                       vDist.z() * newvrt.vertexMom.Pz()) /
                      newvrt.vertexMom.Rho();

        // @Luke TODO: need to add the tracks to the vertex
        xAOD::Vertex *GNNvertex = new xAOD::Vertex;
        outVertexContainer->emplace_back(GNNvertex);

        // Registering tracks comprising the vertex to xAOD::Vertex
        // loop over the tracks comprising the vertex
        for( const auto *trk : xAODwrk->listSelTracks ) {
          // Acquire link the track to the vertex
          ElementLink<xAOD::TrackParticleContainer> link_trk( *( dynamic_cast<const xAOD::TrackParticleContainer*>( trk->container() ) ), static_cast<long unsigned int>(trk->index()) );
          // Register the link to the vertex
          GNNvertex->addTrackAtVertex( link_trk, 1. );
        }

        GNNvertex->setVertexType(xAOD::VxType::SecVtx);
        GNNvertex->setPosition(newvrt.vertex);
        GNNvertex->setFitQuality(newvrt.chi2, 1);
        GNNvertex->auxdata<float>("mass") = newvrt.vertexMom.M();
        GNNvertex->auxdata<float>("pT") = newvrt.vertexMom.Perp();
        GNNvertex->auxdata<float>("charge") = newvrt.vertexCharge;
        GNNvertex->auxdata<float>("vPos") = vPos;
        GNNvertex->auxdata<bool>("isFake") = true;

        ElementLink< xAOD::VertexContainer> linkVertex;
        linkVertex.setElement(GNNvertex);
        linkVertex.setStorableObject(*outVertexContainer);
        jetWriteDecorHandleVertexLink(*jet).push_back(linkVertex);

        // @Luke TODO: build vertex links and add to jet
      }
    }
    delete xAODwrk;
  } // end loop over jets
  return StatusCode::SUCCESS;
} // end performVertexFit

StatusCode GNNVertexConstructorTool::finalize()
{
  return StatusCode::SUCCESS;
}

} // namespace Rec
