/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONINFERENCETOOLS_INFERENCEALG_H
#define MUONINFERENCETOOLS_INFERENCEALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "MuonInferenceInterfaces/IGraphInferenceTool.h"

namespace MuonML{
    class InferenceAlg : public AthReentrantAlgorithm {
        public:
            using AthReentrantAlgorithm::AthReentrantAlgorithm;

            virtual StatusCode initialize() override final;
            virtual StatusCode execute(const EventContext& ctx) const override final;
        private:
            ToolHandleArray<IGraphInferenceTool> m_inferenceTools{this, "InferenceTools", {},
                                                                  "List of machine learning inference tools to be processed with the same graph"};
    };
}



#endif