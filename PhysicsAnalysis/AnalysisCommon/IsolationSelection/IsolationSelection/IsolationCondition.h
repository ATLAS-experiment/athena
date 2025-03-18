/*
 Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
 */

#ifndef ISOLATIONSELECTION_ISOLATIONCONDITION_H
#define ISOLATIONSELECTION_ISOLATIONCONDITION_H

#include <IsolationSelection/Defs.h>
#include <xAODBase/IParticle.h>
#include <xAODPrimitives/IsolationType.h>
#include <xAODPrimitives/tools/getIsolationAccessor.h>

#include <map>
#include <memory>
#include <vector>

#include "AthContainers/AuxElement.h"
#include "AsgMessaging/AsgMessaging.h"



namespace CP {
    struct strObj {
        float pt{0.f};
        float eta{0.f};
        std::vector<float> isolationValues;
        xAOD::Type::ObjectType type{xAOD::Type::ObjectType::EventInfo};
    };

    class IsolationCondition : public asg::AsgMessaging {
    public:
        IsolationCondition(const std::string& name, xAOD::Iso::IsolationType isoType, const std::string& isoDecSuffix = "");
        IsolationCondition(const std::string& name, const std::vector<xAOD::Iso::IsolationType>& isoTypes, const std::string& isoDecSuffix = "");
        IsolationCondition(const std::string& name, const std::string& isoType, const std::string& isoDecSuffix = "");
        IsolationCondition(const std::string& name, const std::vector<std::string>& isoTypes, const std::string& isoDecSuffix = "");

        IsolationCondition(const IsolationCondition& rhs) = delete;
        IsolationCondition& operator=(const IsolationCondition& rhs) = delete;
        virtual ~IsolationCondition() = default;

        const std::string& name() const;

        unsigned int num_types() const;
        xAOD::Iso::IsolationType type(unsigned int n = 0) const;
        const FloatAccessor& accessor(unsigned int n = 0) const;
        const FloatAccessor& accessor_noCloseBy(unsigned int n = 0) const;

        virtual bool accept(const xAOD::IParticle& x) const = 0;
        virtual bool accept(const strObj& x) const = 0;

    private:
        std::string m_name;
        std::vector<xAOD::Iso::IsolationType> m_isolationType;
        std::vector<FloatAccessor> m_acc;
        std::vector<FloatAccessor> m_acc_noCloseBy;
        
    protected:
        std::string m_isoDecSuffix{};
    };
}  // namespace CP
#endif
