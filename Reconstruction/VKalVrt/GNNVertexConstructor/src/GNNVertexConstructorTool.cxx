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

namespace Rec {
    
    GNNVertexConstructorTool::GNNVertexConstructorTool(const std::string& type, const std::string& name, const IInterface* parent)
    : AthAlgTool(type,name,parent)
    //,      m_fillHist(true)
      
    {

      declareInterface< IGNNVertexConstructorInterface >(this);
      declareProperty("ReadKey", m_decorReadKey="InDetTrackParticles.passGNN");
      declareProperty("JetReadKey", m_readJetKey="BTagging_AntiKt4EMPFlowAuxDyn.TrackLinks");
      //declareProperty("FillHist",   m_fillHist, "Fill technical histograms"  );
      declareProperty("GNNTool",m_gnn_Tool, "The GNN Tool");
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
      
      ATH_CHECK(m_readJetKey.initialize());
      ATH_CHECK(m_decorReadKey.initialize());
      ATH_CHECK( m_gnn_Tool.retrieve() );
      
      //ATH_CHECK(m_inTrackLinkKey.initialize());
      //ATH_CHECK(m_jetContainerKey.initialize());
      ATH_CHECK(m_eventInfoKey.initialize());
    
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
      ATH_MSG_DEBUG("A jet decorated pt= " << jet->pt()/1000);
    /*  auto out = m_gnn_Tool->getDecoratorKeys();
      
      for (auto const& na : out){
      ATH_MSG_DEBUG("Outputs " << na);
    }*/
    }

    
    return StatusCode::SUCCESS;
    }
    
//Read Decoration from a Jet Container
    StatusCode GNNVertexConstructorTool::readDecorJet ( const xAOD::JetContainer* jetCont, const EventContext& ctx ) const{
    
    ATH_MSG_DEBUG("Reading a Decor in Jet");
    
    SG::ReadDecorHandle<xAOD::JetContainer, std::vector<ElementLink<DataVector<xAOD::TrackParticle_v1 > > > > readJetHandle(m_readJetKey, ctx);
    
    //Opening the TrackParticle Container
    

      
      
    for (auto jet : *jetCont){
      
        //auto link=readJetHandle(*jet);
        ATH_MSG_DEBUG("TrackLinks with *"<< readJetHandle(*jet) );
        auto trackCollection=readJetHandle(*jet);
        for (auto track : trackCollection){
          ATH_MSG_DEBUG("thingy "<< (*track)->pt()); 
        }
   }
    
   /* for (auto tl : *link){
    
      ATH_MSG_DEBUG("Track?????? " <<     
    }*/
    return StatusCode::SUCCESS;
    }
    

/*
The following functions will be used to perform the Union Find Algorithm
-> Make the edge scores symmetric
-> Run a single step of the union find alg
-> Run edge score symmetrication and union find returning new vertex indices

Following example from union_find.py on salt

Do i want new cxx + .h files for these and then call in?
*/   
/*
    StatusCode GNNVertexConstructorTool::EdgeScoreSym( const xAOD::JetContainer* jetCont, const EventContext& ctx ) const{
    
      
      ATH_MSG_DEBUG("Reading a Decor in Jet");
      SG::ReadDecorHandle<xAOD::JetContainer, float> readJetHandle(m_readJetKey, ctx); //Change read handle to get node numbers
      SG::ReadDecorHandle<xAOD::JetContainer, float> readJetHandle(m_readJetKey, ctx); //Change read handle to get edge numbers      
      
      //Remove jets without edges
      //if node number is greater than 1?
      
      //Need to find names
      node_numbers= decos from gnn
      edge_numbers= decos from gnn
      
      //Will be vectors
      auto EmptyJet =[](const std::float &s){
      return s.find_first_not_of(" \t")==std::float::npos;
      };
      
      node_numbers.erase(std::remove_if(node_numbers.begin(), node_numbers.end(), EmptyJet), node_numbers.end();
      edge_numbers.erase(std::remove_if(edge_numbers.begin(), edge_numbers.end(), EmptyJet), edge_numbers.end();
      
      std::vector<float> cum_score{};
      
      
      
      //Calculate cumulative edge numbers and offsets for symmetric index calculations
      
      //Calculate opposite edge indices (asssumes edges sorted by source->destination and vice versa)
      
      //Return the new edge scores 
      //what does torch.sigmoid do???
      
      
      ATH_MSG_DEBUG("Blah");  
      
      
    
    return StatusCode::SUCCESS;
    }
    



    StatusCode GNNVertexConstructorTool::UnionFindSingle( const xAOD::JetContainer* jetCont, const EventContext& ctx ) const{
    
      ATH_MSG_DEBUG("Reading a Decor in Jet");
      SG::ReadDecorHandle<xAOD::JetContainer, float> readJetHandle(m_readJetKey, ctx);
      
      
/*Run a single step of the union find algorithm.

Takes a score matrix with shape (edges in batch, 1) and a tensor with
the number of nodes in each graph of the batch as well as a tensor specifying
for which graphs the algorithm has already terminated. Returns updated vertex
index and algorithm termination tensors.
*/


//First need a for loop to go over all node numbers

//Create arrays with node indices for source & destination nodes of each edge

//Pick out minimum betwwen source and destination node ids, filter edges with scores <0.5

//Get the lowest node id for each node across all edges for each node

//Return the node indices and updates node indices     
      
      
 /*     
      ATH_MSG_DEBUG("Blah");
      
      
      
      
    
    return StatusCode::SUCCESS;
    }
        
    StatusCode GNNVertexConstructorTool::UnionFindAlg( const xAOD::JetContainer* jetCont, const EventContext& ctx ) const{
    
    //MAY NEED TO MOVE TO ALG CXX FILE
    //CAN CALL PREVIOUS FUNCTIONS WITHIN TOOL??????
    
    
    //This will run edge score symmetrization and union find
    
    //Wrapper functino which returns the reconstructed vertex indices in shape (nodes in batch, 1). Assumes mask of shape (batch, max_tracks)
    
    
    //if mask has shape  (batch, max_tracks, 1) then squeeze last dimension???
    
    //pad mask with additional track to avoid onnx error
    
    //symmetrize edge scores
    
    //update node asignemtns until no more changes occur
    
    //return the node indices unsqueezed
      ATH_MSG_DEBUG("Blah");
      
      
      
      
    
    return StatusCode::SUCCESS;
    }

*/




  
}  // end Rec namespace
