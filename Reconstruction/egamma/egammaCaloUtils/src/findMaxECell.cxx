/*
 Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#include "egammaCaloUtils/findMaxECell.h"

#include "CaloEvent/CaloClusterCellLinkContainer.h"
#include "CaloEvent/CaloCellContainer.h"

namespace egammaCellUtils
{
  MaxECell::MaxECell(
   const xAOD::CaloCluster *clus,
   const std::string &cellCKey, bool UseWeightForMaxCell) :
    AthMessaging("egammaCellUtils::MaxECell") {
    
    const CaloClusterCellLink* cellLinks = clus->getCellLinks();
    if (!cellLinks) {
      ATH_MSG_WARNING("No cell link for cluster. Do nothing");
    } else {
    
      // First check :
      const CaloCellContainer *caloCells = cellLinks->getCellContainer();
      if (!caloCells) {
	ATH_MSG_WARNING("No cells for cluster. Do nothing");
      } else if (cellLinks->getCellContainerLink().dataID() != cellCKey) {
	// Second check :
	// one might have used a custom cell container for the linked cells
	ATH_MSG_ERROR("Wrong key for the calo cells");
      } else {
    
	CaloClusterCellLink::const_iterator it_cell = cellLinks->begin(),
	  it_cell_e = cellLinks->end();
	
	// find maximum cell energy in layer 2
	double emax = 0.;
	std::pair<const CaloCell*,double> maxcell{nullptr,0}; //just for debug
	for(; it_cell != it_cell_e; ++it_cell) {
	  const CaloCell* cell = (*it_cell);
	  if (cell) {
	    if (!cell->caloDDE()) {
	      ATH_MSG_WARNING("Calo cell without detector element ?? eta = "
			      << cell->eta() << " phi = " << cell->phi());
	      continue;
	    }
	    int layer = cell->caloDDE()->getSampling();
	    if (layer == CaloSampling::EMB2 || layer == CaloSampling::EME2) {
	      double w     = it_cell.weight();
	      double eCell = cell->energy();
	      if (UseWeightForMaxCell) eCell *= w;
	      if (eCell > emax) {
		emax           = eCell;
		maxcell.first  = cell;
		maxcell.second = w;
	      }
	    }
	  }
	}
	
	if (emax > 0) {
	  etaCell = maxcell.first->caloDDE()->eta_raw();
	  phiCell = maxcell.first->caloDDE()->phi_raw();
	  if (msgLvl(MSG::DEBUG)) {
	    CaloSampling::CaloSample sam = maxcell.first->caloDDE()->getSampling();
	    double etaAmax = clus->etamax(sam);
	    double phiAmax = clus->phimax(sam);
	    double vemax   = clus->energy_max(sam);
	    ATH_MSG_DEBUG("Cluster energy in sampling 2 = " << clus->energyBE(2)
			  << " maximum layer 2 energy cell, E = " << maxcell.first->energy()
			  << " check E = " << vemax
			  << " w = " << maxcell.second << "\n"
			  << " in calo  frame, eta = " << etaCell << " phi = " << phiCell << "\n"
			  << " in ATLAS frame, eta = " << etaAmax << " phi = " << phiAmax);
	  }
	} else {
	  ATH_MSG_WARNING("No layer 2 cell with positive energy ! Should never happen");
	  sc = StatusCode::FAILURE;
	}
	sc = StatusCode::SUCCESS;
      } // links OK, good cell container
    }   // no links 
  } // end function
}

