/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TrackCaloEntryExtrapolationTool.h"

#include "StoreGate/ReadHandle.h"
#include "TrkCaloExtension/CaloExtension.h"
#include "TrkParameters/TrackParameters.h"

#include <cmath>

StatusCode TrackCaloEntryExtrapolationTool::initialize()
{
  ATH_CHECK( m_caloExtKey.initialize() );
  return StatusCode::SUCCESS;
}

std::optional<TrackCaloEntryPoint>
TrackCaloEntryExtrapolationTool::caloEntryPoint(const EventContext& ctx, const xAOD::TrackParticle* track) const
{
  SG::ReadHandle<CaloExtensionCollection> caloExts(m_caloExtKey, ctx);

  // IMPORTANT : this assumes a correspondance between the TrackParticleContainer and the CaloExtensionCollection !
  const Trk::CaloExtension* caloExtension = (*caloExts)[track->index()];
  if (caloExtension == nullptr) {
    ATH_MSG_DEBUG(" Track " << track->index() << " has no caloExtension ");
    return std::nullopt;
  }

  const Trk::TrackParameters* pars = caloExtension->caloEntryLayerIntersection();
  if (!pars) {
    ATH_MSG_WARNING(" NO TrackParameters caloExtension.caloEntryLayerIntersection() ");
    return std::nullopt;
  }

  TrackCaloEntryPoint entry;
  entry.eta = pars->position().eta();
  entry.phi = pars->position().phi();
  if (const auto* cov = pars->covariance()) {
    const double sinTheta = std::sin(pars->position().theta());
    entry.uncertEta = std::sqrt((*cov)(Trk::theta, Trk::theta)) / sinTheta;
    entry.uncertPhi = std::sqrt((*cov)(Trk::phi, Trk::phi));
    // eta = -ln tan(theta/2) => d eta/d theta = -1/sin(theta)
    entry.covEtaEta = (*cov)(Trk::theta, Trk::theta) / (sinTheta * sinTheta);
    entry.covPhiPhi = (*cov)(Trk::phi, Trk::phi);
    entry.covEtaPhi = -(*cov)(Trk::theta, Trk::phi) / sinTheta;
  }
  return entry;
}
