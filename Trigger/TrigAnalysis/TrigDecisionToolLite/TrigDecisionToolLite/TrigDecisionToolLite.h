/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TrigDecision_TrigDecisionToolLite_h
#define TrigDecision_TrigDecisionToolLite_h
/**********************************************************************************
 * @class  : TrigDecisionToolLite
 *
 * @brief Lightweight alternate to the TrigDecisionTool, without configuration access.
 * 
 * The TrigDecisionToolLite provides a minimal interface with the per-event trigger payload.
 * Unlike the full version, this tool does NOT interface with the trigger configuration data.
 * This tool will only allow access to information from the HLT navigation container which can be
 * interpreted independently of the configuration. This includes per-chain feature access. 
 * And HLT chain isPassed information, as returned from the HLT navigation terminus node.
 *
 ***********************************************************************************/

#include "AsgTools/AsgTool.h"
#include "AsgTools/CurrentContext.h"
#include <AsgTools/PropertyWrapper.h>
#include "AsgDataHandles/ReadHandleKey.h"

#include "TrigCompositeUtils/TrigCompositeUtils.h"
#include "TrigCompositeUtils/TypelessLinkInfo.h"

#include "TrigAnalysisHelpers/FeatureRequestDescriptor.h"

#include "TrigDecisionInterface/ITrigDecisionToolLite.h"

namespace Trig {
  class TrigDecisionToolLite : public asg::AsgTool, virtual public ITrigDecisionToolLite
  {
    ASG_TOOL_CLASS1(TrigDecisionToolLite, ITrigDecisionToolLite)

  public:

    TrigDecisionToolLite(const std::string& name);
    ~TrigDecisionToolLite() = default;

    StatusCode initialize();

    /**
     * @brief true if the given HLT chain passed for physics
     * @param[in] chain A HLT::Identifier wrapping a single HLT chain.
     * Only chains which are included in the trigger navigation payload of the file may be queried in this way.
     **/ 
    virtual bool isPassed(const HLT::Identifier& chain, const EventContext& ctx = Gaudi::Hive::currentContext()) const final;

    /**
     * @brief true if the given HLT chain passed for physics
     * @param[in] chain This version of the call wraps the HLT::Identifier(string) conversion.
     * Best performance is obtained by the caller using the HLT::Identifier method.
     * Only chains which are included in the trigger navigation payload of the file may be queried in this way.
     **/ 
    virtual bool isPassed(const std::string& chain, const EventContext& ctx = Gaudi::Hive::currentContext()) const final;

    /**
     * @brief true if any of the given HLT chain passed for physics (logical OR)
     * @param[in] chains A vector of HLT::Identifier with each entry wrapping a single HLT chain.
     * Only chains which are included in the trigger navigation payload of the file may be queried in this way.
     **/ 
    virtual bool isPassed(const std::vector<HLT::Identifier>& chains, const EventContext& ctx = Gaudi::Hive::currentContext()) const final;

    /**
     * @brief true if any of the given HLT chain passed for physics (logical OR)
     * @param[in] chains This version of the call wraps the HLT::Identifier(string) conversion.
     * Best performance is obtained by the caller using the std::vector<HLT::Identifier> method.
     * Only chains which are included in the trigger navigation payload of the file may be queried in this way.
     **/ 
    virtual bool isPassed(const std::vector<std::string>& chains, const EventContext& ctx = Gaudi::Hive::currentContext()) const final;

  private:

    /**
     * @brief Internal type erased features retrieval implementation call.
     * Please use the typed feature interface provided by ITrigDecisionToolLite
     * @see ITrigDecisionToolLite::features(const Trig::FeatureRequestDescriptor& frd, const EventContext& ctx)
     * @see ITrigDecisionToolLite::features(const std::string& chain, const EventContext& ctx)
     * @see ITrigDecisionToolLite::features(const Trig::FeatureRequestDescriptor& frd, const asg::EventStoreType* eventStore)
     * @see ITrigDecisionToolLite::features(const std::string& chain, const asg::EventStoreType* eventStore)
     * @param[in] frd Feature request descriptor object describing the feature request.
     * @param[in] clid Class ID corresponding to the typed feature request.
     * @param[in] ctx Event context, used in Athena.
     * @param[out] Vector of type erased feature link info objects, converted by interface class to typed versions 
     **/ 
    virtual std::vector<TrigCompositeUtils::TypelessLinkInfo> 
      typelessFeatures( const Trig::FeatureRequestDescriptor& frd, const CLID clid, const EventContext& ctx) const final;

    virtual const asg::EventStoreType* getEventStore() const final;

    /** 
     * @brief Internal. Obtains the set of passing chain IDs from the trigger navigation's terminus node.
     * @param[in] setOfAllPassingChains Mutable set of DecisionIDs to be populated with passing chain IDs.
     **/
    StatusCode fillPassingChainsSet(TrigCompositeUtils::DecisionIDContainer& setOfAllPassingChains, const EventContext& ctx) const;

    SG::ReadHandleKey<TrigCompositeUtils::DecisionContainer> m_HLTSummaryKeyIn {this, "HLTSummary",
      "HLTNav_Summary_DAODSlimmed", "HLT navigation summary container key"};

  public:

    static std::atomic<bool> s_printWarningMessages ATLAS_THREAD_SAFE;

  };

} // End of namespace

#endif
