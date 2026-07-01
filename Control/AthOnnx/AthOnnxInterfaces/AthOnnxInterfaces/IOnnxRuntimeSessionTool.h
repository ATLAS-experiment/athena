// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#ifndef AthOnnx_IOnnxRUNTIMESESSIONTool_H
#define AthOnnx_IOnnxRUNTIMESESSIONTool_H

#include "AsgTools/IAsgTool.h"

#include <onnxruntime_cxx_api.h>


namespace AthOnnx {
    // class IAlgTool
    //
    // Interface class for creating Onnx Runtime sessions.
    //
    // @author Xiangyang Ju <xju@cern.ch>
    //
    class IOnnxRuntimeSessionTool : virtual public asg::IAsgTool
    {
        ASG_TOOL_INTERFACE(IOnnxRuntimeSessionTool)

        public:

        // Create Onnx Runtime session
        virtual Ort::Session& session() const = 0;

        // Check if returned sessions support asynchronous inference
        [[nodiscard]] virtual bool supportsAsync() const = 0;

    };

} // namespace AthOnnx

#endif
