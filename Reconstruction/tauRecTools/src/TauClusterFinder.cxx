/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAOD_ATHLYSIS

#include "TauClusterFinder.h"
#include "tauRecTools/HelperFunctions.h"

#include "xAODBase/IParticleHelpers.h"


TauClusterFinder::TauClusterFinder(const std::string& name) :
  TauRecToolBase(name) {
}


StatusCode TauClusterFinder::execute(xAOD::TauJet& tau) const {
  tau.clearClusterLinks();
    
  if (tau.jet() == nullptr) {
    ATH_MSG_ERROR("Tau jet link is invalid.");
    return StatusCode::FAILURE;
  }

  const xAOD::Jet* jetSeed = tau.jet();

  // Find all the clusters in the JetConstituent
  std::vector<const xAOD::CaloCluster*> clusterList = getClusterList(*jetSeed);

  for (const xAOD::CaloCluster* cluster : clusterList) {
    if (cluster == nullptr) {
      ATH_MSG_WARNING("Find cluster with nullptr, please check the configuration !");
      continue;
    }
    if (inEleRM()){
      static const SG::ConstAccessor<ElementLink<xAOD::CaloClusterContainer>> acc_originalObject("ERMOriginalCaloCluster");
      auto original_cluster_link = acc_originalObject(*cluster);
        if (!original_cluster_link.isValid()){
            ATH_MSG_ERROR("Original ERM cluster link is not valid");
        }
        cluster = *original_cluster_link;
    }
    
    // Clusters with negative energy will be thinned, and the elementlinks to these
    // clusters will not be valid. 
    if (cluster->rawE() <= 0.) continue;

    ElementLink<xAOD::IParticleContainer> linkToCluster;
    linkToCluster.toContainedElement(
				     *(static_cast<const xAOD::IParticleContainer*> (cluster->container())), 
				     static_cast<const xAOD::IParticle*>(cluster));
    tau.addClusterLink(linkToCluster);
  }

  return StatusCode::SUCCESS;
}



std::vector<const xAOD::CaloCluster*> TauClusterFinder::getClusterList(const xAOD::Jet& jet) const {
  std::vector<const xAOD::CaloCluster*> clusterList;

  xAOD::JetConstituentVector constituents = jet.getConstituents();
  for (const xAOD::JetConstituent* constituent : constituents) {
    ATH_MSG_DEBUG("JetConstituent: ");
    ATH_MSG_DEBUG("eta: " << constituent->eta() << " phi: " << constituent->phi() << " e: " << constituent->e()); 

    if( constituent->type() == xAOD::Type::CaloCluster ) {
      const xAOD::CaloCluster* cluster = static_cast<const xAOD::CaloCluster*>(constituent->rawConstituent());
      ATH_MSG_DEBUG("CaloCluster: ");
      ATH_MSG_DEBUG("eta: " << cluster->eta() << " phi: " << cluster->phi() << " e: " << cluster->e());
	  
      // If jet perfroms the vertex correction, then cluster in Topo jets is in a shallow copy of CaloCalTopoCluster. 
      // Since jets and the shallow copy may not be stored to AOD in R22, these clusters will be invalid with AOD as input. 
      // To fix this issue, we now retrieve the clusters in the original CaloCalTopoCluster. 
      const xAOD::IParticle* originalParticle = xAOD::getOriginalObject(*cluster);
      if (m_useOrigCluster and originalParticle != nullptr) {
        clusterList.push_back( static_cast<const xAOD::CaloCluster*>(originalParticle) );
      }
      else {
        clusterList.push_back(cluster);
      }
    }
    else if ( constituent->type() == xAOD::Type::FlowElement ) {
      const xAOD::FlowElement* fe = static_cast<const xAOD::FlowElement*>( constituent->rawConstituent() ); 
     
      if (fe->isCharged()) continue;
      if (fe->nOtherObjects()!=1){
        ATH_MSG_WARNING("Neutral PFO has " << std::to_string(fe->nOtherObjects()) << " clusters, expected exactly 1!\n");
        continue;
      }

      clusterList.push_back( dynamic_cast<const xAOD::CaloCluster*>(fe->otherObject(0)) );
    }
    else {
      ATH_MSG_WARNING("Seed jet constituent type not supported ! Skip");
      continue;
    }
  }
  
  for (const xAOD::JetConstituent* constituent : constituents) {
    // There is only one type in the constituents
    if (constituent->type() != xAOD::Type::FlowElement) break;

    const xAOD::FlowElement* fe = static_cast<const xAOD::FlowElement*>( constituent->rawConstituent() );
    if (! fe->isCharged()) continue;

    for (u_int index=0; index<fe->nOtherObjects(); index++){
      const xAOD::CaloCluster* cluster = dynamic_cast<const xAOD::CaloCluster*>(fe->otherObject(index));
      // Check it is not duplicate of one in neutral list
      if ( std::find(clusterList.begin(), clusterList.end(), cluster) == clusterList.end() ) {
         clusterList.push_back(cluster);
      }
    }
  }
 
  return clusterList;
} 

#endif
