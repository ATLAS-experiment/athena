/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

// author: cpollard@cern.ch

#include "ParticleJetTools/JetParticleAssociation.h"
#include "AsgDataHandles/WriteDecorHandle.h"
#include "AthContainers/ConstDataVector.h"
#include "AsgDataHandles/ReadDecorHandle.h"

const std::vector<std::vector<ElementLink<xAOD::IParticleContainer> > >*
JetParticleAssociation::matchOriginTrk(SG::ReadDecorHandleKey<xAOD::IParticleContainer> trk_origin_vtx, const xAOD::JetContainer& jets, const xAOD::IParticleContainer& parts) const {

    std::cout << "Mario: name of container one : " << trk_origin_vtx << std::endl;
    //Get the vertex associated to each track by reading the decoration
    SG::ReadDecorHandle<xAOD::IParticleContainer, ElementLink<xAOD::VertexContainer>> trkOrigin(trk_origin_vtx);
    //Create the 2d output vector 
    std::vector<std::vector<ElementLink<xAOD::IParticleContainer> > >* matchedparts =
        new std::vector<std::vector<ElementLink<xAOD::IParticleContainer> > >(jets.size());
    
    //loop through the tracks
    int i=0;
    int j =0;
    std::cout << "Mario: name of container: " << trk_origin_vtx << std::endl;
    for (const xAOD::IParticle* part: parts) {
      i++;
      // Retrieve the ElementLink to the vertex
      std::cout << "Mario: try" << std::endl;
      const ElementLink<xAOD::VertexContainer>& vertexLink = trkOrigin(*part);
      std::cout << "Mario: try2" <<std::endl;
      // Check if the link is valid
      if (vertexLink.isValid()) {
     
        j++;
        // Continue processing
        //Get vertex associated with the track
        const xAOD::Vertex* vtx_to_trk = *vertexLink; //&trkOrigin(*part); //change here
        int matchjetidx = -1;
        for (unsigned int iJet = 0; iJet < jets.size(); iJet++) {
            //get jet
            const xAOD::Jet* jet = jets[iJet];
            // if origin of jet is the same as the vertex associated to the track then get index of jet
            if (jet->getAssociatedObject<xAOD::Vertex>("OriginVertex") == vtx_to_trk){
                matchjetidx = iJet;
            }else{
                continue;
            }

        }
        if (matchjetidx >= 0) {
          ElementLink<xAOD::IParticleContainer> EL; 
          EL.toContainedElement(parts, part);
          (*matchedparts)[matchjetidx].push_back(EL);
        }
      //}
      
      } else {
          ATH_MSG_WARNING("Track decoration 'btagIp_TrkOriginVertex' is missing for this track");
      }
      
      

      

      
    }

    std::cout << "Mario Tracks: " << i << " with origin: " << j << std::endl;

    return matchedparts;
    
}

JetParticleAssociation::JetParticleAssociation(const std::string& name)
    : asg::AsgTool(name) {
}

StatusCode JetParticleAssociation::initialize() {

    ATH_MSG_DEBUG("Initializing JetParticleAssociator");
    m_decKey = m_jetContainerName + "." + m_decKey.key();
    std::cout << "Mario see the name : " << m_trk_origin_vtx << std::endl;
    ATH_CHECK(m_decKey.initialize());
    ATH_CHECK(m_particleKey.initialize());
    ATH_CHECK(m_trk_origin_vtx.initialize());
    
    ATH_MSG_DEBUG("Minimum pt threshold: " << m_ptMinimum);
    if (m_ptMinimum > 0.0) {
      if (!m_passPtKey.key().empty()) {
        m_passPtKey = m_jetContainerName + "." + m_passPtKey.key();
        ATH_MSG_DEBUG("Pass Key Decorator: " << m_passPtKey);
      }
    }

    ATH_CHECK(m_passPtKey.initialize(!m_passPtKey.key().empty()));

    return StatusCode::SUCCESS;
}

StatusCode JetParticleAssociation::decorate(const xAOD::JetContainer& jets) const{
    SG::WriteDecorHandle<xAOD::JetContainer, std::vector<ElementLink<xAOD::IParticleContainer> > > decHandle(m_decKey);

    const std::vector<std::vector<ElementLink<xAOD::IParticleContainer> > >* matches;

    SG::ReadHandle<xAOD::IParticleContainer> parts(m_particleKey);
    if( !parts.isValid() ) {
      ATH_MSG_WARNING ("Couldn't retrieve particles with key: " << m_particleKey.key() );
      return StatusCode::FAILURE;
    }

    ConstDataVector<xAOD::JetContainer> viewJets(SG::VIEW_ELEMENTS);
    std::vector<unsigned int> skipped;
    for (unsigned int i = 0; i < jets.size(); i++) {
      const xAOD::Jet* jet = jets.at(i);
      if (jet->pt() > m_ptMinimum) {
        ATH_MSG_VERBOSE("Adding jet, pt = " << jet->pt());
        viewJets.push_back(jet);
      } else {
        ATH_MSG_VERBOSE("Skipping jet, pt = " << jet->pt());
        skipped.push_back(i);
      }
    }
    std::cout << "Mario: jet name: " << m_jetContainerName << std::endl;

    //matches = matchOriginTrk(m_trk_origin_vtx, *viewJets.asDataVector(), *parts);
   // matches = match(*viewJets.asDataVector(), *parts);
   // std::cout << "Mario: jet name: " << m_jetContainerName << std::endl;
    
    if ((m_jetContainerName == "AntiKt4EMPFlowByVertexJets") && (m_particleKey.key()=="InDetTrackParticles")){
      matches = matchOriginTrk(m_trk_origin_vtx, *viewJets.asDataVector(), *parts);
    }else{
      matches = match(*viewJets.asDataVector(), *parts);
    }
    
    
    ATH_MSG_DEBUG("About to decorate jets with" << m_decKey);

    for (unsigned int iJet = 0; iJet < viewJets.size(); iJet++) {
      decHandle(*(viewJets.at(iJet))) = (*matches)[iJet];
    }
    for (unsigned int iJet: skipped) {
      decHandle(*jets.at(iJet)) = {};
    }

    if (!m_passPtKey.empty()) {
      SG::WriteDecorHandle<xAOD::JetContainer, char> passHandle(m_passPtKey);
      for (const xAOD::Jet* jet: viewJets) {
        passHandle(*jet) = 1;
      }
      for (unsigned int iJet: skipped) {
        passHandle(*jets.at(iJet)) = 0;
      }
    }

    delete matches;

    return StatusCode::SUCCESS;
}
