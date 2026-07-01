/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGGERMATCHINGTOOL_R3MATCHINGTOOL_H
#define TRIGGERMATCHINGTOOL_R3MATCHINGTOOL_H

#include "AsgTools/AsgTool.h"
#include "AsgTools/ToolHandle.h"
#include "TriggerMatchingTool/IMatchingTool.h"
#include "TriggerMatchingTool/IMatchScoringTool.h"
#include "TrigDecisionTool/TrigDecisionTool.h"
#include "CxxUtils/checker_macros.h"

#include <mutex>

namespace Trig
{
  class R3MatchingTool : public asg::AsgTool, virtual public IMatchingTool
  {
    ASG_TOOL_CLASS(R3MatchingTool, IMatchingTool)
  public:
    using multInfo_t = std::vector<std::size_t>;
    using typeInfo_t = std::vector<xAODType::ObjectType>;
    using chainInfo_t = std::pair<multInfo_t, typeInfo_t>;

    R3MatchingTool(const std::string &name);
    ~R3MatchingTool();

    virtual StatusCode initialize() override;

    virtual bool match(
        const std::vector<const xAOD::IParticle *> &recoObjects,
        std::string_view chain,
        double matchThreshold,
        bool rerun) const override;

    virtual bool match(
        const xAOD::IParticle &recoObject,
        std::string_view chain,
        double matchThreshold,
        bool rerun) const override;

  private:
    ToolHandle<TrigDecisionTool> m_trigDecTool;
    ToolHandle<Trig::IMatchScoringTool> m_scoreTool{
        this, "ScoringTool", "Trig::DRScoringTool","Tool to score pairs of particles"};
    bool matchObjects(
        const xAOD::IParticle *reco,
        const ElementLink<xAOD::IParticleContainer> &onlineLink,
        xAODType::ObjectType onlineType,
        std::map<std::pair<uint32_t, uint32_t>, bool> &cache,
        double drThreshold) const;



    // Keep a cache of the interpreted chain information
    mutable std::map<std::string, chainInfo_t> m_chainInfoCache ATLAS_THREAD_SAFE;
    mutable std::mutex m_chainInfoMutex ATLAS_THREAD_SAFE;
    const chainInfo_t &getChainInfo(const std::string &chain) const;


    // Internal functions
    /// Inherited from the interface but does nothing
    virtual MatchingImplementation *impl() const override { return nullptr; }

  }; //> end class R3MatchingTool
} // namespace Trig

#endif //> !TRIGGERMATCHINGTOOL_R3MATCHINGTOOL_H
