/*
 Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
 */

#include <IsolationSelection/IsolationWP.h>

namespace CP {

    const std::string& IsolationWP::name() const { return m_name; }
    void IsolationWP::name(const std::string& name) { m_name = name; }
    asg::AcceptData IsolationWP::accept(const xAOD::IParticle& p) const {
        asg::AcceptData result(&m_acceptInfo);
        for (const auto& c : m_cuts) {
            if (c->accept(p)) result.setCutResult(c->name(), true);
            else{
                result.clear();
                return result;
            } 
        }
        return result;
    }

    asg::AcceptData IsolationWP::accept(const strObj& p) const {
        asg::AcceptData result(&m_acceptInfo);
        for (const auto& c : m_cuts) {
            if (c->accept(p)) { result.setCutResult(c->name(), true); }
            else{
                result.clear();
                return result;
            } 
        }
        return result;
    }

    void IsolationWP::addCut(std::unique_ptr<IsolationCondition> i) {
        m_acceptInfo.addCut(i->name(), i->name());
        m_cuts.emplace_back(std::move(i));
    }
    const asg::AcceptInfo& IsolationWP::getAccept() const { return m_acceptInfo; }
    const std::vector<std::unique_ptr<IsolationCondition>>& IsolationWP::conditions() const { return m_cuts; }

    IsolationWP::IsolationWP(const std::string& name) : m_name(name) {}
}  // namespace CP
