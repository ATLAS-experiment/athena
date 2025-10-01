/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAOD_ANALYSIS

#include "TauPi0CreateROI.h"
#include "tauRecTools/HelperFunctions.h"

#include "CaloUtils/CaloCellList.h"
#include <memory>

TauPi0CreateROI::TauPi0CreateROI(const std::string& name) :
     TauRecToolBase(name) {
}
   


StatusCode TauPi0CreateROI::initialize() {
    
    ATH_CHECK( m_caloCellInputContainer.initialize() );
    ATH_CHECK( m_caloMgrKey.initialize() );
    ATH_CHECK( m_removedClusterInputContainer.initialize(SG::AllowEmpty) );
  ATH_MSG_INFO("Find Pi0 in context: " << (inEleRM() ? "`EleRM`" : "`Standard`") << ", with Electron cell removal Flag: " << m_removeElectronCells);
    return StatusCode::SUCCESS;
}



StatusCode TauPi0CreateROI::executePi0CreateROI(xAOD::TauJet& tau, CaloConstCellContainer& pi0CellContainer, boost::dynamic_bitset<>& addedCellsMap) const {

  // only run on 0-5 prong taus
  if (!tauRecTools::doPi0andShots(tau)) {
    return StatusCode::SUCCESS;
  }  

  SG::ReadHandle<CaloCellContainer> caloCellInHandle( m_caloCellInputContainer );
  if (!caloCellInHandle.isValid()) {
    ATH_MSG_ERROR ("Could not retrieve HiveDataObj with key " << caloCellInHandle.key());
    return StatusCode::FAILURE;
  }
  
  const CaloCellContainer *cellContainer = caloCellInHandle.cptr();
  std::vector<const CaloCell*> removed_cells;
  if (inEleRM() && m_removeElectronCells){
    SG::ReadHandle<xAOD::CaloClusterContainer> removedClustersHandle( m_removedClusterInputContainer );
    if (!removedClustersHandle.isValid()){
      ATH_MSG_ERROR ("Could not retrieve HiveDataObj with key " << removedClustersHandle.key());
      return StatusCode::FAILURE;
    }
    const xAOD::CaloClusterContainer *removed_clusters_cont = removedClustersHandle.cptr();
    
    for (auto cluster : *removed_clusters_cont){
      for(auto cell_it = cluster->cell_cbegin(); cell_it != cluster->cell_cend(); cell_it++){
        removed_cells.push_back(*cell_it);
      }
    }
  }
  
  SG::ReadCondHandle<CaloDetDescrManager> caloMgrHandle{m_caloMgrKey};
  const CaloDetDescrManager* caloDDMgr = *caloMgrHandle;
  
  // get only EM cells within dR < 0.4
  // TODO: change hardcoded 0.4 to meaningful variable
  std::vector<CaloCell_ID::SUBCALO> emSubCaloBlocks;
  emSubCaloBlocks.push_back(CaloCell_ID::LAREM);
  std::unique_ptr<CaloCellList> cellList = std::make_unique<CaloCellList>(caloDDMgr, cellContainer, emSubCaloBlocks);
  // FIXME: tau p4 is corrected to point at tau vertex, but the cells are not
  cellList->select(tau.eta(), tau.phi(), 0.4);

  for (const CaloCell* cell : *cellList) {
    // only keep cells that are in Ecal (PS, EM1, EM2 and EM3, both barrel and endcap).
    int sampling = cell->caloDDE()->getSampling();
    if (sampling > 7) continue;
    // if in EleRM, check the clusters do not include electron activities
    if (m_removeElectronCells && inEleRM() && std::find(removed_cells.cbegin(), removed_cells.cend(), cell) != removed_cells.cend()) continue;
    // Store cell in output container
    const IdentifierHash cellHash = cell->caloDDE()->calo_hash();

    if (!addedCellsMap.test(cellHash)) {
      pi0CellContainer.push_back(cell);
      addedCellsMap.set(cellHash);
    }
  }

  return StatusCode::SUCCESS;
}

#endif
