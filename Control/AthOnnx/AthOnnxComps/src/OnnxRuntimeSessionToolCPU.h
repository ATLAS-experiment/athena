// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#ifndef OnnxRuntimeSessionToolCPU_H
#define OnnxRuntimeSessionToolCPU_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "AthOnnxInterfaces/IOnnxRuntimeSessionTool.h"
#include "AthOnnxInterfaces/IOnnxRuntimeSvc.h"
#include "GaudiKernel/ServiceHandle.h"

namespace AthOnnx {
    // @class OnnxRuntimeSessionToolCPU
    // 
    // @brief Tool to create Onnx Runtime session with CPU backend
    //
    // @author Xiangyang Ju <xiangyang.ju@cern.ch>
    class OnnxRuntimeSessionToolCPU :  public extends<AthAlgTool, IOnnxRuntimeSessionTool>
    {
        public:
        /// Standard constructor
        OnnxRuntimeSessionToolCPU( const std::string& type,
                                const std::string& name,
                                const IInterface* parent );
        virtual ~OnnxRuntimeSessionToolCPU() = default;

        /// Initialize the tool
        virtual StatusCode initialize() override final;
        /// Finalize the tool
        virtual StatusCode finalize() override final;

        /// Create Onnx Runtime session
        virtual Ort::Session& session() const override final;

        protected:
        OnnxRuntimeSessionToolCPU() = delete;
        OnnxRuntimeSessionToolCPU(const OnnxRuntimeSessionToolCPU&) = delete;
        OnnxRuntimeSessionToolCPU& operator=(const OnnxRuntimeSessionToolCPU&) = delete;

        private:
        StringProperty m_modelFileName{this, "ModelFileName", "", "The model file name"};
        ServiceHandle<IOnnxRuntimeSvc> m_onnxRuntimeSvc{"AthOnnx::OnnxRuntimeSvc", "AthOnnx::OnnxRuntimeSvc"};
        std::unique_ptr<Ort::Session> m_session;
    };
}

#endif
