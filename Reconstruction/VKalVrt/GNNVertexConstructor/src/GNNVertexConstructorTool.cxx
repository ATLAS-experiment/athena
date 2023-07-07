// Headers
#include "GNNVertexConstructor/GNNVertexConstructorTool.h"
//Headers to Read & Write Decorations
#include "StoreGate/WriteDecorHandle.h"
#include "StoreGate/ReadDecorHandle.h"
#include "GaudiKernel/ITHistSvc.h"
#include "GaudiKernel/ConcurrencyFlags.h"
#include "TH1.h"
#include "TH2.h"
#include "TTree.h"
#include "TMath.h"
#include "TFile.h"

namespace Rec {
    
    GNNVertexConstructorTool::GNNVertexConstructorTool(const std::string& type, const std::string& name, const IInterface* parent)
    : AthAlgTool(type,name,parent),
      m_fillHist(true)
    {

      declareInterface< IGNNVertexConstructorInterface >(this);
      declareProperty("ReadKey", m_decorReadKey="InDetTrackParticles.passGNN");
      declareProperty("JetReadKey", m_readJetKey="BTagging_AntiKt4EMPFlowAuxDyn.pb");
      declareProperty("FillHist",   m_fillHist, "Fill technical histograms"  );
      
      m_instanceName="Test-Plot";
      
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
      
      ITHistSvc*     hist_root=0;
       if(m_fillHist){
       if (Gaudi::Concurrency::ConcurrencyFlags::numThreads() > 1) {
         ATH_MSG_FATAL("Filling histograms not supported in MT jobs.");
         return StatusCode::FAILURE;
       }

       StatusCode sc = service( "THistSvc", hist_root); 
       if( sc.isFailure() )  ATH_MSG_DEBUG("Could not find THistSvc service");
       else                  ATH_MSG_DEBUG("NewVrtSecInclusiveTool Histograms found");
       std::string histDir;
       histDir="run/"+m_instanceName+"/";

       m_h = std::make_unique<Hists>();
       ATH_CHECK( m_h->book (*hist_root, histDir) );

       m_w_1 = 1.;
     }


      //ANA_CHECK (book (TH1F ("PB scores from GNN", "PB scores from GNN", 10, -10, 10))); // pb scores
      
      return StatusCode::SUCCESS;
    }
   
  StatusCode GNNVertexConstructorTool::Hists::book (ITHistSvc& histSvc,
                                                  const std::string& histDir)
  {
    m_hb_pb_score = new TH1F("pbScoreGNN","GNNPBscore",50,0.0,1.0);

    ATH_CHECK( histSvc.regHist(histDir+"pbScoreGNN", m_hb_pb_score) );




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

//Read Decoration from a Jet Container
    StatusCode GNNVertexConstructorTool::readDecorJet ( const xAOD::JetContainer* jetCont, const EventContext& ctx ) const{
    
    ATH_MSG_DEBUG("Reading a Decor in Jet");
    
    SG::ReadDecorHandle<xAOD::JetContainer, float> readJetHandle(m_readJetKey, ctx);
    
    for (auto jet : *jetCont){
    
     ATH_MSG_DEBUG("pb score = "<< readJetHandle(*jet));
    // h.m_pb_score->Fill(readJetHandle(*jet));
     if(m_fillHist){
      Hists& h = getHists();
      ATH_MSG_DEBUG("Plot");
      h.m_hb_pb_score->Fill(readJetHandle(*jet));
      //h.m_hb_pb_score->Draw();
    };
     //h.m_pb_score ->Fill(readJetHandle(*jet));
    }
    
    return StatusCode::SUCCESS;
    }





  GNNVertexConstructorTool::Hists&
  GNNVertexConstructorTool::getHists() const
  {
    // We earlier checked that no more than one thread is being used.
    Hists* h ATLAS_THREAD_SAFE = m_h.get();
    return *h;
  }

  
}  // end Rec namespace