/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EGAMMAANALYSISINTERFACES__IPHOTONOBSERVABLETOOL__
#define EGAMMAANALYSISINTERFACES__IPHOTONOBSERVABLETOOL__

#include <stdexcept>
#include <string>
#include <type_traits>
#include "AsgAnalysisInterfaces/IObservableTool.h"
#include "xAODEgamma/Photon.h"

class IPhotonObservableTool : public virtual IObservableTool {
    ASG_TOOL_INTERFACE(IPhotonObservableTool)
 
    public:
        virtual ~IPhotonObservableTool() = default;
 
        virtual double evaluate(const xAOD::Photon* photon) const = 0;
 
        double evaluate(const xAOD::IParticle* particle) const override final {
            if (const auto* photon = dynamic_cast<const xAOD::Photon*>(particle)) {
                return evaluate(photon);
            } else {
                throw std::invalid_argument("Wrong particle type, input=" + std::to_string(particle->type()));
            }
        }
};
 
#endif