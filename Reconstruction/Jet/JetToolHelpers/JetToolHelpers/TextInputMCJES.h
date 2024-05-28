/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JETTOOLHELPERS_TEXTINPUTMCJES_H
#define JETTOOLHELPERS_TEXTINPUTMCJES_H

#include "TH1.h"
#include "AsgTools/AsgTool.h"
#include "AsgTools/ToolHandle.h"

#include "JetToolHelpers/MCJESInputBase.h"
#include "JetAnalysisInterfaces/IVarTool.h"
namespace JetHelper {

    /// Class TextInputMCJES
    /// User interface to read text files containing MCJES calibration factors

class TextInputMCJES : public MCJESInputBase
{
    ASG_TOOL_CLASS(TextInputMCJES,IVarTool)
    
    public:
        /// Constructor for standalone usage
        TextInputMCJES(const std::string& name);
        /// Function initialising the tool
        virtual StatusCode initialize() override;
        /// return value of histogram at jet variable
        virtual float getValue(const xAOD::Jet& jet, const JetContext& event) const override;
        using IVarTool::getValue;

    private:
        /// interface for xAOD::jet variable to be defined by user, this must correspond to jet E in currect version of jet calibration files 
        ToolHandle<IVarTool> m_vartool1{this, "varTool1", "VarTool", "InputVariable instance E" };
        /// interface for xAOD::jet variable to be defined by user, this must correspond to jet Eta in currect version of jet calibration files
        ToolHandle<IVarTool> m_vartool2{this, "varTool2", "VarTool", "InputVariable instance eta" };

};
} // namespace JetHelper
#endif
