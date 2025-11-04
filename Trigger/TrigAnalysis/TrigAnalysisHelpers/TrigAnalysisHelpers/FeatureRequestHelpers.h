/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TrigAnalysisHelpers_FeatureRequestHelper_h
#define TrigAnalysisHelpers_FeatureRequestHelper_h

#include "AsgDataHandles/ReadHandleKey.h"

#include "xAODTrigger/TrigCompositeContainer.h"
#include "xAODTrigger/TrigCompositeAuxContainer.h"

#include "FeatureRequestDescriptor.h"

#include "TrigCompositeUtils/NavGraph.h"
#include "TrigCompositeUtils/LinkInfo.h"
#include "TrigCompositeUtils/TypelessLinkInfo.h"

#include "CxxUtils/sgkey_t.h"

#include <vector>

namespace FeatureRequestHelpers {
  using sgkey_t = SG::sgkey_t;

  /**
   * @brief Wrapper function to convert between vector<TypelessLinkInfo> and vector<LinkInfo<T>>.
   * @param[in] typelessLinkInfos Vector of type erased LinkInfo objects of which we will apply type wrapping.
   * @param[in] ctx Event context for Element Link construction.
   * @param[in] eventStore Pointer to event store, only used in AnalysisBase for Element Link construction.
   * @return Vector of typed LinkInfo objects each containing an ElementLink to a feature and a pointer to the feature's Decision object in the navigation.
   **/
  template<class CONTAINER>
  std::vector<TrigCompositeUtils::LinkInfo<CONTAINER>> typedFeaturesWrapper(
    const std::vector<TrigCompositeUtils::TypelessLinkInfo>& typelessLinkInfos,
    const EventContext& ctx,
    const asg::EventStoreType* eventStore = nullptr);

  /**
   * @brief Standalone implementation of feature retrieval, common between TrigDecisionTool and TrigDecisionToolLite
   * This is a type erased implementation which accepts a CLID. Users are expected to use typed interfaces in the respective tools.
   * @param[in] frd Feature request descriptor containing details of the feature request.
   * @param[in] clid The Class ID of the requested feature's container. Originates from typed caller template parameter. May be CLID of xAOD::IParticleContainer for generic four vector access.
   * @param[in] navRH Read handle key for the primary navigation container (containing the terminus node)
   * @param[in] msg Reference to message stream for printing
   * @param[in] ctx Event context for Read Handle construction and SG key hash resolution
   * @param[in] eventStore Pointer to event store, only used in AnalysisBase for SG key hash resolution
   * @param[in] printWarningMessages Flag if warnings should be printed regarding leg multiplicity checks
   * @return Typed erased vector of TypelessLinkInfo. Each LinkInfo wraps the raw data needed to construct an ElementLink to a feature and a pointer to the feature's Decision object in the navigation.
   **/
  std::vector<TrigCompositeUtils::TypelessLinkInfo> typelessFeaturesImplimentation(
    const Trig::FeatureRequestDescriptor& frd,
    const CLID clid, 
    const SG::ReadHandleKey<TrigCompositeUtils::DecisionContainer>& navRH,
    MsgStream& msg,
    const EventContext& ctx,
    const asg::EventStoreType* eventStore = nullptr,
    const bool printWarningMessages = true);

  /**
   * @brief Extract features from the supplied navGraph (obtained through typelessGetFeaturesInternal).
   * This is a lower level function call than typelessFeaturesImplimentation, and its use implies that the requisite sub-graph to be explored 
   * and set of chain IDs of interest have already been obtained. The feature request descriptor is still passed down as some aspects of the request
   * are only handled at the lower levels (such as store gate key filtering, and link name matching)
   * @param[in] frd Feature request descriptor containing details of the feature request.
   * @param[in] clid The Class Identifier to use instead of the CONTAINER template type in this type erased function
   * @param[in] navGraph Sub-graph of the trigger navigation which is to be considered.
   * @param[in] chainIDs Set of Chain IDs which features are being requested for. Used to set the ActiveState of returned LinkInfo objects. Empty set denotes no chain filtering.
   * @param[in] ctx Event context.
   * @param[in] eventStore Optional pointer to event store, only needed in AnalysisBase.
   * @return Typed erased vector of TypelessLinkInfo. Each LinkInfo wraps the raw data needed to construct an ElementLink to a feature and a pointer to the feature's Decision object in the navigation.
   **/
  const std::vector<TrigCompositeUtils::TypelessLinkInfo> typelessGetFeatures(
    const TrigCompositeUtils::NavGraph& navGraph,
    const Trig::FeatureRequestDescriptor& frd,
    const CLID clid,  
    const TrigCompositeUtils::DecisionIDContainer chainIDs, 
    const EventContext& ctx,
    const asg::EventStoreType* eventStore = nullptr);

  /**
   * @see typelessGetFeatures
   * @brief Internal implementation called by typelessGetFeatures, and by itself
   * @param[in,out] features The ultimate return vector. New links are to be appended.
   * @param[in,out] fullyExploredFrom Cache of graph nodes which have been fully explored, and hence don't need exploring again should they show up.
   * @param[in] navGraphNode The current node in the navGraph which is being explored.
   **/
  void typelessGetFeaturesInternal(
    std::vector<TrigCompositeUtils::TypelessLinkInfo>& features, 
    std::set<const TrigCompositeUtils::NavGraphNode*>& fullyExploredFrom,
    const TrigCompositeUtils::NavGraphNode* navGraphNode, 
    // Following are passed down from typelessGetFeatures
    const Trig::FeatureRequestDescriptor& frd,
    const CLID clid,  
    const TrigCompositeUtils::DecisionIDContainer chainIDs, 
    const EventContext& ctx,
    const asg::EventStoreType* eventStore = nullptr);

  /**
   * @brief Removes type erased element links from the supplied vectors if they do not come from the specified collection (regex match).
   * @param[in] expression Regex compiled StoreGate key of the collection to match against. Passing "" as the string wrapped in the expression performs no filtering.
   * @param[in,out] keyVec Mutable vector of element link keys on which to filter.
   * @param[in,out] clidVec Mutable vector of element link CLIDs on which to filter.
   * @param[in,out] indexVec Mutable vector of element link indices on which to filter.
   * @param[in] ctx Event context.
   * @param[in] eventStore Optional pointer to event store, only needed in AnalysisBase.
   **/
  void filterLinkVectorByContainerKey(
  	const std::regex& expression,
  	std::vector<sgkey_t>& keyVec,
  	std::vector<CLID>& clidVec,
  	std::vector<TrigCompositeUtils::Decision::index_type>&indexVec,
  	const EventContext& ctx,
  	const asg::EventStoreType* eventStore = nullptr);

}

#include "FeatureRequestHelpers.icc"

#endif