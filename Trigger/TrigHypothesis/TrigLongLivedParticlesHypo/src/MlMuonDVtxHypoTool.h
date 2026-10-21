/*
Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MLMUONDVTXHYPOTOOL_H
#define MLMUONDVTXHYPOTOOL_H

#include "Gaudi/Property.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "TrigCompositeUtils/HLTIdentifier.h"

/** @brief Hypothesis tool for Muon Machine Learning-based
 *         displaced vertex event selection. It simply
 *         applies a cut on the ML score. */

class MlMuonDVtxHypoTool : virtual public ::AthAlgTool {
  public:

    MlMuonDVtxHypoTool(const std::string& type,
                       const std::string& name,
                       const IInterface* parent);

    virtual ~MlMuonDVtxHypoTool() = default;

    struct Info {
        TrigCompositeUtils::Decision* outDecision {nullptr};
        float score {-1.f};
        TrigCompositeUtils::DecisionIDContainer activeChainIDs {};
    };
    StatusCode decide(Info& info ) const;

  private:

    HLT::Identifier m_decisionId;
    Gaudi::Property<float> m_scoreCut{this, "score_cut",{0.5}, "Minimum score for an event to be accepted"};
  };

  #endif
