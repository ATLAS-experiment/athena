/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// Reco_Vertex.cxx
///////////////////////////////////////////////////////////////////
// Author: Daniel Scheirich <daniel.scheirich@cern.ch>
// Based on the Integrated Simulation Framework
//
// Basic Jpsi->mu mu derivation example

#include "Reco_Vertex.h"
#include "BPhysPVTools.h"
#include "xAODTracking/VertexContainer.h"
#include "xAODTracking/VertexAuxContainer.h"
#include "JpsiUpsilonTools/JpsiUpsilonCommon.h"

namespace DerivationFramework {

  StatusCode Reco_Vertex::initialize()
  {
  
    ATH_MSG_DEBUG("in initialize()");
 
    // retrieve V0 tools
    CHECK( m_v0Tools.retrieve() );
    
    // get the Search tool
    CHECK( m_SearchTool.retrieve() );
     
    // get the PrimaryVertexRefitter tool
    CHECK( m_pvRefitter.retrieve() );

    // Get the beam spot service
    ATH_CHECK(m_eventInfo_key.initialize());


    ATH_CHECK(m_outputVtxContainerName.initialize());
    ATH_CHECK(m_pvContainerName.initialize());
    ATH_CHECK(m_refPVContainerName.initialize());
    ATH_CHECK(m_RelinkContainers.initialize());
    ATH_CHECK(m_RelinkMuons.initialize());
    if(m_checkCollections) ATH_CHECK(m_CollectionsToCheck.initialize());
    return StatusCode::SUCCESS;
    
  }

  // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
  
  StatusCode Reco_Vertex::execute(const EventContext& ctx) const
  {
    bool callTool = true;
    if(m_checkCollections) {
      for(const auto &str : m_CollectionsToCheck){
         SG::ReadHandle<xAOD::VertexContainer> handle(str,ctx);
         ATH_CHECK(handle.isValid());
         if(handle->size() == 0) {
            callTool = false;
            ATH_MSG_DEBUG("Container VertexContainer (" << str << ") is empty");
            break;//No point checking other containers
         }
      }
    }

    // Vertex container and its auxilliary store
    std::unique_ptr<xAOD::VertexContainer>    vtxContainer = std::make_unique<xAOD::VertexContainer>();
    std::unique_ptr<xAOD::VertexAuxContainer> vtxAuxContainer = std::make_unique<xAOD::VertexAuxContainer>();
    vtxContainer->setStore(vtxAuxContainer.get());
   
    if(callTool) {
    //----------------------------------------------------
    // call Tool
    //----------------------------------------------------
    if( !m_SearchTool->performSearch(ctx,*vtxContainer).isSuccess() ) {
      ATH_MSG_FATAL("Tool (" << m_SearchTool << ") failed.");
      return StatusCode::FAILURE;
    }
 
    //----------------------------------------------------
    // retrieve primary vertices
    //----------------------------------------------------
    SG::ReadHandle<xAOD::VertexContainer> pvContainer(m_pvContainerName,ctx);

    //----------------------------------------------------
    // Try to retrieve refitted primary vertices
    //----------------------------------------------------
    std::unique_ptr<xAOD::VertexContainer>    refPvContainer;
    std::unique_ptr<xAOD::VertexAuxContainer> refPvAuxContainer;
    if(m_refitPV) {
        // refitted PV container does not exist. Create a new one.
        refPvContainer = std::make_unique<xAOD::VertexContainer>();
        refPvAuxContainer = std::make_unique<xAOD::VertexAuxContainer>();
        refPvContainer->setStore(refPvAuxContainer.get());
    }
    
    // Give the helper class the ptr to v0tools and beamSpotsSvc to use
    SG::ReadHandle<xAOD::EventInfo> evt(m_eventInfo_key, ctx);
    if(not evt.isValid()) ATH_MSG_ERROR("Cannot Retrieve " << evt.key() );
    BPhysPVTools helper(&(*m_v0Tools), evt.cptr());
    helper.SetMinNTracksInPV(m_PV_minNTracks);
    helper.SetSave3d(m_do3d);

    if(m_refitPV){ 
       if(vtxContainer->size() >0){
        if(vtxContainer->size() > 10000){
          ATH_MSG_WARNING("Event Run: " << evt->runNumber() << " Event: " << evt->eventNumber() << " Number of candidates is very high N=" << vtxContainer->size() << " this may crash the sharedwriter");
        }
        StatusCode SC = helper.FillCandwithRefittedVertices(vtxContainer.get(),  pvContainer.cptr(), refPvContainer.get(), &(*m_pvRefitter) , m_PV_max, m_DoVertexType);
        if(SC.isFailure()){
            ATH_MSG_FATAL("refitting failed - check the vertices you passed");
            return SC;
        }
        if(refPvContainer->size() > 10000){
          ATH_MSG_WARNING("Event Run: " << evt->runNumber() << " Event: " << evt->eventNumber() << " Number of refitted vertices is very high N=" << refPvContainer->size() << " this may crash the sharedwriter");
        }
        }
    }else{
        if(vtxContainer->size() >0)CHECK(helper.FillCandExistingVertices(vtxContainer.get(), pvContainer.cptr(), m_DoVertexType));
    }
  
    using Analysis::JpsiUpsilonCommon;

    std::vector<const xAOD::TrackParticleContainer*> trackCols;
    for(const auto &str : m_RelinkContainers){
      SG::ReadHandle<xAOD::TrackParticleContainer> handle(str,ctx);
      trackCols.push_back(handle.cptr());
    }
    if(not trackCols.empty()){
       for(xAOD::Vertex* vtx : *vtxContainer.get()){
          try{
            JpsiUpsilonCommon::RelinkVertexTracks(trackCols, vtx);
          }catch(std::runtime_error const& e){
            ATH_MSG_ERROR(e.what());
            return StatusCode::FAILURE;
          }
       }
    }
    std::vector<const xAOD::MuonContainer*> muCols;
    for(const auto &str : m_RelinkMuons){
      SG::ReadHandle<xAOD::MuonContainer> handle(str,ctx);
      muCols.push_back(handle.cptr());
    }
    if(not muCols.empty()){
       for(xAOD::Vertex* vtx : *vtxContainer.get()){
          try{
             JpsiUpsilonCommon::RelinkVertexMuons(muCols, vtx);
          }catch(std::runtime_error const& e){
             ATH_MSG_ERROR(e.what());
             return StatusCode::FAILURE;
          }
       }
    }
    //----------------------------------------------------
    // save in the StoreGate
    //----------------------------------------------------
    SG::WriteHandle<xAOD::VertexContainer> handle(m_outputVtxContainerName, ctx);
    ATH_CHECK(handle.record(std::move(vtxContainer), std::move(vtxAuxContainer)));
    
    if(m_refitPV) {
       SG::WriteHandle<xAOD::VertexContainer> handle(m_refPVContainerName, ctx);
       ATH_CHECK(handle.record(std::move(refPvContainer), std::move(refPvAuxContainer)));
    }
    }

    if (!callTool) { //Fill with empty containers
      SG::WriteHandle<xAOD::VertexContainer> handle(m_outputVtxContainerName, ctx);
      ATH_CHECK(handle.record(std::unique_ptr<xAOD::VertexContainer>(new xAOD::VertexContainer ),
          std::unique_ptr<xAOD::VertexAuxContainer>(new xAOD::VertexAuxContainer )));
    }
    
    return StatusCode::SUCCESS;
  }



}
