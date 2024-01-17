/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "tauRecTools/HelperFunctions.h"

#include <TObjString.h>
#include <TObjArray.h>
#include <TFile.h>
#include <TTree.h>

#include <iostream>

namespace tauRecTools {
  ANA_MSG_SOURCE(msgHelperFunction, "HelperFunction")
}



const xAOD::Vertex* tauRecTools::getJetVertex(const xAOD::Jet& jet) {
  const xAOD::Vertex* jetVertex = nullptr;
  
  bool isAvailable = jet.getAssociatedObject("OriginVertex", jetVertex); 
  if (! isAvailable) {
    return nullptr;
  }

  return jetVertex;
}



TLorentzVector tauRecTools::getTauAxis(const xAOD::TauJet& tau, bool doVertexCorrection) {
  TLorentzVector tauAxis;
  if (doVertexCorrection) {
    tauAxis = tau.p4(xAOD::TauJetParameters::IntermediateAxis);
  }
  else {
    tauAxis = tau.p4(xAOD::TauJetParameters::DetectorAxis);
  }
 
  return tauAxis;
}

//________________________________________________________________________________
xAOD::TauTrack::TrackFlagType tauRecTools::isolateClassifiedBits(xAOD::TauTrack::TrackFlagType flag){
  const int flagsize=sizeof(flag)*8;
  flag=flag<<(flagsize-xAOD::TauJetParameters::classifiedFake-1);
  flag=flag>>(flagsize-xAOD::TauJetParameters::classifiedCharged+1);
  return flag;
}

//________________________________________________________________________________
bool tauRecTools::sortTracks(const ElementLink<xAOD::TauTrackContainer> &l1, const ElementLink<xAOD::TauTrackContainer> &l2)
{
  //should we be safe and ask if the links are available?
  const xAOD::TauTrack* xTrack1 = *l1;
  const xAOD::TauTrack* xTrack2 = *l2;

  //return classified charged, then isolation (wide tracks), else by pt
  xAOD::TauTrack::TrackFlagType f1 = isolateClassifiedBits(xTrack1->flagSet());
  xAOD::TauTrack::TrackFlagType f2 = isolateClassifiedBits(xTrack2->flagSet());

  if(f1==f2)
    return xTrack1->pt()>xTrack2->pt();
  return f1<f2;
}

//________________________________________________________________________________
TLorentzVector tauRecTools::GetConstituentP4(const xAOD::JetConstituent& constituent) {
  using namespace tauRecTools::msgHelperFunction;

  TLorentzVector constituentP4; 

  if( constituent->type() == xAOD::Type::CaloCluster ) {
    const xAOD::CaloCluster* cluster = static_cast<const xAOD::CaloCluster*>( constituent->rawConstituent() );
    constituentP4 = cluster->p4();
  }
  else if ( constituent->type() == xAOD::Type::FlowElement ) {
    const xAOD::FlowElement* fe = static_cast<const xAOD::FlowElement*>( constituent->rawConstituent() );
    constituentP4 = fe->p4();
  }
  else {
    ANA_MSG_ERROR("GetConstituentP4: Seed jet constituent type not supported!");
    constituentP4.SetPtEtaPhiE(constituent.pt(), constituent.eta(), constituent.phi(), constituent.e());
  }

  return constituentP4;
}



bool tauRecTools::doPi0andShots(const xAOD::TauJet& tau) {
  // build pi0s and shots for 0-5p taus
  return ( tau.nTracks() <= 5 );
}
