/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRACKPARTICLEASSOCIATIONALGS_ITRACKCALOENTRYEXTRAPOLATIONTOOL_H
#define TRACKPARTICLEASSOCIATIONALGS_ITRACKCALOENTRYEXTRAPOLATIONTOOL_H

#include "GaudiKernel/IAlgTool.h"
#include "xAODTracking/TrackParticle.h"

#include <optional>

class EventContext;

/// Position (eta, phi) of a track extrapolated to the calorimeter entrance,
/// together with the angular uncertainties and the (eta, phi) covariance of the extrapolation.
/// All uncertainties/covariances are zero if the extrapolated parameters have no covariance.
struct TrackCaloEntryPoint {
  float eta{0.f};
  float phi{0.f};
  double uncertEta{0.};
  double uncertPhi{0.};
  /// Covariance of the extrapolated (eta, phi), from the (theta, phi) covariance using d eta/d theta = -1/sin(theta)
  double covEtaEta{0.};
  double covPhiPhi{0.};
  double covEtaPhi{0.};
  /// Squared total angular uncertainty of the extrapolation
  double uncertExtrp() const { return uncertEta * uncertEta + uncertPhi * uncertPhi; }
};

/// Interface for tools returning the calorimeter entry point of a track,
/// implemented for both the Trk (CaloExtensionCollection) and ACTS (ActsTrk::CaloExtension) extrapolations.
class ITrackCaloEntryExtrapolationTool : virtual public IAlgTool {
 public:
  DeclareInterfaceID(ITrackCaloEntryExtrapolationTool, 1, 0);

  /// Returns std::nullopt if no usable extrapolation is available for this track
  virtual std::optional<TrackCaloEntryPoint>
  caloEntryPoint(const EventContext& ctx, const xAOD::TrackParticle* track) const = 0;
};

#endif
