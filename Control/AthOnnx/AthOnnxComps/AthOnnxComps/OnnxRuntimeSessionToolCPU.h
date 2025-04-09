// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#ifndef OnnxRuntimeSessionToolCPU_H
#define OnnxRuntimeSessionToolCPU_H

#include "AsgTools/AsgTool.h"
#include "AthOnnxInterfaces/IOnnxRuntimeSessionTool.h"
#include "AthOnnxInterfaces/IOnnxRuntimeSvc.h"
#include "AsgServices/ServiceHandle.h"
#include "AsgTools/PropertyWrapper.h"

#include <string>

namespace AthOnnx {
    // @class OnnxRuntimeSessionToolCPU
    // 
    // @brief Tool to create Onnx Runtime session with CPU backend
    //
    // @author Xiangyang Ju <xiangyang.ju@cern.ch>
    class OnnxRuntimeSessionToolCPU :  public asg::AsgTool, virtual public IOnnxRuntimeSessionTool
    {
        ASG_TOOL_CLASS(OnnxRuntimeSessionToolCPU, IOnnxRuntimeSessionTool)
        public:
        /// Standard constructor
        OnnxRuntimeSessionToolCPU( const std::string& name );
        virtual ~OnnxRuntimeSessionToolCPU() = default;

        /// Initialize the tool
        virtual StatusCode initialize() override final;

        /// Create Onnx Runtime session
        virtual Ort::Session& session() const override final;

        protected:
        OnnxRuntimeSessionToolCPU() = delete;
        OnnxRuntimeSessionToolCPU(const OnnxRuntimeSessionToolCPU&) = delete;
        OnnxRuntimeSessionToolCPU& operator=(const OnnxRuntimeSessionToolCPU&) = delete;

        private:
        Gaudi::Property<std::string> m_modelFileName{this, "ModelFileName", "", "The model file name"};
        ServiceHandle<IOnnxRuntimeSvc> m_onnxRuntimeSvc{this, "OnnxRuntimeSvc", "AthOnnx::OnnxRuntimeSvc", "The Onnx runtime service"};
        std::unique_ptr<Ort::Session> m_session;
    };
}

#endif
