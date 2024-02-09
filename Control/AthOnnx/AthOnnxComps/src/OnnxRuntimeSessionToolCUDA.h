// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#ifndef OnnxRuntimeSessionToolCUDA_H
#define OnnxRuntimeSessionToolCUDA_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "AthOnnxInterfaces/IOnnxRuntimeSessionTool.h"
#include "AthOnnxInterfaces/IOnnxRuntimeSvc.h"
#include "GaudiKernel/ServiceHandle.h"

namespace AthOnnx {
    // @class OnnxRuntimeSessionToolCUDA
    // 
    // @brief Tool to create Onnx Runtime session with CUDA backend
    //
    // @author Xiangyang Ju <xiangyang.ju@cern.ch>
    class OnnxRuntimeSessionToolCUDA :  public extends<AthAlgTool, IOnnxRuntimeSessionTool>
    {
        public:
        /// Standard constructor
        OnnxRuntimeSessionToolCUDA( const std::string& type,
                                const std::string& name,
                                const IInterface* parent );
        virtual ~OnnxRuntimeSessionToolCUDA() = default;

        /// Initialize the tool
        virtual StatusCode initialize() override final;
        /// Finalize the tool
        virtual StatusCode finalize() override final;

        /// Create Onnx Runtime session
        virtual Ort::Session& session() const override final;

        protected:
        OnnxRuntimeSessionToolCUDA() = delete;
        OnnxRuntimeSessionToolCUDA(const OnnxRuntimeSessionToolCUDA&) = delete;
        OnnxRuntimeSessionToolCUDA& operator=(const OnnxRuntimeSessionToolCUDA&) = delete;

        private:
        StringProperty m_modelFileName{this, "ModelFileName", "", "The model file name"};
        /// The device ID to use.
        IntegerProperty m_deviceId{this, "DeviceId", 0};
        BooleanProperty m_enableMemoryShrinkage{this, "EnableMemoryShrinkage", false};

        /// runtime service
        ServiceHandle<IOnnxRuntimeSvc> m_onnxRuntimeSvc{"AthOnnx::OnnxRuntimeSvc", "AthOnnx::OnnxRuntimeSvc"};
        std::unique_ptr<Ort::Session> m_session;
    };
}

#endif
