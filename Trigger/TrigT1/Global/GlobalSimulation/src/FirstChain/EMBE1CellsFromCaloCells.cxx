/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "EMBE1CellsFromCaloCells.h"

namespace GlobalSim {

  EMBE1CellsFromCaloCells::EMBE1CellsFromCaloCells(const std::string& type,
						 const std::string& name,
						 const IInterface* parent):
    base_class(type, name, parent){
  }

  StatusCode EMBE1CellsFromCaloCells::initialize() {
    CHECK(m_caloCellsKey.initialize());
    return StatusCode::SUCCESS;
  }
  
  StatusCode
  EMBE1CellsFromCaloCells::cells(std::vector<const CaloCell*>& cells,
				const EventContext& ctx) const {
    
    // Read in a container containing all CaloCells
    SG::ReadHandle<CaloCellContainer> h_caloCells;    

    h_caloCells = SG::makeHandle(m_caloCellsKey, ctx);
    CHECK(h_caloCells.isValid());
    
    const auto& allCaloCells = *h_caloCells;
    ATH_MSG_DEBUG("allCaloCells size " << allCaloCells.size());
    if (m_makeCaloCellContainerChecks) {
      if(!allCaloCells.checkOrderedAndComplete()){
	ATH_MSG_ERROR("CaloCellCollection fails checks");
	return StatusCode::FAILURE;
      }
    }

  
    // lambda to select EMB1 and EME1 cells
    auto EMBE1_sel = [](const CaloCell* cell) {
      if (cell->caloDDE()->getSampling() == CaloCell_Base_ID::EMB1 || cell->caloDDE()->getSampling() == CaloCell_Base_ID::EME1){
	return true;
      } else {
	return false;
      }
    };
    

    std::copy_if(allCaloCells.beginConstCalo(CaloCell_ID::LAREM),
		 allCaloCells.endConstCalo(CaloCell_ID::LAREM),
		 std::back_inserter(cells),
		 EMBE1_sel);
    
   
    return StatusCode::SUCCESS;
  }
}
