// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#ifndef OnnxRuntimeInferenceTool_H
#define OnnxRuntimeInferenceTool_H

#include "AsgTools/AsgTool.h"
#include "AthOnnxInterfaces/IOnnxRuntimeInferenceTool.h"
#include "AthOnnxInterfaces/IAthInferenceTool.h"

#include "AthOnnxInterfaces/IOnnxRuntimeSvc.h"
#include "AthOnnxInterfaces/IOnnxRuntimeSessionTool.h"
#include "AsgServices/ServiceHandle.h"
#include "AsgTools/ToolHandle.h"

namespace AthOnnx {
    // @class OnnxRuntimeInferenceTool
    // 
    // @brief Tool to create Onnx Runtime session with CPU backend
    //
    // @author Xiangyang Ju <xiangyang.ju@cern.ch>
    class OnnxRuntimeInferenceTool :  public asg::AsgTool, virtual public IOnnxRuntimeInferenceTool, virtual public AthInfer::IAthInferenceTool
    {
        ASG_TOOL_CLASS2(OnnxRuntimeInferenceTool, IOnnxRuntimeInferenceTool, AthInfer::IAthInferenceTool)
        public:
        /// Standard constructor
        OnnxRuntimeInferenceTool( const std::string& name );
        virtual ~OnnxRuntimeInferenceTool() = default;

        /// Initialize the tool
        virtual StatusCode initialize() override;


        virtual void setBatchSize(int64_t batchSize) override final;
        virtual int64_t getBatchSize(int64_t inputDataSize, int idx = 0) const override final;

        virtual StatusCode inference(std::vector<Ort::Value>& inputTensors, std::vector<Ort::Value>& outputTensors) const override final;

        virtual void printModelInfo() const override final;

        virtual StatusCode inference(AthInfer::InputDataMap& inputData, AthInfer::OutputDataMap& outputData) const override final;

        protected:
        OnnxRuntimeInferenceTool() = delete;
        OnnxRuntimeInferenceTool(const OnnxRuntimeInferenceTool&) = delete;
        OnnxRuntimeInferenceTool& operator=(const OnnxRuntimeInferenceTool&) = delete;

        private:
        StatusCode getNodeInfo();

        ServiceHandle<IOnnxRuntimeSvc> m_onnxRuntimeSvc{this, "OnnxRuntimeSvc", "AthOnnx::OnnxRuntimeSvc", "The Onnx runtime service"};
        ToolHandle<IOnnxRuntimeSessionTool> m_onnxSessionTool{
            this, "ORTSessionTool", 
            "AthOnnx::OnnxRuntimeSessionToolCPU",
            "The Onnx session tool"
        };        
        std::vector<std::string> m_inputNodeNames;
        std::vector<std::string> m_outputNodeNames;
    };
} // namespace AthOnnx

#endif
