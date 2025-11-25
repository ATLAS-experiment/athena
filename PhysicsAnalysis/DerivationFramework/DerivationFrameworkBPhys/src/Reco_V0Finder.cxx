/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
/////////////////////////////////////////////////////////////////
// Reco_V0Finder.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
// Author: Adam Barton
#include "Reco_V0Finder.h"
#include "xAODTracking/VertexContainer.h"
#include "xAODTracking/VertexAuxContainer.h"

namespace DerivationFramework {

  Reco_V0Finder::Reco_V0Finder(const std::string& t,
      const std::string& n,
      const IInterface* p) : 
    base_class(t,n,p),
    m_v0FinderTool("InDet::V0FinderTool", this)
  {
    
    // Declare user-defined properties
    declareProperty("CheckVertexContainers", m_CollectionsToCheck);
    declareProperty("V0FinderTool", m_v0FinderTool);
  }

  // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
  
  StatusCode Reco_V0Finder::initialize()
  {
  
    ATH_MSG_DEBUG("in initialize()");
    // get the V0Finder tool
    ATH_CHECK( m_v0FinderTool.retrieve());
    ATH_CHECK( m_v0DecoTool.retrieve());

    ATH_CHECK(m_vertexKey.initialize());
    ATH_CHECK(m_v0Key.initialize());
    ATH_CHECK(m_ksKey.initialize());
    ATH_CHECK(m_laKey.initialize());
    ATH_CHECK(m_lbKey.initialize());

    return StatusCode::SUCCESS;
    
  }
  

  // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
  
  StatusCode Reco_V0Finder::addBranches(const EventContext& ctx) const
  {

    bool callV0Finder = false;
    // Jpsi container and its auxilliary store
    for(const auto &str : m_CollectionsToCheck){
       const xAOD::VertexContainer*    vertContainer = nullptr;
       ATH_CHECK( evtStore()->retrieve(vertContainer, str) );
       if(vertContainer->size() == 0) {
          ATH_MSG_DEBUG("Container VertexContainer (" << str << ") is empty");
       }else{
          callV0Finder = true;
          ATH_MSG_DEBUG("Container VertexContainer (" << str << ") has events N= " << vertContainer->size());
          break;//No point checking other containers
       }
    }

  // InDetV0 container and its auxilliary store
    //---- Recording section: write the results to StoreGate ---//
    SG::WriteHandle<xAOD::VertexContainer> h_V0( m_v0Key, ctx );
    if ( h_V0.record(std::make_unique<xAOD::VertexContainer>() ,std::make_unique<xAOD::VertexAuxContainer>()).isFailure()){
      ATH_MSG_ERROR("Storegate record of v0Container failed.");
      return StatusCode::FAILURE;
    }
  
    SG::WriteHandle<xAOD::VertexContainer> h_Ks( m_ksKey, ctx );
    if ( h_Ks.record(std::make_unique<xAOD::VertexContainer>() ,std::make_unique<xAOD::VertexAuxContainer>()).isFailure()){
      ATH_MSG_ERROR("Storegate record of ksContainer failed.");
      return StatusCode::FAILURE;
    }
  
    SG::WriteHandle<xAOD::VertexContainer> h_La( m_laKey, ctx );
    if( h_La.record(std::make_unique<xAOD::VertexContainer>() ,std::make_unique<xAOD::VertexAuxContainer>()).isFailure()){
      ATH_MSG_ERROR("Storegate record of laContainer failed.");
      return StatusCode::FAILURE;
  
    }
    SG::WriteHandle<xAOD::VertexContainer> h_Lb( m_lbKey, ctx );
    if(h_Lb.record(std::make_unique<xAOD::VertexContainer>() ,std::make_unique<xAOD::VertexAuxContainer>()).isFailure()){
      ATH_MSG_ERROR("Storegate record of lbContainer failed.");
      return StatusCode::FAILURE;
    }

    xAOD::VertexContainer*    v0Container(h_V0.ptr());
    xAOD::VertexContainer*    ksContainer(h_Ks.ptr());
    xAOD::VertexContainer*    laContainer(h_La.ptr());
    xAOD::VertexContainer*    lbContainer(h_Lb.ptr());
    // Call V0Finder
    if (callV0Finder) {
       const xAOD::Vertex * primaryVertex(0);
       SG::ReadHandle<xAOD::VertexContainer> importedVxContainer( m_vertexKey, ctx );
       ATH_CHECK(importedVxContainer.isValid());
       
       if (importedVxContainer->size()==0){
           ATH_MSG_WARNING("You have no primary vertices: " << importedVxContainer->size());
       } else {
           primaryVertex = (*importedVxContainer)[0];
       }
       ATH_CHECK( m_v0FinderTool->performSearch(h_V0.ptr(), 
             h_Ks.ptr(),
             h_La.ptr(),
             h_Lb.ptr(),
             primaryVertex, importedVxContainer.cptr(), ctx));

       ATH_MSG_DEBUG("Reco_V0Finder v0Container->size() " << v0Container->size());
       ATH_MSG_DEBUG("Reco_V0Finder ksContainer->size() " << ksContainer->size());
       ATH_MSG_DEBUG("Reco_V0Finder laContainer->size() " << laContainer->size());
       ATH_MSG_DEBUG("Reco_V0Finder lbContainer->size() " << lbContainer->size());


       ATH_CHECK(m_v0DecoTool->decorateV0(h_V0.ptr(), ctx));
       ATH_CHECK(m_v0DecoTool->decorateks(h_Ks.ptr() ,ctx));
       ATH_CHECK(m_v0DecoTool->decoratela(h_La.ptr(), ctx));
       ATH_CHECK(m_v0DecoTool->decoratelb(h_Lb.ptr(), ctx));
    }

    return StatusCode::SUCCESS;
  }
}




