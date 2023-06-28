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

namespace Rec {

  GNNVertexConstructorAlg::GNNVertexConstructorAlg(const std::string& name, ISvcLocator* pSvcLocator) 
    : AthReentrantAlgorithm( name, pSvcLocator ),
    m_testTool("Rec::GNNVertexConstructorTool/testTool", this), m_props() {
      
      declareProperty("nnFile", m_props.nnFile, "the path to the netowrk file used to run inference");
      declareProperty("TestTool",m_testTool, "The test Tool");
      declareProperty("GNNTool",m_gnn_Tool, "The GNN Tool");
     }

//Initialize  ---------------------------------------------------------------
  StatusCode GNNVertexConstructorAlg::initialize(){
  
    ATH_MSG_DEBUG("devAlg: In devAlg::initialize()");
    ATH_CHECK( m_testTool.retrieve() );
    ATH_CHECK( m_gnn_Tool.retrieve() );
    
    ATH_CHECK( m_inTrackKey.initialize() );
    ATH_CHECK(m_jetContainerKey.initialize());
    ATH_CHECK(m_eventInfoKey.initialize());

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
  
  
    unsigned int sum = m_testTool -> addTwoNumbers(4, 9);
  
    ATH_MSG_DEBUG("SUM is = " << sum );

    //Opening the TrackParticle Container
    
    SG::ReadHandle<xAOD::TrackParticleContainer> inTracks(m_inTrackKey, ctx);
    if (!inTracks.isValid()) {
      if (!inTracks.isValid()) ATH_MSG_ERROR("TrackParticle container not found: " << m_inTrackKey.key());
      return StatusCode::FAILURE;
    }

    //Calling the tools to write decorations and read the the decoration
    ATH_CHECK(m_testTool->decorateTracks(inTracks.ptr(), ctx));
    
    ATH_CHECK(m_testTool->readDecorTracks(inTracks.ptr(), ctx));
    
    
    //Using the GNN Tool to decorate and then read the decorations
    //Using Model /cvmfs/atlas.cern.ch/repo/sw/database/GroupData/dev/BTagging/20230608/gn2v00/antikt4empflow/network.onnx
    //Unsure how to implement model


    // container we read in
    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_eventInfoKey, ctx);
    ATH_CHECK(eventInfo.isValid());

    // get the jets
    SG::ReadHandle<ConstDataVector<xAOD::JetContainer>> jetHandle(
								  m_jetContainerKey, ctx);
    ATH_CHECK(jetHandle.isValid());

    ConstDataVector<xAOD::JetContainer> jets = *jetHandle;

    // apply the GNNTool to the jets
    for (const xAOD::Jet *jet : jets)
    {
      m_gnn_Tool->decorate(*jet);
      ATH_MSG_DEBUG("A jet decorated");
    }
    
    for (const xAOD::Jet *jet : jets)
      {
	m_gnn_Tool->decorateWithDefaults(*jet);
	ATH_MSG_DEBUG("Deco with Default");
      }

    ATH_MSG_DEBUG("Jet should have been decorated");
        
    return StatusCode::SUCCESS;

  }

}
