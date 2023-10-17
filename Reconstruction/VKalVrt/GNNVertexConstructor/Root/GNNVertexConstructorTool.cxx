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
      m_jetCollection("AntiKt4EMPFlowJets")
      /*m_thePV(nullptr)*/{
  declareInterface<IGNNVertexConstructorInterface>(this);

  declareProperty("JetTrackLinks", m_trackLinksKey = "BTagging_AntiKt4EMPFlowAuxDyn.TrackLinks");
  declareProperty("JetVertexLinks",
                  m_vertexLinksKey = "BTagging_AntiKt4EMPFlowAuxDyn.vertex_indices");

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

  //Additional Info for Vertex Fit
  ATH_CHECK(m_beamSpotKey.initialize());
  ATH_CHECK(m_eventInfoKey.initialize());

  return StatusCode::SUCCESS;
}

//Use GNN to decorate the tracks
//May be removed in future
StatusCode GNNVertexConstructorTool::decorateJets(const xAOD::JetContainer *jetCont) const {

  for (auto jet : *jetCont) {
    m_gnn_Tool->decorate(*jet);
  }
  return StatusCode::SUCCESS;
}

//Perform Vertex fit using the jet decorations of the GNN
StatusCode GNNVertexConstructorTool::performVertexFit(const xAOD::JetContainer *inJetContainer,
                            xAOD::VertexContainer *outVertexContainer,
                            const xAOD::Vertex & primVrt,
                            const EventContext &ctx) const {

  using TLC = std::vector<ElementLink<DataVector<xAOD::TrackParticle_v1>>>;
  using TL  = ElementLink<DataVector<xAOD::TrackParticle_v1>>;

  //Read Decor Handle for Track links and Vertex links
  SG::ReadDecorHandle<xAOD::JetContainer, TLC> trackLinksHandle(m_trackLinksKey, ctx);
  SG::ReadDecorHandle<xAOD::JetContainer, std::vector<char, std::allocator<char>>>
      vertexLinksHandle(m_vertexLinksKey, ctx);
  SG::WriteDecorHandle< xAOD::JetContainer, std::vector<ElementLink<xAOD::VertexContainer>>> 
      jetWriteDecorHandleVertexLink (m_jetWriteDecorKeyVertexLink, ctx);
  
  // Vertex decorators
  SG::AuxElement::Decorator<float> decor_mass("mass");
  SG::AuxElement::Decorator<float> decor_pT("pt");
  SG::AuxElement::Decorator<float> decor_charge("charge");
  SG::AuxElement::Decorator<float> decor_vPos("vPos");
  SG::AuxElement::Decorator<float> decor_Lxy("Lxy");
  SG::AuxElement::Decorator<float> decor_significance3d("significance3d");
  SG::AuxElement::Decorator<float> decor_deltaR("deltaR");
  SG::AuxElement::Decorator<float> decor_NGTinSvx("NGTinSvx");
  SG::AuxElement::Decorator<float> decor_L3D("L3d");
  SG::AuxElement::Decorator<float> decor_N2Tpair("N2Tpair");
  //SG::AuxElement::Decorator<float> decor_efracsv("efracsv");
  
  // Create a map of track links and track vertexing values (Using mutlimap)
  std::multimap<int, TL> vertexMap;

  // Loop over the jets
  for (const auto &jet : *inJetContainer) {
    
    //Ensure map is empty from previous iterations
    vertexMap.clear();

    //Retrieve the Vertex and Track Collections
    auto vertexCollection = vertexLinksHandle(*jet);
    auto trackCollection = trackLinksHandle(*jet);

    //Fill the map
    int i = 0;
    for (auto v : vertexCollection) {
      vertexMap.insert(std::pair<int, TL>(v, (trackCollection[i])));
      i++;
    }
    std::multimap<int, TL>::iterator itr;

    //Working xAOD
    workVectorArrxAOD *xAODwrk = new workVectorArrxAOD();
    SG::ReadCondHandle<InDet::BeamSpotData> beamSpotHandle{m_beamSpotKey, ctx};

    //Beam Conditions
    xAODwrk->beamX = beamSpotHandle->beamPos().x();
    xAODwrk->beamY = beamSpotHandle->beamPos().y();
    xAODwrk->beamZ = beamSpotHandle->beamPos().z();
    xAODwrk->tanBeamTiltX = tan(beamSpotHandle->beamTilt(0));
    xAODwrk->tanBeamTiltY = tan(beamSpotHandle->beamTilt(1));

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

        //Retrieve the tracks and push to working xAOD
        for (auto i = elements.first; i != elements.second; ++i) {
          xAODwrk->listSelTracks.push_back(*(i->second));
        }
        
        //Perform the Vertex Fit
        StatusCode sc = (m_vertexFitterTool->VKalVrtFit(
            xAODwrk->listSelTracks, neutralPartDummy, newvrt.vertex, newvrt.vertexMom,
            newvrt.vertexCharge, newvrt.vertexCov, newvrt.chi2PerTrk, newvrt.trkAtVrt, newvrt.chi2,
            *state, false));
        if (sc.isFailure())
          continue;
        
        //Chi2 Cut
        if (newvrt.chi2<20){
        ATH_MSG_DEBUG("Found IniVertex=" << newvrt.vertex[0] << ", " << newvrt.vertex[1] << ", "
                                         << newvrt.vertex[2] << " trks " << newvrt.trkAtVrt.size());

        Amg::Vector3D vDir = newvrt.vertex - primVrt.position();  //Vertex Dirction in relation to Primary
        
        Amg::Vector3D jetDir(jet->p4().Px(),jet->p4().Py(),jet->p4().Pz()); //Jet Direction
        
        
        double vPos = (vDir.x() * newvrt.vertexMom.Px() + vDir.y() * newvrt.vertexMom.Py() +
                       vDir.z() * newvrt.vertexMom.Pz()) /
                      newvrt.vertexMom.Rho();
        
        double L3D =sqrt(vDir[0]*vDir[0]+vDir[1]*vDir[1]+vDir[2]*vDir[2]);
        ATH_MSG_DEBUG("L3D" << L3D);
        
        
        double drJPVSV = Amg::deltaR(jetDir,vDir); //DeltaR
        
        int NGTatVtx=newvrt.trkAtVrt.size();
        
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
       
        //Add Vertex Info into Container        
        GNNvertex->setVertexType(xAOD::VxType::SecVtx);
        GNNvertex->setPosition(newvrt.vertex);
        GNNvertex->setFitQuality(newvrt.chi2, 1);

        decor_mass(*GNNvertex)            = newvrt.vertexMom.M();
        decor_pT(*GNNvertex)              = newvrt.vertexMom.Perp();
        decor_charge(*GNNvertex)          = newvrt.vertexCharge;
        decor_vPos(*GNNvertex)            = vPos;
        decor_Lxy(*GNNvertex)             = sqrt(vDir[0]*vDir[0]+vDir[1]*vDir[1]);
        decor_L3D(*GNNvertex)             = L3D;
        decor_significance3d(*GNNvertex)  = L3D/newvrt.chi2;
        decor_NGTinSvx(*GNNvertex)        = NGTatVtx;
        decor_deltaR(*GNNvertex)          = drJPVSV;
        
        if (newvrt.trkAtVrt.size()==2){
          decor_N2Tpair(*GNNvertex)=newvrt.trkAtVrt.size();
        }
        
        ElementLink< xAOD::VertexContainer> linkVertex;
        linkVertex.setElement(GNNvertex);
        linkVertex.setStorableObject(*outVertexContainer);
        jetWriteDecorHandleVertexLink(*jet).push_back(linkVertex);
      
      }//end of Chi2 cut
      }//end of 2 Track requirement
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
