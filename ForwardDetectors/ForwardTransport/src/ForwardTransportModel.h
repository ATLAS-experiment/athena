/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FORWARDTRANSPORT_FORWARD_TRANSPORT_MODEL_H
#define FORWARDTRANSPORT_FORWARD_TRANSPORT_MODEL_H

#include "G4VFastSimulationModel.hh"
#include "G4Region.hh"
#include "ForwardTransportSvc/IForwardTransportSvc.h"
#include "ForwardTracker/ForwardTrack.h"
#include "GaudiKernel/ServiceHandle.h"

class PrimaryParticleInformation;

class ForwardTransportModel: public G4VFastSimulationModel {

 public:

  ForwardTransportModel(const std::string& name, G4Region* region, const int verboseLevel, const std::string& FwdTrSvcName);

  // methods being inherited from base class
  G4bool IsApplicable(const G4ParticleDefinition&) override final { return true; } // IDLE: we do selection in DoIt method
  G4bool ModelTrigger(const G4FastTrack&)          override final { return true; } // IDLE: we do selection in DoIt method
  void   DoIt        (const G4FastTrack&, G4FastStep&) override final;             // Actual selection and parametrization

private:
  void KillPrimaryTrack(const G4FastTrack&, G4FastStep&);

  PrimaryParticleInformation* getPrimaryParticleInformation(const G4FastTrack& fastTrack) const;

  ServiceHandle<IForwardTransportSvc> m_fwdSvc;
  ForwardTrack             m_fwdTrack;
  ForwardTracker::Particle m_fwdParticle;
  const int                m_verboseLevel{0};
};

#endif //FORWARDTRANSPORT_FORWARD_TRANSPORT_MODEL_H
