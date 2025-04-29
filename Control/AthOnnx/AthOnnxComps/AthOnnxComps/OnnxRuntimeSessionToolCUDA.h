// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#ifndef OnnxRuntimeSessionToolCUDA_H
#define OnnxRuntimeSessionToolCUDA_H

#include "AsgTools/AsgTool.h"
#include "AthOnnxInterfaces/IOnnxRuntimeSessionTool.h"
#include "AthOnnxInterfaces/IOnnxRuntimeSvc.h"
#include "AsgServices/ServiceHandle.h"
#include "AsgTools/PropertyWrapper.h"

#include <string>

namespace AthOnnx {
    // @class OnnxRuntimeSessionToolCUDA
    // 
    // @brief Tool to create Onnx Runtime session with CUDA backend
    //
    // @author Xiangyang Ju <xiangyang.ju@cern.ch>
    class OnnxRuntimeSessionToolCUDA :  public asg::AsgTool, virtual public IOnnxRuntimeSessionTool
    {
        ASG_TOOL_CLASS(OnnxRuntimeSessionToolCUDA, IOnnxRuntimeSessionTool)
        public:
        /// Standard constructor
        OnnxRuntimeSessionToolCUDA( const std::string& name);
        virtual ~OnnxRuntimeSessionToolCUDA() = default;

        /// Initialize the tool
        virtual StatusCode initialize() override final;

        /// Create Onnx Runtime session
        virtual Ort::Session& session() const override final;

        protected:
        OnnxRuntimeSessionToolCUDA() = delete;
        OnnxRuntimeSessionToolCUDA(const OnnxRuntimeSessionToolCUDA&) = delete;
        OnnxRuntimeSessionToolCUDA& operator=(const OnnxRuntimeSessionToolCUDA&) = delete;

        private:
        Gaudi::Property<std::string> m_modelFileName{this, "ModelFileName", "", "The model file name"};
        /// The device ID to use.
        Gaudi::Property<int> m_deviceId{this, "DeviceId", 0, "Device ID to use"};
        Gaudi::Property<bool> m_enableMemoryShrinkage{this, "EnableMemoryShrinkage", false, "Enable automatic memory shrinkage"};

        /// runtime service
        ServiceHandle<IOnnxRuntimeSvc> m_onnxRuntimeSvc{this, "OnnxRuntimeSvc", "AthOnnx::OnnxRuntimeSvc/OnnxRuntimeSvc", "The Onnx runtime service"};
        std::unique_ptr<Ort::Session> m_session;
    };
}

#endif
