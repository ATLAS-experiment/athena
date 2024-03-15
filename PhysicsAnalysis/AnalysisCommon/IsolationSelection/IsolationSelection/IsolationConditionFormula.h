/*
 Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
 */

#ifndef ISOLATIONSELECTION_ISOLATIONCONDITIONFORMULA_H
#define ISOLATIONSELECTION_ISOLATIONCONDITIONFORMULA_H

#include <memory>

#include "IsolationSelection/IsolationCondition.h"

// Forward Declaration(s)
class TF1;
class TH3F;

namespace CP {
    class IsolationConditionFormula : public IsolationCondition {
    public:
        IsolationConditionFormula(const std::string& name, xAOD::Iso::IsolationType isoType, const std::string& cutFunction,
                                  bool invertCut = false, const std::string& isoDecSuffix = "");
        IsolationConditionFormula(const std::string& name, const std::string& isoType, const std::string& cutFunction, bool invertCut = false, const std::string& isoDecSuffix = "");
        virtual ~IsolationConditionFormula() = default;

        virtual bool accept(const xAOD::IParticle& x) const override;
        virtual bool accept(const strObj& x) const override;

    private:
        std::shared_ptr<TF1> m_cutFunction{nullptr};
        bool m_invertCut{false};
    };
}  // namespace CP
#endif
