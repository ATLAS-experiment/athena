/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/


// TrigT1 common definitions
#include "TrigT1Interfaces/TrigT1CaloDefs.h"

// Local include(s):
#include "TrigT1CaloCalibTools/L1CaloTriggerTowerDecoratorAlg.h"

// EDM include(s):
#include "xAODTrigL1Calo/TriggerTowerContainer.h"

#include <optional>
namespace LVL1 {

L1CaloTriggerTowerDecoratorAlg::L1CaloTriggerTowerDecoratorAlg(
    const std::string& name, ISvcLocator* svcLoc)
    : AthAlgorithm(name, svcLoc),
      m_ttTools(this)
{
  declareProperty("TriggerTowerTools", m_ttTools);
}

StatusCode L1CaloTriggerTowerDecoratorAlg::initialize() {
  ATH_MSG_INFO(
      "TrigT1CaloCalibTools/L1CaloTriggerTowerDecoratorAlg::initialize()");
  CHECK(m_ttTools.retrieve());
  CHECK( m_triggerTowerContainerKey.initialize(SG::AllowEmpty) );

  const std::string baseName = m_triggerTowerContainerKey.key();

  if (!baseName.empty()) {
    if (!m_caloCellEnergyKey.empty()) {
      CHECK(m_caloCellEnergyKey.initialize());
    }
    if (!m_caloCellETKey.empty()) {
      CHECK(m_caloCellETKey.initialize());
    }
    if (!m_caloCellsQualityKey.empty()) {
      CHECK(m_caloCellsQualityKey.initialize());
    }
    if (!m_caloCellEnergyByLayerKey.empty()) {
      CHECK(m_caloCellEnergyByLayerKey.initialize());
    }
    if (!m_caloCellETByLayerKey.empty()) {
      CHECK(m_caloCellETByLayerKey.initialize());
    }
    if (!m_caloCellEnergyByLayerByReceiverKey.empty()) {
      CHECK(m_caloCellEnergyByLayerByReceiverKey.initialize());
    }
    if (!m_caloCellETByLayerByReceiverKey.empty()) {
      CHECK(m_caloCellETByLayerByReceiverKey.initialize());
    }
  }

  // Return gracefully:
  return StatusCode::SUCCESS;
}

StatusCode
L1CaloTriggerTowerDecoratorAlg::execute()
{
  // use decorators to avoid the costly name -> auxid lookup

  // Shall I proceed?
  if (!m_triggerTowerContainerKey.empty()) {
     const EventContext& ctx = Gaudi::Hive::currentContext();
     SG::ReadHandle<xAOD::TriggerTowerContainer> tts(m_triggerTowerContainerKey,
                                                    ctx);
     CHECK(m_ttTools->initCaloCells());

     // We have optional Write Decor handles outside the loop
     // And bools to check outside/inside the loop.
     const bool doCellEnergy = !m_caloCellEnergyKey.empty();
     const bool doCellET = !m_caloCellETKey.empty();
     const bool doCellsQuality = !m_caloCellsQualityKey.empty();
     const bool doCellEnergyByLayer = !m_caloCellEnergyByLayerKey.empty();
     const bool doCellETByLayer = !m_caloCellETByLayerKey.empty();
     const bool doCellEnergyByLayerByReceiver = !m_caloCellEnergyByLayerByReceiverKey.empty();
     const bool doCellETByLayerByReceiver = !m_caloCellETByLayerByReceiverKey.empty();
     std::optional<SG::WriteDecorHandle<xAOD::TriggerTowerContainer, float>>
       caloCellEnergyDecorator;
     std::optional<SG::WriteDecorHandle<xAOD::TriggerTowerContainer, float>>
       caloCellETDecorator;
     std::optional<SG::WriteDecorHandle<xAOD::TriggerTowerContainer, float>>
       caloCellsQualityDecorator;
     std::optional<
       SG::WriteDecorHandle<xAOD::TriggerTowerContainer, std::vector<float>>>
       caloCellEnergyByLayerDecorator;
     std::optional<
       SG::WriteDecorHandle<xAOD::TriggerTowerContainer, std::vector<float>>>
       caloCellETByLayerDecorator;
     std::optional<
       SG::WriteDecorHandle<xAOD::TriggerTowerContainer, std::vector<std::vector<float>>>>
       caloCellEnergyByLayerByReceiverDecorator;
     std::optional<
       SG::WriteDecorHandle<xAOD::TriggerTowerContainer, std::vector<std::vector<float>>>>
       caloCellETByLayerByReceiverDecorator;

     if (doCellEnergy) {
       caloCellEnergyDecorator.emplace(m_caloCellEnergyKey, ctx);
     }
     if (doCellET) {
       caloCellETDecorator.emplace(m_caloCellETKey, ctx);
     }
     if (doCellsQuality) {
       caloCellsQualityDecorator.emplace(m_caloCellsQualityKey, ctx);
     }
     if (doCellEnergyByLayer) {
       caloCellEnergyByLayerDecorator.emplace(m_caloCellEnergyByLayerKey, ctx);
     }
     if (doCellETByLayer) {
       caloCellETByLayerDecorator.emplace(m_caloCellETByLayerKey, ctx);
     }
     if (doCellEnergyByLayerByReceiver) {
       caloCellEnergyByLayerByReceiverDecorator.emplace(m_caloCellEnergyByLayerByReceiverKey, ctx);
     }
     if (doCellETByLayerByReceiver) {
       caloCellETByLayerByReceiverDecorator.emplace(m_caloCellETByLayerByReceiverKey, ctx);
     }
     //Loop filling the decorations
     for (const auto* x : *tts) {
       if (doCellEnergy) {
         caloCellEnergyDecorator.value()(*x) = m_ttTools->caloCellsEnergy(*x);
       }
       if (doCellET) {
         caloCellETDecorator.value()(*x) = m_ttTools->caloCellsET(*x);
       }
       if (doCellEnergyByLayer) {
         caloCellEnergyByLayerDecorator.value()(*x) = m_ttTools->caloCellsEnergyByLayer(*x);
       }
       if (doCellETByLayer) {
         caloCellETByLayerDecorator.value()(*x) = m_ttTools->caloCellsETByLayer(*x);
       }
       if (doCellsQuality) {
         caloCellsQualityDecorator.value()(*x) = m_ttTools->caloCellsQuality(*x);
       }
       if (doCellEnergyByLayerByReceiver) {
         caloCellEnergyByLayerByReceiverDecorator.value()(*x) = m_ttTools->caloCellsEnergyByLayerByReceiver(*x);
       }
       if (doCellETByLayerByReceiver) {
         caloCellETByLayerByReceiverDecorator.value()(*x) = m_ttTools->caloCellsETByLayerByReceiver(*x);
       }
     } //end of decoration loop
  } // Trigger towers present in Storegate

  // Return gracefully:
  return StatusCode::SUCCESS;
}
}
