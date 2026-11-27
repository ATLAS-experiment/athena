/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TrackCaloEntryExtrapolationActsTool.h"

#include "ActsEvent/CaloExtension.h"
#include "CaloGeoHelpers/CaloSampling.h"
#include "PFlowUtils/ActsCaloParametersChecker.h"
#include "StoreGate/ReadDecorHandle.h"

#include <array>
#include <cmath>
#include <sstream>

StatusCode TrackCaloEntryExtrapolationActsTool::initialize()
{
  ATH_CHECK( m_ctxProvider.initialize() );
  ATH_CHECK( m_extensionReadDecorKey.initialize() );
  return StatusCode::SUCCESS;
}

std::string TrackCaloEntryExtrapolationActsTool::listCaloLayersHit(const ActsTrk::CaloExtension& extension)
{
  std::ostringstream layers;
  bool first = true;
  for (int i = CaloSampling::PreSamplerB; i < CaloSampling::Unknown; ++i) {
    const auto sample = static_cast<CaloSampling::CaloSample>(i);
    const std::size_t nPars = extension.caloParameters(sample).size();
    if (nPars == 0) continue;
    if (!first) layers << ", ";
    layers << CaloSampling::getSamplingName(sample) << " (" << nPars << ")";
    first = false;
  }
  if (first) layers << "none";
  return layers.str();
}

std::optional<TrackCaloEntryPoint>
TrackCaloEntryExtrapolationActsTool::caloEntryPoint(const EventContext& ctx, const xAOD::TrackParticle* track) const
{
  SG::ReadDecorHandle<xAOD::TrackParticleContainer, ElementLink<ActsTrk::CaloExtensionContainer>> extensionReadDecorHandle(m_extensionReadDecorKey, ctx);
  const ElementLink<ActsTrk::CaloExtensionContainer>& extensionLink = extensionReadDecorHandle(*track);

  if (!extensionLink.isValid()) {
    ATH_MSG_DEBUG(" Track " << track->index() << " has no valid ACTS CaloExtension link ");
    return std::nullopt;
  }
  const ActsTrk::CaloExtension& extension = **extensionLink;

  // Preferred: the presampler. Fall back: the first EM layer.
  constexpr std::array<CaloSampling::CaloSample, 4> samplesInOrder{CaloSampling::PreSamplerB, CaloSampling::PreSamplerE,
                                                                   CaloSampling::EMB1, CaloSampling::EME1};

  std::vector<Acts::BoundTrackParameters> caloPars;
  CaloSampling::CaloSample selectedSample = CaloSampling::Unknown;
  for (CaloSampling::CaloSample sample : samplesInOrder) {
    caloPars = extension.caloParameters(sample);
    if (!caloPars.empty()) {
      selectedSample = sample;
      ATH_MSG_DEBUG("Using parameters in " << CaloSampling::getSamplingName(sample));
      break;
    }
  }

  if (caloPars.empty()) {
    ATH_MSG_WARNING("Track with pt, eta, phi " << track->pt() << ", " << track->eta() << ", " << track->phi()
                    << " has no PreSamplerB/E or EMB1/EME1 parameters, no calo entry point available."
                    << " Calo layers hit (number of parameters): " << listCaloLayersHit(extension));
    return std::nullopt;
  }

  const Acts::GeometryContext gctx = m_ctxProvider.getGeometryContext(ctx);
  checkMultipleActsCaloParameters(caloPars, selectedSample, gctx, msg());
  const Acts::BoundTrackParameters& pars = caloPars.front();
  const Acts::Vector3 position = pars.position(gctx);

  TrackCaloEntryPoint entry;
  entry.eta = position.eta();
  entry.phi = position.phi();
  if (pars.covariance()) {
    const auto& cov = *pars.covariance();
    const double sinTheta = std::sin(position.theta());
    entry.uncertEta = std::sqrt(cov(Acts::eBoundTheta, Acts::eBoundTheta)) / sinTheta;
    entry.uncertPhi = std::sqrt(cov(Acts::eBoundPhi, Acts::eBoundPhi));
    // eta = -ln tan(theta/2) => d eta/d theta = -1/sin(theta)
    entry.covEtaEta = cov(Acts::eBoundTheta, Acts::eBoundTheta) / (sinTheta * sinTheta);
    entry.covPhiPhi = cov(Acts::eBoundPhi, Acts::eBoundPhi);
    entry.covEtaPhi = -cov(Acts::eBoundTheta, Acts::eBoundPhi) / sinTheta;
  }
  return entry;
}
