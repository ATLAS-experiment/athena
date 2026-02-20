/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "FlowEnergyDecorator.h"
#include "xAODPFlow/FlowElement.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/ReadDecorHandle.h"
#include "StoreGate/WriteDecorHandle.h"
#include "xAODPFlow/FlowElementContainer.h"
#include "CxxUtils/close_to_zero.h"

using CxxUtils::close_to_zero;

FlowEnergyDecorator::FlowEnergyDecorator(const std::string& name, ISvcLocator* loc)
  : AthReentrantAlgorithm(name, loc)
{}

StatusCode FlowEnergyDecorator::initialize() {
  ATH_MSG_INFO("Initializing " << name() << "... ");

  ATH_CHECK(m_FlowContainerKey.initialize());
  ATH_CHECK(m_PFlowContainerKey.initialize());

  // Initialize the decorations keys for total EM and HAD energies and fractions
  ATH_CHECK(m_eEMKey.initialize());
  ATH_CHECK(m_eHADKey.initialize());
  ATH_CHECK(m_eFracEMKey.initialize());
  ATH_CHECK(m_eFracHADKey.initialize());

  // Initialize the layer energy accessors and decorators keys for both EM and HAD layers
  ATH_CHECK(m_emReadDecorKeys.initialize());
  ATH_CHECK(m_emWriteDecorKeys.initialize());
  ATH_CHECK(m_hadReadDecorKeys.initialize());
  ATH_CHECK(m_hadWriteDecorKeys.initialize());

  return StatusCode::SUCCESS;
}

StatusCode FlowEnergyDecorator::execute(const EventContext& ctx) const {
  ATH_MSG_DEBUG("Executing " << name() << "... ");


  SG::ReadHandle<xAOD::FlowElementContainer> pflowCont(m_PFlowContainerKey, ctx);
  if ( !pflowCont.isValid() ) {
    ATH_MSG_ERROR("Failed to retrieve " << m_PFlowContainerKey.key() << " !");
    return StatusCode::FAILURE;
  }

  SG::ReadHandle<xAOD::FlowElementContainer> ufoCont(m_FlowContainerKey, ctx);
  if ( !ufoCont.isValid() ) {
    ATH_MSG_ERROR("Failed to retrieve " << m_FlowContainerKey.key() << " !");
    return StatusCode::FAILURE;
  }

  // Declare the WriteDecorHandles for the total EM and HAD energies and fractions (no accessors needed here)
  SG::WriteDecorHandle<xAOD::FlowElementContainer, float> eEMDecorHandle(m_eEMKey, ctx);
  SG::WriteDecorHandle<xAOD::FlowElementContainer, float> eHADDecorHandle(m_eHADKey, ctx);
  SG::WriteDecorHandle<xAOD::FlowElementContainer, float> eFracEMDecorHandle(m_eFracEMKey, ctx);
  SG::WriteDecorHandle<xAOD::FlowElementContainer, float> eFracHADDecorHandle(m_eFracHADKey, ctx);

  // Declare the ReadDecorHandles and WriteDecorHandles for each EM layer energy accessor and decorator
  std::vector< SG::ReadDecorHandle<xAOD::FlowElementContainer, float> > layerAccessors_em;
  layerAccessors_em.reserve(m_emReadDecorKeys.size());
  for (const SG::ReadDecorHandleKey<xAOD::FlowElementContainer>& key: m_emReadDecorKeys) {
    layerAccessors_em.emplace_back(key, ctx);
  }
  std::vector< SG::WriteDecorHandle<xAOD::FlowElementContainer, float> > layerDecors_em;
  layerDecors_em.reserve(m_emWriteDecorKeys.size());
  for (const SG::WriteDecorHandleKey<xAOD::FlowElementContainer>& key: m_emWriteDecorKeys) {
    layerDecors_em.emplace_back(key, ctx);
  }

  // Declare the ReadDecorHandles and WriteDecorHandles for each HAD layer energy accessor and decorator
  std::vector< SG::ReadDecorHandle<xAOD::FlowElementContainer, float> > layerAccessors_had;
  layerAccessors_had.reserve(m_hadReadDecorKeys.size());
  for (const SG::ReadDecorHandleKey<xAOD::FlowElementContainer>& key: m_hadReadDecorKeys) {
    layerAccessors_had.emplace_back(key, ctx);
  }
  std::vector< SG::WriteDecorHandle<xAOD::FlowElementContainer, float> > layerDecors_had;
  layerDecors_em.reserve(m_hadWriteDecorKeys.size());
  for (const SG::WriteDecorHandleKey<xAOD::FlowElementContainer>& key: m_hadWriteDecorKeys) {
    layerDecors_had.emplace_back(key, ctx);
  }

  // loop over all UFOs from UFOCSSK
  for ( const xAOD::FlowElement* flow : *ufoCont ) {
    float eEM = 0.;
    float eHAD = 0.;

    // loop over all EM layers in the calorimeter
    for (size_t iem = 0; iem < m_emReadDecorKeys.size(); ++iem) {

      // declare references to the accessor and decorator for a given HAD layer
      const auto& layerAccessor = layerAccessors_em.at(iem);
      auto& layerDecor = layerDecors_em.at(iem);

      float e = 0.;

      // get the links to the pflow constituents of the UFO
      const std::vector<ElementLink<xAOD::IParticleContainer>>& pflowLinks = flow->otherObjectLinks();
      // loop over all available pflows of a given UFO
      for ( auto& el : pflowLinks ) {
        if ( !el.isValid() ) {throw std::runtime_error("Invalid ElementLink found.");};
        const xAOD::FlowElement* c = dynamic_cast<const xAOD::FlowElement*>(*el);
        if (not c) continue;
        if (c->charge() != 0) continue; // if the constituent is a track, continue
        e += layerAccessor(*c); // else the constituent is a cluster, so add the energy of every cluster for a given layer for the considered UFO
      }
      layerDecor(*flow) = e; // decorate the UFO with the energy associated to a given EM layer (once the energies from every cluster in this layer have been summed up)
      eEM += e;
    }

    // loop over all HAD layers in the calorimeter
    for (size_t ihad = 0; ihad < m_hadReadDecorKeys.size(); ++ihad) {

      // declare references to the accessor and decorator for a given HAD layer
      const auto& layerAccessor = layerAccessors_had.at(ihad);
      auto& layerDecor = layerDecors_had.at(ihad);

      float e = 0.;

      // get the links to the pflow constituents of the UFO
      const std::vector<ElementLink<xAOD::IParticleContainer>>& pflowLinks = flow->otherObjectLinks();

      // loop over all available pflows of a given UFO
      for ( auto& el : pflowLinks ) {
        if ( !el.isValid() ) {throw std::runtime_error("Invalid ElementLink found.");};
        const xAOD::FlowElement* c = static_cast<const xAOD::FlowElement*>(*el);
        if (c->charge() != 0) continue;
        e += layerAccessor(*c);
      }
      layerDecor(*flow) = e; // decorate the UFO with the energy associated to a given HAD layer (once the energies from every cluster in this layer have been summed up)
      eHAD += e;
    }

    // decorate the UFO with the total electromagnetic or hadronic energy once the loop over the layers is over for a given UFO
    eEMDecorHandle(*flow) = eEM;
    eHADDecorHandle(*flow) = eHAD;

    // decorate the UFO with the electromagnetic or hadronic energy fractions once the loop over the layers is over for a given UFO
    const float eTOT = eEM + eHAD;
    const bool eTotZero = close_to_zero(eTOT);
    eFracEMDecorHandle(*flow) = (!eTotZero) ? (eEM / eTOT) : 0.;
    eFracHADDecorHandle(*flow) = (!eTotZero) ? (eHAD / eTOT) : 0.;
  }

  return StatusCode::SUCCESS;
}
