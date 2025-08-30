/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// See similar workaround the lack of CLID in standalone releases in TrigComposite_v1.h
#include "xAODBase/IParticleContainer.h"

#include "AsgDataHandles/WriteHandle.h"
#include "AsgDataHandles/ReadHandle.h"
#include "AthContainers/AuxElement.h"

#include "TrigCompositeUtils/TrigCompositeUtils.h"

#include <format>

static const SG::AuxElement::Accessor< std::vector<TrigCompositeUtils::DecisionID> > readWriteAccessor("decisions");
static const SG::AuxElement::ConstAccessor< std::vector<TrigCompositeUtils::DecisionID> > readOnlyAccessor("decisions");



namespace TrigCompositeUtils {  

  ANA_MSG_SOURCE (msgFindLink, "TrigCompositeUtils.findLink")
  ANA_MSG_SOURCE (msgRejected, "TrigCompositeUtils.getRejectedDecisionNodes")


  SG::WriteHandle<DecisionContainer> createAndStore( const SG::WriteHandleKey<DecisionContainer>& key, const EventContext& ctx ) {
    SG::WriteHandle<DecisionContainer> handle( key, ctx );
    auto data = std::make_unique<DecisionContainer>() ;
    auto aux = std::make_unique<DecisionAuxContainer>() ;
    data->setStore( aux.get() );
    handle.record( std::move( data ), std::move( aux )  ).ignore();
    return handle;
  }

  void createAndStore( SG::WriteHandle<DecisionContainer>& handle ) {
    auto data = std::make_unique<DecisionContainer>() ;
    auto aux = std::make_unique<DecisionAuxContainer>() ;
    data->setStore( aux.get() );
    handle.record( std::move( data ), std::move( aux )  ).ignore();
  }

  Decision* newDecisionIn ( DecisionContainer* dc, const std::string& name) {
    Decision * x = new Decision;
    dc->push_back( x );
    if ( ! name.empty() ) {
      x->setName( name );
    }
    return x;
  }

  Decision* newDecisionIn ( DecisionContainer* dc, const Decision* dOld, const std::string& name, const EventContext& ctx ) {
    Decision* dNew =  newDecisionIn( dc, name );
    linkToPrevious(dNew, dOld, ctx); // Sets up link to 'seed' collection, points to dOld
    return dNew;
  }

  void addDecisionID( DecisionID id,  Decision* d ) {   
    std::vector<DecisionID>& decisions = readWriteAccessor( *d );
    if ( decisions.size() == 0 or decisions.back() != id) 
      decisions.push_back( id );
  }
  
  void decisionIDs( const Decision* d, DecisionIDContainer& destination ) {    
    const std::vector<DecisionID>& decisions = readOnlyAccessor( *d );    
    destination.insert( decisions.begin(), decisions.end() );
  }

  const std::vector<DecisionID>& decisionIDs( const Decision* d ) {    
    return readOnlyAccessor( *d );    
  }

  std::vector<DecisionID>& decisionIDs( Decision* d ) {
    return readWriteAccessor( *d );
  }

  void insertDecisionIDs(const Decision* src, Decision* dest ){
    DecisionIDContainer srcIds;
    decisionIDs( src, srcIds ); // Now stored in a set
    insertDecisionIDs( srcIds, dest);
  }

  void insertDecisionIDs( const DecisionIDContainer& src, Decision* dest ) {
    DecisionIDContainer collateIDs;
    // Decision are xAOD objects backed by a std::vector
    // Here we use a std::set to de-duplicate IDs from src and dest before setting dest
    decisionIDs( dest, collateIDs ); // Set operation 1. Get from dest
    collateIDs.insert( src.begin(), src.end() ); // Set operation 2. Get from src
    std::vector<DecisionID>& vdest = decisionIDs( dest );
    // Clear and reserve target
    vdest.clear();
    vdest.reserve(collateIDs.size());
    // Copy from set to (ordered) vector
    vdest.insert( vdest.end(), collateIDs.begin(), collateIDs.end() );
  }

  void uniqueDecisionIDs(Decision* dest) {
    // Re-use above insertDecisionIDs method.
    // This implicitly performs de-duplication
    return insertDecisionIDs(dest, dest);
  }

  bool allFailed( const Decision* d ) {
    const std::vector<DecisionID>& decisions = readOnlyAccessor( *d );    
    return decisions.empty();
  }

  bool isAnyIDPassing( const Decision* d,  const DecisionIDContainer& required ) {
    for ( DecisionID id : readOnlyAccessor( *d ) ) {
      if ( required.find( id ) != required.end() ) {
        return true;
      }
    }
    return false;
  }

  bool passed( DecisionID id, const DecisionIDContainer& idSet ) {
    return idSet.find( id ) != idSet.end();
  }

#if !defined(XAOD_STANDALONE) && !defined(XAOD_ANALYSIS) // Full athena
  ElementLink<DecisionContainer> decisionToElementLink(const Decision* d, const EventContext& ctx) {
    const DecisionContainer* container = dynamic_cast<const DecisionContainer*>( d->container() );
    if( ! container ) {
      throw std::runtime_error("TrigCompositeUtils::convertToElementLink Using convertToElementLink(d) requires that the Decision d is already in a container");
    }
    return ElementLink<DecisionContainer>(*container, d->index(), ctx);
  }
#else // Analysis or Standalone
  ElementLink<DecisionContainer> decisionToElementLink(const Decision* d, const EventContext&) {
    const DecisionContainer* container = dynamic_cast<const DecisionContainer*>( d->container() );
    if( ! container ) {
      throw std::runtime_error("TrigCompositeUtils::convertToElementLink Using convertToElementLink(d) requires that the Decision d is already in a container");
    }
    return ElementLink<DecisionContainer>(*container, d->index());
  }
#endif

  void linkToPrevious( Decision* d, const std::string& previousCollectionKey, size_t previousIndex  ) {
    ElementLink<DecisionContainer> seed = ElementLink<DecisionContainer>( previousCollectionKey, previousIndex );
    if (!seed.isValid()) {
      throw std::runtime_error("TrigCompositeUtils::linkToPrevious Invalid Decision Link key or index provided");
    } else {
      d->addObjectCollectionLink(seedString(), seed);
    }
  }

  void linkToPrevious( Decision* d, const Decision* dOld, const EventContext& ctx ) {
    d->addObjectCollectionLink(seedString(), decisionToElementLink(dOld, ctx));
  }

  bool hasLinkToPrevious( const Decision* d ) {
    return d->hasObjectCollectionLinks( seedString() );
  }

  const std::vector<ElementLink<DecisionContainer>> getLinkToPrevious( const Decision* d ) {
    return d->objectCollectionLinks<DecisionContainer>( seedString() );
  }


  bool copyLinks(const Decision* src, Decision* dest) {
    return dest->copyAllLinksFrom(src);
  }

  HLT::Identifier createLegName(const HLT::Identifier& chainIdentifier, size_t counter) {
    return createLegName( chainIdentifier.name(), counter );
  }

  HLT::Identifier createLegName(const std::string& name, size_t counter) {
    if (!isChainId(name)) {
      throw std::runtime_error("TrigCompositeUtils::createLegName chainIdentifier '"+name+"' does not start with 'HLT_'");
    }
    if (counter > 999) {
      throw std::runtime_error("TrigCompositeUtils::createLegName Leg counters above 999 are invalid.");
    }
    return HLT::Identifier( std::format("leg{:0>3d}_{}", counter, name) );
  }

  HLT::Identifier getIDFromLeg(const HLT::Identifier& legIdentifier) {
    const std::string& name = legIdentifier.name();
    if (isChainId(name)){
      return legIdentifier;
    } else if (isLegId(name)){
      return HLT::Identifier(name.substr(7));
    } else{
      throw std::runtime_error("TrigCompositeUtils::getIDFromLeg legIdentifier '"+name+"' does not start with 'HLT_' or 'leg' ");
    }
  }

  int32_t getIndexFromLeg(const HLT::Identifier& legIdentifier) {
    return getIndexFromLeg(legIdentifier.name());
  }

  int32_t getIndexFromLeg(const std::string& name) {
    int32_t id = 0;
    if (isChainId(name)){
      // pass
    } else if (isLegId(name)) {
      std::from_chars(name.data()+3, name.data()+6, id);
    } else {
      throw std::runtime_error("TrigCompositeUtils::getIndexFromLeg legIdentifier '"+name+"' does not start with 'HLT_' or 'leg' ");
    }
    return id;
  }

  std::pair<std::string, int32_t> getNameAndIndexFromLeg(const std::string& name) {
    int32_t id = 0;
    if (isChainId(name)) {
      return {name, id};
    } else if (isLegId(name)) {
      std::from_chars(name.data()+3, name.data()+6, id);
      return {name.substr(7), id};
    } else {
      throw std::runtime_error("TrigCompositeUtils::getIDFromLeg legIdentifier '"+name+"' does not start with 'HLT_' or 'leg' ");
    }
  }

  bool isLegId(const HLT::Identifier& legIdentifier) {
    return isLegId(legIdentifier.name());
  }
  
  bool isLegId(const std::string& name) {
    return name.starts_with("leg");
  }

  bool isChainId(const HLT::Identifier& chainIdentifier) {
    return isChainId(chainIdentifier.name());
  }

  bool isChainId(const std::string& name) {
    return name.starts_with("HLT_");
  }
  
  
  const Decision* find( const Decision* start, const std::function<bool( const Decision* )>& filter ) {
    if ( filter( start ) ) return start;

    if ( hasLinkToPrevious(start) ) {
      const std::vector<ElementLink<DecisionContainer>> seeds = getLinkToPrevious(start);
      for (const ElementLink<DecisionContainer>& seedEL : seeds) {
        const Decision* result = find( *seedEL, filter );
        if (result) return result;
      }
    }
    
    return nullptr;
  }

  bool HasObject::operator()( const Decision* composite ) const {
    return composite->hasObjectLink( m_name );
  }

  bool HasObjectCollection::operator()( const Decision* composite ) const {
    return composite->hasObjectCollectionLinks( m_name );
  }

  const Decision* getTerminusNode(SG::ReadHandle<DecisionContainer>& container) {
    return getTerminusNode(*container);
  }

  const Decision* getTerminusNode(const DecisionContainer& container) {
    return getNodeByName(container, summaryPassNodeName());
  }

  const Decision* getExpressTerminusNode(const DecisionContainer& container) {
    return getNodeByName(container, summaryPassExpressNodeName());
  }

  const Decision* getNodeByName(const DecisionContainer& container, const std::string& nodeName) {
    const auto it = std::find_if(container.begin(), container.end(), [&nodeName](const Decision* d){return d->name()==nodeName;});
    if (it==container.end()) {return nullptr;}
    return *it;
  }

  std::vector<const Decision*> getRejectedDecisionNodes(const asg::EventStoreType* eventStore,
    const std::string& summaryCollectionKey,
    const DecisionIDContainer& ids,
    const std::set<std::string>& keysToIgnore) {

    // The following list contains all known summary store identifiers where the graph nodes are spread out over O(100s) or O(1000s)
    // of different SG collections. This is the raw output from running the trigger online.
    // When dealing with this, we need to query eventStore->keys in every event to obtain the full set of collections to process.
    static const std::vector<std::string> knownDistributedSummaryStores{
      "HLTNav_Summary",
      "_HLTNav_Summary"
    };

    // The following list contains all known summary store identifiers where all nodes from the graph have been compactified / condensed
    // down into a single container. Here we just have to search this one container.
    static const std::vector<std::string> knownCompactSummaryStores{"HLTNav_Summary_OnlineSlimmed",
      "HLTNav_Summary_ESDSlimmed",
      "HLTNav_Summary_AODSlimmed",
      "HLTNav_Summary_DAODSlimmed",
      "HLTNav_R2ToR3Summary"
    };

    std::vector<std::string> keys; // The SG keys we will be exploring to find rejected decision nodes

    if (std::find(knownDistributedSummaryStores.cbegin(), knownDistributedSummaryStores.cend(), summaryCollectionKey) != knownDistributedSummaryStores.end() or summaryCollectionKey == "") {
      
      // If we have a distributed store then we need to query SG to find all keys.
      // This should be a rare case now that we run compactification "online" (i.e. immediately after the trigger has executed) 
#ifndef XAOD_STANDALONE
      // The list of containers we need to read can change on a file-by-file basis (it depends on the SMK)
      // Hence we query SG for all collections rather than maintain a large and ever changing ReadHandleKeyArray
      eventStore->keys(static_cast<CLID>( ClassID_traits< DecisionContainer >::ID() ), keys);
#else
      eventStore->event(); // Avoid unused warning
      throw std::runtime_error("Cannot obtain rejected HLT features in AnalysisBase when reading from uncompactified navigation containers, run trigger navigation slimming first if you really need this.");
#endif

    } else if (std::find(knownCompactSummaryStores.cbegin(), knownCompactSummaryStores.cend(), summaryCollectionKey) != knownCompactSummaryStores.end()) {

      keys.push_back(summaryCollectionKey);

    } else {

      using namespace msgRejected;
      ANA_MSG_WARNING("getRejectedDecisionNodes has not been told about final collection " << summaryCollectionKey << " please update this function. Assuming that it is already compact.");
      // Safest to assume that this is a compact summary store
      keys.push_back(summaryCollectionKey);

    }

    std::vector<const Decision*> output; // The return vector of identified nodes where one of the chains in 'ids' was rejected

    // Loop over each DecisionContainer,
    for (const std::string& key : keys) {
      // Get and check this container
      if ( ! (key.starts_with( "HLTNav_") ||  key.starts_with("_HLTNav_")) ) {
        continue; // Only concerned about the decision containers which make up the navigation, they have name prefix of HLTNav (or _HLTNav for transient-only mode)
      }
      if (keysToIgnore.count(key) == 1) {
        continue; // Have been asked to not explore this SG container
      }
      SG::ReadHandle<DecisionContainer> containerRH(key);
      if (!containerRH.isValid()) {
        throw std::runtime_error("Unable to retrieve " + key + " from event store.");
      }

      for (const Decision* d : *containerRH) {
        if ( not (d->name() == hypoAlgNodeName() or d->name() == comboHypoAlgNodeName()) ) {
          continue; // Only want Decision objects created by HypoAlgs or ComboHypoAlgs
        }
        const std::vector<ElementLink<DecisionContainer>> mySeeds = d->objectCollectionLinks<DecisionContainer>(seedString());
        if (mySeeds.size() == 0) {
          continue;
        }
        const bool allSeedsValid = std::all_of(mySeeds.begin(), mySeeds.end(), [](const ElementLink<DecisionContainer>& s) { return s.isValid(); });
        if (!allSeedsValid) {
          using namespace msgRejected;
          ANA_MSG_WARNING("A Decision object in " << key << " has invalid seeds. "
            << "The trigger navigation information is incomplete. Skipping this Decision object.");
          continue;
        }

        DecisionIDContainer activeChainsIntoThisDecision;
        decisionIDs(*(mySeeds.at(0)), activeChainsIntoThisDecision); // Get list of active chains from the first parent
        if (mySeeds.size() > 1) {
          for (size_t i = 1; i < mySeeds.size(); ++i) {
            // If there are more than one parent, we only want to keep the intersection of all of the seeds
            DecisionIDContainer moreActiveChains;
            decisionIDs(*(mySeeds.at(i)), moreActiveChains);
            DecisionIDContainer intersection;
            std::set_intersection(activeChainsIntoThisDecision.begin(), activeChainsIntoThisDecision.end(),
              moreActiveChains.begin(), moreActiveChains.end(),
              std::inserter(intersection, intersection.begin()));
            activeChainsIntoThisDecision = intersection; // Update the output to only be the intersection and continue to any other seeds
          }
        }
        // We now know what chains were active coming into this Decision (d) from ALL seeds
        // This is the logic required for each HypoTool to have activated and checked if its chain passes
        // So the size of activeChainsIntoThisDecision corresponds to the number of HypoTools which will have run
        // What do we care about? A chain, or all chains?
        DecisionIDContainer chainsToCheck;
        if (ids.size() == 0) { // We care about *all* chains
          chainsToCheck = activeChainsIntoThisDecision;
        } else { // We care about specified chains
          chainsToCheck = ids;
        }
        // We have found a rejected decision node *iff* a chainID to check is *not* present here
        // I.e. the HypoTool for the chain returned a NEGATIVE decision
        DecisionIDContainer activeChainsPassedByThisDecision;
        decisionIDs(d, activeChainsPassedByThisDecision);
        for (const DecisionID checkID : chainsToCheck) {
          if (activeChainsPassedByThisDecision.find(checkID) == activeChainsPassedByThisDecision.end() && // I was REJECTED here ...
              activeChainsIntoThisDecision.count(checkID) == 1) { // ... but PASSSED by all my inputs
            output.push_back(d);
            break;
          }
        }
      }
    }
    return output;
  }

  void recursiveGetDecisionsInternal(const Decision* node, 
    const Decision* comingFrom, 
    NavGraph& navGraph, 
    std::set<const Decision*>& fullyExploredFrom,
    const DecisionIDContainer& ids,
    const bool enforceDecisionOnNode) {

    // Does this Decision satisfy the chain requirement?
    if (enforceDecisionOnNode && ids.size() != 0 && !isAnyIDPassing(node, ids)) {
      return; // Stop propagating down this leg. It does not concern the chain with DecisionID = id
    }

    // This Decision object is part of this path through the Navigation
    navGraph.addNode(node, comingFrom);

#if TRIGCOMPUTILS_ENABLE_EARLY_EXIT == 1
    // Note we have to do this check here (after calling addNode) rather than just before calling recursiveGetDecisionsInternal
    if (fullyExploredFrom.count(node) == 1) {
      // We have fully explored this branch
      return;
    }
#endif
    
    // Continue to the path(s) by looking at this Decision object's seed(s)
    if ( hasLinkToPrevious(node) ) {
      // Do the recursion
      for ( const ElementLink<DecisionContainer>& seed : getLinkToPrevious(node)) {
        const Decision* seedDecision = *(seed); // Dereference ElementLink
        // Sending true as final parameter for enforceDecisionOnStartNode as we are recursing away from the supplied start node
        recursiveGetDecisionsInternal(seedDecision, node, navGraph, fullyExploredFrom, ids, /*enforceDecisionOnNode*/ true);
      }
    }

    // Have fully explored down from this point
    fullyExploredFrom.insert(node);

    return;
  }

  void recursiveGetDecisions(const Decision* start, 
    NavGraph& navGraph, 
    const DecisionIDContainer& ids,
    const bool enforceDecisionOnStartNode) {

    std::set<const Decision*> fullyExploredFrom;
    // Note: we do not require navGraph to be an empty graph. We can extend it.
    recursiveGetDecisionsInternal(start, /*comingFrom*/nullptr, navGraph, fullyExploredFrom, ids, enforceDecisionOnStartNode);
    
    return;
  }


  void recursiveFlagForThinning(NavGraph& graph, 
    const bool keepOnlyFinalFeatures,
    const bool removeEmptySteps,
    const std::vector<std::string>& nodesToDrop)
  {
    std::set<NavGraphNode*> fullyExploredFrom;
    for (NavGraphNode* finalNode : graph.finalNodes()) {
      recursiveFlagForThinningInternal(finalNode, /*modeKeep*/true, fullyExploredFrom, keepOnlyFinalFeatures, removeEmptySteps, nodesToDrop);
    }
  }


  void recursiveFlagForThinningInternal(NavGraphNode* node,
    bool modeKeep,
    std::set<NavGraphNode*>& fullyExploredFrom,
    const bool keepOnlyFinalFeatures,
    const bool removeEmptySteps,
    const std::vector<std::string>& nodesToDrop)
  {

    // If modeKeep == true, then by default we are KEEPING the nodes as we walk up the navigation (towards L1),
    // otherwise by default we are THINNING the nodes
    bool keep = modeKeep;

    // The calls to node->node() here are going from the transient NavGraphNode 
    // to the underlying const Decision* from the input collection. Cache these in local stack vars for better readability.

    const Decision* const me = node->node();
    const Decision* const myFirstParent = (node->seeds().size() ? node->seeds().at(0)->node() : nullptr); 
    const Decision* const myFirstChild = (node->children().size() ? node->children().at(0)->node() : nullptr); 

    // KEEP Section: The following code blocks may override the modeKeep default by setting keep=True for this node.

    if (keepOnlyFinalFeatures) {
      // Check if we have reached the first feature
      if ( modeKeep == true && (me->hasObjectLink(featureString()) || me->hasObjectLink("subfeature")) ) {
        // Just to be explicit, we keep this node
        keep = true;

        // Special BLS case: The bphysics object is attached exceptionally at the ComboHypo.
        // We want to keep going up one more level in this case to get the Feature node too (we save the 4-vec of the BPhys and both of the muons/electrons)
        const bool specialBphysCase = (me->name() == comboHypoAlgNodeName());

        // Special R2->R3 case: We can get steps with dummy features, these feature links point back to the same node.
        // First check again the feature, with an addition requirement on the feature CLID. Then check if the feature points back to the node.
        const bool specialR2toR3Case = (me->hasObjectLink(featureString(), ClassID_traits<DecisionContainer>::ID()) && me->object<Decision>(featureString()) == me);

        // We change the default behaviour to be modeKeep = false (unless we want to explore up one more level for BLS's special case
        // or to the next feature for the R2->R3 special case) such that by default we start to NOT flag all the parent nodes to be kept
        if (!specialBphysCase && !specialR2toR3Case) {
          modeKeep = false;
        }
      }
    }

    // We always by default keep the initial node from the HLTSeeding, but this may be overridden below by nodesToDrop
    if (me->name() == hltSeedingNodeName()) {
      keep = true;
    }

    // DROP Section: The following code blocks may override both the current modeKeep default and the above KEEP section by setting keep=False for this node.

    // Check also against NodesToDrop
    for (const std::string& toDrop : nodesToDrop) {
      if (me->name() == toDrop) {
        keep = false;
        break;
      }
    }

    // Check against RemoveEmptySteps
    // The signature we look for here is a InputMaker node connected directly to a ComboHypo node 
    // The key thing to identify is NO intermediate Hypo node
    // This structure is created in Steps where some chain legs are running reconstruction, but other legs are idle - either due to
    // menu alignment, or due to the legs being of different length.
    // For passed chains, there is little point keeping these CH and IM nodes on the idle legs. So we have the option of slimming them here.
    // First for the ComboHypo ...
    if (removeEmptySteps && me->name() == comboHypoAlgNodeName() && myFirstParent && myFirstParent->name() == inputMakerNodeName()) {
      keep = false;
    }
    // ... and also for the InputMaker, with flipped logic.
    if (removeEmptySteps && me->name() == inputMakerNodeName() && myFirstChild && myFirstChild->name() == comboHypoAlgNodeName()) {
      keep = false;
    }

    // Check against RemoveEmptySteps
    // Check for the R2->R3 empty step case
    if (removeEmptySteps 
      && me->name() == hypoAlgNodeName()
      && me->hasObjectLink(featureString(), ClassID_traits<DecisionContainer>::ID())
      && me->object<Decision>(featureString()) == me)
    {
      keep = false;
    }
    // ... and also for the InputMaker
    if (removeEmptySteps 
      && me->name() == inputMakerNodeName()
      && myFirstChild
      && myFirstChild->name() == hypoAlgNodeName()
      && myFirstChild->hasObjectLink(featureString(), ClassID_traits<DecisionContainer>::ID())
      && myFirstChild->object<Decision>(featureString()) == myFirstChild)
    {
      keep = false;
    }

    // APPLICATION Section:

    if (keep) {
      node->keep(); // Inform the node that it should NOT be thinned away.
    }

    for (NavGraphNode* seed : node->seeds()) {
#if TRIGCOMPUTILS_ENABLE_EARLY_EXIT == 1
      // NOTE: This code block cuts short the recursive graph exploration for any node which has already been fully-explored-from.
      // 
      // If we are keeping final features, then processing each node exactly once is actually insufficient.
      // This is because we may reach a node, X, with a feature which is penultimate/generally-non-final for some chain, A,
      // but then later on we may follow Terminus->SummaryFilter->Hypothesis(X) for some chain, B, where this same
      // feature from X _is_ the final-feature for chain B. Here we would not process node X again as we have already dealt with it.
      // But we need to process it again as for chain B, modeKeep==true still and chain B wants to flag the node as keep().
      //
      // We only however need to force the exploration while we are in the mode where we are nominally keeping nodes (modeKeep == true).
      // Once we have switched to dropping nodes by default, we should be OK to once again skip over nodes which we already processed.
      // The only thing we should be keeping when modeKeep == false is the L1 node, any this is not dependent on path.
      //
      // See: ATR-28061
      bool allowEarlyExit = true;
      if (keepOnlyFinalFeatures) {
        allowEarlyExit = (modeKeep == false);
      }
      if (allowEarlyExit && fullyExploredFrom.count(seed) == 1) {
        // We have fully explored this branch
        continue;
      }
#endif
      // Recursively call all the way up the graph to the initial nodes from the HLTSeeding
      recursiveFlagForThinningInternal(seed, modeKeep, fullyExploredFrom, keepOnlyFinalFeatures, removeEmptySteps, nodesToDrop);
    }

    // Have fully explored down from this point
    fullyExploredFrom.insert(node);
  }

  // Note: This version of the function recurses through a full navigation graph (initial input: Decision Object)
  bool typelessFindLinks(const Decision* start, 
    const std::string& linkName,
    std::vector<sgkey_t>& keyVec, 
    std::vector<uint32_t>& clidVec, 
    std::vector<Decision::index_type>& indexVec, 
    std::vector<const Decision*>& sourceVec,
    const unsigned int behaviour, 
    std::set<const Decision*>* fullyExploredFrom)
  {
    using namespace msgFindLink;

    // As the append vectors are user-supplied, perform some input validation. 
    if (keyVec.size() != clidVec.size() or clidVec.size() != indexVec.size()) {
      ANA_MSG_WARNING("In typelessFindLinks, keyVec, clidVec, indexVec must all be the same size. Instead have:"
        << keyVec.size() << ", " << clidVec.size()  << ", " << indexVec.size());
      return false;
    }

    // Locate named links. Both collections of links and individual links are supported.
    bool found = typelessFindLinksCommonLinkCollection(start, linkName, keyVec, clidVec, indexVec, sourceVec);
   
    // Early exit
    if (found && behaviour == TrigDefs::lastFeatureOfType) {
      return true;
    }
    // If not Early Exit, then recurse
    for (const ElementLink<DecisionContainer>& seed : getLinkToPrevious(start)) {
#if TRIGCOMPUTILS_ENABLE_EARLY_EXIT == 1
      if (fullyExploredFrom != nullptr) {
        // We only need to recursively explore back from each node in the graph once.
        // We can keep a record of nodes which we have already explored, these we can safely skip over.
        if (fullyExploredFrom->count(*seed) == 1) {
          continue; 
        }
      }
#endif
      found |= typelessFindLinks(*seed, linkName, keyVec, clidVec, indexVec, sourceVec, behaviour, fullyExploredFrom);
    }
    // Fully explored this node
    if (fullyExploredFrom != nullptr) {
      fullyExploredFrom->insert(start);
    }
    return found;
  }

  // Note: This version of the function recurses through a sub-graph of the full navigation graph (initial input: NavGraphNode)
  bool typelessFindLinks(const NavGraphNode* start, 
    const std::string& linkName,
    std::vector<sgkey_t>& keyVec, 
    std::vector<uint32_t>& clidVec, 
    std::vector<Decision::index_type>& indexVec, 
    std::vector<const Decision*>& sourceVec,
    const unsigned int behaviour, 
    std::set<const Decision*>* fullyExploredFrom)
  {
    using namespace msgFindLink;

    // As the append vectors are user-supplied, perform some input validation. 
    if (keyVec.size() != clidVec.size() or clidVec.size() != indexVec.size()) {
      ANA_MSG_WARNING("In typelessFindLinks, keyVec, clidVec, indexVec must all be the same size. Instead have:"
        << keyVec.size() << ", " << clidVec.size()  << ", " << indexVec.size());
      return false;
    }

    const Decision* start_decisionObject = start->node(); 
    // Locate named links. Both collections of links and individual links are supported.
    bool found = typelessFindLinksCommonLinkCollection(start_decisionObject, linkName, keyVec, clidVec, indexVec, sourceVec);

    // Early exit
    if (found && behaviour == TrigDefs::lastFeatureOfType) {
      return true;
    }
    // If not Early Exit, then recurse
    for (const NavGraphNode* seed : start->seeds()) {
#if TRIGCOMPUTILS_ENABLE_EARLY_EXIT == 1
      if (fullyExploredFrom != nullptr) {
        // We only need to recursively explore back from each node in the graph once.
        // We can keep a record of nodes which we have already explored, these we can safely skip over.
        const Decision* seed_decisionObject = seed->node(); 
        if (fullyExploredFrom->count(seed_decisionObject) == 1) {
          continue; 
        }
      }
#endif
      found |= typelessFindLinks(seed, linkName, keyVec, clidVec, indexVec, sourceVec, behaviour, fullyExploredFrom);
    }
    // Fully explored this node
    if (fullyExploredFrom != nullptr) {
      fullyExploredFrom->insert(start_decisionObject);
    }
    return found;
  }


  bool typelessFindLinksCommonLinkCollection(const Decision* start,
    const std::string& linkName,
    std::vector<sgkey_t>& keyVec, 
    std::vector<uint32_t>& clidVec,
    std::vector<Decision::index_type>& indexVec, 
    std::vector<const Decision*>& sourceVec) 
  {
    bool found = false;
    std::vector<sgkey_t> tmpKeyVec;
    std::vector<uint32_t> tmpClidVec;
    std::vector<Decision::index_type> tmpIndexVec;
    if (start->hasObjectCollectionLinks(linkName)) {
      found = start->typelessGetObjectCollectionLinks(linkName, tmpKeyVec, tmpClidVec, tmpIndexVec);
    }
    if (start->hasObjectLink(linkName)) {
      sgkey_t tmpKey{0};
      uint32_t tmpClid{0};
      Decision::index_type tmpIndex{0};
      found |= start->typelessGetObjectLink(linkName, tmpKey, tmpClid, tmpIndex);
      tmpKeyVec.push_back(tmpKey);
      tmpClidVec.push_back(tmpClid);
      tmpIndexVec.push_back(tmpIndex);
    }
    // De-duplicate
    for (size_t tmpi = 0; tmpi < tmpKeyVec.size(); ++tmpi) {
      bool alreadyAdded = false;
      const uint32_t tmpKey = tmpKeyVec.at(tmpi);
      const uint32_t tmpClid = tmpClidVec.at(tmpi);
      const Decision::index_type tmpIndex = tmpIndexVec.at(tmpi);
      for (size_t veci = 0; veci < keyVec.size(); ++veci) {
        if (SG::sgkeyEqual (keyVec.at(veci), tmpKey)
          and clidVec.at(veci) == tmpClid
          and indexVec.at(veci) == tmpIndex)
        {
          alreadyAdded = true;
          break;
        }
      }
      if (!alreadyAdded) {
        keyVec.push_back( tmpKey );
        clidVec.push_back( tmpClid );
        indexVec.push_back( tmpIndex );
        sourceVec.push_back( start );
      }
    }
    return found;
  }



  bool typelessFindLink(const Decision* start,
    const std::string& linkName, 
    sgkey_t& key, 
    uint32_t& clid, 
    Decision::index_type& index,
    const Decision*& source,
    const bool suppressMultipleLinksWarning)
  {
    using namespace msgFindLink;
    // We use findLink in cases where there is only one link to be found, or if there are multiple then we 
    // only want the most recent.
    // Hence we can supply TrigDefs::lastFeatureOfType.                                                         /--> parent3(link)
    // We can still have more then one link found if there is a branch in the navigation. E.g. start --> parent1 --> parent2(link)
    // If both parent2 and parent3 possessed an admissible ElementLink, then the warning below will trigger, and only one of the
    // links will be returned (whichever of parent2 or parent3 happened to be the first seed of parent1).
    std::vector<sgkey_t> keyVec;
    std::vector<uint32_t> clidVec;
    std::vector<Decision::index_type> indexVec;
    std::vector<const Decision*> sourceVec;
    std::set<const xAOD::TrigComposite*> fullyExploredFrom;

    const bool result = typelessFindLinks(start, linkName, keyVec, clidVec, indexVec, sourceVec, TrigDefs::lastFeatureOfType, &fullyExploredFrom);
    if (!result) {
      return false; // Nothing found
    }

    if (keyVec.size() > 1 && !suppressMultipleLinksWarning) {
      ANA_MSG_WARNING (keyVec.size() << " typeless links found for " << linkName
                       << " returning the first link, consider using findLinks.");
    }
    key = keyVec.at(0);
    clid = clidVec.at(0);
    index = indexVec.at(0);
    source = sourceVec.at(0);
    return true; 
  }


  bool typelessFindLink(const NavGraph& subGraph,
    const std::string& linkName, 
    sgkey_t& key,
    uint32_t& clid,
    Decision::index_type& index,
    const Decision*& source,
    const bool suppressMultipleLinksWarning)
  {
    using namespace msgFindLink;
    // Note: This function should be the same as its predecessor, just using a NavGraph to start rather than a Decision*
    // As a result, it can search from more than one Decision* (the NavGraph may have more than one final node)
    // but it will still warn if this results in more than one link being located.
    std::vector<sgkey_t> keyVec;
    std::vector<uint32_t> clidVec;
    std::vector<Decision::index_type> indexVec;
    std::vector<const Decision*> sourceVec;
    std::set<const Decision*> fullyExploredFrom;

    bool result = false;
    for (const NavGraphNode* finalNode : subGraph.finalNodes()) {
      result |= typelessFindLinks(finalNode, linkName, keyVec, clidVec, indexVec, sourceVec, TrigDefs::lastFeatureOfType, &fullyExploredFrom);
    }

    if (!result) {
      return false; // Nothing found
    }

    if (keyVec.size() > 1 && !suppressMultipleLinksWarning) {
      ANA_MSG_WARNING (keyVec.size() << " typeless links found for " << linkName
                       << " returning the first link, consider using findLinks.");
    }
    key = keyVec.at(0);
    clid = clidVec.at(0);
    index = indexVec.at(0);
    source = sourceVec.at(0);
    return true; 
  }


  Combinations buildCombinations(
    const std::string& chainName,
    const std::vector<LinkInfo<xAOD::IParticleContainer>>& features,
    const std::vector<std::size_t>& legMultiplicities,
    const std::function<bool(const std::vector<LinkInfo<xAOD::IParticleContainer>>&)>& filter)
  {
    Combinations combinations(filter);
    combinations.reserve(legMultiplicities.size());
    if (legMultiplicities.size() == 1)
      combinations.addLeg(legMultiplicities.at(0), features);
    else
      for (std::size_t legIdx = 0; legIdx < legMultiplicities.size(); ++legIdx)
      {
        // Skip any that will not provide IParticle features
        if (legMultiplicities[legIdx] == 0)
          continue;
        HLT::Identifier legID = createLegName(chainName, legIdx);
        std::vector<LinkInfo<xAOD::IParticleContainer>> legFeatures;
        for (const LinkInfo<xAOD::IParticleContainer>& info : features)
          if (passed(legID.numeric(), info.decisions))
            legFeatures.push_back(info);
      combinations.addLeg(legMultiplicities.at(legIdx), std::move(legFeatures));
      }
    return combinations;
  }


  Combinations buildCombinations(
    const std::string& chainName,
    const std::vector<LinkInfo<xAOD::IParticleContainer>>& features,
    const std::vector<std::size_t>& legMultiplicities,
    FilterType filter)
  {
    return buildCombinations(chainName, features, legMultiplicities, getFilter(filter));
  }

  Combinations buildCombinations(
    const std::string& chainName,
    const std::vector<LinkInfo<xAOD::IParticleContainer>>& features,
    const TrigConf::HLTChain *chainInfo,
    const std::function<bool(const std::vector<LinkInfo<xAOD::IParticleContainer>>&)>& filter)
  {
    return buildCombinations(chainName, features, chainInfo->leg_multiplicities(), filter);
  }

  Combinations buildCombinations(
    const std::string& chainName,
    const std::vector<LinkInfo<xAOD::IParticleContainer>>& features,
    const TrigConf::HLTChain *chainInfo,
    FilterType filter)
  {
    return buildCombinations(chainName, features, chainInfo, getFilter(filter));
  }


  std::string dump( const Decision* tc, const std::function< std::string( const Decision* )>& printerFnc ) {
    std::string ret; 
    ret += printerFnc( tc );
    if ( hasLinkToPrevious(tc) ) {
      const std::vector<ElementLink<DecisionContainer>> seeds = getLinkToPrevious(tc);
      for (const ElementLink<DecisionContainer>& seedEL : seeds) {
        ret += " -> " + dump( *seedEL, printerFnc );
      }
    }
    return ret;
  }

  
  const std::string& initialRoIString() {
    return Decision::s_initialRoIString;
  }

  const std::string& initialRecRoIString() {
    return Decision::s_initialRecRoIString;
  }

  const std::string& roiString() {
    return Decision::s_roiString;
  }

  const std::string& viewString() {
    return Decision::s_viewString;
  }

  const std::string& featureString() {
    return Decision::s_featureString;
  }

  const std::string& seedString() {
    return Decision::s_seedString;
  }
  
  const std::string& hltSeedingNodeName(){
    return Decision::s_hltSeedingNodeNameString;
  }

  const std::string& filterNodeName(){
    return Decision::s_filterNodeNameString;
  }

  const std::string& inputMakerNodeName(){
    return Decision::s_inputMakerNodeNameString;
  }

  const std::string& hypoAlgNodeName(){
    return Decision::s_hypoAlgNodeNameString;
  }

  const std::string& comboHypoAlgNodeName(){
    return Decision::s_comboHypoAlgNodeNameString;
  }

  const std::string& summaryFilterNodeName(){
    return Decision::s_summaryFilterNodeNameString;
  }

  const std::string& summaryPassNodeName(){
    return Decision::s_summaryPassNodeNameString;
  }

  const std::string& summaryPassExpressNodeName(){
    return Decision::s_summaryPassExpressNodeNameString;
  }

  const std::string& summaryPrescaledNodeName(){
    return Decision::s_summaryPrescaledNodeNameString;
  }

}

