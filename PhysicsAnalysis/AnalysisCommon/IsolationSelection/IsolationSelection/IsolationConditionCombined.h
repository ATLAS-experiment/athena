/*
 Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
 */

#ifndef ISOLATIONSELECTION_ISOLATIONCONDITIONCOMBINED_H
#define ISOLATIONSELECTION_ISOLATIONCONDITIONCOMBINED_H

#include <TF1.h>

#include "IsolationSelection/IsolationCondition.h"

namespace CP {
    class IsolationConditionCombined : public IsolationCondition {
    public:
        IsolationConditionCombined(const std::string& name, const std::vector<xAOD::Iso::IsolationType>& isoType,
                                   std::unique_ptr<TF1> isoFunction, const std::string& cutFunction, const std::string& isoDecSuffix = "", bool invertCut = false);
        IsolationConditionCombined(const std::string& name, const std::vector<std::string>& isoType,
                                   std::unique_ptr<TF1> isoFunction, const std::string& cutFunction, const std::string& isoDecSuffix = "", bool invertCut = false);
        virtual ~IsolationConditionCombined() = default;

        bool accept(const xAOD::IParticle& x) const override;
        bool accept(const strObj& x) const override;

    private:
        std::unique_ptr<TF1> m_cutFunction;
        std::unique_ptr<TF1> m_isoFunction;
        bool m_invertCut{false};
    };
}  // namespace CP
#endif
