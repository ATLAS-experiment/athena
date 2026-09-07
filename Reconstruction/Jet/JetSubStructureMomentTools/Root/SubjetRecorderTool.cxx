/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "JetSubStructureMomentTools/SubjetRecorderTool.h"
#include "xAODJet/JetContainer.h"
#include "xAODJet/JetAuxContainer.h"
#include "JetEDM/JetConstituentFiller.h"
#include "JetEDM/FastJetUtils.h"

SubjetRecorderTool::SubjetRecorderTool(const std::string& name) : 
  AsgTool(name), m_subjetlabel(""), m_subjetcontainername("")
{
  declareProperty("SubjetLabel", m_subjetlabel);
  declareProperty("SubjetContainerName", m_subjetcontainername);
}

std::vector<xAOD::Jet *> SubjetRecorderTool::recordSubjets(const std::vector<fastjet::PseudoJet>& subjets, xAOD::Jet &jet) const
{
  // Retrieve or set up subjet container
  std::string subjet_name = m_subjetlabel;
  std::string subjet_container_name = m_subjetcontainername;
  if(subjet_name.empty() || subjet_container_name.empty()) {
    ATH_MSG_ERROR("Invalid subjet label or container name");
    return {};
  }
  xAOD::JetContainer *subjet_container = nullptr;
#ifdef ROOTCORE
  subjet_container = evtStore()->retrieve<xAOD::JetContainer>(subjet_container_name);
#else
  // Need tryRetrieve to supress some Athena warning. Unfortuantely tryRetrieve isn't in RootCore
  subjet_container = evtStore()->tryRetrieve<xAOD::JetContainer>(subjet_container_name);
#endif
  if(subjet_container == nullptr) {
    StatusCode sc;
    subjet_container = new xAOD::JetContainer;
    subjet_container->setStore(new xAOD::JetAuxContainer);
    sc = evtStore()->record(subjet_container, subjet_container_name);
    if(sc.isFailure()) {
      ATH_MSG_ERROR("Error recording subjet container (" << subjet_container_name << ")");
      return {};
    }
    sc = evtStore()->record(dynamic_cast<xAOD::JetAuxContainer*>(subjet_container->getStore()), subjet_container_name + "Aux.");
    if(sc.isFailure()) {
      ATH_MSG_ERROR("Error recording subjet aux container (" << subjet_container_name << "Aux.)");
      return {};
    }
  }

  // Create xAOD::Jet's
  std::vector<const xAOD::Jet *> subj_ptrs_const;
  std::vector<xAOD::Jet *> subj_ptrs;
  for(auto it = subjets.begin(); it != subjets.end(); it++) {
    xAOD::Jet *subj = new xAOD::Jet();
    subjet_container->push_back(subj);
    subj_ptrs.push_back(subj);
    subj_ptrs_const.push_back(subj);
    
    // Set 4-mom
    subj->setJetP4(xAOD::JetFourMom_t(it->pt(), it->eta(), it->phi(), it->m()));
    subj->setJetP4(xAOD::JetConstitScaleMomentum, subj->jetP4());

    // Set constituents
    jet::JetConstituentFiller confiller;
    confiller.extractConstituents(*subj, &(*it));

    // Set author/radius
    double radius = it->associated_cluster_sequence()->jet_def().R();
		subj->setSizeParameter(radius);
		xAOD::JetAlgorithmType::ID alg = xAOD::JetAlgorithmType::algId(it->associated_cluster_sequence()->jet_def().jet_algorithm());
		subj->setAlgorithmType(alg);

    // Set association to parent
    const xAOD::JetContainer *parent_container = dynamic_cast<const xAOD::JetContainer*>(jet.container());
    ElementLink<xAOD::JetContainer> el_parent(*parent_container, jet.index());
    static const std::string parentStr{"Parent"};
    subj->setAttribute(parentStr, el_parent);
  }
  
  // Set association to subjets
  jet.setAssociatedObjects(subjet_name, subj_ptrs_const);

  return subj_ptrs;
}
