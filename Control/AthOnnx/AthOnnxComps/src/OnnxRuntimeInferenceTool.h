// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#ifndef OnnxRuntimeInferenceTool_H
#define OnnxRuntimeInferenceTool_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "AthOnnxInterfaces/IOnnxRuntimeInferenceTool.h"
#include "AthOnnxInterfaces/IOnnxRuntimeSvc.h"
#include "AthOnnxInterfaces/IOnnxRuntimeSessionTool.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"

namespace AthOnnx {
    // @class OnnxRuntimeInferenceTool
    // 
    // @brief Tool to create Onnx Runtime session with CPU backend
    //
    // @author Xiangyang Ju <xiangyang.ju@cern.ch>
    class OnnxRuntimeInferenceTool :  public extends<AthAlgTool, IOnnxRuntimeInferenceTool>
    {
        public:
        /// Standard constructor
        OnnxRuntimeInferenceTool( const std::string& type,
                                const std::string& name,
                                const IInterface* parent );
        virtual ~OnnxRuntimeInferenceTool() = default;

        /// Initialize the tool
        virtual StatusCode initialize() override;
        /// Finalize the tool
        virtual StatusCode finalize() override;

        virtual void setBatchSize(int64_t batchSize) override final;
        virtual int64_t getBatchSize(int64_t inputDataSize, int idx = 0) const override final;

        virtual StatusCode inference(std::vector<Ort::Value>& inputTensors, std::vector<Ort::Value>& outputTensors) const override final;

        virtual void printModelInfo() const override final;

        protected:
        OnnxRuntimeInferenceTool() = delete;
        OnnxRuntimeInferenceTool(const OnnxRuntimeInferenceTool&) = delete;
        OnnxRuntimeInferenceTool& operator=(const OnnxRuntimeInferenceTool&) = delete;

        private:
        StatusCode getNodeInfo();

        ServiceHandle<IOnnxRuntimeSvc> m_onnxRuntimeSvc{"AthOnnx::OnnxRuntimeSvc", "AthOnnx::OnnxRuntimeSvc"};
        ToolHandle<IOnnxRuntimeSessionTool> m_onnxSessionTool{
            this, "ORTSessionTool", 
            "AthOnnx::OnnxRuntimeSessionToolCPU"
        };        
        std::vector<std::string> m_inputNodeNames;
        std::vector<std::string> m_outputNodeNames;
    };
} // namespace AthOnnx

#endif
