/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file CaloCellDecorator.cxx
 * @author Gabriel P. Matos <gpinheir@cern.ch>
 * @date Nov 2025
*/

#include "DerivationFrameworkCalo/CaloCellDecorator.h"
#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "StoreGate/ReadHandle.h"

// Calo includes
#include "CaloEvent/CaloCellContainer.h"
#include "CaloUtils/CaloLayerCalculator.h"
#include "CaloUtils/CaloCellList.h"
#include "egammaInterfaces/IegammaLargeClusterCellRecoveryTool.h"

#include <cstdint>
#include <string>
#include <sys/types.h>
#include <vector>


// Destructor
DerivationFramework::CaloCellDecorator::~CaloCellDecorator() = default;

// Athena initialize
StatusCode
DerivationFramework::CaloCellDecorator::initialize()
{
  ATH_MSG_VERBOSE("initialize() ...");

  // Cabling and calo initialize
  ATH_CHECK(m_cablingKey.initialize());
  ATH_CHECK(m_SGKey_CaloCells.initialize());
  ATH_CHECK(m_caloDetDescrMgrKey.initialize());

  // Setup for photons
  ATH_CHECK(m_SGKey_photons.initialize(SG::AllowEmpty));
  if (!m_SGKey_photons.key().empty()) {
    ATH_MSG_INFO("Decorating photons with calo cells using " << m_SGKey_photons.key() << " container");
  }
  ATH_CHECK(m_SGKey_photons_decorations.initialize(!m_SGKey_photons.key().empty()));

  // Setup for electrons
  ATH_CHECK(m_SGKey_electrons.initialize(SG::AllowEmpty));
  if (!m_SGKey_electrons.key().empty()) {
    ATH_MSG_INFO("Decorating electrons with calo cells using " << m_SGKey_electrons.key() << " container");
  }
  ATH_CHECK(m_SGKey_electrons_decorations.initialize(!m_SGKey_electrons.key().empty()));

  // Initialize tools
  ATH_CHECK(m_egammaCellRecoveryTool.retrieve());
  ATH_CHECK(m_egammaLargeClusterCellRecoveryTool.retrieve());

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

  // Retrieve containers
  SG::ReadHandle<xAOD::EgammaContainer> egammaContainer(contKey, ctx);
  SG::ReadHandle<CaloCellContainer> caloCellContainer(m_SGKey_CaloCells, ctx);

  // Calo detector description manager
  SG::ReadCondHandle<CaloDetDescrManager> caloDetDescrMgrHandle(m_caloDetDescrMgrKey, ctx);
  ATH_CHECK(caloDetDescrMgrHandle.isValid());

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
  SG::WriteDecorHandle<xAOD::EgammaContainer, std::vector<uint8_t>>
    decoration11(decorKeys[11], ctx);

  // Loop through egamma objects and decorate
  const xAOD::EgammaContainer* importedEgamma = egammaContainer.ptr();
  const CaloCellContainer* caloCells = caloCellContainer.ptr();
  const CaloDetDescrManager* cmgr = *caloDetDescrMgrHandle;

  for (const auto* egamma : *importedEgamma) {
    const xAOD::CaloCluster *cluster = egamma->caloCluster();
    DerivationFramework::CaloCellDecorator::CellDecorationData res =
      getDecorations(cluster, caloCells, cmgr, ctx);

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
    decoration11(*egamma) = res.cells_clusterOriginInfo;
  }

  return StatusCode::SUCCESS;

};

DerivationFramework::CaloCellDecorator::CellDecorationData
DerivationFramework::CaloCellDecorator::getDecorations(
                                                       const xAOD::CaloCluster* cluster,
                                                       const CaloCellContainer* caloCells,
                                                       const CaloDetDescrManager* cmgr,
                                                       const EventContext& ctx) const
{

  DerivationFramework::CaloCellDecorator::CellDecorationData decorations;

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

    std::unordered_map<const CaloCell*, CaloCellDecorator::CellClusterInfo> info;

    // Lambda function to keep track of cells already added to the decorations
    // and to update origin info depending on whether they were in the supercluster,
    // recovered by timing cut recovery, or in the 7x11 cluster.
    auto registerCell = [&](const CaloCell* cell, uint8_t originMask) {
      int layer = layerFromSampling(cell->caloDDE()->getSampling());
      if (layer < 0) return; // Ignore non-LAr cells

      auto [it, inserted] = info.emplace(cell, CellClusterInfo{});
      CellClusterInfo& ci = it->second;

      // Update origin mask
      ci.mask |= originMask;

      if (inserted) {
        ci.index = decorations.cells_E.size();

        uint64_t onlId = (uint64_t)(cabling->createSignalChannelID(cell->caloDDE()->identify())).get_compact();
        decorations.cells_E.push_back(cell->e());
        decorations.cells_time.push_back(cell->time());
        decorations.cells_eta.push_back(cell->eta());
        decorations.cells_phi.push_back(cell->phi());
        decorations.cells_x.push_back(cell->x());
        decorations.cells_y.push_back(cell->y());
        decorations.cells_z.push_back(cell->z());
        decorations.cells_gain.push_back((int)cell->gain());
        decorations.cells_layer.push_back(layer);
        decorations.cells_quality.push_back(cell->quality());
        decorations.cells_clusterOriginInfo.push_back(ci.mask);
        decorations.cells_onlId.push_back(onlId);
      } 
      else {
        // Update origin info if cell already exists
        decorations.cells_clusterOriginInfo[ci.index] = ci.mask;
      }

    };

    double emax = 0.;
    const CaloCell* maxcell = nullptr;

    // Loop through supercluster cells
    for (const CaloCell* cell : *cluster) {
      // Find maximum energy cell in L2 from those in the supercluster
      int layer = layerFromSampling(cell->caloDDE()->getSampling());
      if (layer == 2) {
        if (cell->e() > emax) {
          emax = cell->e();
          maxcell = cell;
        }
      }
      registerCell(cell,  0x01); // Supercluster cell
    }

    IegammaCellRecoveryTool::Info timeCutRecoveryInfo{};
    if (maxcell->e() > 0.) {
      timeCutRecoveryInfo.etamax = maxcell->caloDDE()->eta_raw();
      timeCutRecoveryInfo.phimax = maxcell->caloDDE()->phi_raw();
      if (m_egammaCellRecoveryTool->execute(*cluster, timeCutRecoveryInfo).isFailure()) {
        ATH_MSG_WARNING("egammaCellRecoveryTool execution failed");
      }
    } else {
      ATH_MSG_WARNING("Max cell in L2 has zero energy, should never happen! Skipping timing cut recovery");
    }

    // Loop through timing cut recovered cells
    for (const CaloCell* cell : timeCutRecoveryInfo.addedCells) {
      registerCell(cell, 0x02); // Timing cut recovered cell
    }

    IegammaLargeClusterCellRecoveryTool::Info largeClusterInfo{};
    if (m_egammaLargeClusterCellRecoveryTool->execute(cluster, cmgr, caloCells, largeClusterInfo).isFailure()) {
      ATH_MSG_WARNING("egammaLargeClusterCellRecoveryTool execution failed");
    }

    // Loop through 7x11 cluster cells
    for (const CaloCell* cell : largeClusterInfo.cells711) {
      registerCell(cell, 0x04); // 7x11 cluster cell
    }
  }

  return decorations;
};