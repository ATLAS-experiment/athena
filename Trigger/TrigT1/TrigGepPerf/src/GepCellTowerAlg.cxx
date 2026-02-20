/*
*   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "./GepCellTowerAlg.h"
#include "CaloDetDescr/CaloDetDescrManager.h"
#include "xAODCaloEvent/CaloClusterAuxContainer.h"
#include "xAODCaloEvent/CaloTower.h"
#include "xAODCaloEvent/CaloTowerContainer.h"
#include "xAODCaloEvent/CaloTowerAuxContainer.h"

GepCellTowerAlg::GepCellTowerAlg( const std::string& name, ISvcLocator* pSvcLocator ) : 
   AthReentrantAlgorithm( name, pSvcLocator ){
}


GepCellTowerAlg::~GepCellTowerAlg() {}


StatusCode GepCellTowerAlg::initialize() {
  ATH_MSG_DEBUG ("Initializing " << name() << "...");

  // Retrieve AlgTools
  CHECK(m_outputCellTowerKey.initialize());
  CHECK(m_gepCellsKey.initialize());

  return StatusCode::SUCCESS;
}

StatusCode GepCellTowerAlg::finalize() {
  ATH_MSG_DEBUG ("Finalizing " << name() << "...");
  return StatusCode::SUCCESS;
}

StatusCode GepCellTowerAlg::execute(const EventContext& context) const {
  ATH_MSG_DEBUG ("Executing " << name() << "...");
  setFilterPassed(false, context); //optional: start with algorithm not passed

  std::vector<Gep::GepCaloCell> cells;

  auto h_gepCellsMap = SG::makeHandle(m_gepCellsKey, context);
  CHECK(h_gepCellsMap.isValid());
  const auto & gepCellsMap = *h_gepCellsMap;

  auto cell_map = gepCellsMap.getCellMap();

  // Loop over all cells
  for (auto const& cell_itr : *cell_map) {
    cells.push_back(cell_itr.second);
  }

  // container for CaloCluster wrappers for Gep Clusters
  SG::WriteHandle<xAOD::CaloClusterContainer> h_outputCaloClusters =
    SG::makeHandle(m_outputCellTowerKey, context);
  CHECK(h_outputCaloClusters.record(std::make_unique<xAOD::CaloClusterContainer>(),
                                    std::make_unique<xAOD::CaloClusterAuxContainer>()));

  // Use the indexing and cell energy accumulation functionality 
  // already implemented in xAOD::CaloTower and xAOD::CaloTowerContainer
  std::vector<std::vector<unsigned int>> cell_ids;
  auto customTowers = std::make_unique<xAOD::CaloTowerContainer>();
  auto aux = std::make_unique<xAOD::CaloTowerAuxContainer>();
  customTowers->setStore(aux.get());

  // Define tower array (98 eta bins x 64 phi bins)
  static constexpr int nEta{98};
  static constexpr int nPhi{64};
  customTowers->configureGrid(nEta,-4.9,4.9,nPhi);

  int nTowers = customTowers->nTowers();
  cell_ids.resize(nTowers);

  for (int iTower=0; iTower < nTowers; ++iTower) {
      auto tower = std::make_unique<xAOD::CaloTower>();
      customTowers->push_back(std::move(tower));
      customTowers->at(iTower)->reset();
  }

  // Single loop over cells to accumulate energy into the correct tower
  for (const auto& cell : cells) {
      if (cell.sigma < 2) continue;
      if (cell.isBadCell()) continue;

      int idx = customTowers->index(cell.eta,cell.phi);
      if (idx < 0) continue;
      // Internally, this results in the cell et being added to the energy (i.e. e, not et) of a
      // 4-vector with the eta and phi set to the center point of the tower
      // Effectively accumulating the et of the tower's constituent cells
      customTowers->at(idx)->addEnergy(cell.et);
      cell_ids[idx].push_back(cell.id);
  }

  // Store the Gep clusters to a CaloClusters, and write out.
  for(auto tower: *customTowers){
    auto p4 = tower->p4();
    // This is equivalent to checking the et of the tower 
    // since e() is the sum of the et of all constituent cells
    if ( p4.E() == 0 ) continue;

    // store the calCluster to fix up the Aux container:
    auto *ptr = h_outputCaloClusters->push_back(std::make_unique<xAOD::CaloCluster>());

    // Unlike what was done here, downstream code will assume that e 
    // is the energy and et is the transverse energy,
    // so we need to "flip" e and et and make a proper 4-vector here
    // reminder - p4.e() was until now the et of the tower
    double eta = p4.Eta();
    double phi = p4.Phi();
    double e = p4.E() * std::cosh(eta);

    // ptr is a massless pseudo-particle with energy e
    // representing a tower. The eta,phi coordinates are
    // the center of the tower.
    // When downstream code reads its et, it will
    // get the measured energy in the tower.
    ptr->setE(e);
    ptr->setEta(eta);
    ptr->setPhi(phi);
    ptr->setTime(0);

    auto cccl = std::make_unique<CaloClusterCellLink>();

    for (auto cell_id : cell_ids[tower->index()])
      cccl->addCell(cell_map->at(cell_id).index, 1.0);

    ptr->addCellLink(std::move(cccl));
  }

  setFilterPassed(true,context); //if got here, assume that means algorithm passed
  return StatusCode::SUCCESS;
}
