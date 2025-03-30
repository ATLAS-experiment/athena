/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#include "DiTauRec/DiTauConstituentFinder.h"

DiTauConstituentFinder::DiTauConstituentFinder(const std::string& type,
		       const std::string& name,
		       const IInterface * parent) :
  DiTauToolBase(type, name, parent),
  m_Rsubjet(0.2)
{
  declareInterface<DiTauToolBase > (this);
  declareProperty("Rsubjet", m_Rsubjet);
}


DiTauConstituentFinder::~DiTauConstituentFinder() = default;


StatusCode DiTauConstituentFinder::initialize() {

  return StatusCode::SUCCESS;
}


StatusCode DiTauConstituentFinder::execute(DiTauCandidateData * data,
                               const EventContext& /*ctx*/) const {

  ATH_MSG_DEBUG("execute DiTauConstituentFinder...");

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

  std::vector<TLorentzVector> subjetConstituentsP4;
  // loop over seed jet constituents
  for (const auto *const seedConst: pSeed->getConstituents()) {
    TLorentzVector tmp_const_p4;
    tmp_const_p4.SetPtEtaPhiM(seedConst->pt(), seedConst->eta(), seedConst->phi(), seedConst->m());
    for (const auto& subjet : vSubjets) {
       TLorentzVector tmp_subjet_p4;
       tmp_subjet_p4.SetPtEtaPhiM(subjet.pt(), subjet.eta(), subjet.phi_std(), subjet.m());
       if (tmp_const_p4.DeltaR(tmp_subjet_p4) < m_Rsubjet) {
        subjetConstituentsP4.push_back(tmp_const_p4);
       }
    }	    
  }

  ATH_MSG_DEBUG("subjetConstituents.size()=" << subjetConstituentsP4.size());

  std::vector<float> vec_f_core(vSubjets.size(), 0);
  for (unsigned int i = 0; i < vSubjets.size(); i++) {
    const fastjet::PseudoJet& subjet = vSubjets.at(i);
    float ptAll = 0.;
    float ptCore = 0.;
    float f_core = 0.;

    TLorentzVector temp_sub_p4;
    temp_sub_p4.SetPtEtaPhiM(subjet.pt(), subjet.eta(), subjet.phi_std(), subjet.m());

    for (TLorentzVector const_p4 : subjetConstituentsP4) {
      // ATH_MSG_DEBUG("dR(const, subjet" << i << ") = " << const_p4.DeltaR(temp_sub_p4) << "const pt = " << const_p4.Pt());
      if (const_p4.DeltaR(temp_sub_p4) < data->Rsubjet) {
        ptAll += const_p4.Pt();
      }

      if (const_p4.DeltaR(temp_sub_p4) < data->Rcore) {
        ptCore += const_p4.Pt();
      }
    }

    if (ptAll != 0.)
      f_core = ptCore/ptAll;
    else
      f_core = -999.;

    ATH_MSG_DEBUG("subjet "<< i << ": f_cluster_core=" << f_core);
    vec_f_core.at(i) = f_core;    
    pDiTau->setfCore(i, f_core);
  }

  const static SG::Accessor<std::vector<float>> mDecor("f_constituents_core");
  mDecor(*pDiTau) = std::move(vec_f_core);
  return StatusCode::SUCCESS;
}
