/*
 Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
 */

#ifndef ISOLATIONSELECTION_ISOLATIONWP_H
#define ISOLATIONSELECTION_ISOLATIONWP_H

#include <IsolationSelection/IsolationCondition.h>
#include <xAODPrimitives/tools/getIsolationAccessor.h>

#include <map>
#include <memory>

#include "PATCore/AcceptData.h"
namespace CP {
    class IsolationWP {
    public:
        IsolationWP(const std::string& name);
        ~IsolationWP() = default;
        const std::string& name() const;
        void name(const std::string& name);

        asg::AcceptData accept(const xAOD::IParticle& p) const;
        asg::AcceptData accept(const strObj& p) const;
        void addCut(std::unique_ptr<IsolationCondition> cut);

        const asg::AcceptInfo& getAccept() const;
        const std::vector<std::unique_ptr<IsolationCondition>>& conditions() const;

    private:
        std::string m_name{};
        std::vector<std::unique_ptr<IsolationCondition>> m_cuts;
        asg::AcceptInfo m_acceptInfo;
    };
}  // namespace CP
#endif  // ISOLATIONSELECTION_ISOLATIONWP_H
