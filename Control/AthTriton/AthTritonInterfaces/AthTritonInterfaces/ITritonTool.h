
/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#pragma once

#include "GaudiKernel/IAlgTool.h"

#include <vector>
#include <variant>
#include <map>
#include <string>

namespace AthInfer {

    using DataVariant = std::variant<std::vector<float>, std::vector<int64_t> >;
    using InferenceData = std::pair<std::vector<int64_t>, DataVariant>;
    using InputDataMap = std::map<std::string, InferenceData>;
    using OutputDataMap = std::map<std::string, InferenceData>;

    class ITritonTool : virtual public IAlgTool 
    {
        public:
        DeclareInterfaceID(ITritonTool, 1, 0);

        // Run inference with multiple inputs and multiple outputs
        virtual StatusCode inference(InputDataMap& inputData, OutputDataMap& outputData) const = 0;
    };
}

