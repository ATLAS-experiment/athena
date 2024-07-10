// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#ifndef AthInfer_IAthInferenceTool_H
#define AthInfer_IAthInferenceTool_H

#include "AsgTools/IAsgTool.h"

#include <vector>
#include <string>
#include <variant>
#include <map>

namespace AthInfer {
 
    using DataVariant = std::variant<std::vector<float>, std::vector<int64_t> >;
    using InferenceData = std::pair<std::vector<int64_t>, DataVariant>;
    using InputDataMap = std::map<std::string, InferenceData>;
    using OutputDataMap = std::map<std::string, InferenceData>;

    // class asg::IAsgTool
    // Interface class for running inferences in Athena.
    // @author Xiangyang Ju <xju@cern.ch>

    class IAthInferenceTool : virtual public asg::IAsgTool 
    {
        ASG_TOOL_INTERFACE(IAthInferenceTool)
        public:

        // Run inference with multiple inputs and multiple outputs
        virtual StatusCode inference(InputDataMap& inputData, OutputDataMap& outputData) const = 0;
    };
}

#endif
