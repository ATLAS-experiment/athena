/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// TruthLinkRepointTool.cxx
// Truth links on some objects point to the main truth particle
// container, or to some other container that won't be saved in the
// output derivation.  This re-points the links from the old 
// container to the new container (and serves as a chance to clean
// up / harmonize the names of the decorations).

#include "TruthLinkRepointTool.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODEgamma/PhotonContainer.h"
#include "xAODTruth/xAODTruthHelpers.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "StoreGate/ReadHandle.h"

#include "TruthUtils/MagicNumbers.h"

// Constructor
DerivationFramework::TruthLinkRepointTool::TruthLinkRepointTool(const std::string& t,
        const std::string& n,
        const IInterface* p ) :
    base_class(t,n,p) {
}
StatusCode DerivationFramework::TruthLinkRepointTool::initialize(){
  ATH_CHECK(m_recoKey.initialize());
  if (m_decOutput.value().empty()) {
     ATH_MSG_FATAL("Please enter a a valid output decorator");
     return StatusCode::FAILURE;
  }
  ATH_CHECK(m_targetKeys.initialize());
  m_decorKey = m_recoKey.key() + "." + m_decOutput;
  ATH_CHECK(m_decorKey.initialize());
  return StatusCode::SUCCESS;
}

// Destructor
DerivationFramework::TruthLinkRepointTool::~TruthLinkRepointTool() = default;

// Function to do dressing, implements interface in IAugmentationTool
StatusCode DerivationFramework::TruthLinkRepointTool::addBranches() const {
  const EventContext& ctx = Gaudi::Hive::currentContext();
  // Retrieve the truth collections
  std::vector<const xAOD::TruthParticleContainer*> targets{};
  targets.reserve(m_targetKeys.size());

  SG::WriteDecorHandle<xAOD::IParticleContainer, ElementLink<xAOD::TruthParticleContainer> >
    output_decorator (m_decorKey, ctx);

  for (const SG::ReadHandleKey<xAOD::TruthParticleContainer>& key : m_targetKeys) {
    SG::ReadHandle<xAOD::TruthParticleContainer> readHandle{key, ctx};
    if (!readHandle.isValid()) {
      ATH_MSG_FATAL("Failed to retrieve "<<key.fullKey());
      return StatusCode::FAILURE;
    }
    targets.emplace_back(readHandle.cptr());
  }
  
  
  SG::ReadHandle<xAOD::IParticleContainer> inputCont{m_recoKey, ctx};
  if (!inputCont.isValid()) {
    ATH_MSG_FATAL("Failed to retrive "<<m_recoKey.fullKey());
    return StatusCode::FAILURE;
  }
  for ( const xAOD::IParticle* input : *inputCont) {
    const xAOD::TruthParticle* truthPart = xAOD::TruthHelpers::getTruthParticle(*input);
    output_decorator(*input) = ElementLink<xAOD::TruthParticleContainer>{};
    for (const xAOD::TruthParticleContainer* target : targets) {
        int index = find_match(truthPart, target);
        
        if (index >=0) output_decorator(*input) = ElementLink<xAOD::TruthParticleContainer>(*target, index);
        
    }
  }
  
  return StatusCode::SUCCESS;
}

// Find a match by unique ID in a different container
int DerivationFramework::TruthLinkRepointTool::find_match(const xAOD::TruthParticle* p, const xAOD::TruthParticleContainer* c)
{
  // See if it's already gone
  if (!p) return -1;
  // Look through the mini-collection
  for (int i=0;i<int(c->size());++i){
    if (c->at(i) && HepMC::is_same_particle(p,c->at(i))) return i;
  }
  // Note: just fine if it wasn't in the mini-collection
  return -1;
}
