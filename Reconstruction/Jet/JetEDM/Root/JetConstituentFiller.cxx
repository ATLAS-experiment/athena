/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// JetConstituentFiller.cxx

#include "JetEDM/JetConstituentFiller.h"
#include <map>
#include "JetEDM/IConstituentUserInfo.h"
#include "JetEDM/IndexedTConstituentUserInfo.h"
#include "JetEDM/LabelIndex.h"
#include "xAODJet/JetContainer.h"
#include "fastjet/PseudoJet.hh"
#include "xAODMuon/MuonSegmentContainer.h"
#include <iostream>

using PseudoJetVector = std::vector<fastjet::PseudoJet>;
using xAOD::IParticle;
using jet::IConstituentUserInfo;
using Label = IConstituentUserInfo::Label;
using ParticleVector = std::vector<const IParticle *>;
using ParticleMap = std::vector<ParticleVector>;
using PtMap = std::vector<double>;
using jet::JetConstituentFiller;
using xAOD::MuonSegment;
using jet::IndexedTConstituentUserInfo;
using MuonSegmentVector = std::vector<const MuonSegment *>;
using MuonSegmentCUI = IndexedTConstituentUserInfo<MuonSegment>;
using MuonSegmentMap = std::vector<MuonSegmentVector>;

//**********************************************************************

namespace {
  enum ParType { UNDEF, IPART, MUSEG };
}
using ParTypeVector = std::vector<ParType>;

//**********************************************************************

int JetConstituentFiller::
extractConstituents(xAOD::Jet& jet, const NameList* pghostlabs,
                    const fastjet::PseudoJet* ppj2) {

  static const SG::AuxElement::Accessor<const fastjet::PseudoJet*> pjAccessor("PseudoJet");
  const fastjet::PseudoJet* ppseudojet = nullptr; 
  if(pjAccessor.isAvailable(jet)) ppseudojet = pjAccessor(jet);

  if ( ppseudojet == nullptr ) {
    ppseudojet = ppj2;
  }
  if ( ppseudojet == nullptr ) return -1;
  int nbad = 0;
  const PseudoJetVector& cons = ppseudojet->constituents();
  ParticleMap out;
  MuonSegmentMap outms;
  PtMap ptout;
  ParTypeVector partypes;
  const LabelIndex* pli = nullptr;
  for ( PseudoJetVector::const_iterator icon=cons.begin(); icon!=cons.end(); ++icon ) {
    if ( icon->has_user_info<IConstituentUserInfo>() ) {
      const IConstituentUserInfo& cui = icon->user_info<IConstituentUserInfo>();
      // Initialize with the first constituent.
      if ( pli == nullptr ) {
        pli = cui.labelMap();
        if ( pli == nullptr ) return -2;
        unsigned int maxli = pli->size() + 1;  // +1 because LabelIndex starts at 1;
        out.resize(maxli);  // +1 because LabelIndex starts at 1
        outms.resize(maxli);
        for ( unsigned int ili=0; ili<maxli; ++ili ) {
          ptout.push_back(0.0);
        }
        for ( ParType& partype : partypes ) partype = UNDEF;
        for ( unsigned int idx=0; idx<maxli; ++idx ) {
          std::string lab = pli->label(idx);
          if ( lab.empty() ) {
            partypes.push_back(UNDEF);
            if ( idx ) return -3;
          } else if ( lab.find("MuonSegment") != std::string::npos ) {
            partypes.push_back(MUSEG);
          } else {
            partypes.push_back(IPART);
          }
        }
      }
      if ( pli != cui.labelMap() ) return -4;
      const IParticle* ppar = cui.particle();
      const MuonSegment* pms = nullptr;
      // If this is not an IParticle, find the full type.
      unsigned int icui = cui.index();
      ParType partype = partypes[icui];
      if ( ppar == nullptr ) {
        const MuonSegmentCUI* pmscui = dynamic_cast<const MuonSegmentCUI*>(&cui);
        if ( pmscui == nullptr ) return -5;
        pms = pmscui->object();
        if ( partype != MUSEG ) return -6;
      } else {
        if ( partype != IPART ) return -7;
      }
      Label lab = cui.label();
      // Add to constituent list or associated object list.
      if ( ! cui.isGhost() ) {
        if (!ppar) {
          ++nbad;
        }
        else {
          jet.addConstituent(ppar);
          jet.setConstituentsSignalState( cui.constitScale() );
        }
      } else {
        if ( partype == MUSEG ) {
          outms[icui].push_back(pms); 
        } else if ( partype == IPART ) {
          out[icui].push_back(ppar); 
        } 
      }
    } else {
      ++nbad;
    }
  }
  
  // Set ghost associated particles:
  if (pli){
    for ( size_t i=1; i<out.size(); ++i ) {
      if ( pghostlabs) {
        const NameList& ghostlabs = *pghostlabs;
        if ( find(ghostlabs.begin(), ghostlabs.end(), pli->label(i)) == ghostlabs.end() ) {
          nbad += out[i].size();
          continue;
        }
      }
      ParType& partype = partypes[i];
      std::string cname = pli->label(i) + "Count";
      std::string ptname = pli->label(i) + "Pt";
      // Check if this is in the parent jet
      int count_test; // dummy var to retrieve into -- we don't care about the value
      const static SG::AuxElement::ConstAccessor<ElementLink<xAOD::JetContainer> > cacc_parent("Parent");
      if(!m_isTrigger) {
        if(cacc_parent.isAvailable(jet) && cacc_parent(jet).isValid()) {
          if(!(*cacc_parent(jet))->getAttribute(cname,count_test)) {
            nbad += out[i].size(); // Skip if the parent does not have this
            continue;
          }
        }
      }
      if ( partype == MUSEG ) {
        // Record associated muons.
        jet.setAssociatedObjects(pli->label(i) , outms[i]);
        jet.setAttribute<int>(cname, outms[i].size());
      } else if ( partype == IPART ) {
        // Record associated particles.
        jet.setAssociatedObjects(pli->label(i), out[i]);
        jet.setAttribute<int>(cname, out[i].size());
        jet.setAttribute<float>(ptname, ptout[i]);
        if ( ! outms[i].empty() ) return -9;
      } else {
        return -10;
      }
    }
  }
  return nbad;
}

int JetConstituentFiller::extractConstituents(xAOD::Jet& jet, const fastjet::PseudoJet* ppj2) {
  return extractConstituents(jet, nullptr, ppj2);
}

//**********************************************************************

PseudoJetVector JetConstituentFiller::constituentPseudoJets(const xAOD::Jet& jet, bool ignoreGhosts, bool requireJetStructure){

  static const SG::AuxElement::Accessor<const fastjet::PseudoJet*> pjAccessor("PseudoJet");
  const fastjet::PseudoJet* jet_pj = nullptr;
  if(pjAccessor.isAvailable(jet)) jet_pj = pjAccessor(jet);

  PseudoJetVector constituents;

  if(jet_pj && !ignoreGhosts ){ // Retrieve constituents from the PseudoJet
    constituents = jet_pj->constituents(); // all constituents, including ghosts
    // Removed block for (existing jet_pt and ignoreGhosts), because
    // in the PseudoJetContainer model, the constituent user info is
    // not attached, and hence it's not possible to know if a pj is
    // ghost or constituent.
  } else { // no PseudoJet in jet or need to reject ghosts. Build them from constituents.

    // For SoftDrop variables like zg, rg, the structure of the jet is needed and
    // constituents have to be retrieved from the Pseudojet
    if(jet_pj && requireJetStructure){
      PseudoJetVector constituents_all = jet_pj->constituents(); // all constituents, including ghosts
      //Filter out ghosts before calculating variables (comparison to constituents)
      xAOD::JetConstituentVector constituents_tmp = jet.getConstituents();
      constituents.reserve( jet.numConstituents() );
      for(size_t i = 0; i < constituents_all.size(); i++){
	for(size_t j = 0; j < constituents_tmp.size(); j++){
	  if(std::abs((constituents_all[i].px()-constituents_tmp[j].Px())/constituents_tmp[j].Px()) < 0.0001 &&
	     std::abs((constituents_all[i].py()-constituents_tmp[j].Py())/constituents_tmp[j].Py()) < 0.0001 && 
	     std::abs((constituents_all[i].pz()-constituents_tmp[j].Pz())/constituents_tmp[j].Pz()) < 0.0001){
	    constituents.push_back(constituents_all[i]);
	    break;
	  }
	}
      }
    }
    else{
      xAOD::JetConstituentVector constituents_tmp = jet.getConstituents();
      constituents.reserve( jet.numConstituents() );
      for(size_t i = 0; i < constituents_tmp.size(); i++){
	constituents.emplace_back( constituents_tmp[i].Px(), constituents_tmp[i].Py(), constituents_tmp[i].Pz(), constituents_tmp[i].E() );
      }
    }
  }
  return constituents;
}
//**********************************************************************

