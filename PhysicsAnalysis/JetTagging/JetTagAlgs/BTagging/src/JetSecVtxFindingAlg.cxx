/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "BTagging/JetSecVtxFindingAlg.h"

#include "StoreGate/ReadDecorHandle.h"
#include "StoreGate/ReadHandle.h"

namespace Analysis {

  JetSecVtxFindingAlg::JetSecVtxFindingAlg(const std::string& name, ISvcLocator* pSvcLocator):
    AthReentrantAlgorithm(name,pSvcLocator),
    m_secVertexFinderToolHandle(this)
  {
    //List of the secondary vertex finders in jet to be run
    declareProperty("SecVtxFinder",          m_secVertexFinderToolHandle);
  }

  StatusCode JetSecVtxFindingAlg::initialize()
  {
    // This will check that the properties were initialized properly
    // by job configuration.
    m_TracksToTag = m_JetCollectionName.key() + "." + m_TracksToTag.key();

    ATH_CHECK( m_JetCollectionName.initialize() );
    ATH_CHECK( m_TracksToTag.initialize() );
    ATH_CHECK( m_VertexCollectionName.initialize() );
    ATH_CHECK( m_VxSecVertexInfoName.initialize() );

    /* ----------------------------------------------------------------------------------- */
    /*                        RETRIEVE SERVICES FROM STOREGATE                             */
    /* ----------------------------------------------------------------------------------- */

    if ( m_secVertexFinderToolHandle.retrieve().isFailure() ) {
      ATH_MSG_ERROR("#BTAG# Failed to retrieve " << m_secVertexFinderToolHandle);
    } else {
      ATH_MSG_DEBUG("#BTAG# Retrieved " << m_secVertexFinderToolHandle);
    }

    return StatusCode::SUCCESS;
  }


  StatusCode JetSecVtxFindingAlg::execute(const EventContext& ctx) const {
    //retrieve the Jet container
    SG::ReadHandle<xAOD::JetContainer> h_JetCollectionName (m_JetCollectionName, ctx);
    if (!h_JetCollectionName.isValid()) {
      ATH_MSG_ERROR( " cannot retrieve jet container with key " << m_JetCollectionName.key()  );
      return StatusCode::FAILURE;
    }

    /* Record the VxSecVertexInfo output container */
    SG::WriteHandle<Trk::VxSecVertexInfoContainer> h_VxSecVertexInfoName (m_VxSecVertexInfoName, ctx);
    ATH_CHECK( h_VxSecVertexInfoName.record(std::make_unique<Trk::VxSecVertexInfoContainer>()));

    if (h_JetCollectionName->empty()) {
      ATH_MSG_DEBUG("#BTAG# Empty Jet collection");
      return StatusCode::SUCCESS;
    }

    SG::ReadDecorHandle<xAOD::JetContainer, std::vector<ElementLink< xAOD::IParticleContainer> > >
      h_TracksToTag (m_TracksToTag, ctx);

    if (!h_TracksToTag.isAvailable()) {
      ATH_MSG_ERROR( "cannot retrieve jet container particle EL decoration with key " << h_TracksToTag.decorKey()  );
      return StatusCode::FAILURE;
    }
 
    const xAOD::Vertex* primaryVertex(nullptr);

    //retrieve primary vertex
    SG::ReadHandle<xAOD::VertexContainer> h_VertexCollectionName (m_VertexCollectionName, ctx);
    if (!h_VertexCollectionName.isValid()) {
        ATH_MSG_ERROR( " cannot retrieve primary vertex container with key " << m_VertexCollectionName.key()  );
        return StatusCode::FAILURE;
    }
    unsigned int nVertexes = h_VertexCollectionName->size();
    if (nVertexes == 0) {
      ATH_MSG_DEBUG("#BTAG#  Vertex container is empty");
      return StatusCode::SUCCESS;
    }
    for (const auto *fz : *h_VertexCollectionName) {
      if (fz->vertexType() == xAOD::VxType::PriVtx) {
	      primaryVertex = fz;
	      break;
      }
    }


    if (! primaryVertex) {
      ATH_MSG_DEBUG("#BTAG#  No vertex labeled as VxType::PriVtx!");
      xAOD::VertexContainer::const_iterator fz = h_VertexCollectionName->begin();
      primaryVertex = *fz;
        if (primaryVertex->nTrackParticles() == 0) {
	      ATH_MSG_DEBUG("#BTAG#  PV==BeamSpot: probably poor tagging");
        }
    }

    const xAOD::Vertex& PrimaryVtx = *primaryVertex;

    for (const auto *jetIter : *h_JetCollectionName) {
      const xAOD::Jet& jetToTag = *jetIter;

      const std::vector<ElementLink< xAOD::IParticleContainer > >& tracksInJet
        = h_TracksToTag(jetToTag);

      if(tracksInJet.empty()){
        ATH_MSG_DEBUG("#BTAG# No track in Jet");
        h_VxSecVertexInfoName->push_back(nullptr);
        continue;
      } 

      std::vector<const xAOD::IParticle*> inputIParticles;
       

      inputIParticles.reserve(tracksInJet.size());
      for (const auto& iparticle : tracksInJet)
	      /// warning -> will not work if at some point we decide to associate to several track collections at the same time (in the same assoc object)
        inputIParticles.push_back(*iparticle);

      ATH_MSG_DEBUG("#BTAG#  Running " << m_secVertexFinderToolHandle);

      Trk::VxSecVertexInfo* myVertexInfo = m_secVertexFinderToolHandle->findSecVertex(PrimaryVtx, jetIter->p4(), inputIParticles);
      ATH_MSG_DEBUG("#BTAG# Number of vertices found: " << myVertexInfo->vertices().size());
      h_VxSecVertexInfoName->push_back(myVertexInfo); 
    }// for loop on jets

    return StatusCode::SUCCESS;
  } 

} // namespace
