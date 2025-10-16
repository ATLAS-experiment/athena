/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/*
 * @file DerivationFrameworkCalo/src/CaloCellDecorator.cxx
 * @author Gabriel P. Matos <gpinheir@cern.ch>, adapted from MaxCellDecorator by Nikiforos K. Nikiforou and others.
 * @date Aug, 2025
 * @brief Adds cell-level features as decorations to e/gamma objects.
 */

/////////////////////////////////////////////////////////////////
// CaloCellDecorator.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#include "DerivationFrameworkCalo/CaloCellDecorator.h"
#include "CaloIdentifier/CaloCell_ID.h"
#include "CaloUtils/CaloClusterStoreHelper.h"

#include <string>
#include <vector>


// Destructor
DerivationFramework::CaloCellDecorator::~CaloCellDecorator() = default;

// Athena initialize
StatusCode
DerivationFramework::CaloCellDecorator::initialize()
{
  ATH_MSG_VERBOSE("initialize() ...");

  ATH_CHECK(m_cablingKey.initialize());

  // Setup for photons
  if (!m_SGKey_photons.key().empty()) {
    const std::string key = m_SGKey_photons.key();
    ATH_MSG_INFO("Using " << key << " for photons");
    ATH_CHECK(m_SGKey_photons.initialize());

    m_SGKey_photons_decorations.emplace_back(key + ".cells_E");
    m_SGKey_photons_decorations.emplace_back(key + ".cells_time");
    m_SGKey_photons_decorations.emplace_back(key + ".cells_eta");
    m_SGKey_photons_decorations.emplace_back(key + ".cells_phi");
    m_SGKey_photons_decorations.emplace_back(key + ".cells_x");
    m_SGKey_photons_decorations.emplace_back(key + ".cells_y");
    m_SGKey_photons_decorations.emplace_back(key + ".cells_z");
    m_SGKey_photons_decorations.emplace_back(key + ".cells_gain");
    m_SGKey_photons_decorations.emplace_back(key + ".cells_layer");
    m_SGKey_photons_decorations.emplace_back(key + ".cells_quality");
    m_SGKey_photons_decorations.emplace_back(key + ".cells_onlId");
    m_SGKey_photons_decorations.emplace_back(key + ".ncells");

    ATH_CHECK(m_SGKey_photons_decorations.initialize());
  }

  // Setup for electrons
  if (!m_SGKey_electrons.key().empty()) {
    const std::string key = m_SGKey_electrons.key();
    ATH_MSG_INFO("Using " << key << " for electrons");
    ATH_CHECK(m_SGKey_electrons.initialize());

    m_SGKey_electrons_decorations.emplace_back(key + ".cells_E");
    m_SGKey_electrons_decorations.emplace_back(key + ".cells_time");
    m_SGKey_electrons_decorations.emplace_back(key + ".cells_eta");
    m_SGKey_electrons_decorations.emplace_back(key + ".cells_phi");
    m_SGKey_electrons_decorations.emplace_back(key + ".cells_x");
    m_SGKey_electrons_decorations.emplace_back(key + ".cells_y");
    m_SGKey_electrons_decorations.emplace_back(key + ".cells_z");
    m_SGKey_electrons_decorations.emplace_back(key + ".cells_gain");
    m_SGKey_electrons_decorations.emplace_back(key + ".cells_layer");
    m_SGKey_electrons_decorations.emplace_back(key + ".cells_quality");
    m_SGKey_electrons_decorations.emplace_back(key + ".cells_onlId");
    m_SGKey_electrons_decorations.emplace_back(key + ".ncells");

    ATH_CHECK(m_SGKey_electrons_decorations.initialize());

  }

  return StatusCode::SUCCESS;

}

StatusCode
DerivationFramework::CaloCellDecorator::addBranches(const EventContext& ctx) const
{

  if (!m_SGKey_photons.key().empty()) {

    // Decorate photons
    ATH_CHECK(
      DerivationFramework::CaloCellDecorator::decorateCells(
        m_SGKey_photons,
        m_SGKey_photons_decorations,
        ctx
      )
    );

  }
    
  if (!m_SGKey_electrons.key().empty()) {

    // Decorate electrons
    ATH_CHECK(
      DerivationFramework::CaloCellDecorator::decorateCells(
        m_SGKey_electrons,
        m_SGKey_electrons_decorations,
        ctx
      )
    );

  }

  return StatusCode::SUCCESS;
}

StatusCode 
DerivationFramework::CaloCellDecorator::decorateCells(
  const SG::ReadHandleKey<xAOD::EgammaContainer>& contKey,
  const SG::WriteDecorHandleKeyArray<xAOD::EgammaContainer>& decorKeys,
  const EventContext& ctx) const
{

  // Retrieve container
  SG::ReadHandle<xAOD::EgammaContainer> egammaContainer(contKey, ctx);

  // Setup decorators
  SG::WriteDecorHandle<xAOD::EgammaContainer, std::vector<float>> 
    decoration0(decorKeys[0], ctx);
  SG::WriteDecorHandle<xAOD::EgammaContainer, std::vector<float>> 
    decoration1(decorKeys[1], ctx);
  SG::WriteDecorHandle<xAOD::EgammaContainer, std::vector<float>> 
    decoration2(decorKeys[2], ctx);
  SG::WriteDecorHandle<xAOD::EgammaContainer, std::vector<float>> 
    decoration3(decorKeys[3], ctx);
  SG::WriteDecorHandle<xAOD::EgammaContainer, std::vector<float>> 
    decoration4(decorKeys[4], ctx);
  SG::WriteDecorHandle<xAOD::EgammaContainer, std::vector<float>> 
    decoration5(decorKeys[5], ctx);
  SG::WriteDecorHandle<xAOD::EgammaContainer, std::vector<float>> 
    decoration6(decorKeys[6], ctx);
  SG::WriteDecorHandle<xAOD::EgammaContainer, std::vector<int>> 
    decoration7(decorKeys[7], ctx);
  SG::WriteDecorHandle<xAOD::EgammaContainer, std::vector<int>> 
    decoration8(decorKeys[8], ctx);
  SG::WriteDecorHandle<xAOD::EgammaContainer, std::vector<int>> 
    decoration9(decorKeys[9], ctx);
  SG::WriteDecorHandle<xAOD::EgammaContainer, std::vector<uint64_t>> 
    decoration10(decorKeys[10], ctx);
  SG::WriteDecorHandle<xAOD::EgammaContainer, int> 
    decoration11(decorKeys[11], ctx);

  // Loop through egamma objects and decorate
  const xAOD::EgammaContainer* importedEgamma = egammaContainer.ptr();
  for (const auto* egamma : *importedEgamma) {
    const xAOD::CaloCluster *cluster = egamma->caloCluster();
    DerivationFramework::CaloCellDecorator::cell_decorations res =
    getDecorations(cluster, ctx);

    // Decorate
    decoration0(*egamma) = res.cells_E;
    decoration1(*egamma) = res.cells_time;
    decoration2(*egamma) = res.cells_eta;
    decoration3(*egamma) = res.cells_phi;
    decoration4(*egamma) = res.cells_x;
    decoration5(*egamma) = res.cells_y;
    decoration6(*egamma) = res.cells_z;
    decoration7(*egamma) = res.cells_gain;
    decoration8(*egamma) = res.cells_layer;
    decoration9(*egamma) = res.cells_quality;
    decoration10(*egamma) = res.cells_onlId;
    decoration11(*egamma) = res.ncells;
  }

  return StatusCode::SUCCESS;

};

DerivationFramework::CaloCellDecorator::cell_decorations
DerivationFramework::CaloCellDecorator::getDecorations(
  const xAOD::CaloCluster* cluster,
  const EventContext& ctx) const
{

  DerivationFramework::CaloCellDecorator::cell_decorations decorations;

  if (cluster) {
    if (!cluster->getCellLinks()) {
      ATH_MSG_WARNING("CellLinks not found");
      return decorations;
    }

    const LArOnOffIdMapping* cabling{nullptr};
    if (!SG::get(cabling, m_cablingKey, ctx).isSuccess()){
      ATH_MSG_ERROR("Do not have mapping object " << m_cablingKey.key());
      throw std::runtime_error("Cabling retrieval failed");    
    }

    for (const CaloCell* cell : *cluster) {

      int sampling = cell->caloDDE()->getSampling();

      // Keep track of total number of cells
      decorations.ncells++;

      // Fill with layer information
      if (sampling == CaloCell_ID::PreSamplerB || sampling == CaloCell_ID::PreSamplerE){
        decorations.cells_layer.push_back(0);
      }

      else if (sampling == CaloCell_ID::EMB1 || sampling == CaloCell_ID::EME1){
        decorations.cells_layer.push_back(1);
      }

      else if (sampling == CaloCell_ID::EMB2 || sampling == CaloCell_ID::EME2){
        decorations.cells_layer.push_back(2);
      }

      else if (sampling == CaloCell_ID::EMB3 || sampling == CaloCell_ID::EME3){
        decorations.cells_layer.push_back(3);
      }

      else {
        // Don't do anything with Tile cells aside from tabulating
        continue;
      }

      // Fill vectors with cell features
      decorations.cells_E.push_back(cell->e());
      decorations.cells_time.push_back(cell->time());
      decorations.cells_eta.push_back(cell->eta());
      decorations.cells_phi.push_back(cell->phi());
      decorations.cells_x.push_back(cell->x());
      decorations.cells_y.push_back(cell->y());
      decorations.cells_z.push_back(cell->z());
      decorations.cells_gain.push_back((int)cell->gain());
      decorations.cells_quality.push_back(cell->quality());

      // Keep online ID to debug
      decorations.cells_onlId.push_back(
          (uint64_t)(cabling->createSignalChannelID(cell->caloDDE()->identify()))
          .get_compact()
      );
    }
  }

  return decorations;
}
