// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#ifndef AthOnnx_IOnnxRUNTIMESESSIONTool_H
#define AthOnnx_IOnnxRUNTIMESESSIONTool_H

// Gaudi include(s).
#include "GaudiKernel/IAlgTool.h"

#include <onnxruntime_cxx_api.h>


namespace AthOnnx {
    // class IAlgTool
    //
    // Interface class for creating Onnx Runtime sessions.
    //
    // @author Xiangyang Ju <xju@cern.ch>
    //
    class IOnnxRuntimeSessionTool : virtual public IAlgTool
    {
        public:

        virtual ~IOnnxRuntimeSessionTool() = default;

        // @name InterfaceID
        DeclareInterfaceID(IOnnxRuntimeSessionTool, 1, 0);

        // Create Onnx Runtime session
        virtual Ort::Session& session() const = 0;

    };

} // namespace AthOnnx

#endif
