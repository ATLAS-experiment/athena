/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRACKPARTICLEASSOCIATIONALGS_TRACKCALOENTRYEXTRAPOLATIONACTSTOOL_H
#define TRACKPARTICLEASSOCIATIONALGS_TRACKCALOENTRYEXTRAPOLATIONACTSTOOL_H

#include "ITrackCaloEntryExtrapolationTool.h"

#include "AthenaBaseComps/AthAlgTool.h"
#include "ActsEvent/ContextUtility.h"
#include "StoreGate/ReadDecorHandleKey.h"
#include "xAODTracking/TrackParticleContainer.h"

#include <string>

namespace ActsTrk {
  class CaloExtension;
}

/// Returns the calo entry point of a track from the ActsTrk::CaloExtension linked
/// to the track via the caloExtensionLink decoration.
/// The presampler (PreSamplerB, then PreSamplerE) parameters are used. If neither is present
/// EMB1, then EME1 are used as fall back and a WARNING listing all calo layers hit is printed.
class TrackCaloEntryExtrapolationActsTool : public extends<AthAlgTool, ITrackCaloEntryExtrapolationTool> {
 public:
  using base_class::base_class;

  virtual StatusCode initialize() override;
  virtual std::optional<TrackCaloEntryPoint>
  caloEntryPoint(const EventContext& ctx, const xAOD::TrackParticle* track) const override;

 private:
  /// Builds a string listing every calo sampling with stored parameters in the extension
  static std::string listCaloLayersHit(const ActsTrk::CaloExtension& extension);

  /// The ContextUtility gives access to the Acts geometry context
  ActsTrk::ContextUtility m_ctxProvider{this};

  SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_extensionReadDecorKey{this, "ExtensionDecoration", "InDetTrackParticles.caloExtensionLink"};
};

#endif
