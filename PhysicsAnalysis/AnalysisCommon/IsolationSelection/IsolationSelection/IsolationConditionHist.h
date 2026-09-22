// Dear emacs, this is -*- c++ -*-

/*
 Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */

#ifndef ISOLATIONSELECTION_ISOLATIONCONDITIONHIST_H
#define ISOLATIONSELECTION_ISOLATIONCONDITIONHIST_H

#include "IsolationSelection/IsolationCondition.h"
#include <memory>

// Forward Declaration(s)
class TF1;
class TH3F;

class Interp3D;

namespace CP {
    class IsolationConditionHist : public IsolationCondition {
    public:
        IsolationConditionHist(const std::string& name, xAOD::Iso::IsolationType isoType, const std::string& isolationFunction,
                               std::unique_ptr<TH3F> efficiencyHisto3D);
        virtual ~IsolationConditionHist() = default;

        bool accept(const xAOD::IParticle& x) const override;
        bool accept(const strObj& x) const override;
        void setInterp(std::shared_ptr<Interp3D> interp) { m_interp = interp; }

    private:
        float getCutValue(const float pt, const float eta) const;
        std::shared_ptr<TH3F> m_efficiencyHisto3D{nullptr};
        std::unique_ptr<TF1> m_isolationFunction{nullptr};
        bool m_ptGeV{false};
        std::shared_ptr<Interp3D> m_interp;
    };
}  // namespace CP
#endif
