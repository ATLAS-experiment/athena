/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "EMB1CellsFromCaloCells.h"

EMB1CellsFromCaloCells::EMB1CellsFromCaloCells(const std::string& type,
					       const std::string& name,
					       const IInterface* parent):
  base_class(type, name, parent){
}

StatusCode EMB1CellsFromCaloCells::initialize() {
  CHECK(m_caloCellsKey.initialize());
  CHECK(detStore()->retrieve (m_calocell_id, "CaloCell_ID"));

  return StatusCode::SUCCESS;
}
  
StatusCode
EMB1CellsFromCaloCells::cells(std::vector<std::vector<const CaloCell*>>& cells,
			      const EventContext& ctx) const {
  // Read in a container containing (all) CaloCells

  auto h_caloCells = SG::makeHandle(m_caloCellsKey, ctx);
  CHECK(h_caloCells.isValid());

  const auto & allCaloCells = *h_caloCells;
  
  
  // limit cells to  LAREM CaloCells
  if(! allCaloCells.hasCalo(m_calocell_id->LAREM)){
    ATH_MSG_ERROR("CaloCellCollection does not contain LAREM cells");
    return StatusCode::FAILURE;
  }

  std::vector<const CaloCell*>
    laremCells(allCaloCells.beginConstCalo(CaloCell_ID::LAREM),
	       allCaloCells.endConstCalo(CaloCell_ID::LAREM));

  // selector
  auto EMB1_sel = [&calocell_id=m_calocell_id](const CaloCell* cell) {
    return 
      calocell_id->calo_sample(calocell_id->calo_cell_hash(cell->ID())) ==
      CaloCell_Base_ID::EMB1;
  };


  std::vector<const CaloCell*> emb1_cells;
  std::copy_if(laremCells.cbegin(),
	       laremCells.cend(),
	       std::back_inserter(emb1_cells),
	       EMB1_sel);

  cells.push_back(emb1_cells);
   
  return StatusCode::SUCCESS;
}
