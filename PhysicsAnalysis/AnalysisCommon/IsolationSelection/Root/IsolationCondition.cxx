/*
 Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
 */

#include "IsolationSelection/IsolationCondition.h"

#include "AthContainers/AuxElement.h"
#include "xAODPrimitives/IsolationCorrection.h"
namespace CP {

    IsolationCondition::IsolationCondition(const std::string& name, 
                                           const std::vector<xAOD::Iso::IsolationType>& isoTypes, 
                                           const std::string& isoDecSuffix) :
        asg::AsgMessaging{"IsolationCondition_"+name},
        m_name(name), m_isolationType(isoTypes), m_isoDecSuffix(isoDecSuffix) {
        for (const xAOD::Iso::IsolationType& iso_type : m_isolationType) { 
            std::string accName = std::string(toCString(iso_type)) + (isoDecSuffix.empty() ? "" : "_") + isoDecSuffix; 
            m_acc.emplace_back(accName); 
    }
    }
    IsolationCondition::IsolationCondition(const std::string& name, const std::vector<std::string>& isoTypes, const std::string& isoDecSuffix) : 
            asg::AsgMessaging{"IsolationCondition_"+name},
            m_name(name), m_isoDecSuffix(isoDecSuffix) {
        for (const std::string& iso_type : isoTypes) {
            m_isolationType.push_back(xAOD::Iso::IsolationType(0));
            std::string accName = iso_type + (isoDecSuffix.empty() ? "" : "_") + isoDecSuffix;
            m_acc.emplace_back(accName);
        }
    }
    IsolationCondition::IsolationCondition(const std::string& name, xAOD::Iso::IsolationType isoType, const std::string& isoDecSuffix) :
        IsolationCondition(name, std::vector<xAOD::Iso::IsolationType>{isoType}, isoDecSuffix) {}
    IsolationCondition::IsolationCondition(const std::string& name, const std::string& isoType, const std::string& isoDecSuffix) :
        IsolationCondition(name, std::vector<std::string>{isoType}, isoDecSuffix) {}

    unsigned int IsolationCondition::num_types() const { return m_isolationType.size(); }
    std::string IsolationCondition::name() const { return m_name; }
    xAOD::Iso::IsolationType IsolationCondition::type(unsigned int n) const { return m_isolationType[n]; }
    const SG::AuxElement::ConstAccessor<float>& IsolationCondition::accessor(unsigned int n) const { return m_acc.at(n); }
}  // namespace CP
