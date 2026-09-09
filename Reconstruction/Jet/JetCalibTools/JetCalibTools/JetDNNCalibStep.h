/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef JETCALIBTOOLS_JETDNNCALIBSTEP_H
#define JETCALIBTOOLS_JETDNNCALIBSTEP_H

#include "AsgTools/AsgTool.h"
#include "AsgTools/AsgToolMacros.h"
#include "AsgTools/ToolHandle.h"
#include "AsgTools/ToolHandleArray.h"
#include <AsgTools/PropertyWrapper.h>

#include "JetAnalysisInterfaces/IJetCalibStep.h"
#include "JetAnalysisInterfaces/IVarTool.h"
#include "AthOnnxInterfaces/IAthInferenceTool.h"

#include "xAODTracking/VertexContainer.h"
#include "xAODEventInfo/EventInfo.h"

class JetDNNCalibStep
  : public asg::AsgTool,
      virtual public IJetCalibStep {

        ASG_TOOL_CLASS(JetDNNCalibStep, IJetCalibStep)

    public:
        using asg::AsgTool::AsgTool;
  
        virtual StatusCode initialize() override;
        virtual StatusCode calibrate(xAOD::JetContainer&) const override;

    private:
    
        Gaudi::Property<std::string> m_jetInScale {this, "InScale", "JetConstitScaleMomentum", "Starting jet scale"};
        Gaudi::Property<std::string> m_jetOutScale {this, "OutScale", "JetDNNScaleMomentum", "Ending jet scale"};


        Gaudi::Property<int> m_onnxInputShape {this, "onnxInputShape", -1}; //Value -1 is just a default value
        Gaudi::Property<int> m_onnxOutputShape {this, "onnxOutputShape", -1};

        //For accessing the YAML contents
        ToolHandleArray<JetHelper::IVarTool> m_inputVariables {this, "InputVarTool", {}, "Instance of VarTool for accessing input variables"};
        Gaudi::Property<std::vector<float>> m_eScales {this, "EScales", {}, "Vector of E scales for each input variable"};
        Gaudi::Property<std::vector<float>> m_normOffsets {this, "NormOffsets", {}, "Vector of normalisation offsets for each input variable"};
        Gaudi::Property<std::vector<float>> m_normScales {this, "NormScales", {}, "Vector of normalisation scales for each input variable"};

        //For running the ONNX inference
        ToolHandle<AthInfer::IAthInferenceTool> m_onnxTool {this, "ORTInferenceTool", "AthOnnx::OnnxRuntimeInferenceTool"};

        //For accessing mu and NPV (from jet context)
        //Note: JetCalibrationTool.cxx uses "actualInteractionsPerCrossing" (not "average...") specifically
        //for the DNN calibration path (m_doDNNCal), so we match that here rather than Pileup1DResidualCalibStep's default.
        SG::ReadDecorHandleKey<xAOD::EventInfo> m_muKey {this, "actualInteractionsPerCrossing", "EventInfo.actualInteractionsPerCrossing", "Decoration for Actual Number of Interactions Per Crossing"};
        SG::ReadHandleKey<xAOD::VertexContainer> m_npvKey {this, "VertexContainer", "PrimaryVertices"};
        
};
#endif


