/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
                         InDetV0Finder.cxx  -  Description
                             -------------------
    begin   : 20-07-2005
    authors : Evelina Bouhova-Thacker (Lancaster University), Rob Henderson (Lancater University)
    email   : e.bouhova@cern.ch, r.henderson@lancaster.ac.uk
    changes : December 2014
    author  : Evelina Bouhova-Thacker <e.bouhova@cern.ch> 
              Changed to use xAOD

 ***************************************************************************/

#include "InDetV0Finder.h"

#include "xAODTracking/VertexContainer.h"
#include "xAODTracking/VertexAuxContainer.h"
#include "InDetV0Finder/InDetV0FinderTool.h"
#include "StoreGate/WriteDecorHandle.h"
#include <vector>
#include <string>


namespace InDet
{
  
InDetV0Finder::InDetV0Finder(const std::string &n, ISvcLocator *pSvcLoc)
  :
  AthAlgorithm(n, pSvcLoc) {}

InDetV0Finder::~InDetV0Finder() = default;

StatusCode InDetV0Finder::initialize()
{
  ATH_CHECK( resetStatistics() );

  ATH_CHECK( m_vertexKey.initialize() );
  ATH_CHECK( m_v0Key.initialize() );
  ATH_CHECK( m_ksKey.initialize() );
  ATH_CHECK( m_laKey.initialize() );
  ATH_CHECK( m_lbKey.initialize() );

// uploading the V0Finding tools
  ATH_CHECK( m_v0FinderTool.retrieve() ); 
  ATH_MSG_DEBUG("Retrieved tool " << m_v0FinderTool);
  ATH_CHECK(m_v0DecoTool.retrieve(DisableTool{!m_decorate}));

  ATH_MSG_DEBUG("Initialization successful");

  return StatusCode::SUCCESS;
}


StatusCode InDetV0Finder::execute(const EventContext& ctx)
{

  m_events_processed++;
// Get primary vertex from StoreGate
  const xAOD::Vertex* primaryVertex = nullptr;
  SG::ReadHandle<xAOD::VertexContainer> importedVxContainer( m_vertexKey, ctx );
  if ( !importedVxContainer.isValid() )
  {
    ATH_MSG_WARNING("No xAOD::VertexContainer named " << m_vertexKey.key() << " found in StoreGate");
    return StatusCode::RECOVERABLE;    
  } else {
    ATH_MSG_DEBUG("Found xAOD::VertexContainer named " << m_vertexKey.key() );
  }
  if ( importedVxContainer->empty() ){
    ATH_MSG_WARNING("Primary vertex container is empty.");
  } else {
    primaryVertex = importedVxContainer->front();
  }

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


  const auto statusOfSearch = m_v0FinderTool->performSearch(h_V0.ptr(),
             h_Ks.ptr(),
             h_La.ptr(),
             h_Lb.ptr(),
             primaryVertex, importedVxContainer.cptr(), ctx);

  if (statusOfSearch != StatusCode::SUCCESS){
    ATH_MSG_ERROR("Vertex search of v0Container failed.");
    return StatusCode::FAILURE;
  }

  if (m_decorate) {
    ATH_CHECK(m_v0DecoTool->decorateV0(h_V0.ptr(), ctx));
    ATH_CHECK(m_v0DecoTool->decorateks(h_Ks.ptr() ,ctx));
    ATH_CHECK(m_v0DecoTool->decoratela(h_La.ptr(), ctx));
    ATH_CHECK(m_v0DecoTool->decoratelb(h_Lb.ptr(), ctx));
  }

  m_V0s_stored += h_V0->size();
  m_Kshort_stored += h_Ks->size();
  m_Lambda_stored += h_La->size();
  m_Lambdabar_stored += h_Lb->size();

  return StatusCode::SUCCESS;
}// end execute block

StatusCode InDetV0Finder::finalize()
{
  msg(MSG::INFO)
    << "----------------------------------------------------------------------------------------------------------------------------------------------" << endmsg
    << "\tSummary" << endmsg 
    << "\tProcessed              : " << m_events_processed            << " events" << endmsg
    << "\tStored                 : " << m_V0s_stored                  << " V0s" << endmsg
    << "\tof which               : " << m_Kshort_stored               << " Kshorts" << endmsg
    << "\t                       : " << m_Lambda_stored               << " Lambdas" << endmsg
    << "\t                       : " << m_Lambdabar_stored            << " Lambdabars" << endmsg;
  msg(MSG::INFO) << "----------------------------------------------------------------------------------------------------------------------------------------------" << endmsg;

  return StatusCode::SUCCESS;
}

StatusCode InDetV0Finder::resetStatistics() {
    m_events_processed           = 0;
    m_V0s_stored                 = 0;
    m_Kshort_stored              = 0;
    m_Lambdabar_stored           = 0;
    m_Lambda_stored              = 0;
  
    return StatusCode :: SUCCESS;
}


}//end of namespace InDet

