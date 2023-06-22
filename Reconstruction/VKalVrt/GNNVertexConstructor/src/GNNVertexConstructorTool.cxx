// Headers
#include "GNNVertexConstructor/GNNVertexConstructorTool.h"
//Headers to Read & Write Decorations
#include "StoreGate/WriteDecorHandle.h"
#include "StoreGate/ReadDecorHandle.h"





namespace Rec {
    
    GNNVertexConstructorTool::GNNVertexConstructorTool(const std::string& type, const std::string& name, const IInterface* parent)
    : AthAlgTool(type,name,parent){

      declareInterface< IGNNVertexConstructorInterface >(this);
      declareProperty("ReadKey", m_decorReadKey="InDetTrackParicles.passGNN");
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

//Initialize ---------------------------------------------------------------------------
    StatusCode GNNVertexConstructorTool::initialize(){

      ATH_MSG_DEBUG("GNNVertexConstructor Tool in initialize()");

      ATH_CHECK(initKey(m_tracksKey, m_decorTrackKey));      
      //ATH_CHECK(initReadKey(m_readKey, m_readDecorKey);
      
      
      ATH_CHECK(m_decorReadKey.initialize());

      return StatusCode::SUCCESS;
    }

//Finalize -------------------------------------------------------------------------------    
    StatusCode GNNVertexConstructorTool::finalize(){

      ATH_MSG_DEBUG("GNNVertexConstructor Tool in finalize()");
    
      return StatusCode::SUCCESS;
    }

//Will be called in the excute section in the Algorithm cxx file ------------------------------

//Dummy Tool that adds 2 numbers
    unsigned int GNNVertexConstructorTool::addTwoNumbers( const unsigned int & NoOne, const unsigned int & NoTwo) const {
      unsigned int sum=NoOne+NoTwo;
      return sum;
    }


//Decoration Tool that adds a decoration to the container --------------------------------------------------------------------
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
    
    
//Read a decoration tool from a container  ---------------------------------------------------------------------------    
    StatusCode GNNVertexConstructorTool::readDecorTracks( const xAOD::TrackParticleContainer* trkCont, const EventContext& ctx ) const {
    
    
      ATH_MSG_DEBUG("GNNVertexConstructor Tool reading decoratorations from a container");
      
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

}  // end Rec namespace

