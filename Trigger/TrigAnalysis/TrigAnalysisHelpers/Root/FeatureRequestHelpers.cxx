/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include <vector>

#include "TrigAnalysisHelpers/FeatureRequestHelpers.h"
#include "TrigCompositeUtils/ChainNameParser.h"

namespace FeatureRequestHelpers {

  std::vector<TrigCompositeUtils::TypelessLinkInfo> typelessFeaturesImplimentation(
    const Trig::FeatureRequestDescriptor& frd,
    const CLID clid,
    const SG::ReadHandleKey<TrigCompositeUtils::DecisionContainer>& navRHKey,
    MsgStream& msg,
    const EventContext& ctx,
    const asg::EventStoreType* eventStore,
    const bool printWarningMessages)
  {
    HLT::Identifier chainID(frd.chainGroup());

    bool errState = false;
    if ( frd.condition() != TrigDefs::Physics && frd.condition() != TrigDefs::includeFailedDecisions ) {
      msg << MSG::ERROR << "features may only be called with: "
        "TrigDefs::Physics - features from the legs of the chain(s) which passed the trigger. "
        "TrigDefs::includeFailedDecisions - all features from the chain(s) irrespective of pass/fail of each Step." << endmsg;
      errState = true;
    }

    if ( frd.featureCollectionMode() != TrigDefs::lastFeatureOfType && frd.featureCollectionMode() != TrigDefs::allFeaturesOfType ) {
      msg << MSG::ERROR << "featureCollectionMode may only be called with: "
        "TrigDefs::lastFeatureOfType - stop exploring each route through the navigation once a feature matching all requirements is found. "
        "TrigDefs::allFeaturesOfType - always fully explore each route through the navigation graph and collect all matching features." << endmsg;
      errState = true; 
    }

    SG::ReadHandle<TrigCompositeUtils::DecisionContainer> navigationRH = SG::ReadHandle(navRHKey, ctx);
    if (!navigationRH.isValid()) {
      msg << MSG::ERROR << "Unable to read trigger navigation from " << navRHKey.key() << ". Cannot retrieve features." << endmsg;
      errState = true;
    }

    // We always want to search from the passed raw terminus node to find features for passed chains.
    const TrigCompositeUtils::Decision* terminusNode = nullptr;
    if (!errState) {
      terminusNode = TrigCompositeUtils::getTerminusNode(navigationRH);
      if (terminusNode == nullptr) {
        msg << MSG::ERROR << "Unable to locate HLTPassRaw element of " << navRHKey.key() 
          << ", collection contains " << navigationRH->size() << " nodes."  << endmsg;
        errState = true;
      }
    }

    if (errState) {
      msg << MSG::ERROR << "Encountered one or more errors in FeatureRequestHelpers::typelessFeaturesImplimentation" << endmsg;
      throw std::runtime_error("Encountered one or more errors in FeatureRequestHelpers::typelessFeaturesImplimentation");
    }

    // The sub-graph from which we will extract features
    TrigCompositeUtils::NavGraph navGraph; 

    // Collect the set of chain legs for the chain we are fetching
    TrigCompositeUtils::DecisionIDContainer chainIDs;
    chainIDs.insert( chainID.numeric() );

    const std::vector<int> legMultiplicites = ChainNameParser::multiplicities(chainID.name());
    if (legMultiplicites.size() == 0) {
      msg << MSG::ERROR << "chain " << chainID << " has invalid configuration, no parsed multiplicity data." << endmsg;
      throw std::runtime_error("Encountered chain which could not be parsed in FeatureRequestHelpers::typelessFeaturesImplimentation");
    } else if (legMultiplicites.size() > 1) {
      if (frd.restrictRequestToLeg() >= static_cast<int>(legMultiplicites.size()) && printWarningMessages) {
        msg << MSG::WARNING << "Requested features from leg index " << frd.restrictRequestToLeg() << " for chain " << chainID <<
          " but this chain only has " << legMultiplicites.size() << " legs" << endmsg;
      }
      // For multi-leg chains, the DecisionIDs are handled per leg.
      // We don't care here exactly how many objects are required per leg, just that there are two-or-more legs
      for (size_t legNumeral = 0; legNumeral < legMultiplicites.size(); ++legNumeral) {
        // If frd.restrictRequestToLeg() is -1 then we are NOT filtering on legs, we return features over all legs.
        if (frd.restrictRequestToLeg() != -1 and frd.restrictRequestToLeg() != static_cast<int>(legNumeral)) { 
          continue;
        }
        HLT::Identifier legID = TrigCompositeUtils::createLegName(chainID, legNumeral);
        chainIDs.insert( legID.numeric() );
      }
    }
    if (msg.level() <= MSG::DEBUG) msg << MSG::DEBUG << "Fetching navigation data for chain " << chainID << " with " << legMultiplicites.size() << " leg(s)." << endmsg;
    if (msg.level() <= MSG::VERBOSE) {
      for (const TrigCompositeUtils::DecisionID printID : chainIDs) {
        msg << MSG::VERBOSE << " -- Collecting for chain or chain-leg: " << HLT::Identifier(printID) << endmsg;
      }
    }

    // Obtain navigation routes for objects which pass
    // Final parameter TRUE as the chain passed (has its ID in terminusNode)
    TrigCompositeUtils::recursiveGetDecisions(terminusNode, navGraph, chainIDs, true);

    if (msg.level() <= MSG::DEBUG) {
      msg << "Added all passed navigation data for chain " << chainID
        << ", total nodes:" << navGraph.nodes() << " total edges:" << navGraph.edges() << " final nodes:" << navGraph.finalNodes().size() << endmsg;
    }

    // Obtain navigation routes for objects which fail.
    // Supplying nullptr as the first argument instead of a pointer to the event store means that we can
    // only seach in compact navigation collections (everything in a single container).
    // This is always the case offline.
    if (frd.condition() == TrigDefs::includeFailedDecisions) {
      std::vector<const TrigCompositeUtils::Decision*> rejectedDecisionNodes = 
        TrigCompositeUtils::getRejectedDecisionNodes(nullptr, ctx, navRHKey.key(), chainIDs);

      if (msg.level() <= MSG::DEBUG) {
        msg << MSG::DEBUG << "Chain " << chainID << " has " << rejectedDecisionNodes.size() 
          << " dangling nodes in the graph from objects which were rejected." << endmsg;
      }

      for (const TrigCompositeUtils::Decision* rejectedNode : rejectedDecisionNodes) {
        // Final parameter FALSE as the chain failed here (its ID was removed from rejectedNode)
        TrigCompositeUtils::recursiveGetDecisions(rejectedNode, navGraph, chainIDs, false);
      }

      if (msg.level() <= MSG::DEBUG) {
        msg << MSG::DEBUG << "Added all failed navigation data for chain " << chainID
         << ", total nodes:" << navGraph.nodes() << " total edges:" << navGraph.edges() << " final nodes:" << navGraph.finalNodes().size() << endmsg;
      }
    }

    if (msg.level() <= MSG::DEBUG) {
      msg << MSG::DEBUG << "Finished adding nodes to sub-graph. "
        << "Total nodes:" << navGraph.nodes() << " total edges:" << navGraph.edges() << " final nodes:" << navGraph.finalNodes().size() << endmsg;
    }
    if (msg.level() <= MSG::VERBOSE && navGraph.finalNodes().size()) {
      for (const TrigCompositeUtils::NavGraphNode* n : navGraph.finalNodes()) {
        msg << MSG::VERBOSE << "  Final node:" << TrigCompositeUtils::decisionToElementLink(n->node(), ctx).dataID() << " #" << n->node()->index() << endmsg;
      }
      navGraph.printAllPaths(msg, MSG::VERBOSE);
    }

    return typelessGetFeatures(
      navGraph, 
      frd,
      clid,
      chainIDs,
      ctx,
      eventStore);
  }


  const std::vector<TrigCompositeUtils::TypelessLinkInfo> typelessGetFeatures(
    const TrigCompositeUtils::NavGraph& navGraph,
    const Trig::FeatureRequestDescriptor& frd,
    const CLID clid,  
    const TrigCompositeUtils::DecisionIDContainer chainIDs, 
    const EventContext& ctx,
    const asg::EventStoreType* eventStore)
  {
    std::vector<TrigCompositeUtils::TypelessLinkInfo> features; // The return vector
    std::set<const TrigCompositeUtils::NavGraphNode*> fullyExploredFrom; // Lets us navigate more efficiently

    // For each starting point through the navigation for a given chain-group
    for (const TrigCompositeUtils::NavGraphNode* finalNode : navGraph.finalNodes()) {
      typelessGetFeaturesInternal(
        features,
        fullyExploredFrom,
        finalNode,
        //
        frd,
        clid,
        chainIDs,
        ctx,
        eventStore);
    }

    return features;
  }


  void typelessGetFeaturesInternal(
    std::vector<TrigCompositeUtils::TypelessLinkInfo>& features, 
    std::set<const TrigCompositeUtils::NavGraphNode*>& fullyExploredFrom,
    const TrigCompositeUtils::NavGraphNode* navGraphNode, 
    // The following are passed down from typelessGetFeatures
    const Trig::FeatureRequestDescriptor& frd,
    const CLID clid,  
    const TrigCompositeUtils::DecisionIDContainer chainIDs, 
    const EventContext& ctx,
    const asg::EventStoreType* eventStore)
  {

    const TrigCompositeUtils::Decision* decisionObj = navGraphNode->node();
    const TrigCompositeUtils::Decision* decisionObjChild = (navGraphNode->children().size() == 1 ? navGraphNode->children()[0]->node() : nullptr);
    const std::vector<TrigCompositeUtils::DecisionID> &ids = TrigCompositeUtils::decisionIDs(decisionObj);
    std::vector<sgkey_t> featureKeys;
    std::vector<CLID> featureClids;
    std::vector<TrigCompositeUtils::Decision::index_type> featureIndices;    

    // Look up what named links are available in the Decision Object
    std::vector<std::string> availableLinkNames;
    if (frd.linkName().empty()) { // If no link name is requested, then use all available names (look these up)
      const std::vector<std::string> getSingleLinkNames = decisionObj->getObjectNames(clid);
      const std::vector<std::string> getCollectionLinkNames = decisionObj->getObjectCollectionNames(clid);
      std::copy(getSingleLinkNames.begin(), getSingleLinkNames.end(), std::back_inserter(availableLinkNames));
      std::copy(getCollectionLinkNames.begin(), getCollectionLinkNames.end(), std::back_inserter(availableLinkNames));
    } else { // Just looking for an explicitly named feature
      availableLinkNames.push_back( frd.linkName() );
    }

    // Fetch the named links that we're interested in
    for (const std::string& featureNameToGet : availableLinkNames) {
      // This try block protects against ExcCLIDMismatch throws from
      // features which do not derive from IParticle, when an IParticle interface is requested.
#ifndef XAOD_STANDALONE
      try {
#endif
        // Slices may have added link collections. These links may have been to objects in different containers.
        if (decisionObj->hasObjectCollectionLinks(featureNameToGet, clid)) {
          std::vector<sgkey_t> keyVec;
          std::vector<CLID> clidVec;
          std::vector<TrigCompositeUtils::Decision::index_type> indexVec;
          decisionObj->typelessGetObjectCollectionLinks(featureNameToGet, keyVec, clidVec, indexVec);
          filterLinkVectorByContainerKey(frd.SGKeyExpression(), keyVec, clidVec, indexVec, ctx, eventStore);
          std::copy(keyVec.begin(), keyVec.end(), std::back_inserter(featureKeys));
          std::copy(clidVec.begin(), clidVec.end(), std::back_inserter(featureClids));
          std::copy(indexVec.begin(), indexVec.end(), std::back_inserter(featureIndices));
        }
#ifndef XAOD_STANDALONE
      } catch (SG::ExcCLIDMismatch&) {
        // This is in place to catch the exception caused by non-IParticle features when an IParticle interface is requested.
        // We're fine to catch this silently and carry on looking at the next Decision object in the graph
      }
      try {
#endif
        // Slices may have added single links. Note: the framework-specified "feature" link is always a single link.
        if (decisionObj->hasObjectLink(featureNameToGet, clid)) {
          sgkey_t fKey;
          CLID fClid;
          TrigCompositeUtils::Decision::index_type fIndex;
          decisionObj->typelessGetObjectLink(featureNameToGet, fKey, fClid, fIndex);
          // Filtering function operates on a vector
          std::vector<sgkey_t> keyVec = {fKey};
          std::vector<CLID> clidVec = {fClid};
          std::vector<TrigCompositeUtils::Decision::index_type> indexVec = {fIndex};
          filterLinkVectorByContainerKey(frd.SGKeyExpression(), keyVec, clidVec, indexVec, ctx, eventStore);
          std::copy(keyVec.begin(), keyVec.end(), std::back_inserter(featureKeys));
          std::copy(clidVec.begin(), clidVec.end(), std::back_inserter(featureClids));
          std::copy(indexVec.begin(), indexVec.end(), std::back_inserter(featureIndices));
        }
#ifndef XAOD_STANDALONE
      } catch (SG::ExcCLIDMismatch&) {
        // Silently. As above.
      }
#endif
    }

    // Check if the Decision object is active for a specific set of Chains-of-interest (as supplied by the TDT)
    TrigCompositeUtils::ActiveState state = TrigCompositeUtils::ActiveState::UNSET;
    if (chainIDs.size() > 0) {
      // If we were given a list of chains to consider then we start assuming none passed this decisionObj
      state = TrigCompositeUtils::ActiveState::INACTIVE;
      for (TrigCompositeUtils::DecisionID id : chainIDs) {
        if (std::count(decisionObj->decisions().begin(), decisionObj->decisions().end(), id) == 1) {
          state = TrigCompositeUtils::ActiveState::ACTIVE;
          break;
        }
      }
      // But consider also the ComboHypo, this will have run immediately after the Hypo and could have failed this chain due to combinatorics.
      // This statement will only activate for the case that we are looking for "feature" edges (which is the default).
      if (state == TrigCompositeUtils::ActiveState::ACTIVE && decisionObjChild && decisionObjChild->name() == TrigCompositeUtils::comboHypoAlgNodeName()) {
        state = TrigCompositeUtils::ActiveState::INACTIVE;
        for (TrigCompositeUtils::DecisionID id : chainIDs) {
          if (std::count(decisionObjChild->decisions().begin(), decisionObjChild->decisions().end(), id) == 1) {
            state = TrigCompositeUtils::ActiveState::ACTIVE;
            break;
          }
        }
      } 
    }
    
    for (size_t i = 0; i < featureKeys.size(); ++i) {
      typename std::vector<TrigCompositeUtils::TypelessLinkInfo>::iterator vecIt = 
        std::find_if(features.begin(), features.end(), [&](const auto& li) { return li.key == featureKeys[i] and li.clid == featureClids[i] and li.index == featureIndices[i]; } );
      if (vecIt == features.end()) {
        // Link did not already exist - add it to the output
        features.emplace_back( decisionObj, featureKeys[i], featureClids[i], featureIndices[i], state ); // Note: This populates the set of passed IDs from decisionObj
      } else {
        // Link already existed - if the link's state in the return vector is INACTIVE but is ACTIVE here,
        // then we need to change it to ACTIVE as this denotes one-or-more of the requested chains were active for the object.
        if (vecIt->state == TrigCompositeUtils::ActiveState::INACTIVE and state == TrigCompositeUtils::ActiveState::ACTIVE) {
          vecIt->state = TrigCompositeUtils::ActiveState::ACTIVE;
        }
        // Always update the set of passed IDs for feature retrieval. The std::optional must have_value due to the choice of constructor above
        vecIt->decisions->insert(ids.begin(), ids.end());
      }
    }

    // Stop processing this path through the navigation if the lastFeatureOfType flag is set
    if (featureKeys.size() && (frd.featureCollectionMode() & TrigDefs::lastFeatureOfType)) {
      return;
    }

    // Recurse to decisionObj's seeds
    for (const TrigCompositeUtils::NavGraphNode* seedNavNode : navGraphNode->seeds()) {
      if (fullyExploredFrom.count(seedNavNode) == 1) {
        continue; // Already explored down from here
      }
      typelessGetFeaturesInternal(
        features,
        fullyExploredFrom,
        seedNavNode,
        // Following are passed down from typelessGetFeatures
        frd,
        clid,
        chainIDs,
        ctx,
        eventStore);
    }

    // If we encounter this node again in the future, we don't need to explore it again as it's now fully explored.

    fullyExploredFrom.insert( navGraphNode );
  }

  void filterLinkVectorByContainerKey(
    const std::regex& expression,
    std::vector<sgkey_t>& keyVec,
    std::vector<CLID>& clidVec,
    std::vector<TrigCompositeUtils::Decision::index_type>& indexVec,
    [[maybe_unused]] const EventContext& ctx,
    [[maybe_unused]] const asg::EventStoreType* eventStore)
  {
    if (std::regex_match("", expression)) { // No filtering if empty string supplied as pattern
      return;
    }
    std::vector<size_t> indicesToRemove;
    for (size_t i = 0; i < keyVec.size(); ++i) {
#ifdef XAOD_STANDALONE
      const std::string keyStr = eventStore->event()->getName(keyVec[i]);
      const std::string* keyStrPtr = (keyStr.empty() ? nullptr : &keyStr);
#else
      const std::string* keyStrPtr = Atlas::getExtendedEventContext(ctx).proxy()->keyToString(keyVec[i], clidVec[i]);

#endif
      // We will not be able to resolve the key for any collection which isn't in the POOL file.
      // We reject the link in this case
      if (not keyStrPtr or not std::regex_match(*keyStrPtr, expression)) {
        indicesToRemove.push_back(i);
      }
    }
    // Do the erasing. NOTE, must back-iterate to keep the indices matching
    for (auto it = indicesToRemove.rbegin(); it != indicesToRemove.rend(); ++it) {
      keyVec.erase(keyVec.begin() + *it);
      clidVec.erase(clidVec.begin() + *it);
      indexVec.erase(indexVec.begin() + *it);
    }
  }

} // namespace FeatureRequestHelpers