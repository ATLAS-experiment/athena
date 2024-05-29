/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
 */

#include "egammaCellRecoveryTool.h"
#include "xAODCaloEvent/CaloCluster.h"
#include "CaloEvent/CaloCellContainer.h"

egammaCellRecoveryTool::egammaCellRecoveryTool(const std::string& type,
					       const std::string& name,
					       const IInterface* parent)
  : AthAlgTool(type, name, parent){
  // declare Interface
  declareInterface<IegammaCellRecoveryTool>(this);
}

StatusCode egammaCellRecoveryTool::initialize(){
  ATH_MSG_DEBUG("Initializing egammaCellRecoveryTool");
  return StatusCode::SUCCESS;
}

StatusCode egammaCellRecoveryTool::execute(const xAOD::CaloCluster& cluster,
					   Info& info) const {

  double etamax = info.etamax;
  double phimax = info.phimax;
  ATH_MSG_DEBUG("etamax = " << etamax << " phimax = " << phimax);
  info.addedCells.reserve(10);

  // Build rectangular cluster arount it (and also in L3)
  egammaCellRecoveryTool::existingCells arrayCells =
    buildCellArrays(&cluster,etamax,phimax);

  //Now compare with the cells stored in the CaloCell container
  // and compute the energy from missed cells
 
  // define required size = 3x7 in barrel and 5x7 in endcap for layer 2
  int iphi1 = 0, iphi2 = 6;
  int ieta1 = 0, ieta2 = 4;
  if (std::abs(etamax) < 1.475) { 
    ieta1 = 1;
    ieta2 = 3;
  }

  const CaloCellContainer *caloCells = cluster.getCellLinks()->getCellContainer();
  for (const auto * cell : *caloCells) {
    if (!cell->caloDDE()) {
      ATH_MSG_WARNING("Calo cell without detector element ?? eta = "
		      << cell->eta() << " phi = " << cell->phi());
      continue;
    }
    
    double deta = cell->caloDDE()->eta_raw()-etamax;
    double dphi = cell->caloDDE()->phi_raw()-phimax;
    if (dphi < -M_PI) dphi += 2*M_PI;
    if (dphi > M_PI)  dphi -= 2*M_PI;
    
    if (std::abs(deta) > 0.1 || std::abs(dphi) > 0.1) continue;
    
    int layer = cell->caloDDE()->getSampling();
    if (layer != CaloSampling::EMB2 && layer != CaloSampling::EME2 &&
	layer != CaloSampling::EMB3 && layer != CaloSampling::EME3)
      continue;
    
    if ((layer == CaloSampling::EMB2 || layer == CaloSampling::EME2) &&
	std::abs(deta) < 0.07 && std::abs(dphi) < 0.085) {
      int ieta = (int)((deta+0.052)/0.025);
      int iphi = (int)((dphi+0.075)/m_phiSize);
      if (ieta >= ieta1 && ieta <= ieta2 && iphi >= iphi1 && iphi <= iphi2) {
	int index = 7*ieta+iphi;
	if (!arrayCells.existL2[index]) {
	  if (std::abs(cell->time()) > m_timeCut) {
	    info.nCells[0]++;
	    info.eCells[0] += cell->energy();
	    info.addedCells.push_back(cell);
	  } // would have been rejected by time cut
	}   // cell not in cluster
      }     // eta-phi window cut
    } else if ((layer == CaloSampling::EMB3 || layer == CaloSampling::EME3) &&
	       std::abs(deta) < 0.05 && std::abs(dphi) < 0.06) {
      int ieta = (int)(std::abs(deta)/0.025);
      int iphi = (int)((dphi+0.05)/m_phiSize);
      if (ieta <= 1 && iphi >= 0 && iphi <= 4) {
	int index = 5*ieta+iphi;
	if (!arrayCells.existL3[index]) {
	  if (std::abs(cell->time()) > m_timeCut) {
	    info.nCells[1]++;
	    info.eCells[1] += cell->energy();
	    info.addedCells.push_back(cell);
	  }
	}
      }
    } 
  }  // loop over cell container

  if (msgLvl(MSG::DEBUG)) {

    ATH_MSG_DEBUG("Added cells in L2, n = " << info.nCells[0] << " E = " << info.eCells[0]
		  << " and in L3, n = " << info.nCells[1] << " E = " << info.eCells[1]
		  << " n(L2+L3) = " << info.addedCells.size());

    for (const auto *c : info.addedCells) {
      ATH_MSG_DEBUG("cell layer " << c->caloDDE()->getSampling() << " E = " << c->energy()
		    << " eta = " << c->caloDDE()->eta_raw()
		    << " phi = " << c->caloDDE()->phi_raw());
    }
  }

  return StatusCode::SUCCESS;
}

egammaCellRecoveryTool::existingCells egammaCellRecoveryTool::buildCellArrays(
   const xAOD::CaloCluster* cluster, double etamax, double phimax) const {

  egammaCellRecoveryTool::existingCells result{};
  // Just to be sure. But should not be needed
  for (int i = 0; i < m_nL2; i++)
    { result.existL2[i] = 0; }
  for (int i = 0; i < m_nL3; i++)
    { result.existL3[i] = 0; }
  
  const CaloClusterCellLink* cellLinks = cluster->getCellLinks();
  if (!cellLinks) {
    ATH_MSG_WARNING("No cell link for cluster. Do nothing");
    return result;
  }
  
  CaloClusterCellLink::const_iterator it_cell = cellLinks->begin(),
    it_cell_e = cellLinks->end();
  
  for (; it_cell != it_cell_e; ++it_cell) {
    const CaloCell* cell = (*it_cell);
    if (cell) {
      if (!cell->caloDDE()) {
	ATH_MSG_WARNING("Calo cell without detector element ?? eta = "
			<< cell->eta() << " phi = " << cell->phi());
	continue;
      }
      int layer = cell->caloDDE()->getSampling();
      if (layer != CaloSampling::EMB2 && layer != CaloSampling::EME2 &&
	  layer != CaloSampling::EMB3 && layer != CaloSampling::EME3)
	continue;
      
       double deta = cell->caloDDE()->eta_raw() - etamax;
       double dphi = cell->caloDDE()->phi_raw() - phimax;
       if (dphi < -M_PI) dphi += 2*M_PI;
       if (dphi > M_PI)  dphi -= 2*M_PI;
       if ((layer == CaloSampling::EMB2 || layer == CaloSampling::EME2) && 
	   std::abs(deta) < 0.07 && std::abs(dphi) < 0.085) {
	 int ieta = (int)((deta+0.052)/0.025);
	 int iphi = (int)((dphi+0.075)/m_phiSize);
	 if (ieta < 0 || ieta > 4 || iphi < 0 || iphi > 6) {
	   ATH_MSG_WARNING("Should never happen ieta = " << ieta
			   << " iphi = " << iphi);
	 } else {
	   int index = 7*ieta+iphi;
	   result.existL2[index] = true;
	 }
       } else if ((layer == CaloSampling::EMB3 || layer == CaloSampling::EME3) &&
		  std::abs(deta) < 0.05 && std::abs(dphi) < 0.06) {
	 int ieta = (int)(std::abs(deta)/0.025);
	 int iphi = (int)((dphi+0.05)/m_phiSize);
	 if (ieta > 1 || iphi < 0 || iphi > 4) {
	   ATH_MSG_WARNING("Should never happen ieta = " << ieta
			   << " iphi = " << iphi);
	 } else {
	   int index = 5*ieta+iphi;
	   result.existL3[index] = true; 
	 }
       } // L2 or L3, in window vs max
    }   // cell exists
  }     // loop over cells
  
  return result;
}
