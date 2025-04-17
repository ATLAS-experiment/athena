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
        declareProperty("dzCut", m_dzCut=10);
        declareProperty("useMinZ0Vertex", m_useMinZ0Vertex=false);
        return;
    }


const std::vector<std::vector<ElementLink<xAOD::IParticleContainer> > >*
JetParticleOriginVertexAssociation::match(const xAOD::JetContainer& jets, const xAOD::IParticleContainer& parts) const {

    //Get the vertex associated to each track by reading the decoration
    const SG::AuxElement::ConstAccessor<std::vector<ElementLink<xAOD::VertexContainer>>> trkOrigin("btagIp_ByVertex_TrkOriginVtx");
    const SG::AuxElement::ConstAccessor<std::vector<float>> z0SinTheta("btagIp_ByVertex_z0SinTheta");
    //Create the 2d output vector 
    std::vector<std::vector<ElementLink<xAOD::IParticleContainer> > >* matchedparts =
        new vector<std::vector<ElementLink<xAOD::IParticleContainer> > >(jets.size());
    
    //loop through the tracks
    for (const IParticle* part: parts) {
      
      // Retrieve the vector of vertex ElementLinks associated with the track and the corresponding z0 vector
      const std::vector<ElementLink<xAOD::VertexContainer>>& vertexLink_vec = trkOrigin(*part);
      const std::vector<float>& z0SinTheta_vec = z0SinTheta(*part);
      // check if the vectors are not empty
      if(z0SinTheta_vec.empty()) continue;
      // index of vertex with min(z0)
      int selectedVertexIndex = -1;
      if (m_useMinZ0Vertex) {
        // Find the vertex with the minimum absolute value of z0SinTheta
        auto minAbsIt = std::min_element(z0SinTheta_vec.begin(), z0SinTheta_vec.end(), 
                                        [](float a, float b) {
                                            return std::abs(a) < std::abs(b);
                                        });
        // find index of the vertex with mindZ                               
        if (minAbsIt != z0SinTheta_vec.end()) {
          selectedVertexIndex = std::distance(z0SinTheta_vec.begin(), minAbsIt);
        }
      }
      // loop through vertices
      for (unsigned int iVtx=0; iVtx < z0SinTheta_vec.size(); iVtx++){
        // if we want to do exclusive, only use the vertex with the min(z0)
        if (m_useMinZ0Vertex && selectedVertexIndex!=iVtx) continue;
        // retrieve vertex link
        const ElementLink<xAOD::VertexContainer>& vertexLink = vertexLink_vec.at(iVtx);
        // check if link is valid
        if (!vertexLink.isValid()) {
          ATH_MSG_WARNING("Track decoration 'btagIp_TrkOriginVertex' is missing for this track");
          continue;
        }
        // if the dz(track, vertex) doesnt pass the cut then continue to next vertex
        if (z0SinTheta_vec.at(iVtx) > m_dzCut) continue;
        
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
          if (jet->getAssociatedObject<xAOD::Vertex>("OriginVertex") != vtx_to_trk) continue;
          
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
          ElementLink<xAOD::IParticleContainer> EL; 
          EL.toContainedElement(parts, part);
          (*matchedparts)[matchjetidx].push_back(EL);
        } 
      }
           
    }

    return matchedparts;
    
}
