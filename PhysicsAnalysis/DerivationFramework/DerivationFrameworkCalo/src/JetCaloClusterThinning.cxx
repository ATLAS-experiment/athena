/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// JetCaloClusterThinning.cxx, (c) ATLAS Detector software
// @author Danilo E. Ferreira de Lima <dferreir@cern.ch>
///////////////////////////////////////////////////////////////////

#include "DerivationFrameworkCalo/JetCaloClusterThinning.h"

#include "xAODCaloEvent/CaloCluster.h"
#include "xAODJet/Jet.h"

#include "GaudiKernel/ThreadLocalContext.h"
#include "StoreGate/ThinningHandle.h"
#include "xAODJet/JetConstituentVector.h"

#include <string>
#include <vector>

// Constructor
DerivationFramework::JetCaloClusterThinning::JetCaloClusterThinning(
  const std::string& t,
  const std::string& n,
  const IInterface* p)
  : base_class(t, n, p)
  , m_ntotTopo(0)
  , m_npassTopo(0)
  , m_selectionString("")
{
  declareProperty("SelectionString", m_selectionString);
}

// Destructor
DerivationFramework::JetCaloClusterThinning::~JetCaloClusterThinning() = default;

// Athena initialize and finalize
StatusCode
DerivationFramework::JetCaloClusterThinning::initialize()
{
  // Decide which collections need to be checked for ID TrackParticles
  ATH_MSG_VERBOSE("initialize() ...");
  ATH_CHECK(m_TopoClSGKey.initialize(m_streamName));
  ATH_MSG_INFO("Using " << m_TopoClSGKey.key()
                        << "as the source collection for topo calo clusters");
  if (m_sgKey.empty()) {
    ATH_MSG_FATAL("No jet collection provided for thinning.");
    return StatusCode::FAILURE;
  } else {
    ATH_MSG_INFO(
      "Calo clusters associated with objects in "
      << m_sgKey.key()
      << " will be retained in this format with the rest being thinned away");
    ATH_CHECK(m_sgKey.initialize());
  }

  for(unsigned int i=0; i < m_addClusterSGKey.size(); i++){
    m_tmpAddClusterKey = m_addClusterSGKey[i];
    ATH_CHECK(m_tmpAddClusterKey.initialize(m_streamName));
    m_addClusterKeys.push_back(m_tmpAddClusterKey);
  }

  // Set up the text-parsing machinery for selectiong the photon directly
  // according to user cuts
  if (!m_selectionString.empty()) {
    ATH_CHECK(initializeParser(m_selectionString));
  }

  return StatusCode::SUCCESS;
}

StatusCode
DerivationFramework::JetCaloClusterThinning::finalize()
{
  ATH_MSG_VERBOSE("finalize() ...");
  ATH_MSG_INFO("Processed " << m_ntotTopo << " topo clusters, of which "
                            << m_npassTopo << " were retained ");
  ATH_CHECK(finalizeParser());
  return StatusCode::SUCCESS;
}

// The thinning itself
StatusCode
DerivationFramework::JetCaloClusterThinning::doThinning() const
{
  const EventContext& ctx = Gaudi::Hive::currentContext();

  // Retrieve CalCaloTopo collection if required
  SG::ThinningHandle<xAOD::CaloClusterContainer> importedTopoCaloCluster(
    m_TopoClSGKey, ctx);

  // Check the event contains tracks
  unsigned int nTopoClusters = importedTopoCaloCluster->size();
  if (nTopoClusters == 0)
    return StatusCode::SUCCESS;

  // Set up a mask with the same entries as the full CaloCalTopoClusters collection(s)
  std::vector<bool> topomask;
  topomask.assign(nTopoClusters, false);
  m_ntotTopo += nTopoClusters;

  // Retrieve jet container
  const xAOD::JetContainer* importedJets(nullptr);
  SG::ReadHandle<xAOD::JetContainer> importedJetsHandle{ m_sgKey, ctx };
  importedJets = importedJetsHandle.ptr();
  if (importedJets == nullptr) {
    ATH_MSG_ERROR("No jet collection with name " << m_sgKey.key()
                                                 << " found in StoreGate!");
    return StatusCode::FAILURE;
  }
  unsigned int nJets(importedJets->size());
  if (nJets == 0)
    return StatusCode::SUCCESS;
  std::vector<const xAOD::Jet*> jetToCheck;
  jetToCheck.clear();

  // Execute the text parsers if requested
  if (!m_selectionString.empty()) {
    std::vector<int> entries = m_parser->evaluateAsVector();
    unsigned int nEntries = entries.size();
    // check the sizes are compatible
    if (nJets != nEntries) {
      ATH_MSG_ERROR("Sizes incompatible! Are you sure your selection string "
                    "used jets objects??");
      return StatusCode::FAILURE;
    } else {
      // identify which e-gammas to keep for the thinning check
      for (unsigned int i = 0; i < nJets; ++i)
        if (entries[i] == 1)
          jetToCheck.push_back((*importedJets)[i]);
    }

    if(jetToCheck.empty())
      return StatusCode::SUCCESS;
    
    for( const xAOD::Jet* jet : jetToCheck){
      const auto& links = jet->constituentLinks();
      for( const auto& link : links ) {
        // Check that the link is valid:                                                                                                                                                                    
        if( ! link.isValid() ) {
          continue;
        }
        topomask.at( link.index() ) = true;
      }
    }
  }
  else{
    for( const xAOD::Jet* jet : *importedJets){
      const auto& links = jet->constituentLinks();
      for( const auto& link : links ) {
	// Check that the link is valid:
	if( ! link.isValid() ) {
	  continue;
	}
	topomask.at( link.index() ) = true;
      }
    }
  }

  // Count up the mask contents
  for (unsigned int i = 0; i < nTopoClusters; ++i) {
    if (topomask[i])
      ++m_npassTopo;
  }

  // Execute the thinning service based on the mask. Finish.
  importedTopoCaloCluster.keep(topomask);

  for(const auto & addClusterKey : m_addClusterKeys){
    SG::ThinningHandle<xAOD::CaloClusterContainer>  tempClusters(addClusterKey);
    tempClusters.keep(topomask);
  }

  return StatusCode::SUCCESS;
}
