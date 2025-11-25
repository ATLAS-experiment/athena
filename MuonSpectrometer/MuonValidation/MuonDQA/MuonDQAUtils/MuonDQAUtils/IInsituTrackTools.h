/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// IInsituTrackTools.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
#ifndef IMSINSITUMSINSITUTOOLS_H
#define IMSINSITUMSINSITUTOOLS_H 

#include "GaudiKernel/IAlgTool.h"

class INavigable4Momentum;

static const InterfaceID IID_IInsituTrackTools("IInsituTrackTools", 1, 0);

class IInsituTrackTools : virtual public IAlgTool {
 public:
  static const InterfaceID& interfaceID( ) ;

  // enter declaration of your interface-defining member functions here
  virtual	bool	isZBosonCandidate(const INavigable4Momentum *track1, const INavigable4Momentum *track2) = 0;
  virtual	bool	isTriggeredMuon(const INavigable4Momentum *track1) = 0;
  virtual	bool	isTriggeredElectron(const INavigable4Momentum *track1) = 0;
  virtual	bool	isCloseTrack(const INavigable4Momentum *track1, const INavigable4Momentum *track2) = 0;
  virtual	bool	isIsolatedTrack(const INavigable4Momentum *track1) = 0;
  virtual	bool	isIsolatedMuon(const INavigable4Momentum *track1) = 0;
  virtual	bool	isElectronCandidate(const INavigable4Momentum *track1) = 0;
  virtual	double	getInvariantMass(const INavigable4Momentum *track1, const INavigable4Momentum *track2) = 0;
  virtual	double	getDistance(const INavigable4Momentum *track1, const INavigable4Momentum *track2) = 0;
  virtual	bool	getTrackIsolation(const INavigable4Momentum *trackParticle, float &PtIsolation, int &NIsolation) = 0;
  virtual	float	getTrackPtIsolation(const INavigable4Momentum *trackParticle) = 0;
  virtual	int	    getNTrackIsolation(const INavigable4Momentum *trackParticle) = 0;
  virtual	float	getJetIsolation(const INavigable4Momentum *trackParticle) = 0;

};

inline const InterfaceID& IInsituTrackTools::interfaceID()
{ return IID_IInsituTrackTools; }

#endif 
