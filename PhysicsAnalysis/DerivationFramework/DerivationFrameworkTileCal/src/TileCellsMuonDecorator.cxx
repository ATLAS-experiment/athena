///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// TileCellsMuonDecorator.cxx
// Implementation file for class TileCellsMuonDecorator
///////////////////////////////////////////////////////////////////

// Tile includes
#include "TileCellsMuonDecorator.h"

// Athena
#include "AthenaKernel/errorcheck.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteDecorHandle.h"
#include "CaloGeoHelpers/proxim.h"
#include "xAODMuon/MuonContainer.h"

#include <string>
#include <vector>
#include <algorithm>

namespace DerivationFramework {

  StatusCode TileCellsMuonDecorator::initialize() {

    ATH_CHECK(m_trackInCalo.retrieve());
    ATH_CHECK(m_tracksInCone.retrieve());
    ATH_CHECK(m_cellsDecorator.retrieve());

    ATH_CHECK( m_muonContainerKey.initialize() );
    ATH_CHECK( m_cellContainerKey.initialize(SG::AllowEmpty) );
    ATH_CHECK( m_clusterContainerKey.initialize(SG::AllowEmpty) );

    const std::string baseName = m_muonContainerKey.key() + ".";

    m_selectedMuKey = baseName + m_prefix + m_selectedMuKey.key();
    ATH_CHECK( m_selectedMuKey.initialize() );

    m_econeMuKey = baseName + m_prefix + m_econeMuKey.key() + std::to_string(int(m_isoCone * 100));
    ATH_CHECK( m_econeMuKey.initialize() );

    m_cellsMuonXKey = baseName + m_prefix + m_cellsMuonXKey.key();
    ATH_CHECK( m_cellsMuonXKey.initialize() );

    m_cellsMuonYKey = baseName + m_prefix + m_cellsMuonYKey.key();
    ATH_CHECK( m_cellsMuonYKey.initialize() );

    m_cellsMuonZKey = baseName + m_prefix + m_cellsMuonZKey.key();
    ATH_CHECK( m_cellsMuonZKey.initialize() );

    m_cellsMuonEtaKey = baseName + m_prefix + m_cellsMuonEtaKey.key();
    ATH_CHECK( m_cellsMuonEtaKey.initialize() );

    m_cellsMuonPhiKey = baseName + m_prefix + m_cellsMuonPhiKey.key();
    ATH_CHECK( m_cellsMuonPhiKey.initialize() );

    m_cellsToMuonDxKey = baseName + m_prefix + m_cellsToMuonDxKey.key();
    ATH_CHECK( m_cellsToMuonDxKey.initialize() );

    m_cellsToMuonDyKey = baseName + m_prefix + m_cellsToMuonDyKey.key();
    ATH_CHECK( m_cellsToMuonDyKey.initialize() );

    m_cellsToMuonDzKey = baseName + m_prefix + m_cellsToMuonDzKey.key();
    ATH_CHECK( m_cellsToMuonDzKey.initialize() );

    m_cellsToMuonDetaKey = baseName + m_prefix + m_cellsToMuonDetaKey.key();
    ATH_CHECK( m_cellsToMuonDetaKey.initialize() );

    m_cellsToMuonDphiKey = baseName + m_prefix + m_cellsToMuonDphiKey.key();
    ATH_CHECK( m_cellsToMuonDphiKey.initialize() );

    m_cellsMuonDxKey = baseName + m_prefix + m_cellsMuonDxKey.key();
    ATH_CHECK( m_cellsMuonDxKey.initialize() );

    m_cellsMuonDeDxKey = baseName + m_prefix + m_cellsMuonDeDxKey.key();
    ATH_CHECK( m_cellsMuonDeDxKey.initialize() );

    if (!m_clusterContainerKey.empty()) {
      for (double dr : m_drCones) {
        m_larEnergyInConeKeyArray.emplace_back(baseName + m_prefix + "elarcone" + std::to_string(int(dr * 100)));
      }
    }
    ATH_CHECK( m_larEnergyInConeKeyArray.initialize(SG::AllowEmpty) );

    for (unsigned int layer : m_energyInLayers) {
      m_energyInSamplings.emplace(static_cast<xAOD::CaloCluster::CaloSample>(layer));
    }

    return StatusCode::SUCCESS;
  }

  StatusCode TileCellsMuonDecorator::addBranches(const EventContext& ctx) const {


    SG::ReadHandle<xAOD::MuonContainer> muons(m_muonContainerKey, ctx);
    ATH_CHECK( muons.isValid() );

    const CaloCellContainer* cellContainer = nullptr;
    if (!m_cellContainerKey.empty()) {
      SG::ReadHandle<CaloCellContainer> caloCells(m_cellContainerKey, ctx);
      ATH_CHECK( caloCells.isValid() );
      cellContainer = caloCells.cptr();
    }

    const xAOD::CaloClusterContainer* clusterContainer = nullptr;
    if (!m_clusterContainerKey.empty()) {
      SG::ReadHandle<xAOD::CaloClusterContainer> caloClusters(m_clusterContainerKey, ctx);
      ATH_CHECK( caloClusters.isValid() );
      clusterContainer = caloClusters.cptr();
    }

    SG::WriteDecorHandle<xAOD::MuonContainer, int> selected_mu(m_selectedMuKey, ctx);
    SG::WriteDecorHandle<xAOD::MuonContainer, float> econe_mu(m_econeMuKey, ctx);
    SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<float>> cellsMuonX(m_cellsMuonXKey, ctx);
    SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<float>> cellsMuonY(m_cellsMuonYKey, ctx);
    SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<float>> cellsMuonZ(m_cellsMuonZKey, ctx);
    SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<float>> cellsMuonEta(m_cellsMuonEtaKey, ctx);
    SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<float>> cellsMuonPhi(m_cellsMuonPhiKey, ctx);
    SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<float>> cellsToMuonDx(m_cellsToMuonDxKey, ctx);
    SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<float>> cellsToMuonDy(m_cellsToMuonDyKey, ctx);
    SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<float>> cellsToMuonDz(m_cellsToMuonDzKey, ctx);
    SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<float>> cellsToMuonDeta(m_cellsToMuonDetaKey, ctx);
    SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<float>> cellsToMuonDphi(m_cellsToMuonDphiKey, ctx);
    SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<float>> cellsMuonDx(m_cellsMuonDxKey, ctx);
    SG::WriteDecorHandle<xAOD::MuonContainer, std::vector<float>> cellsMuonDeDx(m_cellsMuonDeDxKey, ctx);
    std::vector<SG::WriteDecorHandle<xAOD::MuonContainer, float>> larEnergyInCones;
    for (const SG::WriteDecorHandleKey<xAOD::MuonContainer>& key : m_larEnergyInConeKeyArray) {
      larEnergyInCones.emplace_back(key, ctx);
    }

    std::map<const xAOD::IParticle*, std::vector<const CaloCell*>> muonCellsMap;

    for ( const xAOD::Muon* mu : *muons ) {

      std::vector<const CaloCell*> cells;

      std::vector< float > cells_mu_x;
      std::vector< float > cells_mu_y;
      std::vector< float > cells_mu_z;
      std::vector< float > cells_mu_eta;
      std::vector< float > cells_mu_phi;

      std::vector< float > cells_to_mu_dx;
      std::vector< float > cells_to_mu_dy;
      std::vector< float > cells_to_mu_dz;
      std::vector< float > cells_to_mu_deta;
      std::vector< float > cells_to_mu_dphi;

      std::vector< float > cells_mu_dx;
      std::vector< float > cells_mu_dedx;
      std::vector< float > lar_energy_in_cones;

      if (m_selectMuons &&
          (mu->muonType() != xAOD::Muon::Combined
           || mu->pt() < m_minPt
           || std::abs(mu->eta()) > m_maxAbsEta)) {

        selected_mu(*mu) = 0;
        continue;
      }

      const xAOD::TrackParticle* mu_track = mu->trackParticle(xAOD::Muon::InnerDetectorTrackParticle);
      const xAOD::CaloCluster* mu_cluster = mu->cluster();
      if (mu_track && mu_cluster && mu_cluster->getCellLinks()) {

        float e_trk_in_isocone(0.0);
        std::vector<const xAOD::TrackParticle*> tracks_in_cone;
        m_tracksInCone->particlesInCone(mu_track->eta(), mu_track->phi(), m_isoCone, tracks_in_cone);
        for (const xAOD::TrackParticle* track : tracks_in_cone) {
          if (track != mu_track) e_trk_in_isocone += track->e();
        }

        econe_mu(*mu) = e_trk_in_isocone;

        if (m_selectMuons && (e_trk_in_isocone > m_maxRelEtrkInIsoCone * mu_track->e())) {
          selected_mu(*mu) = 0;
          continue;
        }

        selected_mu(*mu) = 1;

        cells.clear();
        bool addAdditionalGapCrackCells = false;
        for (const CaloCell* cell : *mu_cluster) {
          const CaloDetDescrElement* cell_dde = cell->caloDDE();
          if ((cell_dde->is_tile())) {
            if (cellContainer && (cell_dde->getSampling() == CaloCell_ID::TileGap3)) {
              addAdditionalGapCrackCells = true;
              continue;
            }
            cells.push_back(cell);
          }
        }

        if (addAdditionalGapCrackCells) {
          std::vector<double> coordinates = m_trackInCalo->getXYZEtaPhiInCellSampling(mu_track, CaloCell_ID::TileGap3);
          if (coordinates.size() == 5 ) {
            double eta = coordinates[3];
            double phi = coordinates[4];
            for (const CaloCell* cell : *cellContainer) {
              const CaloDetDescrElement* cell_dde = cell->caloDDE();
              if (cell_dde->getSampling() == CaloCell_ID::TileGap3) {
                if (std::fabs(eta - cell->eta()) < m_gapCrackCellsInDeltaEta
                    && std::fabs(KinematicUtils::deltaPhi(phi, cell->phi())) < m_gapCrackCellsInDeltaPhi) {
                  cells.push_back(cell);
                }
              }
            }
          }
        }

        for (const CaloCell* cell : cells) {

          std::vector<double> coordinates = m_trackInCalo->getXYZEtaPhiInCellSampling(mu_track, cell);

          if (coordinates.size() == 5 ) {

            float path_length = m_trackInCalo->getPathInsideCell(mu_track, cell);
            cells_mu_dx.push_back( path_length );
            cells_mu_dedx.push_back( (path_length > 0 ? (cell->energy() / path_length) : -1.0) );

            cells_mu_x.push_back(coordinates[0]);
            cells_mu_y.push_back(coordinates[1]);
            cells_mu_z.push_back(coordinates[2]);
            cells_mu_eta.push_back(coordinates[3]);
            cells_mu_phi.push_back(coordinates[4]);

            cells_to_mu_dx.push_back(cell->x() - coordinates[0]);
            cells_to_mu_dy.push_back(cell->y() - coordinates[1]);
            cells_to_mu_dz.push_back(cell->z() - coordinates[2]);
            cells_to_mu_deta.push_back(cell->eta() - coordinates[3]);
            cells_to_mu_dphi.push_back( KinematicUtils::deltaPhi(coordinates[4], cell->phi()) );

          } else {

            cells_mu_dx.push_back( 0.0 );
            cells_mu_dedx.push_back( -2.0 );

            cells_mu_x.push_back(0.0);
            cells_mu_y.push_back(0.0);
            cells_mu_z.push_back(0.0);
            cells_mu_eta.push_back(0.0);
            cells_mu_phi.push_back(0.0);

            cells_to_mu_dx.push_back(0.0);
            cells_to_mu_dy.push_back(0.0);
            cells_to_mu_dz.push_back(0.0);
            cells_to_mu_deta.push_back(0.0);
            cells_to_mu_dphi.push_back(0.0);

          }
        }

        muonCellsMap[mu] = cells;

        if (clusterContainer) {
          lar_energy_in_cones = m_trackInCalo->getEnergyInCones(mu_track, clusterContainer, m_energyInSamplings, m_drCones, ctx);
        }

      } else {
        selected_mu(*mu) = 0;
      }

      cellsMuonX(*mu) = std::move(cells_mu_x);
      cellsMuonY(*mu) = std::move(cells_mu_y);
      cellsMuonZ(*mu) = std::move(cells_mu_z);
      cellsMuonEta(*mu) = std::move(cells_mu_eta);
      cellsMuonPhi(*mu) = std::move(cells_mu_phi);
      cellsToMuonDx(*mu) = std::move(cells_to_mu_dx);
      cellsToMuonDy(*mu) = std::move(cells_to_mu_dy);
      cellsToMuonDz(*mu) = std::move(cells_to_mu_dz);
      cellsToMuonDeta(*mu) = std::move(cells_to_mu_deta);
      cellsToMuonDphi(*mu) = std::move(cells_to_mu_dphi);
      cellsMuonDx(*mu) = std::move(cells_mu_dx);
      cellsMuonDeDx(*mu) = std::move(cells_mu_dedx);

      for (unsigned int icone = 0; icone < larEnergyInCones.size(); ++icone) {
        larEnergyInCones[icone](*mu) = std::move(lar_energy_in_cones[icone]);
      }

    }

    ATH_CHECK( m_cellsDecorator->decorate(muonCellsMap, ctx) );

    return StatusCode::SUCCESS;
  }

}
