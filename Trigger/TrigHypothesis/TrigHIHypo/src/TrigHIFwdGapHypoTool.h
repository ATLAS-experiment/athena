/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGHIHYPO_TRIGHIFWDGAPHYPOTOOL_H
#define TRIGHIHYPO_TRIGHIFWDGAPHYPOTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "TrigCompositeUtils/TrigCompositeUtils.h"
#include "TrigCompositeUtils/HLTIdentifier.h"

#include "ITrigHIEventShapeHypoTool.h"
#include "xAODHIEvent/HIEventShapeContainer.h"

class TrigHIFwdGapHypoTool : public extends<AthAlgTool, ITrigHIEventShapeHypoTool> {

  public:

    TrigHIFwdGapHypoTool(const std::string& type,
                         const std::string& name,
                         const IInterface* parent);
    virtual ~TrigHIFwdGapHypoTool() {};
    virtual StatusCode initialize() override;
    virtual StatusCode finalize() override;

    virtual StatusCode decide(const xAOD::HIEventShapeContainer* eventShapeContainer, bool& pass) const override;
    virtual const HLT::Identifier& getId() const override;

  private:

    // Identifier is used to keep track of which tool made which decision.
    // The information is stored in the event store.
    HLT::Identifier m_decisionId;

    Gaudi::Property<float> m_maxFCalEt{this, "maxFCalEt", 10., "upper limit on FCal ET in GeV"};
    Gaudi::Property<bool> m_useDoubleSidedGap{this, "useDoubleSidedGap", false, "use double- or one-sided gap calculation"};
    Gaudi::Property<bool> m_useSideA{this, "useSideA", true, "use side A for one-sided gap calculation"};

};

#endif

