/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef __ITRIGMUONBACKEXTRAPOLATOR_H__ 
#define __ITRIGMUONBACKEXTRAPOLATOR_H__ 

#include "GaudiKernel/IAlgTool.h"

#include "xAODTrigMuon/L2StandAloneMuon.h"

static const InterfaceID IID_ITrigMuonBackExtrapolator("ITrigMuonBackExtrapolator", 1 , 0); 

class ITrigMuonBackExtrapolator: virtual public IAlgTool 
{
 public:

  static const InterfaceID& interfaceID() {
    return IID_ITrigMuonBackExtrapolator;
  }

  // extrapolate without using the ID vertex measurement
  virtual 
  StatusCode give_eta_phi_at_vertex (const xAOD::L2StandAloneMuon*, // input muon track
				     double& extEta,                // vertex eta
				     double& sigmaEta,              // sigma vertex eta
				     double& extPhi,                // vertex phi
				     double& sigmaPhi,              // sigma vertex phi
				     double PT) const = 0;          // PT of the window

  virtual
  StatusCode give_eta_phi_at_vertex (double pt,                     // pt of muon track
                                     const xAOD::L2StandAloneMuon*, // input muon track
				     double& extEta,                // vertex eta
				     double& sigmaEta,              // sigma vertex eta
				     double& extPhi,                // vertex phi
				     double& sigmaPhi,              // sigma vertex phi
				     double PT) const = 0;          // PT of the window

  // extrapolate using the ID vertext measurement
  virtual 
  StatusCode give_eta_phi_at_vertex( const xAOD::L2StandAloneMuon*, // input muon track
                                     double ZetaID,                 // Z vertex from ID
				     double& extEta,                // vertex eta
				     double& sigmaEta,              // sigma vertex eta
				     double& extPhi,                // vertex phi
				     double& sigmaPhi,              // sigma vertex phi
				     double PT) const = 0;          // PT of the window

  virtual 
  StatusCode give_eta_phi_at_vertex( double pt,                     // pt of muon track
                                     const xAOD::L2StandAloneMuon*, // input muon track
                                     double ZetaID,                 // Z vertex from ID
				     double& extEta,                // vertex eta
				     double& sigmaEta,              // sigma vertex eta
				     double& extPhi,                // vertex phi
				     double& sigmaPhi,              // sigma vertex phi
				     double PT) const = 0;          // PT of the window
 
};

#endif


