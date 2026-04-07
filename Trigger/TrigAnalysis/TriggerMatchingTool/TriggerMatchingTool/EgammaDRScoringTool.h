/*
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */

#ifndef TRIGGERMATCHINGTOOL_EGAMMADRSCORINGTOOL_H
#define TRIGGERMATCHINGTOOL_EGAMMADRSCORINGTOOL_H

#include "AsgTools/AsgTool.h"
#include "TriggerMatchingTool/IMatchScoringTool.h"
#include <string>

namespace xAOD{
 class IParticle;
}

namespace Trig {
    /// Score pairs of particles based on their deltaR
    class EgammaDRScoringTool : public asg::AsgTool, virtual public IMatchScoringTool
    {
    ASG_TOOL_CLASS(EgammaDRScoringTool, IMatchScoringTool)
    public:
        EgammaDRScoringTool(const std::string &name);
        virtual ~EgammaDRScoringTool() override = default;

        virtual StatusCode initialize() override;
        virtual float score(
                const xAOD::IParticle &online,
                const xAOD::IParticle &offline) const override;

    private:
        /// Use decorated cluster information
        bool m_useClusterDecorator;

    }; //> end class EgammaDRScoringTool
} //> end namespace Trig

#endif //> !TRIGGERMATCHINGTOOL_EGAMMADRSCORINGTOOL_H
