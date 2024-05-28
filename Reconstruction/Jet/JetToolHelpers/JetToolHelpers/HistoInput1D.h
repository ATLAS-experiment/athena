/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JETTOOLHELPERS_HISTOINPUT1D_H
#define JETTOOLHELPERS_HISTOINPUT1D_H

#include "TH1.h"
#include "AsgTools/AsgTool.h"
#include "AsgTools/ToolHandle.h"

#include "JetToolHelpers/HistoInputBase.h"
#include "JetAnalysisInterfaces/IVarTool.h"
namespace JetHelper{

     /// Class HistoInput1D
     /// User interface to read 1D histograms 

class HistoInput1D : public HistoInputBase
{
    ASG_TOOL_CLASS(HistoInput1D,IVarTool)
    
    public:
        /// Constructor for standalone usage
        HistoInput1D(const std::string& name);
        /// Function initialising the tool
        virtual StatusCode initialize() override;

        /// return value of histogram at jet variable
        virtual float getValue(const xAOD::Jet& jet, const JetContext& event) const override;
        using IVarTool::getValue;

        /// standalone test, no implemented yet
        virtual bool runUnitTests() const;

    private:
        /// interface to read xAOD::jet variable to be defined by user
        ToolHandle<IVarTool> m_vartool {this, "varTool1", "VarTool", "InputVariable instance" };
};
} // namespace JetHelper
#endif
