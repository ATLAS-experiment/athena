
/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigNavSlimmingMTAlg.h"

#include "TrigTimeAlgs/TrigTimeStamp.h"

#include "xAODParticleEvent/ParticleAuxContainer.h"
#include "xAODTrigMissingET/TrigMissingETAuxContainer.h"

using namespace TrigCompositeUtils;


// Particle Specialization
// We transiently interact via the xAOD::IParticle interface.
// There is code inside the TrigCompositeUtils::Decision to check
// at runtime if this inheritance is possible.
// But we copy into a xAOD::Particle object.
template<>
StatusCode TrigNavSlimmingMTAlg::doRepack(TrigCompositeUtils::Decision* decision,
  SG::WriteHandle<xAOD::ParticleContainer>* writeHandle,
  const std::string& edgeName) const
{

  if (not decision->hasObjectLink(edgeName, ClassID_traits<xAOD::IParticleContainer>::ID())) { // Note: IParticle
    // Nothing to do
    return StatusCode::SUCCESS;
  }

  ElementLink<xAOD::IParticleContainer> currentEL = decision->objectLink<xAOD::IParticleContainer>(edgeName); // Note: IParticle

  if (!currentEL.isValid()) {
    // TODO: Upgrade this first message to a WARNING once the TriggerAPI for Run3 is filtering on the chains whose final-features get saved into the DAOD_PHYS
    ATH_MSG_DEBUG("Unable to repack '" << edgeName << "' of container type xAOD::IParticleContainer for '"
      << decision->name() << "' node, the link is invalid.");
    ATH_MSG_DEBUG("Dump of DecisionObject: " << *decision);
    return StatusCode::SUCCESS;
  }

  if (not m_repackFeaturesExclusionList.empty()) {
    // Check the feature's StoreGate key against the exclusion list. If it is on the list, then we don't repack the feature.
    // We instead leave it pointing to its current online physics object. It is assumed that this online physics object will 
    // be kept in the DAOD via signature specific logic.
    const std::string featureStoreGateKey = currentEL.dataID();
    for (const std::string& exclusionEntry : m_repackFeaturesExclusionList) {
      if (featureStoreGateKey == exclusionEntry) {
        ATH_MSG_VERBOSE("Will not repack this feature " << currentEL.index() << " from " <<  currentEL.dataID() << ", as this container is on the exclusion list");
        return StatusCode::SUCCESS;
      }
    }
  }

  (**writeHandle).push_back( new xAOD::Particle() ); // Need to do this before performing the copy to assign with the Aux store

  const xAOD::IParticle* current = *currentEL;
  xAOD::Particle* remapped = (**writeHandle).back();

  remapped->setP4( current->p4() );

  ElementLink<xAOD::ParticleContainer> remappedEL(**writeHandle, (**writeHandle).size()-1);
  decision->setObjectLink<xAOD::ParticleContainer>(edgeName, remappedEL); // Overwrite the existing link

  ATH_MSG_VERBOSE("Repacked from index:" << currentEL.index() << " from key:" << currentEL.dataID() 
    << ", to index:" << remappedEL.index() << " to key:" << remappedEL.dataID());

  return StatusCode::SUCCESS;
}


// ROI Specialization.
// Not an xAOD object, can use a direct copy constructor.
template<>
StatusCode TrigNavSlimmingMTAlg::doRepackCopy(const TrigRoiDescriptor* object,
  SG::WriteHandle<TrigRoiDescriptorCollection>* writeHandle) const
{
  // Specialization for ROIs, utilize copy constructor (no Aux store here)
  (**writeHandle).push_back( new TrigRoiDescriptor(*object) );
  return StatusCode::SUCCESS;
}


TrigNavSlimmingMTAlg::TrigNavSlimmingMTAlg(const std::string& name, ISvcLocator* pSvcLocator)
  : AthReentrantAlgorithm(name, pSvcLocator)
{
}


StatusCode TrigNavSlimmingMTAlg::initialize() {
  ATH_CHECK( m_primaryInputCollection.initialize() );
  ATH_CHECK( m_outputCollection.initialize() );
  ATH_CHECK( m_outputRepackedROICollectionKey.initialize(m_repackROIs) );
  ATH_CHECK( m_outputRepackedFeaturesCollectionKey_Particle.initialize(m_repackFeatures) );
  ATH_CHECK( m_outputRepackedFeaturesCollectionKey_MET.initialize(m_repackFeatures) );

  const bool removeRoI = (std::find(m_edgesToDrop.begin(), m_edgesToDrop.end(), roiString()) != m_edgesToDrop.end());
  const bool removeInitialRoI = (std::find(m_edgesToDrop.begin(), m_edgesToDrop.end(), initialRoIString()) != m_edgesToDrop.end());

  if (m_repackROIs and (removeRoI or removeInitialRoI)) {
    ATH_MSG_WARNING("Possible miss-configuration. Cannot repack ROIs in the navigation slimming if they are being dropped");
  }

  if (not m_trigDec.empty()) {
    ATH_CHECK( m_trigDec.retrieve() );
  }
  for (const std::string& output : m_allOutputContainers) {
    if (output == m_primaryInputCollection.key()) {
      continue; // We do want to search for failed nodes in the primary input (it may already be merged)
    }
    // We don't want to search for failed nodes in other possible summary keys, we might read in the 
    // summary collection from another running instance (e.g. an AODSlim alg reading in the output of
    // ESDSlim in a RAWtoALL job).
    m_allOutputContainersSet.insert(output);
  }
  m_allOutputContainersSet.insert(m_outputCollection.key());
  msg() << MSG::INFO << "Initialized. Will *not* inspect the following SG Keys: ";
  for (const std::string& key : m_allOutputContainersSet) {
    msg() << key << " ";
  }
  msg() << endmsg;
  return StatusCode::SUCCESS;
}


StatusCode TrigNavSlimmingMTAlg::execute(const EventContext& ctx) const {

  // Prepare IO
  Outputs outputContainers;

  SG::WriteHandle<TrigRoiDescriptorCollection> outputROIs;
  SG::WriteHandle<xAOD::ParticleContainer> outputParticles; 
  SG::WriteHandle<xAOD::TrigMissingETContainer> outputMETs;
  if (m_repackROIs) {
    outputROIs = createAndStoreNoAux(m_outputRepackedROICollectionKey, ctx);
    outputContainers.rois = &outputROIs;
  }
  if (m_repackFeatures) {
    outputParticles = createAndStoreWithAux<xAOD::ParticleContainer, xAOD::ParticleAuxContainer>(m_outputRepackedFeaturesCollectionKey_Particle, ctx);
    outputContainers.particles = &outputParticles;
  }
  if (m_repackMET) {
    outputMETs = createAndStoreWithAux<xAOD::TrigMissingETContainer, xAOD::TrigMissingETAuxContainer>(m_outputRepackedFeaturesCollectionKey_MET, ctx);
    outputContainers.mets = &outputMETs;
  }

  SG::WriteHandle<DecisionContainer> outputNavigation = createAndStore(m_outputCollection, ctx);
  outputContainers.nav = &outputNavigation;

  SG::ReadHandle<DecisionContainer> primaryInputHandle = SG::ReadHandle(m_primaryInputCollection, ctx);
  ATH_CHECK(primaryInputHandle.isValid());

  const Decision* terminusNode = TrigCompositeUtils::getTerminusNode(*primaryInputHandle);
  if (!terminusNode) {
    ATH_MSG_ERROR("Unable to locate the HLTPassRaw from the primary input navigation collection, size:" << primaryInputHandle->size());
    return StatusCode::FAILURE;
  }

  const Decision* expressTerminusNode = TrigCompositeUtils::getExpressTerminusNode(*primaryInputHandle);
  
  // Stage 1. Build a transient representation of the navigation graph.
  TrigTimeStamp stage1;
  NavGraph transientNavGraph;

  // We can optionally only keep data for a given set of chains. An empty set means to keep for all chains.
  DecisionIDContainer chainIDs = {};
  if (not m_chainsFilter.empty()) {
    ATH_CHECK(fillChainIDs(chainIDs));
    ATH_MSG_DEBUG("Supplied " << m_chainsFilter.size() << " chain patterns. This converts to " << chainIDs.size() << " DecisionIDs to be preserved.");
  }
  if (chainIDs.size() == 0) {
    ATH_MSG_DEBUG("chainIDs size is zero. No HLT-chain based filtering of the navigation graph will be performed.");
  }

  std::set<const Decision*> fullyExploredFrom;
  // Note: We use the "internal" version of this call such that we maintain our own cache, 
  // as we may need to call this more than once if keepFailedBranches is true
  TrigCompositeUtils::recursiveGetDecisionsInternal(terminusNode, 
    nullptr, // 'Coming from' is nullptr for the first call of the recursive function
    transientNavGraph,
    fullyExploredFrom, 
    chainIDs, 
    /*enforce chainIDs on terminus node*/ true);

  ATH_MSG_DEBUG("Collated nodes from passing paths, now have " << transientNavGraph.nodes() << " nodes with " << transientNavGraph.edges() << " edges");

  // Stage 2. We can optionally include branches through the graph which were never accepted by any chain.
  TrigTimeStamp stage2;
  // These branches do not connect to the terminusNode, so we have to go hunting them explicitly.
  // We need to pass in the evtStore as these nodes can be spread out over numerous collections.
  // Like with the terminus node, we can restrict this search to only nodes which were rejected by certain chains.
  // We also want to restrict the search to exclude the output collections of any other TrigNavSlimminMTAlg instances
  // and let the function know what the primary input collection is - from the name of this we can tell if we need to search one or many containers.
  if (m_keepFailedBranches) {
    std::vector<const Decision*> rejectedNodes = TrigCompositeUtils::getRejectedDecisionNodes(&*evtStore(), ctx, m_primaryInputCollection.key(), chainIDs, m_allOutputContainersSet);
    for (const Decision* rejectedNode : rejectedNodes) {
      // We do *not* enforce that a member of chainIDs must be present in the starting node (rejectedNode)
      // specifically because we know that at least one of chainIDs was _rejected_ here, but is active in the rejected
      // node's seeds.
      TrigCompositeUtils::recursiveGetDecisionsInternal(rejectedNode, 
        nullptr, // 'Coming from' is nullptr for the first call of the recursive function
        transientNavGraph,
        fullyExploredFrom, 
        chainIDs, 
        /*enforce chainIDs on terminus node*/ false);
    }
    ATH_MSG_DEBUG("Collated nodes from failing paths, now have " << transientNavGraph.nodes() << " nodes with " << transientNavGraph.edges() << " edges");
  }

  // Stage 3. Walk all paths through the graph. Flag for thinning.
  TrigTimeStamp stage3;
  // Final nodes includes the terminus node, plus any rejected nodes (if these were collated).
  TrigCompositeUtils::recursiveFlagForThinning(transientNavGraph, m_keepOnlyFinalFeatures, m_removeEmptySteps, m_nodesToDrop);

  if (msg().level() <= MSG::VERBOSE) {
    ATH_MSG_VERBOSE("The navigation graph entering the slimming is:");
    transientNavGraph.printAllPaths(msg(), MSG::VERBOSE);
  }

  // Stage 4. Do the thinning. Re-wire removed nodes as we go.
  TrigTimeStamp stage4;
  const size_t nodesBefore = transientNavGraph.nodes();
  const size_t edgesBefore = transientNavGraph.edges();
  std::vector<const Decision*> thinnedInputNodes = transientNavGraph.thin();

  // TODO - thinnedInputNodes will be dropped, these may link to "features", "roi", or other objects in other containers.
  // Need to let the slimming svc know that we no longer need the objects pointed to here, and hence they can be thinned.

  ATH_MSG_DEBUG("Trigger navigation graph thinning going from " << nodesBefore << " nodes with " << edgesBefore << " edges, to "
    << transientNavGraph.nodes() << " nodes with " << transientNavGraph.edges() << " edges");

  if (msg().level() <= MSG::VERBOSE) {
    ATH_MSG_VERBOSE("The navigation graph has been slimmed to the following paths:");
    transientNavGraph.printAllPaths(msg(), MSG::VERBOSE);
  }

  // Stage 5. Fill the transientNavGraph structure (with NavGraphNode* nodes) back into an xAOD::DecisionContainer (with xAOD::Decision* nodes).
  TrigTimeStamp stage5;
  IOCacheMap cache; // Used to keep a one-to-one relationship between the const input Decision* and the mutable output Decision*
  // Do the terminus node first - such that it ends up at index 0 of the outputNavigation (fast to locate in the future)
  Decision* terminusNodeOut = nullptr;
  ATH_CHECK(inputToOutput(terminusNode, &terminusNodeOut, cache, outputContainers, chainIDs, ctx));
  if (expressTerminusNode) {
    // Do the express terminus node second - such that it ends up at index 1 of the outputNavigation (fast to locate in the future)
    Decision* expressTerminusNodeOut = nullptr;
    ATH_CHECK(inputToOutput(expressTerminusNode, &expressTerminusNodeOut, cache, outputContainers, chainIDs, ctx));
  }
  // Don't have to walk the graph here, just iterate through the set of (thinned) nodes.
  // We won't end up with two terminus nodes because of this (it checks that the node hasn't already been processed)
  const std::vector<NavGraphNode*> allNodes = transientNavGraph.allNodes();
  for (const NavGraphNode* inputNode : allNodes) {
    Decision* outputNode = nullptr;
    ATH_CHECK(inputToOutput(inputNode->node(), &outputNode, cache, outputContainers, chainIDs, ctx));
    // TODO - anything else to do here with outputNode? We cannot hook up its seeding yet, we may not yet have output nodes for all of its seeds.
  }
  // Now we have all of the new nodes in the output collection, can link them all up with their slimmed seeding relationships.
  for (const NavGraphNode* inputNode : allNodes) {
    ATH_CHECK(propagateSeedingRelation(inputNode, cache, ctx));
  }

  // We can perform an additional check on the output graph, we put a veto on the m_keepFailedBranches option as we are currently just exploring
  // from the 'terminusNodeOut', more code would be needed to locate failing branches also in the output graph structure.
  if (msg().level() <= MSG::VERBOSE && !m_keepFailedBranches) {
    ATH_MSG_VERBOSE("The output navigation graph looks like this (output terminus node search only, converted back into a NavGraph one final time for printing)");
    std::set<const Decision*> fullyExploredFromOut;
    NavGraph transientNavGraphOut;
    TrigCompositeUtils::recursiveGetDecisionsInternal(terminusNodeOut, 
      nullptr, // 'Coming from' is nullptr for the first call of the recursive function
      transientNavGraphOut,
      fullyExploredFromOut, 
      chainIDs, 
      /*enforce chainIDs on terminus node*/ true);
    transientNavGraphOut.printAllPaths(msg(), MSG::VERBOSE);
  }

  if (msg().level() <= MSG::DEBUG) {
    ATH_MSG_DEBUG("Navigation slimming and thinning timings:");
    ATH_MSG_DEBUG("  1. Transient Graph of Passed Nodes = " << stage1.millisecondsDifference(stage2) << " ms");
    ATH_MSG_DEBUG("  2. Transient Graph of Failed Nodes = " << stage2.millisecondsDifference(stage3) << " ms");
    ATH_MSG_DEBUG("  3. Flag Transient Graph For Thinning = " << stage3.millisecondsDifference(stage4) << " ms");
    ATH_MSG_DEBUG("  4. Perform Transient Graph Thinning = " << stage4.millisecondsDifference(stage5) << " ms");
    ATH_MSG_DEBUG("  5. Write xAOD Graph = " << stage5.millisecondsSince() << " ms");
  }

  return StatusCode::SUCCESS;  
}

std::vector<size_t> TrigNavSlimmingMTAlg::lookupHardCodedLegMultiplicities(const std::string& chain) const {
  if (chain == "HLT_id_cosmicid_L1MU11_EMPTY") return std::vector<size_t>(1,1); // size = 1, value at index 0 = 1
  return std::vector<size_t>();
}

StatusCode TrigNavSlimmingMTAlg::fillChainIDs(DecisionIDContainer& chainIDs) const {
  for (const std::string& filter : m_chainsFilter) {
    // We do this as filter->chains stage as filter could be a regexp matching a large number of chains
    const Trig::ChainGroup* cg = m_trigDec->getChainGroup(filter);
    std::vector<std::string> chains = cg->getListOfTriggers();
    for (const std::string& chain : chains) {
      const TrigConf::HLTChain* hltChain = m_trigDec->ExperimentalAndExpertMethods().getChainConfigurationDetails(chain);
      const HLT::Identifier chainID( hltChain->chain_name() );
      chainIDs.insert( chainID.numeric() );
      std::vector<size_t> legMultiplicites = hltChain->leg_multiplicities();
      ATH_MSG_VERBOSE("Including " << chain << " and its " << legMultiplicites.size() << " legs in the trigger slimming output");
      if (legMultiplicites.size() == 0) {
        legMultiplicites = lookupHardCodedLegMultiplicities(chain);
        if (legMultiplicites.size() == 0) {
          ATH_MSG_ERROR("chain " << chainID << " has invalid configuration, no multiplicity data.");
          return StatusCode::FAILURE;
        }
      }
      if (legMultiplicites.size() > 1) {
        // For multi-leg chains, the DecisionIDs are handled per leg.
        // We don't care here exactly how many objects are required per leg, just that there are two-or-more legs
        for (size_t legNumeral = 0; legNumeral < legMultiplicites.size(); ++legNumeral) {
          const HLT::Identifier legID = TrigCompositeUtils::createLegName(chainID, legNumeral);
          chainIDs.insert( legID.numeric() );
        }
      }
    }
  }
  return StatusCode::SUCCESS;
}


StatusCode TrigNavSlimmingMTAlg::inputToOutput(
  const TrigCompositeUtils::Decision* input, 
  TrigCompositeUtils::Decision** output,
  IOCacheMap& cache, 
  Outputs& outputContainers,
  const DecisionIDContainer& chainIDs,
  const EventContext& ctx) const
{
  IOCacheMap::const_iterator it = cache.find(input);
  if (it != cache.end()) {
    *output = it->second;
  } else {
    *output = newDecisionIn(outputContainers.nav->ptr(), input, input->name(), ctx);
    ATH_CHECK(propagateLinks(input, *output));
    ATH_CHECK(propagateDecisionIDs(input, *output, chainIDs));
    ATH_CHECK(repackLinks(*output, outputContainers));
    cache[input] = *output;
  }
  return StatusCode::SUCCESS;
}


StatusCode TrigNavSlimmingMTAlg::propagateSeedingRelation( 
  const TrigCompositeUtils::NavGraphNode* inputNode, 
  IOCacheMap& cache,
  const EventContext& ctx) const
{
  const Decision* inputDecision = inputNode->node(); // The incoming Decision objects, with links into the old graph 
  Decision* outputDecision = nullptr; // The outgoing Decision object, without graph links so far
  {
    IOCacheMap::const_iterator it = cache.find(inputDecision);
    ATH_CHECK( it != cache.end() );
    outputDecision = it->second;
  }
  for (const NavGraphNode* seed : inputNode->seeds()) {
    const Decision* inputSeedDecision = seed->node(); // A Decision object the incoming Decision object links to (old graph)
    const Decision* outputSeedDecision = nullptr; // The equivalent Decision Object in the slimmed outgoing graph
    {
      IOCacheMap::const_iterator it = cache.find(inputSeedDecision);
      ATH_CHECK( it != cache.end() );
      outputSeedDecision = it->second;
    }
    // Perform the linking only using nodes from the slimmed output graph
    TrigCompositeUtils::linkToPrevious(outputDecision, outputSeedDecision, ctx);
  }

  // Don't run this check for "HLTPassRaw", this node is expected to link back to every passing physics object. 
  // Hence there may be more than 'sensibleUpperBoundOnNLinks' in aggregate here.
  if (m_runtimeValidation and outputDecision->name() != TrigCompositeUtils::summaryPassNodeName()) { 
    const size_t sensibleUpperBoundOnNLinks = 100;
    const size_t maxUpperBoundOnNLinks = 500;
    // Note: Only in the NavGraphNode do we have the two-way links to check how many children link back to this node
    const bool bad_in = inputNode->children().size() > sensibleUpperBoundOnNLinks; 
    //Note: Here we check more than "seed" links. We pick up external links too like "feature"
    const bool bad_out = outputDecision->linkColNames().size() > sensibleUpperBoundOnNLinks; 
    const bool vbad = inputNode->children().size() > maxUpperBoundOnNLinks or outputDecision->linkColNames().size() > maxUpperBoundOnNLinks;
    if (bad_in) {
      ATH_MSG_WARNING("Saving a Decision object with a very large number of INCOMING graph edges. Number of in-edges: " << inputNode->children().size());
    }
    if (bad_out) {
      ATH_MSG_WARNING("Saving a Decision object with a very large number of OUTGOING graph edges. Number of out-edges: " << outputDecision->linkColNames().size());
    }
    if (bad_in or bad_out) {
      ATH_MSG_WARNING("Comes from: " << TrigCompositeUtils::decisionToElementLink(inputDecision, ctx).dataID());
      ATH_MSG_DEBUG("Output Decision: " << *outputDecision);
    }
    if (vbad) {
      ATH_MSG_ERROR("More than " << maxUpperBoundOnNLinks << " links, printing an ERROR such that this gets promptly investigated and reduced.");
    }
  }
  return StatusCode::SUCCESS;
}


StatusCode TrigNavSlimmingMTAlg::propagateLinks(  
  const TrigCompositeUtils::Decision* input, 
  TrigCompositeUtils::Decision* output) const
{
  // ElementLinks form the edges in the graph. Start by copying all of them over.

  output->copyAllLinksFrom( input );

  // Special behaviour to save additional disk space for keepOnlyFinalFeatures mode.
  // In keepOnlyFinalFeatures we stop at the first hypoAlgNode, hence we will drop the preceding inputMakerNode, and all prior Steps in their entirety.
  // This means we will also drop all "roi" edges, including the final ROI.
  // We may want to keep this final "roi", and so to do this we can copy it down one level (away from L1) to live in the hypoAlgNode along side the "feature" link.
  // Note: If "roi" is listed in m_edgesToDrop then we will still immediately drop this new element link in the m_edgesToDrop loop below. 
  const std::vector<ElementLink<DecisionContainer>> seeds = input->objectCollectionLinks<DecisionContainer>(seedString());
  const Decision* const firstParent = (seeds.size() ? *seeds.at(0) : nullptr);
  if (m_keepOnlyFinalFeatures &&
      input->name() == hypoAlgNodeName() &&
      firstParent &&
      firstParent->name() == inputMakerNodeName() &&
      firstParent->hasObjectLink(roiString()))
  {
    output->copyLinkFrom( firstParent, roiString() );
  }

  for (const std::string& toRemove : m_edgesToDrop) {
    output->removeObjectLink(toRemove);
    output->removeObjectCollectionLinks(toRemove);
    // TODO - let the slimming svc know that we no longer need these objects
  }

  // Do not propagate "seed" links - TrigNavSlimmingMTAlg will 
  // propagate these following additional logic
  output->removeObjectCollectionLinks( seedString() );

  return StatusCode::SUCCESS;
}


StatusCode TrigNavSlimmingMTAlg::propagateDecisionIDs(  
  const TrigCompositeUtils::Decision* input, 
  TrigCompositeUtils::Decision* output,
  const DecisionIDContainer& chainIDs) const
{

  // Get all DecisionIDs from the const input Decision*
  DecisionIDContainer fromInput;
  decisionIDs(input, fromInput);

  DecisionIDContainer toOutput;

  if (chainIDs.size()) {
    // Applying ChainsFilter to the set of DecisionIDs
    std::set_intersection(fromInput.begin(), fromInput.end(), chainIDs.begin(), chainIDs.end(),
      std::inserter(toOutput, toOutput.begin()));
  } else {
    // Copying all DecisionIDs from input to output
    toOutput.insert(fromInput.begin(), fromInput.end());
  }

  // Set the DecisionIDs into the mutable output Decision* 
  insertDecisionIDs(toOutput, output);

  return StatusCode::SUCCESS;
}

void TrigNavSlimmingMTAlg::printIParticleRepackingDebug(const TrigCompositeUtils::Decision* output, const std::string& when) const {
  if (not msgLvl(MSG::DEBUG)) return;
  if (output->hasObjectLink(featureString(), ClassID_traits<xAOD::IParticleContainer>::ID())) {
    ElementLink<xAOD::IParticleContainer> link = output->objectLink<xAOD::IParticleContainer>(featureString());
    if (link.isValid()) {
      const xAOD::IParticle& l = **link;
      ATH_MSG_DEBUG("IParticle repacking debug. " << when << " : " 
        << "(pt:" << l.pt() << ",eta:" << l.eta() << ",phi:" << l.phi() << ",m:" << l.m() << ",e:" << l.e() << ")"
        << " from:" << link.dataID());
      if (when == " After") ATH_MSG_DEBUG("--");
    }
  }
}


StatusCode TrigNavSlimmingMTAlg::repackLinks(
  TrigCompositeUtils::Decision* output,
  Outputs& outputContainers) const
{
  
  if (m_repackROIs) {
    ATH_CHECK( doRepack<TrigRoiDescriptorCollection>(output, outputContainers.rois, roiString()) );
    ATH_CHECK( doRepack<TrigRoiDescriptorCollection>(output, outputContainers.rois, initialRoIString()) );
  }

  if (m_repackFeatures) {
    // Debug printing. Look at the four-momentum of any feature before the repacking.
    // Note: Transiently we interact with the IParticle interface.
    printIParticleRepackingDebug(output, "Before");

    // Do any IParticle repacking
    ATH_CHECK( doRepack<xAOD::ParticleContainer>(output, outputContainers.particles, featureString()) );
    ATH_CHECK( doRepack<xAOD::ParticleContainer>(output, outputContainers.particles, "subfeature") );

    // Debug printing. Look at the four-momentum of any feature after the repacking (the stored link is re-written)
    printIParticleRepackingDebug(output, " After");
  }

  // Some features do not support an IParticle interface. These need their own containers.
  // TODO. Apply some thinning?
  if (m_repackMET) {
    ATH_CHECK( doRepack<xAOD::TrigMissingETContainer>(output, outputContainers.mets, featureString()) );
  }

  return StatusCode::SUCCESS;
}
