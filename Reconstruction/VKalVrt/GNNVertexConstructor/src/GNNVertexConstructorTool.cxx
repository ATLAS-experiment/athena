// Header include
#include "GNNVertexConstructor/GNNVertexConstructorTool.h"
//

#include "StoreGate/WriteDecorHandle.h"

namespace Rec {
    
    GNNVertexConstructorTool::GNNVertexConstructorTool(const std::string& type, const std::string& name, const IInterface* parent)
    : AthAlgTool(type,name,parent){

      declareInterface< IGNNVertexConstructorInterface >(this);
      ATH_MSG_DEBUG("GNNVertexConstructorTool constructor called");
    }   
     /* Destructor */
    
    GNNVertexConstructorTool::~GNNVertexConstructorTool(){

      ATH_MSG_DEBUG("GNNVertexConstructorTool destructor called");
    }

    StatusCode GNNVertexConstructorTool::initKey(const std::string &containerKey,
                              SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> &decokey) const {
     decokey = containerKey + decokey.key();
     ATH_MSG_DEBUG(" : " << decokey.key());
     ATH_CHECK(decokey.initialize(!containerKey.empty()));
     return StatusCode::SUCCESS;
    }



    StatusCode GNNVertexConstructorTool::initialize(){

      ATH_MSG_DEBUG("GNNVertexConstructor Tool in initialize()");

      ATH_CHECK(initKey(m_tracksKey, m_decorTrackKey));      

      //ATH_CHECK(m_decorTrackKey.initialize());

      return StatusCode::SUCCESS;
    }
    
    StatusCode GNNVertexConstructorTool::finalize(){

      ATH_MSG_DEBUG("GNNVertexConstructor Tool in finalize()");
    
      return StatusCode::SUCCESS;
    }


    unsigned int GNNVertexConstructorTool::addTwoNumbers( const unsigned int & NoOne, const unsigned int & NoTwo) const {
      unsigned int sum=NoOne+NoTwo;
      return sum;
    }

    StatusCode GNNVertexConstructorTool::decorateTracks( const xAOD::TrackParticleContainer* trkCont, const EventContext& ctx ) const {

      ATH_MSG_DEBUG("GNNVertexConstructor Tool decorating tracks");

      int sum = trkCont->size();
      ATH_MSG_DEBUG("Size is = " << sum );      

      SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::string> decorTrackKey(m_decorTrackKey, ctx );

      for ( auto track : *trkCont ){

        float pt = track->pt()/1000.;

        if(pt > 150){
    
          ATH_MSG_DEBUG("Track pt is = " << pt );
          decorTrackKey( *track ) = "pass";

        }
      }

      return StatusCode::SUCCESS;
    }

}  // end Rec namespace
