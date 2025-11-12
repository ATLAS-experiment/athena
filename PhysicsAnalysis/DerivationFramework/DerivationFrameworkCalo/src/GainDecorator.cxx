/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// Author: Simone Mazza (simone.mazza@mi.infn.it),
//         Bruno Lenzi,
//         Giovanni Marchiori (giovanni.marchiori@cern.ch)
// Decorate egamma objects with the energy and number of cells per layer per
// gain

#include "DerivationFrameworkCalo/GainDecorator.h"
#include "CaloEvent/CaloCell.h"



// Athena initialize and finalize
StatusCode
DerivationFramework::GainDecorator::initialize()
{
  ATH_MSG_VERBOSE("initialize() ...");

  if (m_SGKey_photons.key().empty() && m_SGKey_electrons.key().empty()) {
    ATH_MSG_FATAL("No e-gamma collection provided for thinning. At least one "
                  "egamma collection (photons/electrons) must be provided!");
    return StatusCode::FAILURE;
  }

  for (const auto& kv : m_gainNames) {
    for (const auto layer : m_layers) {
      m_names_E.emplace_back(kv.first, layer);
    }
  }

  ATH_CHECK(m_SGKey_electrons.initialize(SG::AllowEmpty));
  if (!m_SGKey_electrons.key().empty()) {
    ATH_MSG_DEBUG("Using " << m_SGKey_electrons << " for electrons");
  }
  ATH_CHECK(m_SGKey_electrons_decorations.initialize(!m_SGKey_electrons.key().empty()));

  ATH_CHECK(m_SGKey_photons.initialize(SG::AllowEmpty));
  if (!m_SGKey_photons.key().empty()) {
    ATH_MSG_DEBUG("Using " << m_SGKey_photons << " for photons");
  }
  ATH_CHECK(m_SGKey_photons_decorations.initialize(!m_SGKey_photons.key().empty()));

  return StatusCode::SUCCESS;
}

// The decoration itself
StatusCode
DerivationFramework::GainDecorator::addBranches(const EventContext& ctx) const
{

  // Photon decorations

  if (!m_SGKey_photons.key().empty()) {

    // Retrieve photon container
    SG::ReadHandle<xAOD::EgammaContainer> photonContainer(m_SGKey_photons, ctx);
    const xAOD::EgammaContainer* importedPhotons = photonContainer.ptr();

    // Setup vectors of photon decorations
    std::vector<SG::WriteDecorHandle<xAOD::EgammaContainer, float>>
      decorations_E;
    std::vector<SG::WriteDecorHandle<xAOD::EgammaContainer, float>>
      decorations_rnoW;
    std::vector<SG::WriteDecorHandle<xAOD::EgammaContainer, char>>
      decorations_nCells;
    int i(0);
    for (const auto& kv : m_gainNames) {
      for (const auto layer : m_layers) {
        std::pair<int, int> key(kv.first, layer);
        decorations_E.emplace_back(

                                   m_SGKey_photons_decorations[i * 3], ctx);
        decorations_rnoW.emplace_back(

                                      m_SGKey_photons_decorations[i * 3 + 1], ctx);
        decorations_nCells.emplace_back(

                                        m_SGKey_photons_decorations[i * 3 + 2], ctx);
        i++;
      }
    }

    // Decorate photons
    for (const auto* photon : *importedPhotons) {
      DerivationFramework::GainDecorator::calculation res =
        decorateObject(photon);
      i = 0;
      for (const auto& kv : m_gainNames) {
        for (const auto layer : m_layers) {
          std::pair<int, int> key(kv.first, layer);
          decorations_E[i](*photon) = res.E[key];
          decorations_rnoW[i](*photon) =
            res.EnoW[key] != 0 ? res.E[key]/res.EnoW[key] : 1;
          decorations_nCells[i](*photon) = res.nCells[key];
          i++;
        }
      }
    }
  }

  // Electron decorations

  if (!m_SGKey_electrons.key().empty()) {

    // Retrieve electron container
    SG::ReadHandle<xAOD::EgammaContainer> electronContainer(m_SGKey_electrons,
                                                            ctx);
    const xAOD::EgammaContainer* importedElectrons = electronContainer.ptr();

    // Setup vectors of electron decorations
    std::vector<SG::WriteDecorHandle<xAOD::EgammaContainer, float>>
      decorations_E;
    std::vector<SG::WriteDecorHandle<xAOD::EgammaContainer, float>>
      decorations_rnoW;
    std::vector<SG::WriteDecorHandle<xAOD::EgammaContainer, char>>
      decorations_nCells;
    int i(0);
    for (const auto& kv : m_gainNames) {
      for (const auto layer : m_layers) {
        std::pair<int, int> key(kv.first, layer);
        decorations_E.emplace_back(

                                   m_SGKey_electrons_decorations[i * 3], ctx);
        decorations_rnoW.emplace_back(

                                      m_SGKey_electrons_decorations[i * 3 + 1], ctx);
        decorations_nCells.emplace_back(

                                        m_SGKey_electrons_decorations[i * 3 + 2], ctx);
        i++;
      }
    }

    // Decorate electrons
    for (const auto* electron : *importedElectrons) {
      DerivationFramework::GainDecorator::calculation res =
        decorateObject(electron);
      i = 0;
      for (const auto& kv : m_gainNames) {
        for (const auto layer : m_layers) {
          std::pair<int, int> key(kv.first, layer);
          decorations_E[i](*electron) = res.E[key];
          decorations_rnoW[i](*electron) =
            res.EnoW[key] != 0 ? res.E[key]/res.EnoW[key] : 1;
          decorations_nCells[i](*electron) = res.nCells[key];
          i++;
        }
      }
    }
  }

  return StatusCode::SUCCESS;
}

DerivationFramework::GainDecorator::calculation
DerivationFramework::GainDecorator::decorateObject(
                                                   const xAOD::Egamma*& egamma) const
{

  // Compute energy and number of cells per gain per layer
  // Set the initial values to 0 (needed?)
  DerivationFramework::GainDecorator::calculation result;
  for (const auto& kv : m_names_E) {
    result.E[kv] = 0.;
    result.EnoW[kv] = 0.;
    result.nCells[kv] = 0;
  }

  // Skip the computation for missing cell links (like topo-seeded photons)
  // but decorate anyway
  const CaloClusterCellLink* cellLinks =
    egamma->caloCluster() ? egamma->caloCluster()->getCellLinks() : nullptr;
  if (cellLinks) {
    CaloClusterCellLink::const_iterator it = cellLinks->begin(),
      itE = cellLinks->end();
    for (; it != itE; ++it) {
      const CaloCell *cell = *it;
      if (!cell)
        continue;
      std::pair<int, int> key(static_cast<int>(cell->gain()), getLayer(cell));
      // Increment the corresponding entry (not important if it is not
      // initialised)
      double weight = it.weight();
      result.E[key] += cell->energy()*weight;
      result.EnoW[key] += cell->energy();
      result.nCells[key]++;
    }
  }

  return result;
}

int
DerivationFramework::GainDecorator::getLayer(const CaloCell* cell)
{
  int sampling =
    (cell && cell->caloDDE() ? cell->caloDDE()->getSampling() : -1);
  if (sampling == CaloSampling::PreSamplerB ||
      sampling == CaloSampling::PreSamplerE)
    return 0;
  if (sampling == CaloSampling::EMB1 || sampling == CaloSampling::EME1)
    return 1;
  if (sampling == CaloSampling::EMB2 || sampling == CaloSampling::EME2)
    return 2;
  if (sampling == CaloSampling::EMB3 || sampling == CaloSampling::EME3)
    return 3;
  return -1;
}
