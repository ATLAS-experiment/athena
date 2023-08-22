// Headers
#include "GNNVertexConstructor/GNNVertexConstructorTool.h"
//Headers to Read & Write Decorations
#include "StoreGate/WriteDecorHandle.h"
#include "StoreGate/ReadDecorHandle.h"
#include "GaudiKernel/ITHistSvc.h"
#include "GaudiKernel/ConcurrencyFlags.h"
#include "vector"
#include "iostream"
#include "TH1.h"
#include "TH2.h"
#include "TTree.h"
#include "TMath.h"
#include "TFile.h"
#include "map"
#include "set"
#include "iterator"
#include "iostream"
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
      m_thePV(nullptr)
      
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
      
      //ATH_CHECK(m_inTrackLinkKey.initialize());
      //ATH_CHECK(m_jetContainerKey.initialize());
      ATH_CHECK( m_eventInfoKey.initialize());
    
    
      //Making a Vertex Container
      
 /*     const <xAOD::Vertex> GNNVertexContainer = nullptr;
      const <xAOD::Vertex> GNNVertexAuxContainer = nullptr;
    
      std::pair<xAOD::VertexContainer*, xAOD::VertexAuxContainer*> InDetAdaptiveMultiSecVtxFinderTool::doVertexing(
        const std::vector<Trk::ITrackLink*>& trackVector) {
        xAOD::VertexContainer* theVertexContainer = new xAOD::VertexContainer;
        xAOD::VertexAuxContainer* theVertexAuxContainer = new xAOD::VertexAuxContainer;
        theVertexContainer->setStore(theVertexAuxContainer);
        
*/
  
      return StatusCode::SUCCESS;
    }
  
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
    
    ATH_MSG_DEBUG("Reading a Decor in Jet");
    
    SG::ReadDecorHandle<xAOD::JetContainer, std::vector<ElementLink<DataVector<xAOD::TrackParticle_v1 > > > > readJetHandle_TL(m_jetReadKey_TL, ctx);
    SG::ReadDecorHandle<xAOD::JetContainer, std::vector<char,std::allocator<char> > >readJetHandle_TO(m_jetReadKey_TO, ctx);
    SG::ReadDecorHandle<xAOD::JetContainer, std::vector<char,std::allocator<char> > >readJetHandle_TV(m_jetReadKey_TV, ctx);
    
   
    //Create a map of track links and track vertexing values (Using mutlimap)

    std::multimap<int, ElementLink<DataVector<xAOD::TrackParticle_v1 > >  > VertexMap;    //empty Multi Vertex Map Container

    for (auto jet : *jetCont) {    //Loop over Jets
      VertexMap.clear();
      auto vertexCollection=readJetHandle_TV(*jet);
      auto trackCollection=readJetHandle_TL(*jet);
      
      ATH_MSG_DEBUG("New Jet");        
      int i=0;
      
      for (auto v: vertexCollection){
          //ATH_MSG_DEBUG(trackCollection[i]);
          VertexMap.insert(std::pair<int, ElementLink<DataVector<xAOD::TrackParticle_v1 > > >(v, (trackCollection[i]))); 
          i++;
      }
      std::multimap<int, ElementLink<DataVector<xAOD::TrackParticle_v1 > >>::iterator itr;
      ATH_MSG_DEBUG("Printing Map");
      
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

      
      auto GNNVTX=GNNVertexConstructorTool::vrtFitter(tmpVectxAOD, VertexMap);
      
      //std::unique_ptr<Trk::VxSecVertexInfo> res = std::make_unique<Trk::VxSecVertexInfo>(Trk::VxSecVertexInfo(GNNVTX));
     /* for (itr=VertexMap.begin(); itr != VertexMap.end(); ++itr){
      
        ATH_MSG_DEBUG("\t" << itr->first <<"\t"<< itr->second <<"\n");
      
      }*/
    
    }

    return StatusCode::SUCCESS;

    }
    
    //Vertex Fitting

  std::vector<xAOD::Vertex*> GNNVertexConstructorTool::vrtFitter( workVectorArrxAOD * xAODwrk, std::multimap<int, ElementLink<DataVector<xAOD::TrackParticle_v1 > >  > & vrt ) const{

    ATH_MSG_DEBUG("Vertex fitter called ");
    std::vector<xAOD::Vertex*>finalVertices(0);
    //Using the Multimap that has the grouping of the tracks to a single vertex
    //Vertexing tool is <Trk::TrkVKalVrtFitter> m_VrtFit
    
    
    //Retrieve all the tracks linked to a specific key in multimap
    //If key only has 1 track ignore??
    //With # tracks >1 perform a vertex fit
    
    //First need an initial vertex position (use estimVrtPos)
    //then set an approx vertex as a starting point (use setApproximateVertex)
    //Perform a fit (use VKalVrtFit)
    
    /*VKalVrtFit requirements
    
    
    */
   // Create the new container and its auxiliary store.
   //  auto GNNvertex = std::make_unique<xAOD::VertexContainer>();
   //  auto GNNvertexAux = std::make_unique<xAOD::AuxContainerBase>();
   //  GNNvertex->setStore (GNNvertexAux.get()); //< Connect the two
   
   
    xAOD::TrackParticle *Test =new xAOD::TrackParticle();
    ATH_MSG_DEBUG("GNNblah" );
    Test->setTime(0.5);
    
    xAOD::Vertex *GNNvertex = new xAOD::Vertex;
    ATH_MSG_DEBUG("GNNblah 2" );
    GNNvertex->x();
    //GNNvertex->setVertexType(xAOD::VxType::SecVtx);
    ATH_MSG_DEBUG("GNNblah 3" );
    
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
    
    ATH_MSG_DEBUG("Test 1");
    
    xAODwrk->tmpListTracks.clear();
    auto lastKey = (vrt.end())->first;          //returns next value after last key - easier for "for loop"
    
    for (int k=0; k<lastKey; ++k){
       
      if (vrt.count(k)>=2){
        //Need at least 2 tracks to perform a fit
        auto elements = vrt.equal_range(k);
        for (auto i = elements.first; i != elements.second; ++i){        //printing the map 
          //ATH_MSG_DEBUG(" test " << i->first << ": " << i->second << '\n');
          xAODwrk->listSelTracks.push_back(*(i->second));
          //GNNvertex->push_back(xAODwrk->listSelTracks);
          //(xAODwrk->listSelTracks).emplace(GNNvertex);
          ATH_MSG_DEBUG("jbfakjbf");
          
          auto trkLink= i->second;
          //ATH_MSG_DEBUG(*trkLink->type());
          //GNNvertex->addTrackAtVertex(trkLink, 1.);
          ATH_MSG_DEBUG("Test 2");
          } 
        
    
    
        //m_VrtFit->VKalVtrFit(*state, false);
      //auto nTracks =vrt.count(k);
      
      
     // std::vector<double> iniVrtPos=estimVrtPos(nTracks,listSelTracks,foundVrt2t);
      
      //m_VrtFit->setApproximateVertex(iniVrtPos[0], iniVrtPos[1], iniVrtPos[2], *state); /*Use as starting point*/
      
      //xAOD::Vertex* GNNVtx =new xAOD::Vertex();
      
    /*  for (auto *trk: xAODwrk->listSelTracks){
        ElementLink<xAOD::TrackParticleContainer> trkElementLink (*(xAODwrk->listSelTracks, trk->index());
        GNNvertex->addTrackAtVertex(trkElementLink, 1.);
        
      }*/
      
      ATH_MSG_DEBUG("Test 3");
      sc =(m_VrtFit->VKalVrtFit(xAODwrk->listSelTracks, neutralPartDummy,
                                             newvrt.vertex,     newvrt.vertexMom, newvrt.vertexCharge, newvrt.vertexCov,
                                             newvrt.chi2PerTrk, newvrt.trkAtVrt,  newvrt.chi2,
                                             *state, false));
      if( sc.isFailure() )           continue;
      
      /*if(foundVrts && foundVrts->vertices().size()){
         const std::vector<xAOD::Vertex*> vtmp=foundVrts->vertices();
         for(auto & iv :  vtmp) {
           GNNVtxs->push_back(iv);
           }*/
      //GNNVtxs->push_back(GNNVtx);
      //GNNVtx->setPosition(newvrt.vertex);
      //finalVertices.push_back(newvrt.vertex[0,0,0]);
      ATH_MSG_DEBUG("Found IniVertex="<<newvrt.vertex[0]<<", "<<newvrt.vertex[1]<<", "<<newvrt.vertex[2]);
      
      // Compatibility to the primary vertex.
      Amg::Vector3D vDist = newvrt.vertex ;//- m_thePV->position();
      ATH_MSG_DEBUG("Test 5");
      double vPos=(vDist.x()*newvrt.vertexMom.Px()+vDist.y()*newvrt.vertexMom.Py()+vDist.z()*newvrt.vertexMom.Pz())/newvrt.vertexMom.Rho();
      
      ATH_MSG_DEBUG("Test 4");
      GNNvertex->setVertexType(xAOD::VxType::SecVtx);
      GNNvertex->setPosition( newvrt.vertex);
      GNNvertex->setFitQuality(newvrt.chi2, 1);
      GNNvertex->auxdata<float>("mass")      = newvrt.vertexMom.M();
      GNNvertex->auxdata<float>("pT")        = newvrt.vertexMom.Perp();
      GNNvertex->auxdata<float>("charge")    = newvrt.vertexCharge;
      GNNvertex->auxdata<float>("vPos")      = vPos;
      GNNvertex->auxdata<bool> ("isFake")    = true;
      
      ATH_MSG_DEBUG("Test 6");
      //ATH_MSG_VERBOSE("with Chi2="<<newvrt.chi2<<" Ntrk="<<NPTR<<" trk1,2="<<newvrt.selTrk[0]<<", "<<newvrt.selTrk[1]);

      } 
       

      
    }
    ATH_MSG_DEBUG("Finished for loop of K");
    
    //ATH_CHECK (evtStore()->record(GNNvertex.release(), "GNNvertex"));
// Record the objects into the event store
//   ANA_CHECK (evtStore()->record (GNNVtxs.release(), "GNNVtxs"));
//   ANA_CHECK (evtStore()->record (GNNVtxsAux.release(), "GNNVtxsAux."));
   
    
    
    return finalVertices;
  }    

}  // end Rec namespace
