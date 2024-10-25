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

    //Get the vertex associated to each track by reading the decoration
    SG::ReadDecorHandle<xAOD::IParticleContainer, ElementLink<xAOD::VertexContainer>> trkOrigin(trk_origin_vtx);
    //Create the 2d output vector 
    std::vector<std::vector<ElementLink<xAOD::IParticleContainer> > >* matchedparts =
        new std::vector<std::vector<ElementLink<xAOD::IParticleContainer> > >(jets.size());
    
    //loop through the tracks
    for (const xAOD::IParticle* part: parts) {
      // Retrieve the ElementLink to the vertex
      const ElementLink<xAOD::VertexContainer>& vertexLink = trkOrigin(*part);
      // Check if the link is valid
      if (vertexLink.isValid()) {
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
    
      
      } else {
          ATH_MSG_WARNING("Track decoration 'btagIp_TrkOriginVertex' is missing for this track");
      }
      
      

      

      
    }

    

    return matchedparts;
    
}

std::vector<std::vector<ElementLink<xAOD::IParticleContainer>>>* getIntersection(const std::vector<std::vector<ElementLink<xAOD::IParticleContainer>>>*matches1,
                const std::vector<std::vector<ElementLink<xAOD::IParticleContainer>>>* matches2) {


    // Allocate the intersection dynamically, same as in match function
    auto* intersection = new std::vector<std::vector<ElementLink<xAOD::IParticleContainer>>>(matches1->size());

    for (size_t i = 0; i < matches1->size(); ++i) {
        // Convert each inner vector to sets for easy intersection
        std::set<ElementLink<xAOD::IParticleContainer>> set1((*matches1)[i].begin(), (*matches1)[i].end());
        std::set<ElementLink<xAOD::IParticleContainer>> set2((*matches2)[i].begin(), (*matches2)[i].end());

        // Temporary vector to store the intersection of current index
        std::vector<ElementLink<xAOD::IParticleContainer>> tempIntersection;

        // Find intersection between set1 and set2
        std::set_intersection(
            set1.begin(), set1.end(),
            set2.begin(), set2.end(),
            std::back_inserter(tempIntersection)
        );

        // Move the temporary intersection to the final result
        (*intersection)[i] = std::move(tempIntersection);
    }

    return intersection;
}


JetParticleAssociation::JetParticleAssociation(const std::string& name)
    : asg::AsgTool(name) {
}

StatusCode JetParticleAssociation::initialize() {

    ATH_MSG_DEBUG("Initializing JetParticleAssociator");
    m_decKey = m_jetContainerName + "." + m_decKey.key();
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
    
    
    if ((m_jetContainerName == "AntiKt4EMPFlowByVertexJets") && (m_particleKey.key()=="InDetTrackParticles")){
      const std::vector<std::vector<ElementLink<xAOD::IParticleContainer> > >* matches_origin = matchOriginTrk(m_trk_origin_vtx, *viewJets.asDataVector(), *parts);
      const std::vector<std::vector<ElementLink<xAOD::IParticleContainer> > >* matches_dR = match(*viewJets.asDataVector(), *parts);
      matches = getIntersection(matches_origin, matches_dR);
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
