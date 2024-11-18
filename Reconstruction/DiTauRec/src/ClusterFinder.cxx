/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/


#include "DiTauRec/ClusterFinder.h"
#include "DiTauRec/DiTauToolBase.h"
#include "DiTauRec/DiTauCandidateData.h"
#include "CaloEvent/CaloClusterContainer.h"
#include "fastjet/PseudoJet.hh"

ClusterFinder::ClusterFinder(const std::string& type,
		       const std::string& name,
		       const IInterface * parent) :
  DiTauToolBase(type, name, parent),
  m_ClusterContainerName("CaloCalTopoClusters"),
  m_Rsubjet(0.2)
{
  declareInterface<DiTauToolBase > (this);
  declareProperty("ClusterContainer", m_ClusterContainerName);
  declareProperty("Rsubjet", m_Rsubjet);
}


ClusterFinder::~ClusterFinder() = default;


StatusCode ClusterFinder::initialize() {

  return StatusCode::SUCCESS;
}


StatusCode ClusterFinder::execute(DiTauCandidateData * data,
                               const EventContext& /*ctx*/) const {

  ATH_MSG_DEBUG("execute ClusterFinder...");

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
    ATH_MSG_WARNING("No subjets given. Continue without cluster information.");
    return StatusCode::SUCCESS;
  }

  std::vector<const xAOD::CaloCluster*> subjetClusters;
  // loop over seed jet constituents
  for (const auto *const seedConst: pSeed->getConstituents()) {

    // cast jet constituent to cluster object
    const xAOD::CaloCluster* cluster = dynamic_cast<const xAOD::CaloCluster*>( seedConst->rawConstituent() );
    if(cluster == NULL)  continue;

    for (const auto& subjet : vSubjets) {
       TLorentzVector temp_sub_p4;
       temp_sub_p4.SetPtEtaPhiM(subjet.pt(), subjet.eta(), subjet.phi_std(), subjet.m());
       if (cluster->p4().DeltaR(temp_sub_p4) < m_Rsubjet) {
          subjetClusters.push_back(cluster);
       }
    }	    
  }

  ATH_MSG_DEBUG("subjetCluster.size()=" << subjetClusters.size());

  std::vector<float> vec_f_core(vSubjets.size(), 0);
  for (unsigned int i = 0; i < vSubjets.size(); i++) {
    const fastjet::PseudoJet& subjet = vSubjets.at(i);
    float ptAll = 0.;
    float ptCore = 0.;
    float f_core = 0.;

    TLorentzVector temp_sub_p4;
    temp_sub_p4.SetPtEtaPhiM(subjet.pt(), subjet.eta(), subjet.phi_std(), subjet.m());

    for (const auto& cc : subjetClusters) {

      if (cc->p4().DeltaR(temp_sub_p4) < data->Rsubjet) {
        ptAll += cc->pt();
      }

      if (cc->p4().DeltaR(temp_sub_p4) < data->Rcore) {
        ptCore += cc->pt();
      }
    }

    if (ptAll != 0.)
      f_core = ptCore/ptAll;
    else
      f_core = -999.;

    ATH_MSG_DEBUG("subjet "<< i << ": f_cluster_core=" << f_core);
    vec_f_core.at(i) = f_core;    
  }

  const static SG::Decorator<std::vector<float>> mDecor("f_cluster_core");
  mDecor(*pDiTau) = std::move(vec_f_core);

  return StatusCode::SUCCESS;
}
