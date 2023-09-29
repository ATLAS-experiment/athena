// Headers
#include "GNNVertexConstructor/GNNVertexConstructorTool.h"
//Headers to Read & Write Decorations
#include "StoreGate/WriteDecorHandle.h"
#include "StoreGate/ReadDecorHandle.h"
#include "GaudiKernel/ITHistSvc.h"
#include "GaudiKernel/ConcurrencyFlags.h"
#include "vector"
#include "iostream"
#include "TTree.h"
#include "TMath.h"
#include "TFile.h"
#include "map"
#include "set"
#include "iterator"
#include "AnalysisUtils/AnalysisMisc.h"
#include "GeoPrimitives/GeoPrimitivesHelpers.h"
#include "TrkVKalVrtFitter/TrkVKalVrtFitter.h"
#include "MVAUtils/BDT.h" 
#include "VxSecVertex/VxSecVertexInfo.h"
#include "boost/graph/bron_kerbosch_all_cliques.hpp"
#include "PathResolver/PathResolver.h"



//#include "alorgithm"


namespace Rec {
    
    GNNVertexConstructorTool::GNNVertexConstructorTool(const std::string& type, const std::string& name, const IInterface* parent)
    : AthAlgTool(type,name,parent),
      m_VrtFit("Trk::TrkVKalVrtFitter/VertexFitterTool",this),
      m_thePV(nullptr),
      m_calibFileName("myGNNDecoOutputNew.pool.root")
      //m_vertex_chi2 (nullptr)      
      
    //,      m_fillHist(true)
      
    {
      declareInterface< IGNNVertexConstructorInterface >(this);
      
      declareProperty("ReadKey", m_decorReadKey="InDetTrackParticles.passGNN");
      declareProperty("JetReadKey", m_jetReadKey_TL="BTagging_AntiKt4EMPFlowAuxDyn.TrackLinks");
      declareProperty("JetReadKey", m_jetReadKey_TO="BTagging_AntiKt4EMPFlowAuxDyn.track_origin");
      declareProperty("JetReadKey", m_jetReadKey_TV="BTagging_AntiKt4EMPFlowAuxDyn.track_vertexing");
      //declareProperty("FillHist",   m_fillHist, "Fill technical histograms"  );
      declareProperty("GNNTool",m_gnn_Tool, "The GNN Tool");
      declareProperty("VertexFitterTool", m_VrtFit, "The vertex fitting tool");
      
      declareProperty("calibFileName", m_calibFileName, " GNN vertex file" );

      //m_instanceName="Test-Plot";
      
      ATH_MSG_DEBUG("GNNVertexConstructorTool constructor called");
    }   
     
     /* Destructor */
    
    GNNVertexConstructorTool::~GNNVertexConstructorTool(){
      ATH_MSG_DEBUG("GNNVertexConstructorTool destructor called");
    }

//--------Initialize the decoration key
    StatusCode GNNVertexConstructorTool::initKey(const std::string &containerKey,
                              SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> &decokey) const {
     
     decokey = containerKey + decokey.key();
     ATH_MSG_DEBUG(" : " << decokey.key());
     ATH_CHECK(decokey.initialize(!containerKey.empty()));
     
     return StatusCode::SUCCESS;
    }

//---------Initialize 
    StatusCode GNNVertexConstructorTool::initialize(){

      ATH_MSG_DEBUG("GNNVertexConstructor Tool in initialize()");

      ATH_CHECK(initKey(m_tracksKey, m_decorTrackKey));      
      
      ATH_CHECK( m_jetReadKey_TV.initialize());
      ATH_CHECK( m_jetReadKey_TO.initialize());
      ATH_CHECK( m_jetReadKey_TL.initialize());
      ATH_CHECK( m_decorReadKey.initialize());
      ATH_CHECK( m_gnn_Tool.retrieve() );
      
      ATH_CHECK( m_extrapolator.retrieve() );
      ATH_CHECK( m_beamSpotKey.initialize());
      ATH_CHECK( m_VrtFit.retrieve() );
      ATH_CHECK( m_foundVerticesKey.initialize() );
      
      //ATH_CHECK(m_inTrackLinkKey.initialize());
      //ATH_CHECK(m_jetContainerKey.initialize());
      ATH_CHECK( m_eventInfoKey.initialize());
      
      //ATH_CHECK( book (TH1F("Vertex_chi2", "Vertex_chi2", 20, 0, 500)));
      
  
      ITHistSvc*     hist_root=0;
      StatusCode sc = service( "THistSvc", hist_root); 
      if( sc.isFailure() )  ATH_MSG_DEBUG("Could not find THistSvc service");
       else                  ATH_MSG_DEBUG("GNNCONST-Tool Histograms found");
      std::string histDir;
      histDir="/myGNNVertex/";
      
      m_h = std::make_unique<Hists>();
      ATH_CHECK( m_h->book (*hist_root, histDir) );
      
      m_w_1 = 1.;
      
/*           std::string rootFilePath = PathResolver::find_calib_file("GNNBuild/run/"+m_calibFileName);
     TFile* rootFile = TFile::Open(rootFilePath.c_str(), "READ");    
     if (!rootFile) {
        ATH_MSG_FATAL("Could not retrieve root file: " << m_calibFileName);
        return StatusCode::FAILURE;
     }
     TTree * training = (TTree*)rootFile->Get("BDT");
*/
      
      
      return StatusCode::SUCCESS;
    }
    
    
//------------Histograms
    StatusCode GNNVertexConstructorTool::Hists::book (ITHistSvc& histSvc,
                                                  const std::string& histDir)
  {
    
    m_vertex_chi2=new TH1F("Vertex_chi2", "Vertex_chi2", 20, 0, 500);
    
    ATH_CHECK( histSvc.regHist(histDir+"vertex_chi2", m_vertex_chi2) );
    
    m_tuple = new TTree("GNNVertices","GNNVertices");
    
    ATH_CHECK( histSvc.regTree(histDir, m_tuple) );
    
    m_curTup=new DevTuple();
    m_tuple->Branch("Chi2",       &m_curTup->chi2,    "Chi2/I");


    return StatusCode::SUCCESS;
  }
  
//Finalize     
    StatusCode GNNVertexConstructorTool::finalize(){

      ATH_MSG_DEBUG("GNNVertexConstructor Tool in finalize()");
    
      return StatusCode::SUCCESS;
    }


//-------------Decorating the jets using the GNN
    StatusCode GNNVertexConstructorTool::GNNDecoJet ( const xAOD::JetContainer* jetCont, const EventContext& ctx ) const{
    
    ATH_MSG_DEBUG("Using GNN within Tool");
    
    ATH_MSG_DEBUG(jetCont->size());
    // apply the GNNTool to the jets
    if (jetCont->size()!=0){
      ATH_MSG_DEBUG(jetCont->size() << "did it work");
      for (auto jet : *jetCont)
      {
        //ATH_MSG_INFO("Jet size " << (*jet)->size());
        m_gnn_Tool->decorate(*jet);
      }
    }
    else
    {
    ATH_MSG_DEBUG("No Jets");
    }
    return StatusCode::SUCCESS;
    }
    
//------------Read Decoration from a Jet Container
    StatusCode GNNVertexConstructorTool::readDecorJet ( const xAOD::JetContainer* jetCont, const EventContext& ctx ) const{
    
    //ATH_MSG_DEBUG("Reading a Decor in Jet");
    
    SG::ReadDecorHandle<xAOD::JetContainer, std::vector<ElementLink<DataVector<xAOD::TrackParticle_v1 > > > > readJetHandle_TL(m_jetReadKey_TL, ctx);
    SG::ReadDecorHandle<xAOD::JetContainer, std::vector<char,std::allocator<char> > >readJetHandle_TO(m_jetReadKey_TO, ctx);
    SG::ReadDecorHandle<xAOD::JetContainer, std::vector<char,std::allocator<char> > >readJetHandle_TV(m_jetReadKey_TV, ctx);
    
   
    //Create a map of track links and track vertexing values (Using mutlimap)
    std::multimap<int, ElementLink<DataVector<xAOD::TrackParticle_v1 > >  > VertexMap;    //empty Multi Vertex Map Container
    
    // Create the new container and its auxiliary store.
    auto GNNvertexContainer      = std::make_unique<xAOD::VertexContainer>();
    auto GNNvertexAuxContainer   = std::make_unique<xAOD::AuxContainerBase>();
    GNNvertexContainer->setStore (GNNvertexAuxContainer.get()); //< Connect the two

    //xAOD::VertexContainer* GNNvertexContainer = new xAOD::VertexContainer;
    //xAOD::VertexAuxContainer* GNNvertexAuxContainer = new xAOD::VertexAuxContainer;
    //GNNvertexContainer->setStore(GNNvertexAuxContainer);

   
    for (auto jet : *jetCont) {    //Loop over Jets
      VertexMap.clear();
      auto vertexCollection=readJetHandle_TV(*jet);
      auto trackCollection=readJetHandle_TL(*jet);
      
      //ATH_MSG_DEBUG("New Jet");        
      int i=0;
      
      for (auto v: vertexCollection){
          //ATH_MSG_DEBUG(trackCollection[i]);
          VertexMap.insert(std::pair<int, ElementLink<DataVector<xAOD::TrackParticle_v1 > > >(v, (trackCollection[i]))); 
          i++;
      }
      std::multimap<int, ElementLink<DataVector<xAOD::TrackParticle_v1 > >>::iterator itr;
      //ATH_MSG_DEBUG("Printing Map");
      
      workVectorArrxAOD * tmpVectxAOD=new workVectorArrxAOD();
//    tmpVectxAOD->inpTrk.resize(inpTrk.size());
//    tmpVectxAOD->inpTrk.resize(VertexMap.size());
//    std::copy(inpTrk.begin(),inpTrk.end(), tmpVectxAOD->inpTrk.begin());
      SG::ReadCondHandle<InDet::BeamSpotData> beamSpotHandle { m_beamSpotKey, ctx };
      tmpVectxAOD->beamX=beamSpotHandle->beamPos().x();
      tmpVectxAOD->beamY=beamSpotHandle->beamPos().y();
      tmpVectxAOD->beamZ=beamSpotHandle->beamPos().z();
      tmpVectxAOD->tanBeamTiltX=tan(beamSpotHandle->beamTilt(0));
      tmpVectxAOD->tanBeamTiltY=tan(beamSpotHandle->beamTilt(1));
      
      
      auto GNNVTX=GNNVertexConstructorTool::vrtFitter(tmpVectxAOD, VertexMap, ctx, GNNvertexContainer);
      
      delete tmpVectxAOD;
        
    }
    
    ATH_MSG_INFO("Vertex Container " << GNNvertexContainer->size());
    StatusCode sc;
      
      
    sc= (evtStore()->record (GNNvertexContainer.release(), "GNNVtxs"));
    sc= (evtStore()->record (GNNvertexAuxContainer.release(), "GNNVtxsAux."));
    
    ATH_MSG_INFO("Container WRitten?");
    
     return StatusCode::SUCCESS;

    }
    
//--------Vertex Fitting

  std::vector<xAOD::Vertex*> GNNVertexConstructorTool::vrtFitter( workVectorArrxAOD * xAODwrk, 
                                                                  std::multimap<int, ElementLink<DataVector<xAOD::TrackParticle_v1 > >  > & vrt, 
                                                                  const EventContext& ctx, 
                                                                  std::unique_ptr<xAOD::VertexContainer>& VtxCont ) const{

    ATH_MSG_DEBUG("Vertex fitter called ");
    
    std::vector<xAOD::Vertex*>finalVertices(0);
    
    ATH_MSG_DEBUG("Added Vertexes 0= " << VtxCont->size());
       
    /*for (auto *vertex : *m_vertexTES ){
      if( xAOD::VxType::PriVtx != vertex->vertexType() ) continue;
      
      m_thePV = vertex;
    }*/
    
    std::unique_ptr<std::vector<WrkVrt>> wrkVrtSet = std::make_unique<std::vector<WrkVrt>>();
    //int inpNPart=xAODwrk->inpTrk.size();
    //int inpNPart=xAODwrk->vrt.size();
    WrkVrt newvrt; newvrt.Good=true;
    std::unique_ptr<Trk::IVKalState> state = m_VrtFit->makeState();
    std::vector<const xAOD::NeutralParticle*> neutralPartDummy(0);
    
    StatusCode sc;
    
    xAODwrk->tmpListTracks.clear();
    auto lastKey = (vrt.end())->first;          //returns next value after last key - easier for "for loop"
    
    for (int k=0; k<lastKey; ++k){
       
      if (vrt.count(k)>=2){
        //Need at least 2 tracks to perform a fit
        auto elements = vrt.equal_range(k);
        for (auto i = elements.first; i != elements.second; ++i){        //printing the map 
          xAODwrk->listSelTracks.push_back(*(i->second));
          auto trkLink= i->second;
          } 
      
      sc =(m_VrtFit->VKalVrtFit(xAODwrk->listSelTracks, neutralPartDummy,
                                             newvrt.vertex,     newvrt.vertexMom, newvrt.vertexCharge, newvrt.vertexCov,
                                             newvrt.chi2PerTrk, newvrt.trkAtVrt,  newvrt.chi2,
                                             *state, false));
      if( sc.isFailure() )           continue;
      
      ATH_MSG_DEBUG("Found IniVertex="<<newvrt.vertex[0]<<", "<<newvrt.vertex[1]<<", "<<newvrt.vertex[2]);
      
      
      float r= sqrt(newvrt.vertex[0]*newvrt.vertex[0]+newvrt.vertex[1]*newvrt.vertex[1]);
      
      
      //m_vertex_pos->Fill(newvrt.vertex[0]);
      Hists& h = getHists();
      h.m_tuple->Fill();
      h.m_vertex_chi2->Fill(newvrt.chi2, m_w_1);
      
      
//----------Compatibility to the primary vertex. 

      Amg::Vector3D vDist = newvrt.vertex ;// - m_thePV->position();
      
      double vPos=(vDist.x()*newvrt.vertexMom.Px()+vDist.y()*newvrt.vertexMom.Py()+vDist.z()*newvrt.vertexMom.Pz())/newvrt.vertexMom.Rho();
      
      xAOD::Vertex *GNNvertex = new xAOD::Vertex;
      
      
     // xAOD::Vertex * tmpVertex = nullptr;

      //tmpVertex = m_VrtFit->makeXAODVertex( newvrt.vertex, newvrt.chi2PerTrk, newvrt.trkAtVrt, newvrt.chi2, state );
      
      VtxCont->emplace_back(GNNvertex);
      
      GNNvertex->setVertexType(xAOD::VxType::SecVtx);
      GNNvertex->setPosition( newvrt.vertex);
      GNNvertex->setFitQuality(newvrt.chi2, 1);
      GNNvertex->auxdata<float>("mass")      = newvrt.vertexMom.M();
      GNNvertex->auxdata<float>("pT")        = newvrt.vertexMom.Perp();
      GNNvertex->auxdata<float>("charge")    = newvrt.vertexCharge;
      GNNvertex->auxdata<float>("vPos")      = vPos;
      GNNvertex->auxdata<bool> ("isFake")    = true;
      
      }  
    }
    return finalVertices;
  } 
  
      GNNVertexConstructorTool::Hists&
      GNNVertexConstructorTool::getHists() const
      {
        // We earlier checked that no more than one thread is being used.
        Hists* h ATLAS_THREAD_SAFE = m_h.get();
      return *h;
  }   

}  // end Rec namespace
