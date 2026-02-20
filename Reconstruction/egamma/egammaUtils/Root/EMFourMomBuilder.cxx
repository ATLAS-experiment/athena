/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
 */

#include "egammaUtils/EMFourMomBuilder.h"
#include "xAODEgamma/EgammaxAODHelpers.h"
#include "EventPrimitives/EventPrimitives.h"
#include "TruthUtils/ParticleConstants.h"

namespace {
constexpr float el_mass = ParticleConstants::electronMassInMeV;

void
setFromCluster(xAOD::Egamma& eg)
{

  const xAOD::CaloCluster* cluster = eg.caloCluster();
  const float eta = cluster->eta();
  const float phi = cluster->phi();
  const float E = cluster->e();
  if (eg.type() == xAOD::Type::Electron) {
    const double pt =
      E > el_mass ? sqrt(E * E - el_mass * el_mass) / cosh(eta) : 0;
    eg.setPtEtaPhi(pt, eta, phi);
  } else {
    eg.setPtEtaPhi(E / cosh(eta), eta, phi);
  }
}

void
setFromTrkCluster(xAOD::Electron& el)
{

  const xAOD::CaloCluster* cluster = el.caloCluster();
  const xAOD::TrackParticle* trackParticle = el.trackParticle();

  bool goodTrack = (xAOD::EgammaHelpers::numberOfSiHits(trackParticle) >= 4);
  const float E = cluster->e();
  const float eta = goodTrack ? trackParticle->eta() : cluster->eta();
  const float phi = goodTrack ? trackParticle->phi() : cluster->phi();

  const double pt =
    E > el_mass ? sqrt(E * E - el_mass * el_mass) / cosh(eta) : 0;
  el.setPtEtaPhi(pt, eta, phi);
}

void
setFromTrkCluster(xAOD::Photon& ph)
{
  const xAOD::CaloCluster* cluster = ph.caloCluster();
  float E = cluster->e();
  float eta = cluster->eta();
  float phi = cluster->phi();
  Amg::Vector3D momentumAtVertex = xAOD::EgammaHelpers::momentumAtVertex(&ph);
  if (momentumAtVertex.mag() > 1e-5) { // protection against p = 0
    eta = momentumAtVertex.eta();
    phi = momentumAtVertex.phi();
  }
  ph.setPtEtaPhi(E / cosh(eta), eta, phi);
}
}

/////////////////////////////////////////////////////////////////

namespace EMFourMomBuilder
{
void
calculate(xAOD::Electron& electron) {
  if (electron.trackParticle()) {
    return setFromTrkCluster(electron);
  } else {
    setFromCluster(electron);
  }
}

void
calculate(xAOD::Photon& photon) {
  if (xAOD::EgammaHelpers::conversionType(&photon) ==
      xAOD::EgammaParameters::doubleSi) {
    setFromTrkCluster(photon);
  } else {
    setFromCluster(photon);
  }
}
}

