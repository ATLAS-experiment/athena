/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JETTOOLHELPERS_HISTOINPUT2D_H
#define JETTOOLHELPERS_HISTOINPUT2D_H

#include "TH2.h"
#include "TH1.h"
#include "AsgTools/AsgTool.h"

#include "JetToolHelpers/HistoInputBase.h"
#include "AsgTools/ToolHandle.h"
#include "JetAnalysisInterfaces/IVarTool.h"

namespace JetHelper {

    /// Class HistoInput2D
    /// User interface to read 2D histrograms

class HistoInput2D : public HistoInputBase
{
    ASG_TOOL_CLASS(HistoInput2D,IVarTool)

    public:
        /// Constructor for standalone usage
        HistoInput2D(const std::string& name);
        /// Function initialising the tool
        virtual StatusCode initialize() override;
        /// return value of histogram at jet variable
        virtual float getValue(const xAOD::Jet& jet, const JetContext& event) const override;
        using IVarTool::getValue;
        /// standalone test, no implemented yet
        virtual bool runUnitTests() const;

    private:
        
        /// interface for jet InputVariable to be defined by user, correspond to axis X of hist
        ToolHandle<IVarTool> m_varTool1 {this, "varTool1", "VarTool", "InputVariable 1 instance" };
        /// interface for jet InputVariable to be defined by user, correspond to axis Y of hist
        ToolHandle<IVarTool> m_varTool2 {this, "varTool2", "VarTool", "InputVariable 2 instance" };
        // Variable and function to cache projections in case of 1-D interpolaton in 2-D
        std::vector< std::unique_ptr<TH1> > m_cachedProj;
        StatusCode cacheProjections();
};
} // namespace JetHelper
#endif

