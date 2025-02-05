/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

// author: cpollard@cern.ch

#include "ParticleJetTools/JetParticleOriginVertexAssociation.h"
#include "AsgMessaging/Check.h"

using namespace std;
using namespace xAOD;

JetParticleOriginVertexAssociation::JetParticleOriginVertexAssociation(const string& name)
    : JetParticleAssociation(name) {

        declareProperty("coneSizeFitPar1", m_coneSizeFitPar1=0);
        declareProperty("coneSizeFitPar2", m_coneSizeFitPar2=0);
        declareProperty("coneSizeFitPar3", m_coneSizeFitPar3=0);

        return;
    }

const vector<vector<ElementLink<IParticleContainer> > >*
JetParticleOriginVertexAssociation::match(SG::ReadDecorHandleKey<IParticleContainer> trk_origin_vtx, const JetContainer& jets, const IParticleContainer& parts) const {

    //Get the vertex associated to each track by reading the decoration
    SG::ReadDecorHandle<IParticleContainer, ElementLink<VertexContainer>> trkOrigin(trk_origin_vtx);
    //Create the 2d output vector 
    vector<vector<ElementLink<IParticleContainer> > >* matchedparts =
        new std::vector<std::vector<ElementLink<IParticleContainer> > >(jets.size());
    
    
    //loop through the tracks
    for (const IParticle* part: parts) {
      // Retrieve the ElementLink to the vertex
      const ElementLink<VertexContainer>& vertexLink = trkOrigin(*part);
      // check if link is valid
      if (!vertexLink.isValid()) {
        ATH_MSG_WARNING("Track decoration 'btagIp_TrkOriginVertex' is missing for this track");
        continue;
      }
      
      // Continue processing
      //Get vertex associated with the track
      const Vertex* vtx_to_trk = *vertexLink; 
      int matchjetidx = -1;
      double drmin = -1.0;
      // Loop through jets
      for (unsigned int iJet = 0; iJet < jets.size(); iJet++) {
          //get jet
          const Jet* jet = jets[iJet];
          // if origin of jet is not the same as the vertex associated to the track then continue to next jet
          if (jet->getAssociatedObject<Vertex>("OriginVertex") != vtx_to_trk) {
            continue;
          }
          
          // do dR matching between jet and track
          double match_dr = coneSize(jet->pt());
          double dr = jet->p4().DeltaR(part->p4());
          if (dr > match_dr) continue;
          if (drmin < 0 || dr < drmin) {
              drmin = dr;
              matchjetidx = iJet;
          }
      }
      if (matchjetidx >= 0) {
        ElementLink<IParticleContainer> EL; 
        EL.toContainedElement(parts, part);
        (*matchedparts)[matchjetidx].push_back(EL);
      }      
    }

    return matchedparts;
    
}
