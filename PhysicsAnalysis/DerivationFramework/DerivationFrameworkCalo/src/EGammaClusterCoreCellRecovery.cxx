/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// EGammaClusterCoreCellRecovery.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
// Author: G. Unal (Guillaume.Unal@cern.ch)
// Decorate egamma objects with the energies in L2 and L3 that are in cells
// not included in the original supercluser due to the timing cut in topocluster
// building. This is an AOD fix for data (and mc, but effect is small for mc)
// produced with rel23 (and 24 ?)

#include "DerivationFrameworkCalo/EGammaClusterCoreCellRecovery.h"
#include "CaloEvent/CaloCell.h"

#include <TString.h>
#include <string>
namespace {}

// Constructor
DerivationFramework::EGammaClusterCoreCellRecovery::EGammaClusterCoreCellRecovery(const std::string& t,
										  const std::string& n,
										  const IInterface* p)
  : base_class(t, n, p)
{
}


// Athena initialize and finalize
StatusCode
DerivationFramework::EGammaClusterCoreCellRecovery::initialize()
{
  ATH_MSG_VERBOSE("initialize() ...");

  // The main tool
  ATH_CHECK(m_egammaCellRecoveryTool.retrieve());
  
  if (m_SGKey_photons.key().empty() && m_SGKey_electrons.key().empty()) {
    ATH_MSG_FATAL("No e-gamma collection provided for thinning. At least one "
                  "egamma collection (photons/electrons) must be provided!");
    return StatusCode::FAILURE;
  }

  if (!m_SGKey_electrons.key().empty()) {
    ATH_MSG_DEBUG("Using " << m_SGKey_electrons << " for electrons");
    ATH_CHECK(m_SGKey_electrons.initialize());

    const std::string containerKey = m_SGKey_electrons.key();
    for (int i = 2; i <= 3; i++) {
      for (int t = 0; t <=1 ; t++) {
	m_SGKey_electrons_decorations.emplace_back(
	  Form("%s.%sadded_Lr%d", containerKey.c_str(), (t == 0 ? "n" : "E"), i));
      }
    }
    ATH_CHECK(m_SGKey_electrons_decorations.initialize());
    if (msgLvl(MSG::DEBUG)) {
      ATH_MSG_DEBUG("Decorations for " << containerKey);
      for (const auto& s : m_SGKey_electrons_decorations)
	{ ATH_MSG_DEBUG(s.key()); }
    }
  }

  // This tool needs the calo cells linked to the clusters...
  ATH_CHECK(m_SGKey_CaloCells.initialize());

  if (!m_SGKey_photons.key().empty()) {
    ATH_MSG_DEBUG("Using " << m_SGKey_photons << " for photons");
    ATH_CHECK(m_SGKey_photons.initialize());

    const std::string containerKey = m_SGKey_photons.key();
    for (int i = 2; i <= 3; i++) {
      for (int t = 0; t <= 1 ; t++) {
	m_SGKey_photons_decorations.emplace_back(
	  Form("%s.%sadded_Lr%d", containerKey.c_str(), (t == 0 ? "n" : "E"), i));
      }
    }    
    ATH_CHECK(m_SGKey_photons_decorations.initialize());
    if (msgLvl(MSG::DEBUG)) {
      ATH_MSG_DEBUG("Decorations for " << containerKey);
      for (const auto& s : m_SGKey_photons_decorations)
	{ ATH_MSG_DEBUG(s.key()); }
    }
  }

  return StatusCode::SUCCESS;
}


// The decoration itself
StatusCode
DerivationFramework::EGammaClusterCoreCellRecovery::addBranches() const
{
  const EventContext& ctx = Gaudi::Hive::currentContext();
  
  std::vector<SG::WriteDecorHandle<xAOD::EgammaContainer, char>> decon;
  std::vector<SG::WriteDecorHandle<xAOD::EgammaContainer, float>> decoE;
  decon.reserve(2);
  decoE.reserve(2);

  // Photon decorations
  if (!m_SGKey_photons.key().empty()) {

    for (int i = 0; i < 2; i++) {
      decon.emplace_back(m_SGKey_photons_decorations[i * 2], ctx);
      decoE.emplace_back(m_SGKey_photons_decorations[i * 2 + 1], ctx);
    }

    // Retrieve photon container
    SG::ReadHandle<xAOD::EgammaContainer> photonContainer(m_SGKey_photons, ctx);

    // Decorate photons
    for (const auto* photon : *photonContainer.ptr()) {
      IegammaCellRecoveryTool::Info res = decorateObject(photon);
      for (int i = 0; i < 2; i++) {
	decon[i](*photon) = res.nCells[i];
	decoE[i](*photon) = res.eCells[i];
      }
    }
  }

  // Electron decorations
  if (!m_SGKey_electrons.key().empty()) {

    decon.clear(); decon.reserve(2);
    decoE.clear(); decoE.reserve(2);

    for (int i = 0; i < 2; i++) {
      decon.emplace_back(m_SGKey_electrons_decorations[i * 2], ctx);
      decoE.emplace_back(m_SGKey_electrons_decorations[i * 2 + 1], ctx);
    }

    // Retrieve electron container
    SG::ReadHandle<xAOD::EgammaContainer> electronContainer(m_SGKey_electrons,
                                                            ctx);

    // Decorate electrons
    for (const auto* electron : *electronContainer.ptr()) {
      IegammaCellRecoveryTool::Info res = decorateObject(electron);
      for (int i = 0; i < 2; i++) {
	decon[i](*electron) = res.nCells[i];
	decoE[i](*electron) = res.eCells[i];
      }
    }
  }

  return StatusCode::SUCCESS;
}

IegammaCellRecoveryTool::Info
DerivationFramework::EGammaClusterCoreCellRecovery::decorateObject(
  const xAOD::Egamma*& egamma) const
{
  IegammaCellRecoveryTool::Info info{};

  ATH_MSG_DEBUG("Trying to recover cell for object of type " << egamma->type()
		<< " pT = " << egamma->pt()
		<< " eta = " << egamma->eta()
		<< " phi = " << egamma->phi());

  const xAOD::CaloCluster *clus = egamma->caloCluster();
  if (!clus) {
    ATH_MSG_WARNING("No associated egamma cluster. Do nothing");
    return info;
  }
 
  // Find max energy cell in layer 2
  double etamax = -999., phimax = -999.;
  if (findMaxECell(clus,etamax,phimax).isFailure()) {
    ATH_MSG_WARNING("Problem in finding maximum energy cell in layer 2");
    return info;
  }

  info.etamax = etamax;
  info.phimax = phimax;
  if (m_egammaCellRecoveryTool->execute(*clus,info).isFailure()) {
    ATH_MSG_WARNING("Issue trying to recover cells");
  }
  return info;
}

StatusCode
DerivationFramework::EGammaClusterCoreCellRecovery::findMaxECell(
  const xAOD::CaloCluster *clus, double &etamax, double &phimax) const
{
  const CaloClusterCellLink* cellLinks = clus->getCellLinks();
  if (!cellLinks) {
    ATH_MSG_WARNING("No cell link for cluster. Do nothing");
    return StatusCode::FAILURE;
  }

  // First check :
  // if we run in MT, this would be nullptr without initializing a ReadHandleKey
  const CaloCellContainer *caloCells = cellLinks->getCellContainer();
  if (!caloCells) {
    ATH_MSG_WARNING("No cells for cluster. Do nothing");
    return StatusCode::FAILURE;
  }

  // Second check :
  // one might have used a custom cell container for the linked cells
  if (cellLinks->getCellContainerLink().dataID() != m_SGKey_CaloCells.key()) {
    ATH_MSG_ERROR("Wrong key for the calo cells");
    return StatusCode::FAILURE;
  }

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
	if (m_UseWeightForMaxCell) eCell *= w;
	if (eCell > emax) {
	  emax           = eCell;
	  maxcell.first  = cell;
	  maxcell.second = w;
	}
      }
    }
  }

  if (emax > 0) {
    etamax = maxcell.first->caloDDE()->eta_raw();
    phimax = maxcell.first->caloDDE()->phi_raw();
    if (msgLvl(MSG::DEBUG)) {
      CaloSampling::CaloSample sam = maxcell.first->caloDDE()->getSampling();
      double etaAmax = clus->etamax(sam);
      double phiAmax = clus->phimax(sam);
      double vemax   = clus->energy_max(sam);
      ATH_MSG_DEBUG("Cluster energy in sampling 2 = " << clus->energyBE(2)
		    << " maximum layer 2 energy cell, E = " << maxcell.first->energy()
		    << " check E = " << vemax
		    << " w = " << maxcell.second << "\n"
		    << " in calo  frame, eta = " << etamax << " phi = " << phimax << "\n"
		    << " in ATLAS frame, eta = " << etaAmax << " phi = " << phiAmax);
    }
  } else {
    ATH_MSG_WARNING("No layer 2 cell with positive energy ! Should never happen");
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}
