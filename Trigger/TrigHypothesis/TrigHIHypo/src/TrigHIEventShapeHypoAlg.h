/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGHIHYPO_TRIGHIEVENTSHAPEHYPOALG_H
#define TRIGHIHYPO_TRIGHIEVENTSHAPEHYPOALG_H

#include <string>

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "DecisionHandling/HypoBase.h"

#include "ITrigHIEventShapeHypoTool.h"
#include "xAODHIEvent/HIEventShapeContainer.h"

/**
 * @class TrigHIEventShapeHypoAlg
 * @brief Implements FCal-based forward gap selection for the HLT framework
 **/
class TrigHIEventShapeHypoAlg : public ::HypoBase {

  public:

    TrigHIEventShapeHypoAlg(const std::string& name, ISvcLocator* pSvcLocator);

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& context) const override;

  private:

    StatusCode decide(const xAOD::HIEventShapeContainer* eventShapeContainer,
                      TrigCompositeUtils::DecisionContainer* newDecisions,
                      const TrigCompositeUtils::DecisionContainer* oldDecisions,
                      const EventContext& context) const;

    ToolHandleArray<ITrigHIEventShapeHypoTool> m_hypoTools{this, "HypoTools", {}, "Hypo tools"};

    SG::ReadHandleKey<xAOD::HIEventShapeContainer> m_esKey{this, "eventShapeContainerKey", "HLT_HIEventShapeEG", "event shape container name"};

};

#endif

