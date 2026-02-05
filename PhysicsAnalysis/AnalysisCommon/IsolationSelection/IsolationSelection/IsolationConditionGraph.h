/*
 Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */

#ifndef ISOLATIONSELECTION_ISOLATIONCONDITIONGRAPH_H
#define ISOLATIONSELECTION_ISOLATIONCONDITIONGRAPH_H

#include <TF1.h>
#include <TGraph.h>
#include <TH1F.h>

#include "IsolationSelection/IsolationCondition.h"

namespace CP {
class IsolationConditionGraph : public IsolationCondition {
 public:
  IsolationConditionGraph(const std::string& name,
                          const std::vector<std::string>& isoType,
                          std::unique_ptr<TF1> isoFunction,
			  std::vector<std::unique_ptr<TGraph>>&& cutGraphs,
                          std::unique_ptr<TH1F> binning,
                          const std::string& isoDecSuffix = "",
                          bool invertCut = false);
  virtual ~IsolationConditionGraph() override = default;

  bool accept(const xAOD::IParticle& x) const override;
  bool accept(const strObj& x) const override;

 private:
  std::vector<std::unique_ptr<TGraph>> m_cutGraphs;
  std::unique_ptr<TF1> m_isoFunction;
  std::unique_ptr<TH1F> m_binning;
  bool m_invertCut{false};

  float getCutValue(const float pt, const float eta) const;
};
}  // namespace CP
#endif
