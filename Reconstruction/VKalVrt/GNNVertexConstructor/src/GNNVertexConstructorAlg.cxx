#include "GNNVertexConstructor/GNNVertexConstructorAlg.h"
#include <AthContainers/ConstDataVector.h>
#include "AthContainers/AuxElement.h"
#include "FourMomUtils/xAODP4Helpers.h"
#include "Math/GenVector/VectorUtil.h"
#include "StoreGate/ReadDecorHandle.h"
#include <AthContainers/ConstDataVector.h>
#include <xAODJet/JetContainer.h>
#include <xAODTracking/TrackParticleContainer.h>
#include <xAODTracking/VertexContainer.h>
//#include "GaudiKernel/ITHistSvc.h"


namespace Rec {

  GNNVertexConstructorAlg::GNNVertexConstructorAlg(const std::string& name, ISvcLocator* pSvcLocator) 
    : AthReentrantAlgorithm( name, pSvcLocator ),
      m_VtxTool("Rec::GNNVertexConstructorTool/VtxTool", this), m_props() {
      
      declareProperty("nnFile", m_props.nnFile, "the path to the netowrk file used to run inference");
      declareProperty("VtxTool",m_VtxTool, "The GNN Vtxing Tool");
      }

//Initialize  ---------------------------------------------------------------
  StatusCode GNNVertexConstructorAlg::initialize(){
  
    ATH_MSG_DEBUG("devAlg: In devAlg::initialize()");
    
    //Retrieving needed keys
    ATH_CHECK( m_VtxTool.retrieve() );
    
    //Initializing Keys
    ATH_CHECK( m_jetReadKey.initialize() );
    ATH_CHECK( m_inTrackKey.initialize() );
    
    ATH_MSG_DEBUG("Initialize bTagging Tool (GNN) from: " + m_props.nnFile);
    
   
    
    return StatusCode::SUCCESS;

  }

//Finalize -----------------------------------------------------------------------
  StatusCode GNNVertexConstructorAlg::finalize(){

    ATH_MSG_DEBUG("devAlg: In devAlg::finalize()");
  
    return StatusCode::SUCCESS;
  }

//Execute -----------------------------------------------------------------------------
  StatusCode GNNVertexConstructorAlg::execute(const EventContext &ctx) const{

    ATH_MSG_DEBUG("devAlg: In devAlg::execute()");
  
    //Opening the TrackParticle Container
    
    SG::ReadHandle<xAOD::TrackParticleContainer> inTracks(m_inTrackKey, ctx);
    if (!inTracks.isValid()) {
      if (!inTracks.isValid()) ATH_MSG_ERROR("TrackParticle container not found: " << m_inTrackKey.key());
      return StatusCode::FAILURE;
    }

    //Calling the tools to write decorations and read the the decoration
    ATH_CHECK(m_VtxTool->decorateTracks(inTracks.ptr(), ctx));
    
    ATH_CHECK(m_VtxTool->readDecorTracks(inTracks.ptr(), ctx));
    
    
    SG::ReadHandle<xAOD::JetContainer> jetsDecoHandle(m_jetReadKey, ctx);
    ATH_CHECK(jetsDecoHandle.isValid());
    
   
    ATH_CHECK(m_VtxTool->GNNDecoJet(jetsDecoHandle.ptr(), ctx));
    ATH_CHECK(m_VtxTool->readDecorJet(jetsDecoHandle.ptr(), ctx));
    //ATH_CHECK(m_VtxTool->vrtFitter(inTracks.ptr(), ctx)); 
    
    return StatusCode::SUCCESS;

  }

}
