///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// EtaMassJESCalibStep.h 
// Header file for class EtaMassJESCalibStep
// Author: Max Swiatlowski <mswiatlo@cern.ch>
/////////////////////////////////////////////////////////////////// 
#ifndef JETCALIBTOOLS_JESCALIBSTEP_H
#define JETCALIBTOOLS_JESCALIBSTEP_H 1

#include <string.h>

#include <TString.h>
#include <TEnv.h>

#include "AsgTools/AsgTool.h"
#include "AsgTools/AsgToolMacros.h"
#include "AsgTools/ToolHandle.h"

#include "xAODEventInfo/EventInfo.h"

#include "JetAnalysisInterfaces/IJetCalibTool.h"
#include "JetAnalysisInterfaces/IJetCalibStep.h"

#include "JetAnalysisInterfaces/IVarTool.h"

class EtaMassJESCalibStep
    : public asg::AsgTool,
      virtual public IJetCalibStep {

        ASG_TOOL_CLASS(EtaMassJESCalibStep, IJetCalibStep)

    public:
        EtaMassJESCalibStep(const std::string& name = "EtaMassJESCalibStep");

        virtual StatusCode initialize() override;
        virtual StatusCode calibrate(xAOD::JetContainer&) const override;

    private:
        ToolHandle<JetHelper::IVarTool> m_textTool_JES {this, "JESReader", "TextInputMCJES", "TextInput for JES" };
        ToolHandle<JetHelper::IVarTool> m_textTool_Eta {this, "EtaReader", "TextInputMCJES", "TextInput for JES" };
        ToolHandle<JetHelper::IVarTool> m_textTool_EmaxJES {this, "EmaxReader", "TextInputMCJES", "TextInput for JES" };

};

#endif
