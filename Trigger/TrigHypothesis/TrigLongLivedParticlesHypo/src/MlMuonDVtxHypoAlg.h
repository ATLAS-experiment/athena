/*
Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MLMUONDVTXHYPOALG_H
#define MLMUONDVTXHYPOALG_H

// Framework includes
#include "DecisionHandling/HypoBase.h"
#include "MlMuonDVtxHypoTool.h"

/** @brief Hypo for Muon Machine Learning-based
 *         displaced vertex event selection. The score
 *         assigned from ML model to the event is 
 *         attached to the eventInfo. The selection
 *         is performed simply as a cut on the score. */
class MlMuonDVtxHypoAlg : public ::HypoBase  {
  public:
    using HypoBase::HypoBase;

    virtual ~MlMuonDVtxHypoAlg() = default;
    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& context) const override;

private:

  ToolHandleArray< MlMuonDVtxHypoTool > m_hypoTools {this, "HypoTools", {},"Tools that perform actual selection"};
  SG::ReadHandleKey<xAOD::TrigCompositeContainer> m_DVOutputKey{
      this, "DVOutputKey", "EF_MuonDVInferenceOutput", "Dummy xAOD::TrigComposite object decorated with DV inference outputs"};

};
#endif
