/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// JetExternalAssocTool.cxx

#include "JetExternalAssocTool.h"
#include "StoreGate/WriteDecorHandle.h"

namespace DerivationFramework{

  //**********************************************************************

  JetExternalAssocTool::JetExternalAssocTool(const std::string& t, const std::string& n, const IInterface* p):
    base_class(t,n,p)
  {
  }

  //**********************************************************************

  StatusCode JetExternalAssocTool::initialize() {
    ATH_CHECK(m_containerName.initialize());
    ATH_CHECK(m_ExternalJetCollectionName.initialize());
    // get link names
    if(m_VectorOfNewLinkNames.empty()) m_VectorOfNewLinkNames = m_VectorOfOldLinkNames; // FIXME Properties should be set properly in the configuration layer.

    // sanity check
    if(m_VectorOfNewLinkNames.size() != m_VectorOfOldLinkNames.size()){
      ATH_MSG_ERROR("Vector size between old and new link names does not agree!");
      return StatusCode::FAILURE;
    }

    // setup vector of decorator
    for(const std::string& NewLinkName : m_VectorOfNewLinkNames){
      m_dec_keys.emplace_back(  m_containerName.key() + "." + m_momentPrefix.value() + NewLinkName); // FIXME HandleKeys should be set properly in the configuration layer.
    }

    ATH_CHECK(m_dec_keys.initialize());

    return StatusCode::SUCCESS;
  }

  //**********************************************************************

  StatusCode JetExternalAssocTool::addBranches() const{
    const EventContext& ctx = Gaudi::Hive::currentContext();
    // get jet collection to be decorated
    SG::ReadHandle<xAOD::JetContainer> jets{m_containerName, ctx};
    if (!jets.isValid()) {
      ATH_MSG_ERROR("Unable to retrieve jet collection: " << m_containerName << "!");
      return StatusCode::FAILURE;
    }

    // get external jet collection
    SG::ReadHandle<xAOD::JetContainer> ExternalJetCollection{m_ExternalJetCollectionName, ctx};
    if (!ExternalJetCollection.isValid()) {
      ATH_MSG_ERROR("Unable to find external jet collection: " << m_ExternalJetCollectionName << "!");
      return StatusCode::FAILURE;
    }

    if(!m_dRMatch){
      // Here we are assuming there is a strict one-to-one correspondence between two jet collections and both jet collections have been sorted by pT

      // basic sanity check
      if(jets->size() != ExternalJetCollection->size()){
        ATH_MSG_ERROR("Size not the same between collection " << m_containerName << " and " << m_ExternalJetCollectionName << ". Please either double check your jet collections, or turn on dRMatch option");
        return StatusCode::FAILURE;
      }

      for (unsigned int index = 0; index < jets->size(); index++) {
        auto jet = jets->at(index);
        auto jet_external = ExternalJetCollection->at(index);

        if( std::abs(jet->pt() - jet_external->pt()) > 0.1 ){ // 0.1 is hard-coded in
          ATH_MSG_WARNING("Potential inconsistency between two jet collections: " << jet->pt() << " v.s. " << jet_external->pt());
        }

        if (TransferLink(*jet, *jet_external, ctx).isFailure()) {
          ATH_MSG_ERROR("Failure when transferring link from external jet to current jet!");
          return StatusCode::FAILURE;
        }
      }
    }
    else{
      // simple dR matching

      // initialize list of un-assigned external jets
      std::vector<const xAOD::Jet*> UnAssignedExternalJets;
      for (auto jet_external : *ExternalJetCollection) {
        UnAssignedExternalJets.push_back(jet_external);
      }

      // loop
      for (auto jet : *jets) {
        // get associated jet
        double mindR = 9e9;
        const xAOD::Jet* associated_jet = 0;
        std::vector<const xAOD::Jet*>::iterator associated_iter = UnAssignedExternalJets.begin();  // random assignment

        for(auto external_iter = UnAssignedExternalJets.begin(); external_iter != UnAssignedExternalJets.end(); external_iter++){
          auto jet_external = (*external_iter);

          double dR = jet->p4().DeltaR(jet_external->p4());
          if(dR > m_dRCut) continue;
          if(dR < mindR){
            mindR = dR;
            associated_jet = jet_external;
            associated_iter = external_iter;
          }
        }

        if(associated_jet == 0){
          ATH_MSG_WARNING("Unable to find associated external jet! This jet will be skipped");
          continue;
        }
        else{
          // sanity check
          if(associated_jet != (*associated_iter)){
            ATH_MSG_ERROR("Sanity check after association fails!");
            return StatusCode::FAILURE;
          }

          // remove associated jet from unasigned jet list
          UnAssignedExternalJets.erase(associated_iter);
        }

        // transfer the link
        if (TransferLink(*jet, *associated_jet, ctx).isFailure()) {
          ATH_MSG_ERROR("Failure when transferring link from external jet to current jet!");
          return StatusCode::FAILURE;
        }
      }
    }

    return StatusCode::SUCCESS;
  }

  StatusCode JetExternalAssocTool::TransferLink(const xAOD::Jet& jet, const xAOD::Jet& jet_external, const EventContext& ctx) const{

    for(unsigned int index_link = 0; index_link < m_VectorOfOldLinkNames.size(); index_link++){
      const auto & OldLinkName = m_VectorOfOldLinkNames[index_link];
      

      // assume we are always dealing a list of IParticles
      std::vector<const xAOD::IParticle*> targetObjs;
      if (!jet_external.getAssociatedObjects<xAOD::IParticle>(OldLinkName, targetObjs)){
        ATH_MSG_ERROR("Unable to fetch link " << OldLinkName << " under associated external jet");
        return StatusCode::FAILURE;
      }

      // put it under my jet
      type_ghostlink targetLinks;
      for(auto obj : targetObjs){
        type_el el_obj;
        el_obj.toIndexedElement(*(static_cast<const xAOD::IParticleContainer*>(obj->container())), obj->index());

        targetLinks.push_back(el_obj);
      }

      SG::WriteDecorHandle<xAOD::JetContainer, type_ghostlink*> dec_handle(m_dec_keys.at(index_link), ctx);
      *dec_handle(jet) = std::move(targetLinks);

    }

    return StatusCode::SUCCESS;
  }

}
