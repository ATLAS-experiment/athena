/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JETCPINTERFACES_ICPJETUNCERTAINTIESTOOL_H
#define JETCPINTERFACES_ICPJETUNCERTAINTIESTOOL_H

#include "JetInterface/IJetUncertaintiesTool.h"

#include "PATInterfaces/ISystematicsTool.h"
#include "PATInterfaces/CorrectionCode.h"

class ICPJetUncertaintiesTool : virtual public IJetUncertaintiesTool,
                                virtual public CP::ISystematicsTool
{
    // Interface declaration
    ASG_TOOL_INTERFACE(ICPJetUncertaintiesTool)

    public:
        ICPJetUncertaintiesTool& operator=( ICPJetUncertaintiesTool&& ) { return *this; }

        // Apply a systematic variation or get a new copy.
        // Overloads without EventInfo use a cached default — not thread-safe.
        virtual CP::CorrectionCode applyCorrection ATLAS_NOT_THREAD_SAFE (xAOD::Jet& input) const = 0;
        virtual CP::CorrectionCode applyCorrection(xAOD::Jet& input, const xAOD::EventInfo& eInfo) const = 0;
        virtual CP::CorrectionCode correctedCopy ATLAS_NOT_THREAD_SAFE (const xAOD::Jet& input, xAOD::Jet*& output) const = 0;
        virtual CP::CorrectionCode correctedCopy(const xAOD::Jet& input, xAOD::Jet*& output, const xAOD::EventInfo& eInfo) const = 0;
        virtual CP::CorrectionCode applyContainerCorrection ATLAS_NOT_THREAD_SAFE (xAOD::JetContainer& inputs) const = 0;
        virtual CP::CorrectionCode applyContainerCorrection(xAOD::JetContainer& inputs, const xAOD::EventInfo& eInfo) const = 0;
        /// Reentrant correction: applies @p syst without mutating shared state.
        /// Requires applySystematicVariation(@p syst) during initialize().
        virtual CP::CorrectionCode applyContainerCorrection(
            xAOD::JetContainer& inputs,
            const CP::SystematicSet& syst) const = 0;

};

#endif

