/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "FlavorTagDiscriminants/CaloChargedFlowDecoratorAlg.h"
#include "xAODPFlow/FlowElement.h"
#include "xAODCaloEvent/CaloCluster.h"
#include "StoreGate/WriteDecorHandle.h"


namespace FlavorTagDiscriminants {

  CaloChargedFlowDecoratorAlg::CaloChargedFlowDecoratorAlg(
    const std::string& name, ISvcLocator* loc)
    : AthReentrantAlgorithm(name, loc)
  {
  }

  StatusCode CaloChargedFlowDecoratorAlg::initialize() {
    ATH_CHECK(m_caloClusterCollection.initialize());
    ATH_CHECK(m_neutralPFOCollection.initialize());
    ATH_CHECK(m_chargedPFOCollection.initialize());
    ATH_CHECK(m_objUsedInChargedDecorator.initialize());
    return StatusCode::SUCCESS;
  }

  StatusCode CaloChargedFlowDecoratorAlg::execute(
    const EventContext& ctx) const
  {
    SG::ReadHandle<IPC> constituents(m_caloClusterCollection, ctx);
    SG::ReadHandle<IPC> neutralPFOs(m_neutralPFOCollection, ctx);
    SG::ReadHandle<IPC> chargedPFOs(m_chargedPFOCollection, ctx);
    SG::WriteDecorHandle<IPC, int> usedInChargedPFO(
      m_objUsedInChargedDecorator, ctx);

    // Default all clusters to -1 (not linked to any PFO)
    for (const auto* obj : *constituents) {
      usedInChargedPFO(*obj) = -1;
    }

    // Mark clusters linked to neutral PFOs as 0
    for (const auto* obj : *neutralPFOs) {
      const auto* flow = dynamic_cast<const xAOD::FlowElement*>(obj);
      if (!flow) continue;
      for (const auto* otherObject : flow->otherObjects()) {
        if (!otherObject) {
          ATH_MSG_WARNING("Invalid otherObject link, skipping");
          continue;
        }
        usedInChargedPFO(*otherObject) = 0;
      }
    }

    // Mark clusters linked to charged PFOs as 1 (overrides neutral)
    for (const auto* obj : *chargedPFOs) {
      const auto* flow = dynamic_cast<const xAOD::FlowElement*>(obj);
      if (!flow) continue;
      for (const auto* otherObject : flow->otherObjects()) {
        if (!otherObject) {
          ATH_MSG_WARNING("Invalid otherObject link, skipping");
          continue;
        }
        usedInChargedPFO(*otherObject) = 1;
      }
    }

    return StatusCode::SUCCESS;
  }

}
