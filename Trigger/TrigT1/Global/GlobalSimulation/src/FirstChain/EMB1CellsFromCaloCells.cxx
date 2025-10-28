/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "EMB1CellsFromCaloCells.h"

namespace GlobalSim {

  EMB1CellsFromCaloCells::EMB1CellsFromCaloCells(const std::string& type,
						 const std::string& name,
						 const IInterface* parent):
    base_class(type, name, parent){
  }

  StatusCode EMB1CellsFromCaloCells::initialize() {
    CHECK(m_caloCellsKey.initialize());
    return StatusCode::SUCCESS;
  }
  
  StatusCode
  EMB1CellsFromCaloCells::cells(std::vector<const CaloCell*>& cells,
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

  
    // lambda to select EMB1 cells
    auto EMB1_sel = [](const CaloCell* cell) {
      return cell->caloDDE()->getSampling() == CaloCell_Base_ID::EMB1;
    };
    

    std::copy_if(allCaloCells.beginConstCalo(CaloCell_ID::LAREM),
		 allCaloCells.endConstCalo(CaloCell_ID::LAREM),
		 std::back_inserter(cells),
		 EMB1_sel);
    
   
    return StatusCode::SUCCESS;
  }
}
