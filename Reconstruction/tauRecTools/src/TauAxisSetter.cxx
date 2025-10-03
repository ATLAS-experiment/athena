/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAOD_ANALYSIS

#include "TauAxisSetter.h"
#include "tauRecTools/HelperFunctions.h"

#include "xAODTau/TauJetContainer.h"
#include "xAODTau/TauJetAuxContainer.h"
#include "xAODTau/TauJet.h"
#include "xAODCaloEvent/CaloVertexedTopoCluster.h"



TauAxisSetter::TauAxisSetter(const std::string& name) :
TauRecToolBase(name) {
}

StatusCode TauAxisSetter::execute(xAOD::TauJet& tau) const {

  if (tau.jet() == nullptr) {
    ATH_MSG_ERROR("Tau jet link is invalid.");
    return StatusCode::FAILURE;
  }

  const xAOD::Jet* jetSeed = tau.jet();

  // Barycenter is the sum of cluster p4 in the seed jet
  TLorentzVector baryCenter;  
  
  xAOD::JetConstituentVector constituents = jetSeed->getConstituents();
  for (const xAOD::JetConstituent* constituent : constituents) {
    baryCenter += tauRecTools::GetConstituentP4(*constituent);
  }
  
  ATH_MSG_DEBUG("barycenter (eta, phi): "  << baryCenter.Eta() << " " << baryCenter.Phi());

  // Detector axis is the total p4 of clusters within m_clusterCone core of the barycenter 
  TLorentzVector tauDetectorAxis;

  for (const xAOD::JetConstituent* constituent : constituents) {
    TLorentzVector constituentP4 = tauRecTools::GetConstituentP4(*constituent);
    
    if (baryCenter.DeltaR(constituentP4) > m_clusterCone) continue;

    tauDetectorAxis += constituentP4;
  }

  if (tauDetectorAxis.Pt() == 0. && !m_doVertexCorrection) {
    ATH_MSG_DEBUG("this tau candidate does not have any constituent clusters!");
    return StatusCode::FAILURE;
  }

  ATH_MSG_DEBUG("detector axis:" << tauDetectorAxis.Pt()<< " " << tauDetectorAxis.Eta() << " " << tauDetectorAxis.Phi()  << " " << tauDetectorAxis.E());
  tau.setP4(tauDetectorAxis.Pt(), tauDetectorAxis.Eta(), tauDetectorAxis.Phi(), tau.m());
  tau.setP4(xAOD::TauJetParameters::DetectorAxis, tauDetectorAxis.Pt(), tauDetectorAxis.Eta(), tauDetectorAxis.Phi(), tauDetectorAxis.M());


  if (m_doVertexCorrection) {
    // Tau intermediate axis (corrected for tau vertex)
    TLorentzVector tauInterAxis;

    const xAOD::Vertex* jetVertex = tauRecTools::getJetVertex(*jetSeed);

    // Redo the vertex correction when tau vertex is different from jet vertex
    if (jetVertex != tau.vertex()) {

      // If seed jet has a vertex, then tau must have one
      if (tau.vertex() == nullptr) {
        ATH_MSG_WARNING("The seed jet has a vertex, while the tau candidate does not. It should not happen.");
        return StatusCode::FAILURE;
      }
    
      const xAOD::Vertex* tauVertex = tau.vertex();

      // Relative position of the tau vertex and jet vertex
      Amg::Vector3D position = tauVertex->position();
      if (jetVertex != nullptr) {
        position -= jetVertex->position();
      }

      // Barycenter at the tau vertex
      TLorentzVector baryCenterTauVertex; 
 
      // Loop over the jet constituents, and calculate the barycenter using the four momentum 
      // corrected to point at tau vertex 
      for (const xAOD::JetConstituent* constituent : constituents) {
        baryCenterTauVertex += getVertexCorrectedP4(*constituent, position);
      }
      ATH_MSG_DEBUG("barycenter (eta, phi) at tau vertex: "  << baryCenterTauVertex.Eta() << " " << baryCenterTauVertex.Phi());

      // Tau intermediate axis is the four momentum (corrected to point at tau vertex) of clusters 
      // within m_clusterCone of the barycenter
      for (const xAOD::JetConstituent* constituent : constituents) {
        TLorentzVector constituentP4 = getVertexCorrectedP4(*constituent, position);
        if (baryCenterTauVertex.DeltaR(constituentP4) > m_clusterCone) continue;
        
        tauInterAxis += constituentP4;
      }
    }
    else {  
      tauInterAxis = tauDetectorAxis;
    }

    if (tauInterAxis.Pt() == 0.) {
      ATH_MSG_DEBUG("this tau candidate does not have any constituent clusters!");
      return StatusCode::FAILURE;
    }

    ATH_MSG_DEBUG("tau axis:" << tauInterAxis.Pt()<< " " << tauInterAxis.Eta() << " " << tauInterAxis.Phi()  << " " << tauInterAxis.E() );
    tau.setP4(tauInterAxis.Pt(), tauInterAxis.Eta(), tauInterAxis.Phi(), tau.m());
    tau.setP4(xAOD::TauJetParameters::IntermediateAxis, tauInterAxis.Pt(), tauInterAxis.Eta(), tauInterAxis.Phi(), tauInterAxis.M());
  } // End of m_doVertexCorrection

  return StatusCode::SUCCESS;
}



TLorentzVector TauAxisSetter::getVertexCorrectedP4(const xAOD::JetConstituent& constituent,
                                                   const Amg::Vector3D& position) const {
  TLorentzVector vertexCorrectedP4;
  
  if (constituent.type() == xAOD::Type::CaloCluster) {
    const xAOD::CaloCluster* cluster = static_cast<const xAOD::CaloCluster*>(constituent.rawConstituent());
    vertexCorrectedP4 = xAOD::CaloVertexedTopoCluster(*cluster, position).p4();;
  }
  else if ( constituent->type() == xAOD::Type::FlowElement ) {
    const xAOD::FlowElement* fe = static_cast<const xAOD::FlowElement*>( constituent->rawConstituent() );
    vertexCorrectedP4 = getVertexCorrectedP4(*fe, position);
  }
  else {
    ATH_MSG_WARNING("Seed jet constituent type not supported, will not do vertex correction !");
    vertexCorrectedP4 = tauRecTools::GetConstituentP4(constituent);
  }
 
  return vertexCorrectedP4; 
}

TLorentzVector TauAxisSetter::getVertexCorrectedP4(const xAOD::FlowElement& fe,
                                                   const Amg::Vector3D& position) const {

  TLorentzVector vertexCorrectedP4; 
  // Only perfrom vertex corretion for neutral FlowElement
  if (!fe.isCharged()) { 
     TVector3 pos(position.x(), position.y(), position.z());
     vertexCorrectedP4 = FEHelpers::getVertexCorrectedFourVec(fe,pos);
  }
  else {
     vertexCorrectedP4 = fe.p4();
  }

  ATH_MSG_DEBUG("Original fe four momentum, pt: " << fe.pt() <<
                  " eta: " << fe.eta() << " phi: " << fe.phi() << " e: " << fe.e());
  ATH_MSG_DEBUG("Vertex corrected four momentum, pt: " << vertexCorrectedP4.Pt() <<
                " eta: " << vertexCorrectedP4.Eta() << " phi: " << vertexCorrectedP4.Phi() << " e: " << vertexCorrectedP4.E());

  return vertexCorrectedP4;
  
}

#endif
