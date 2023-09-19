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


//#include "alorgithm"


namespace Rec {
    
    GNNVertexConstructorTool::GNNVertexConstructorTool(const std::string& type, const std::string& name, const IInterface* parent)
    : AthAlgTool(type,name,parent),
      m_VrtFit("Trk::TrkVKalVrtFitter/VertexFitterTool",this),
      m_thePV(nullptr),
      m_vertex_pos (nullptr)      
      
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
      
      //declareProperty("
      //m_instanceName="Test-Plot";
      
      ATH_MSG_DEBUG("GNNVertexConstructorTool constructor called");
    }   
     
     /* Destructor */
    
    GNNVertexConstructorTool::~GNNVertexConstructorTool(){

      ATH_MSG_DEBUG("GNNVertexConstructorTool destructor called");
    }

//Initialize the decoration key

    StatusCode GNNVertexConstructorTool::initKey(const std::string &containerKey,
                              SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> &decokey) const {
     
     decokey = containerKey + decokey.key();
     ATH_MSG_DEBUG(" : " << decokey.key());
     ATH_CHECK(decokey.initialize(!containerKey.empty()));
     
     return StatusCode::SUCCESS;
    }

//Initialize 
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
      

/*      ITHistSvc*     hist_root=0;
      StatusCode sc = service( "THistSvc", hist_root); 
      std::string histDir;
      histDir="GNNVertex/";
      
      m_h = std::make_unique<Hists>();
      ATH_CHECK( m_h->book (*hist_root, histDir) );
*/



  
  
      return StatusCode::SUCCESS;
    }
    
    
/*    //Histograms
    StatusCode GNNVertexConstructorTool::Hists::book (ITHistSvc& histSvc,
                                                  const std::string& histDir)
  {
    
    m_vertex_pos=new TH2F ("vertex_pos", "vertex_pos", 10, 0., 500, 10, 0., 1000);
    
    ATH_CHECK( histSvc.regHist(histDir+"vertex_pos", m_vertex_pos) );
  }*/
  
  
  
  
//Finalize     
    StatusCode GNNVertexConstructorTool::finalize(){

      ATH_MSG_DEBUG("GNNVertexConstructor Tool in finalize()");
    
      return StatusCode::SUCCESS;
    }

//Will be called in the excute section in the Algorithm cxx file 

//Dummy Tool that adds 2 numbers
    unsigned int GNNVertexConstructorTool::addTwoNumbers( const unsigned int & NoOne, const unsigned int & NoTwo) const {
      unsigned int sum=NoOne+NoTwo;
      return sum;
    }


//Decoration Tool that adds a decoration to the container 
    StatusCode GNNVertexConstructorTool::decorateTracks( const xAOD::TrackParticleContainer* trkCont, const EventContext& ctx ) const {

      ATH_MSG_DEBUG("GNNVertexConstructor Tool decorating tracks");

      int sum = trkCont->size();
      ATH_MSG_DEBUG("Size is = " << sum );      

      SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::string> decorTrackKey(m_decorTrackKey, ctx );

      for ( auto track : *trkCont ){

        float pt = track->pt()/1000.;

        if(pt > 150){
    
          ATH_MSG_DEBUG("Track pt is = " << pt );
          ATH_MSG_DEBUG("Decorator added!!!!");
          decorTrackKey( *track ) = "pass";

        }
      }

      return StatusCode::SUCCESS;
    }
    
    
//Read a decoration tool from a Track Particle Container
    StatusCode GNNVertexConstructorTool::readDecorTracks( const xAOD::TrackParticleContainer* trkCont, const EventContext& ctx ) const {
    
    
      ATH_MSG_DEBUG("GNNVertexConstructor Tool reading decorations from a container");
      
      SG::ReadDecorHandle<xAOD::TrackParticleContainer, std::string> readTrackKey(m_decorReadKey, ctx);
      
      for ( auto track : *trkCont ){

        float pt = track->pt()/1000.;

        if(pt > 150){
          ATH_MSG_DEBUG("Contains a decorator");
          ATH_MSG_DEBUG("Decorator is " << readTrackKey( *track ));
        }
       
      
      }
      return StatusCode::SUCCESS;
    }
//Decorating the jets using the GNN
    StatusCode GNNVertexConstructorTool::GNNDecoJet ( const xAOD::JetContainer* jetCont, const EventContext& ctx ) const{
    
    ATH_MSG_DEBUG("Using GNN within Tool");
    
    // apply the GNNTool to the jets
    for (auto jet : *jetCont)
    {
      m_gnn_Tool->decorate(*jet);
    }

    
    return StatusCode::SUCCESS;
    }
    
//Read Decoration from a Jet Container
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
 //     tmpVectxAOD->inpTrk.resize(inpTrk.size());
   //   tmpVectxAOD->inpTrk.resize(VertexMap.size());
     // std::copy(inpTrk.begin(),inpTrk.end(), tmpVectxAOD->inpTrk.begin());
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
   
     return StatusCode::SUCCESS;

    }
    
    //Vertex Fitting

  std::vector<xAOD::Vertex*> GNNVertexConstructorTool::vrtFitter( workVectorArrxAOD * xAODwrk, 
                                                                  std::multimap<int, ElementLink<DataVector<xAOD::TrackParticle_v1 > >  > & vrt, 
                                                                  const EventContext& ctx, 
                                                                  std::unique_ptr<xAOD::VertexContainer>& VtxCont ) const{

    ATH_MSG_DEBUG("Vertex fitter called ");
    
    std::vector<xAOD::Vertex*>finalVertices(0);
    
    ATH_MSG_DEBUG("Added Vertexes 0= " << VtxCont->size());
    
         
    
    
/*    xAOD::VertexContainer *TestVertexContainer( nullptr );
    ATH_CHECK( evtStore()->retrieve( TestVertexContainer, "BTagging_AntiKt4EMPFlowSecVtx" ) );
    
    
     
    xAOD::TrackParticle *Test =new xAOD::TrackParticle;
    ATH_MSG_DEBUG("GNNblah" );
    //Test->setTime(0.5);
    
    xAOD::Vertex *GNNvertex = new xAOD::Vertex;
    ATH_MSG_DEBUG("GNNblah 2" );
    TestVertexContainer->emplace_back(GNNvertex);
    
    
    GNNvertex->setX(59.6);
    ATH_MSG_DEBUG(GNNvertex->x());
    //GNNvertex->setVertexType(xAOD::VxType::SecVtx);
    ATH_MSG_DEBUG("GNNblah 3" );*/
    
  /*  ATH_CHECK(evtStore()->retrieve( m_vertexTES, "PrimaryVertices"));
    
    if( sc.isFailure()  ||  !m_vertexTES ) {
       ATH_MSG_WARNING("No xAOD vertex container found in TDS"); 
       return StatusCode::SUCCESS;
     }  
     else {
     }
    
    for (auto *vertex : *m_vertexTES ){
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
          //ATH_MSG_DEBUG(" test " << i->first << ": " << i->second << '\n');
          xAODwrk->listSelTracks.push_back(*(i->second));
          
          
          auto trkLink= i->second;
          //ATH_MSG_DEBUG(*trkLink->type());
          //GNNvertex->addTrackAtVertex(trkLink, 1.);
          
          } 
        
    

      
      
      sc =(m_VrtFit->VKalVrtFit(xAODwrk->listSelTracks, neutralPartDummy,
                                             newvrt.vertex,     newvrt.vertexMom, newvrt.vertexCharge, newvrt.vertexCov,
                                             newvrt.chi2PerTrk, newvrt.trkAtVrt,  newvrt.chi2,
                                             *state, false));
      if( sc.isFailure() )           continue;
      
      ATH_MSG_DEBUG("Found IniVertex="<<newvrt.vertex[0]<<", "<<newvrt.vertex[1]<<", "<<newvrt.vertex[2]);
      
      
      float r= sqrt(newvrt.vertex[0]*newvrt.vertex[0]+newvrt.vertex[1]*newvrt.vertex[1]);
      
      
      //m_vertex_pos->Fill(newvrt.vertex[0]);
      
      //hist("Vertex_chi2")->Fill(r, newvrt.chi2);
      // Compatibility to the primary vertex.
      Amg::Vector3D vDist = newvrt.vertex ;// - m_thePV->position();
      
      double vPos=(vDist.x()*newvrt.vertexMom.Px()+vDist.y()*newvrt.vertexMom.Py()+vDist.z()*newvrt.vertexMom.Pz())/newvrt.vertexMom.Rho();
      
      xAOD::Vertex *GNNvertex = new xAOD::Vertex;
      VtxCont->emplace_back(GNNvertex);
      GNNvertex->setVertexType(xAOD::VxType::SecVtx);
      GNNvertex->setPosition( newvrt.vertex);
      GNNvertex->setFitQuality(newvrt.chi2, 1);
      GNNvertex->auxdata<float>("mass")      = newvrt.vertexMom.M();
      GNNvertex->auxdata<float>("pT")        = newvrt.vertexMom.Perp();
      GNNvertex->auxdata<float>("charge")    = newvrt.vertexCharge;
      GNNvertex->auxdata<float>("vPos")      = vPos;
      GNNvertex->auxdata<bool> ("isFake")    = true;
      
      ATH_MSG_DEBUG("Test 6");
      
      } 
      
    }
   
    return finalVertices;
  }    

}  // end Rec namespace
