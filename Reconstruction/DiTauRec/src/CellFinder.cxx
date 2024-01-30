/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/


#include "DiTauRec/CellFinder.h"
#include "DiTauRec/DiTauToolBase.h"

#include "DiTauRec/DiTauCandidateData.h"

#include "CaloEvent/CaloClusterContainer.h"
#include "CaloEvent/CaloCell.h"
#include "CaloEvent/CaloCellContainer.h"

#include "fastjet/PseudoJet.hh"

CellFinder::CellFinder(const std::string& type,
		       const std::string& name,
		       const IInterface * parent) :
  DiTauToolBase(type, name, parent),
  m_bWriteJetCells(false),
  m_bWriteSubjetCells(false),
  m_ClusterContainerName("CaloCalTopoClusters"),
  m_CellContainerName("AllCalo"),
  m_Rsubjet(0.2)
{
  declareInterface<DiTauToolBase > (this);
  declareProperty("writeJetCells", m_bWriteJetCells);
  declareProperty("writeSubjetCells", m_bWriteSubjetCells);
  declareProperty("ClusterContainer", m_ClusterContainerName);
  declareProperty("CellContainer", m_CellContainerName);
  declareProperty("Rsubjet", m_Rsubjet);
}


CellFinder::~CellFinder() = default;


StatusCode CellFinder::initialize() {

  return StatusCode::SUCCESS;
}


StatusCode CellFinder::execute(DiTauCandidateData * data,
                               const EventContext& /*ctx*/) const {

  ATH_MSG_DEBUG("execute CellFinder...");

  // get ditau and its seed jet

  xAOD::DiTauJet* pDiTau = data->xAODDiTau;
  if (!pDiTau) {
    ATH_MSG_ERROR("no di-tau candidate given");
    return StatusCode::FAILURE;
  }

  const xAOD::Jet* pSeed = data->seed;
  if (!pSeed) {
    ATH_MSG_WARNING("No jet seed given.");
    return StatusCode::FAILURE;
  }

  std::vector<fastjet::PseudoJet> vSubjets = data->subjets;
  if (vSubjets.empty()) {
    ATH_MSG_WARNING("No subjets given. Continue without cell information.");
    return StatusCode::SUCCESS;
  }

  // get clusters linked to the seed jet. Loop over clusters to get linked cells

  std::bitset<200000> cellSeen;
  std::vector<const CaloCell*> subjetCells;

  // loop over seed jet constituents
  for (const auto *const seedConst: pSeed->getConstituents()) {
    // cast jet constituent to cluster object
    const xAOD::CaloCluster* cluster = dynamic_cast<const xAOD::CaloCluster*>( seedConst->rawConstituent() );

    // loop over cells which are linked to the cluster
    for (const auto *const cc : *(cluster->getCellLinks())) {
      // skip if pt<0 or cell already encountered
      if (cc->pt() < 0) continue;
      if (cellSeen.test(cc->caloDDE()->calo_hash())) continue;
      // register cell hash as already seen
      cellSeen.set(cc->caloDDE()->calo_hash());            

      TLorentzVector temp_cc_p4;
      temp_cc_p4.SetPtEtaPhiM(cc->pt(), cc->eta(), cc->phi(), cc->m());

      // check if cell is in one of the subjets cones
      for (const auto& subjet : vSubjets) {
        TLorentzVector temp_sub_p4;
        temp_sub_p4.SetPtEtaPhiM(subjet.pt(), subjet.eta(), subjet.phi_std(), subjet.m());        
	if (temp_cc_p4.DeltaR(temp_sub_p4) < m_Rsubjet) {
	  subjetCells.push_back(cc);
	}
      }
    }
  }

  ATH_MSG_DEBUG("subjetCells.size()=" << subjetCells.size());
  data->subjetCells = subjetCells;

  return StatusCode::SUCCESS;
}
