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
      declareProperty("GNNTool",m_gnn_Tool, "The GNN Tool");
     //declareProperty("GNNJetRead", m_jetReadKey="pb.pb");
     }

//Initialize  ---------------------------------------------------------------
  StatusCode GNNVertexConstructorAlg::initialize(){
  
    ATH_MSG_DEBUG("devAlg: In devAlg::initialize()");
    
    //Retriecing needed keys
    ATH_CHECK( m_VtxTool.retrieve() );
    ATH_CHECK( m_gnn_Tool.retrieve() );
    
    //m_jetReadKey=m_jetContainerName+"."+m_jetReadKey.key();
    
    //Initializing Keys
    ATH_CHECK( m_jetReadKey.initialize() );
    ATH_CHECK( m_inTrackKey.initialize() );
    ATH_CHECK(m_jetContainerKey.initialize());
    ATH_CHECK(m_eventInfoKey.initialize());

    ATH_MSG_DEBUG("Initialize bTagging Tool (GNN) from: " + m_props.nnFile);
    
    //ANA_CHECK (book(TH1F ("PB scores from GNN", "PB scores from GNN", 10, -10, 10))); // pb scores

    
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
  
  
    unsigned int sum = m_VtxTool -> addTwoNumbers(4, 9);
  
    ATH_MSG_DEBUG("SUM is = " << sum );

    //Opening the TrackParticle Container
    
    SG::ReadHandle<xAOD::TrackParticleContainer> inTracks(m_inTrackKey, ctx);
    if (!inTracks.isValid()) {
      if (!inTracks.isValid()) ATH_MSG_ERROR("TrackParticle container not found: " << m_inTrackKey.key());
      return StatusCode::FAILURE;
    }

    //Calling the tools to write decorations and read the the decoration
    ATH_CHECK(m_VtxTool->decorateTracks(inTracks.ptr(), ctx));
    
    ATH_CHECK(m_VtxTool->readDecorTracks(inTracks.ptr(), ctx));
    
    
    //Using the GNN Tool to decorate and then read the decorations
    
    // container we read in
    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_eventInfoKey, ctx);
    ATH_CHECK(eventInfo.isValid());

    // get the jets
    SG::ReadHandle<ConstDataVector<xAOD::JetContainer>> jetHandle(m_jetContainerKey, ctx);
    ATH_CHECK(jetHandle.isValid());

    ConstDataVector<xAOD::JetContainer> jets = *jetHandle;

    // apply the GNNTool to the jets
    for (const xAOD::Jet *jet : jets)
    {
      m_gnn_Tool->decorate(*jet, *jet);
      ATH_MSG_DEBUG("A jet decorated pt= " << jet->pt()/1000);
    }
        

   // SG::ReadHandle<xAOD::JetContainer> jetsDecoHandle(m_jetReadKey, ctx);
    //ATH_CHECK(jetsDecoHandle.isValid());
    
    //ATH_CHECK(m_VtxTool->readDecorJet(jetsDecoHandle.ptr(), ctx));
   // hist ("PB scores from GNN") ->Fill(m_VtxTool->readDecorJet(jetsDecoHandle.ptr(), ctx));
    
    return StatusCode::SUCCESS;

  }

}
