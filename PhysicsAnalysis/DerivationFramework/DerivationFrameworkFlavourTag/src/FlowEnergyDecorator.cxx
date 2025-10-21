/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "FlowEnergyDecorator.h"
#include "xAODPFlow/FlowElement.h"
#include "StoreGate/WriteDecorHandle.h"
#include "StoreGate/ReadHandle.h"
#include "xAODPFlow/FlowElementContainer.h"
#include <utility>

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

  
  m_em.reserve(m_layerEnergiesEM.size());
  m_had.reserve(m_layerEnergiesHAD.size());

  // Initialize the layer energy accessors and decorators keys for both EM and HAD layers
  for (const auto& layer : m_layerEnergiesEM.value()) { 
    const std::string accessorName = "LAYERENERGY_" + layer; 
    const std::string fullAccessorKey = m_PFlowContainerKey.key() + "." + accessorName;
    const std::string decorName = "e" + layer; 
    const std::string fullDecorKey = m_FlowContainerKey.key() + "." + decorName; 
    
    m_em.emplace_back(
      std::piecewise_construct,
      std::forward_as_tuple(this, accessorName, fullAccessorKey, "Layer energy accessor"),
      std::forward_as_tuple(this, decorName, fullDecorKey, "Layer energy decoration")
    );
    ATH_CHECK(m_em.back().first.initialize());
    ATH_CHECK(m_em.back().second.initialize());
  } 

  for (const auto& layer : m_layerEnergiesHAD.value()) { 
    const std::string accessorName = "LAYERENERGY_" + layer; 
    const std::string fullAccessorKey = m_PFlowContainerKey.key() + "." + accessorName;
    const std::string decorName = "e" + layer; 
    const std::string fullDecorKey = m_FlowContainerKey.key() + "." + decorName; 
    
    m_had.emplace_back(
      std::piecewise_construct,
      std::forward_as_tuple(this, accessorName, fullAccessorKey, "Layer energy accessor"),
      std::forward_as_tuple(this, decorName, fullDecorKey, "Layer energy decoration")
    );
    ATH_CHECK(m_had.back().first.initialize());
    ATH_CHECK(m_had.back().second.initialize());

  }
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
  std::vector<SG::ReadDecorHandle<xAOD::FlowElementContainer, float> > layerAccessors_em;
  std::vector<SG::WriteDecorHandle<xAOD::FlowElementContainer, float> > layerDecors_em;
  
  layerDecors_em.reserve(m_em.size());
  layerAccessors_em.reserve(m_em.size());
  for (const auto& [accessor, decorKey]: m_em) {
    layerAccessors_em.emplace_back(accessor, ctx);
    layerDecors_em.emplace_back(decorKey, ctx);
  }
 
  // Declare the ReadDecorHandles and WriteDecorHandles for each HAD layer energy accessor and decorator
  std::vector<SG::ReadDecorHandle<xAOD::FlowElementContainer, float> > layerAccessors_had;
  std::vector<SG::WriteDecorHandle<xAOD::FlowElementContainer, float> > layerDecors_had;
  
  layerDecors_had.reserve(m_had.size());
  layerAccessors_had.reserve(m_had.size());
  for (const auto& [accessor, decorKey]: m_had) {
    layerAccessors_had.emplace_back(accessor, ctx);
    layerDecors_had.emplace_back(decorKey, ctx);
  }


  // loop over all UFOs from UFOCSSK
  for ( const xAOD::FlowElement* flow : *ufoCont ) {  
    float eEM = 0.;
    float eHAD = 0.;

    // loop over all EM layers in the calorimeter
    for (size_t iem = 0; iem < m_em.size(); ++iem) {
      
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
        if (c->charge() != 0) continue; // if the constituent is a track, continue
        e += layerAccessor(*c); // else the constituent is a cluster, so add the energy of every cluster for a given layer for the considered UFO
      }
      layerDecor(*flow) = e; // decorate the UFO with the energy associated to a given EM layer (once the energies from every cluster in this layer have been summed up)
      eEM += e;
    }

    // loop over all HAD layers in the calorimeter
    for (size_t ihad = 0; ihad < m_had.size(); ++ihad) {
      
      // declare references to the accessor and decorator for a given HAD layer
      const auto& layerAccessor = layerAccessors_had.at(ihad);
      auto& layerDecor = layerDecors_had.at(ihad);

      float e = 0.;

      // get the links to the pflow constituents of the UFO
      const std::vector<ElementLink<xAOD::IParticleContainer>>& pflowLinks = flow->otherObjectLinks();
            
        // loop over all available pflows of a given UFO
      for ( auto& el : pflowLinks ) {  
        if ( !el.isValid() ) {throw std::runtime_error("Invalid ElementLink found.");};
        const xAOD::FlowElement* c = dynamic_cast<const xAOD::FlowElement*>(*el);
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
    eFracEMDecorHandle(*flow) = (eTOT > 0) ? eEM / eTOT : 0.;
    eFracHADDecorHandle(*flow) = (eTOT > 0) ? eHAD / eTOT : 0.;
  }

  return StatusCode::SUCCESS;
}
