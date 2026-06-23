/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTS_GNNHOOK_TOOL_H
#define ACTS_GNNHOOK_TOOL_H

#include "ActsPlugins/Gnn/GnnPipeline.hpp"
#include "ActsPlugins/Gnn/Stages.hpp"
#include <vector>
#include <cstdint>


namespace InDet {

  class ScoredGraphHook : public ActsPlugins::GnnHook {
  public:
      void operator()(const ActsPlugins::PipelineTensors& tensors,
                      const ActsPlugins::ExecutionContext& ctx) const override {
          
          ActsPlugins::ExecutionContext cpuCtx;
          cpuCtx.device = ActsPlugins::Device::Cpu();
          cpuCtx.stream = ctx.stream;

          if (tensors.edgeScores.has_value()) {
            const auto& scores = tensors.edgeScores.value();
            auto cpuScores = scores.clone(cpuCtx);
            m_edgeScores.assign(cpuScores.data(), cpuScores.data() + cpuScores.size());
          }

          if (tensors.edgeIndex.size() > 0) {
            const auto& edgeIndex = tensors.edgeIndex;
            auto cpuIndex = edgeIndex.clone(cpuCtx);
            m_edgeIndex.assign(cpuIndex.data(), cpuIndex.data() + cpuIndex.size());
          }
      }

      const std::vector<float>& getEdgeScores() const {
          return m_edgeScores;
      }
      const std::vector<int64_t>& getEdgeIndex() const {
          return m_edgeIndex;
      }

  private:
      mutable std::vector<float> m_edgeScores ATLAS_THREAD_SAFE{};
      mutable std::vector<int64_t> m_edgeIndex ATLAS_THREAD_SAFE{};
  };
}

#endif