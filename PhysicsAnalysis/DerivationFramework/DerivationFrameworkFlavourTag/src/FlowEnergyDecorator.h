/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FLOW_ENERGY_DECORATOR_HH
#define FLOW_ENERGY_DECORATOR_HH

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "xAODPFlow/FlowElementContainer.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadDecorHandleKey.h"
#include "StoreGate/ReadDecorHandleKeyArray.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "StoreGate/WriteDecorHandleKeyArray.h"

#include <string>
#include <vector>
#include <utility> //for std::pair


#include "AthContainers/AuxElement.h"  // For SG::AuxElement::Accessor

class FlowEnergyDecorator : public AthReentrantAlgorithm {
 public:
  FlowEnergyDecorator(const std::string& name, ISvcLocator* pSvcLocator);

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext&) const override;

 private:

  // Input UFO container on which the decorations are done
  SG::ReadHandleKey<xAOD::FlowElementContainer> m_FlowContainerKey {
    this, "UFOContainer", "UFOCSSK", "Key for the input flow collection"
  };

  // Input PFlow container from which the layer energies are accessed
  SG::ReadHandleKey<xAOD::FlowElementContainer> m_PFlowContainerKey {
    this, "PFlowContainer", "GlobalNeutralParticleFlowObjects", "Key for the input pflow collection"
  };

  // // Electromagnetic layer names to sum for each type
  // Gaudi::Property<std::vector<std::string>> m_layerEnergiesEM {
  //   this, "layerEnergiesEM",
  //   { "PreSamplerB", "PreSamplerE", "EMB1", "EMB2", "EMB3", "EME1", "EME2", "EME3", "FCAL0" },
  //   "Electromagnetic layerEnergies used for decorations"
  // };

  // // Hadronic layer names to sum for each type
  // Gaudi::Property<std::vector<std::string>> m_layerEnergiesHAD {
  //   this, "layerEnergiesHAD",
  //   { "TileBar0", "TileBar1", "TileBar2", "TileExt0", "TileExt1", "TileExt2",
  //     "TileGap1", "TileGap2", "TileGap3", "FCAL1", "FCAL2", "HEC1", "HEC2", "HEC3" },
  //   "Hadronic layerEnergies used for decorations"
  // };

  // Decorations for total EM and HAD energies and fractions
  SG::WriteDecorHandleKey<xAOD::FlowElementContainer> m_eEMKey {
    this, "eEMDecorKey", m_FlowContainerKey, "eEM", "EM energy decoration"
  };

  SG::WriteDecorHandleKey<xAOD::FlowElementContainer> m_eHADKey {
    this, "eHADDecorKey", m_FlowContainerKey, "eHAD", "Hadronic energy decoration"
  };

  SG::WriteDecorHandleKey<xAOD::FlowElementContainer> m_eFracEMKey {
    this, "eFracEMDecorKey", m_FlowContainerKey, "eFracEM", "EM energy fraction decoration"
  };

  SG::WriteDecorHandleKey<xAOD::FlowElementContainer> m_eFracHADKey {
    this, "eFracHADDecorKey", m_FlowContainerKey, "eFracHAD", "Hadronic energy fraction decoration"
  };

  SG::ReadDecorHandleKeyArray<xAOD::FlowElementContainer> m_emReadDecorKeys{
    this, "LayerEnergyAccessorsEM", m_PFlowContainerKey, {}, "Layer EM  energy accessors"
  };

  SG::WriteDecorHandleKeyArray<xAOD::FlowElementContainer> m_emWriteDecorKeys{
    this, "LayerEnergyDecoratorsEM", m_FlowContainerKey, {}, "Layer EM energy decoration"
  };

  SG::ReadDecorHandleKeyArray<xAOD::FlowElementContainer> m_hadReadDecorKeys{
    this, "LayerEnergyAccessorsHAD", m_PFlowContainerKey, {}, "Layer Hadronic energy accessors"
  };

  SG::WriteDecorHandleKeyArray<xAOD::FlowElementContainer> m_hadWriteDecorKeys{
    this, "LayerEnergyDecoratorsHAD", m_FlowContainerKey, {}, "Layer Hadronic energy decoration"
  };
};

#endif
