/*
 Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
 */

#include "CxxUtils/checker_macros.h"
#include "IsolationSelection/IsolationConditionCombined.h"

#include <TF2.h>
#include <TH3.h>

#include <algorithm>
#include <cmath>

namespace CP {
    IsolationConditionCombined::IsolationConditionCombined(const std::string& name, const std::vector<xAOD::Iso::IsolationType>& isoTypes,
                                                           std::unique_ptr<TF1> isoFunction, const std::string& cutFunction, const std::string& isoDecSuffix, bool invertCut) :
        IsolationCondition(name, isoTypes, isoDecSuffix),
        m_cutFunction (std::make_unique<TF1>(cutFunction.c_str(), cutFunction.c_str())),
        m_isoFunction (std::move(isoFunction)),
        m_invertCut(invertCut)
    {
    }
    IsolationConditionCombined::IsolationConditionCombined(const std::string& name, const std::vector<std::string>& isoTypes,
                                                           std::unique_ptr<TF1> isoFunction, const std::string& cutFunction, const std::string& isoDecSuffix, bool invertCut) :
        IsolationCondition(name, isoTypes, isoDecSuffix),
        m_cutFunction (std::make_unique<TF1>(cutFunction.c_str(), cutFunction.c_str())),
        m_isoFunction (std::move(isoFunction)),
        m_invertCut(invertCut)
    {
    }

    bool IsolationConditionCombined::accept(const xAOD::IParticle& x) const {
        const float cutValue = m_cutFunction->Eval(x.pt());
        std::vector<double> isoVars(num_types(), 0);
        for (unsigned int iacc = 0; iacc < num_types(); ++iacc) {
            const FloatAccessor& acc = accessor(iacc);

            if (!acc.isAvailable(x)) {
                // Temporary fix for missing closeByCorr variables if no primary vertex exists for the event
                // If closeByCorr variable does not exist, fallback to the standard isolation variable 2025/02
                const FloatAccessor& acc_noCloseBy = accessor_noCloseBy(iacc);
                if (acc_noCloseBy.isAvailable(x)) {
                    isoVars[iacc] = acc_noCloseBy(x);
                }
                else {
                    ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<<"Accessor "<<SG::AuxTypeRegistry::instance().getName(acc.auxid())
                    <<" is not available. Expected when using primary AODs, post-p3793 derivations (only for *FixedRad or FixedCutPflow* for electrons), "
                    <<" pre-p3517 derivations (only for FC*), or pre-p3830 derivations (for other electron WPs)");
                    if (!m_isoDecSuffix.empty()) throw std::runtime_error ("IsolationConditionCombined: IsolationSelectionTool property 'IsoDecSuffix' is set to " + m_isoDecSuffix + ". Must run on derivation made with IsolationCloseByCorrection to create the isolation variables with this suffix, or remove 'IsoDecSuffix'. ");
                    isoVars[iacc] = FLT_MAX;
                }
            } else
                isoVars[iacc] = acc(x);
        }
        // In general TF1::EvalPar is non-const/not thread-safe. But for the special
        // case of using it as TFormula it can be considered const/safe. It would be safer
        // to change the interface to not allow a generic TF1.
        TF1* f ATLAS_THREAD_SAFE = m_isoFunction.get();
        const float isoValue = f->EvalPar(isoVars.data());
        if (!m_invertCut) return isoValue <= cutValue;
        return isoValue > cutValue;
    }

    bool IsolationConditionCombined::accept(const strObj& x) const {
        const float cutValue = m_cutFunction->Eval(x.pt);
        std::vector<double> isoVars;
        for (unsigned int itype = 0; itype < num_types(); ++itype) isoVars.push_back(x.isolationValues[type(itype)]);
        TF1* f ATLAS_THREAD_SAFE = m_isoFunction.get(); // see comment above
        const float isoValue = f->EvalPar(isoVars.data());
        if (!m_invertCut) return isoValue <= cutValue;
        return isoValue > cutValue;
    }
}  // namespace CP
