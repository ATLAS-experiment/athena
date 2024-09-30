/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "EMB1CellsFromCaloClusters.h"
#include "CxxUtils/prefetch.h"

EMB1CellsFromCaloClusters::EMB1CellsFromCaloClusters(const std::string& type,
					       const std::string& name,
					       const IInterface* parent):
  base_class(type, name, parent){
}

StatusCode EMB1CellsFromCaloClusters::initialize() {
  CHECK(m_caloClustersKey.initialize());

  return StatusCode::SUCCESS;
}
  
StatusCode
EMB1CellsFromCaloClusters::cells(std::vector<std::vector<const CaloCell*>>& cells,
			      const EventContext& ctx) const {
  // Read in a container containing  CaloClusters

  auto h_caloClusters = SG::makeHandle(m_caloClustersKey, ctx);
  CHECK(h_caloClusters.isValid());

  auto dvec = *h_caloClusters;
  ATH_MSG_DEBUG("number of retrieved clusters: " << dvec.size());
  
  const CaloCell_ID* calocell_id{nullptr};
  CHECK(detStore()->retrieve (calocell_id, "CaloCell_ID"));
  
  // selector
  auto EMB1_sel = [&calocell_id](const CaloCell* cell) {
    return 
      calocell_id->calo_sample(calocell_id->calo_cell_hash(cell->ID())) ==
      CaloCell_Base_ID::EMB1;
  };



  
  for(const auto cl : dvec){
    const auto *cell_links = cl->getCellLinks();
    if (!cell_links){
      ATH_MSG_ERROR("No link from cluster to cells");
      return StatusCode::FAILURE;
    } else {
      ATH_MSG_DEBUG("Number of cells from cell_links " << cell_links->size());
    }


    auto c_iter = cell_links->begin();
    auto c_end = cell_links->end();

    std::vector<const CaloCell*> cluster_cells;
    std::copy_if(c_iter,
		 c_end,
		 std::back_inserter(cluster_cells),
		 EMB1_sel);
    
    // having less than two cells in the selected cell container
    // will mess up the simple seed selection procedure, so
    // add a requirement on the number of selected cells.
    if (cluster_cells.size() > 1) {
      cells.push_back(cluster_cells);
    }
  }

  return StatusCode::SUCCESS;
}
