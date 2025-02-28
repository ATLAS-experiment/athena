/*
 Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
 */

#include "IsolationSelection/IsolationConditionFormula.h"

#include <TF1.h>
#include <TH3.h>
#include <TString.h>

#include <algorithm>
#include <cmath>

namespace CP {
    IsolationConditionFormula::IsolationConditionFormula(const std::string& name, xAOD::Iso::IsolationType isoType, const std::string& cutFunction,
                                                         bool invertCut, const std::string& isoDecSuffix) :
        IsolationCondition(name, isoType, isoDecSuffix),
        m_cutFunction (std::make_unique<TF1>(cutFunction.c_str(), cutFunction.c_str())),
        m_invertCut (invertCut)
    {
    }
    IsolationConditionFormula::IsolationConditionFormula(const std::string& name, const std::string& isoType, const std::string& cutFunction,
                                                         bool invertCut, const std::string& isoDecSuffix) :
        IsolationCondition(name, isoType, isoDecSuffix),
        m_cutFunction (std::make_unique<TF1>(cutFunction.c_str(), cutFunction.c_str())),
        m_invertCut (invertCut)
    {
    }
    bool IsolationConditionFormula::accept(const xAOD::IParticle& x) const {
        const float cutVal = m_cutFunction->Eval(x.pt());
        const FloatAccessor& acc = accessor();
        if (!acc.isAvailable(x)) {
            // Temporary fix for missing closeByCorr variables if no primary vertex exists for the event
            // If closeByCorr variable does not exist, fallback to the standard isolation variable 2025/02
            const FloatAccessor& acc_noCloseBy = accessor_noCloseBy();
            if (acc_noCloseBy.isAvailable(x)) {
                if (!m_invertCut) return acc_noCloseBy(x) <= cutVal;
                return acc_noCloseBy(x) > cutVal;
            }
            else {
                ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<"Accessor "<<SG::AuxTypeRegistry::instance().getName(acc.auxid())
                    <<" is not available. Expected when using primary AODs, post-p3793 derivations (only for *FixedRad or FixedCutPflow* for electrons), "
                    <<" pre-p3517 derivations (only for FC*), or pre-p3830 derivations (for other electron WPs)");
                if (!m_isoDecSuffix.empty()) throw std::runtime_error ("IsolationConditionCombined: IsolationSelectionTool property 'IsoDecSuffix' is set to " + m_isoDecSuffix + ". Must run on derivation made with IsolationCloseByCorrection to create the isolation variables with this suffix, or remove 'IsoDecSuffix'. ");
                return false;
            }
        }
        if (!m_invertCut) return acc(x) <= cutVal;
        return acc(x) > cutVal;
    }

    bool IsolationConditionFormula::accept(const strObj& x) const {
        const float cutVal = m_cutFunction->Eval(x.pt);
        if (!m_invertCut) return x.isolationValues.at(type()) <= cutVal;
        return x.isolationValues.at(type()) > cutVal;
    }

}  // namespace CP
