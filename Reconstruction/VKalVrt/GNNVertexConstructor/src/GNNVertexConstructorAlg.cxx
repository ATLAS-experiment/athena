#include "GNNVertexConstructor/GNNVertexConstructorAlg.h"

namespace Rec {

  GNNVertexConstructorAlg::GNNVertexConstructorAlg(const std::string& name, ISvcLocator* pSvcLocator) 
    : AthReentrantAlgorithm( name, pSvcLocator ),
    m_testTool("Rec::GNNVertexConstructorTool/testTool", this){

      declareProperty("TestTool",m_testTool, "The test Tool");
      
  }

  StatusCode GNNVertexConstructorAlg::initialize(){
  
    ATH_MSG_DEBUG("devAlg: In devAlg::initialize()");
    ATH_CHECK( m_testTool.retrieve() );
    
    ATH_CHECK(m_inTrackKey.initialize());

    return StatusCode::SUCCESS;

  }

  StatusCode GNNVertexConstructorAlg::finalize(){

    ATH_MSG_DEBUG("devAlg: In devAlg::finalize()");
  
    return StatusCode::SUCCESS;
  }

  StatusCode GNNVertexConstructorAlg::execute(const EventContext &ctx) const{

    ATH_MSG_DEBUG("devAlg: In devAlg::execute()");
  
  
    unsigned int sum = m_testTool -> addTwoNumbers(4, 9);
  
    ATH_MSG_DEBUG("SUM is = " << sum );


    SG::ReadHandle<xAOD::TrackParticleContainer> inTracks(m_inTrackKey, ctx);
    if (!inTracks.isValid()) {
      if (!inTracks.isValid()) ATH_MSG_ERROR("TrackParticle container not found: " << m_inTrackKey.key());
      return StatusCode::FAILURE;
    }


    ATH_CHECK(m_testTool->decorateTracks(*inTracks));

    return StatusCode::SUCCESS;

  }

}
