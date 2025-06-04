/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "fastjet/PseudoJet.hh"
#include "JetSubStructureMomentTools/LundVariablesTool.h"
#include "fastjet/tools/Recluster.hh"
#include "fastjet/contrib/SoftDrop.hh"
#include "fastjet/JetDefinition.hh"
#include <iostream>
#include <queue>
#include "xAODPFlow/FlowElement.h"

LundVariablesTool::LundVariablesTool(const std::string& name)
: JetSubStructureMomentToolsBase(name) {}

int LundVariablesTool::modifyJet(xAOD::Jet& injet) const {
  fastjet::PseudoJet jet;
  bool decorate = SetupDecoration(jet, injet);
  int n_splits = 0;
  std::vector<float> lund_all_lnR;
  std::vector<float> lund_all_lnkT;
  std::vector<float> lund_all_z;
  std::vector<float> lund_all_kt;
  std::vector<float> lund_all_deltaR;
  std::vector<int>   lund_all_idp1;
  std::vector<int>   lund_all_idp2;
  
  if (decorate) {

      std::vector<std::vector<LundVariablesTool::Declustering>> v_LundValues;
      auto & constit_links = injet.constituentLinks();
      std::vector<fastjet::PseudoJet> v_pj_constituents;
      
      v_pj_constituents.clear();
      
      for(size_t ij = 0; ij < injet.numConstituents(); ij++) {
        const xAOD::FlowElement* constit = dynamic_cast<const xAOD::FlowElement*>(*constit_links[ij]);
        if (constit) {
          TLorentzVector tlv_const;
          tlv_const.SetPtEtaPhiE(constit->pt() / 1.e3, constit->eta(), constit->phi(), constit->e() / 1.e3);
	  
          v_pj_constituents.push_back(fastjet::PseudoJet(tlv_const.Px(), tlv_const.Py(), tlv_const.Pz(), tlv_const.E()));
	  
        }
        else {
	  ANA_MSG_WARNING("Failed to cast truth particle  ------");
        }
      }
      v_LundValues.push_back(LundVariablesTool::getLundVar(v_pj_constituents));
      

      std::vector<double> v_jj_pt;
      std::vector<double> v_j1_pt;
      std::vector<double> v_j2_pt;
      std::vector<double> v_jj_kt;


      for (auto declust : v_LundValues.at(0)){
          v_jj_pt.push_back( declust.jj.pt() );
          v_j1_pt.push_back( declust.j1.pt() );
          v_j2_pt.push_back( declust.j2.pt() );
          v_jj_kt.push_back( declust.kt );
      }
      
      for (int id=0; id < int(v_jj_pt.size()); id++) {

           int idp1 = -1;
           int idp2 = -1;

           auto itp1 = find(v_jj_pt.begin(), v_jj_pt.end(), v_j1_pt.at(id));
           auto itp2 = find(v_jj_pt.begin(), v_jj_pt.end(), v_j2_pt.at(id));


           if (itp1 != v_jj_pt.end()) {
             idp1 = itp1 - v_jj_pt.begin();

             if (idp1 == id){ // in case second parent has a very very small pT
               idp1 = -1;
             }


           }
           if (itp2 != v_jj_pt.end()) {
             idp2 = itp2 - v_jj_pt.begin();
           }

           v_LundValues.at(0).at(id).idp1 = idp1;
           v_LundValues.at(0).at(id).idp2 = idp2;
      }


      n_splits = 0;
      for (const auto& split : v_LundValues[0]) {
	if (split.delta_R > 1e-8 && split.kt > 1e-8) {
	  // save declustering data
	  lund_all_lnR.push_back(std::log(1.0 / split.delta_R));
	  lund_all_lnkT.push_back(std::log(split.kt));
	  lund_all_z.push_back(split.z);
	  lund_all_kt.push_back(split.kt);
	  lund_all_deltaR.push_back(split.delta_R);
	  lund_all_idp1.push_back(split.idp1);
	  lund_all_idp2.push_back(split.idp2);
	  n_splits++;
	}
      }
  }
  
  
  // Decore the jet with the ljp variables
  injet.setAttribute(m_prefix + "LundAllLnR", lund_all_lnR);
  injet.setAttribute(m_prefix + "LundAllLnKT", lund_all_lnkT);
  injet.setAttribute(m_prefix + "LundAllZ", lund_all_z);
  injet.setAttribute(m_prefix + "LundAllKt", lund_all_kt);
  injet.setAttribute(m_prefix + "LundAllDeltaR", lund_all_deltaR);
  injet.setAttribute(m_prefix + "LundAllIDP1", lund_all_idp1);
  injet.setAttribute(m_prefix + "LundAllIDP2", lund_all_idp2);
  injet.setAttribute(m_prefix + "nSplits", n_splits);

  return 0;
}

void LundVariablesTool::print() const {
  JetSubStructureMomentToolsBase::print();
  ATH_MSG_INFO("Calculating Lund Plane Variables");
}

std::vector<LundVariablesTool::Declustering> LundVariablesTool::getLundVar( std::vector<fastjet::PseudoJet> v_jcs)
{
  fastjet::JetDefinition jd(fastjet::cambridge_algorithm, 1.0);
  std::vector<LundVariablesTool::Declustering> result;
  fastjet::ClusterSequence cs(v_jcs, jd);
  std::vector<fastjet::PseudoJet> v_pj = sorted_by_pt(cs.inclusive_jets());

  fastjet::PseudoJet j = v_pj.at(0);

  std::queue< fastjet::PseudoJet > jetStack;
  jetStack.push(j);

  while ( jetStack.size() > 0 ) {
    fastjet::PseudoJet thisJ, pJLeft, pJRight;
    thisJ = jetStack.front();
    jetStack.pop();

    bool thisJHasParents = thisJ.has_parents(pJLeft, pJRight);

    if (!thisJHasParents){
      continue;
    }

    if (pJLeft.pt2() < pJRight.pt2()) {
      // std::cout << pJLeft.pt2() << "\t" << pJRight.pt2() << std::endl;
      fastjet::PseudoJet jTemp;
      jTemp = pJLeft;
      pJLeft = pJRight;
      pJRight    = jTemp;
    }

    jetStack.push(pJLeft);
    jetStack.push(pJRight);

    LundVariablesTool::Declustering declust;

    declust.jj = thisJ;
    declust.j1 = pJLeft;
    declust.j2 = pJRight;

    // get info about the jet
    declust.pt = thisJ.pt();
    declust.m  = thisJ.m();

    // collect info about the declustering
    declust.pt1     = pJLeft.pt();
    declust.pt2     = pJRight.pt();
    declust.eta     = pJRight.eta();
    declust.E       = pJRight.E();
    declust.delta_R = pJLeft.delta_R(pJRight);
    declust.z       = declust.pt2 / (declust.pt1 + declust.pt2);
    declust.kt      = pJRight.pt() * declust.delta_R;


    declust.varphi = atan2(pJLeft.rap() - pJRight.rap(), pJLeft.delta_phi_to(pJRight));
    result.push_back(declust);
  }



  return result;

}
