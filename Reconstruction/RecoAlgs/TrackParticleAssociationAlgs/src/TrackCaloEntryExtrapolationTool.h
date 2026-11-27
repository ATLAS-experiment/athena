/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRACKPARTICLEASSOCIATIONALGS_TRACKCALOENTRYEXTRAPOLATIONTOOL_H
#define TRACKPARTICLEASSOCIATIONALGS_TRACKCALOENTRYEXTRAPOLATIONTOOL_H

#include "ITrackCaloEntryExtrapolationTool.h"

#include "AthenaBaseComps/AthAlgTool.h"
#include "StoreGate/ReadHandleKey.h"
#include "TrkCaloExtension/CaloExtensionCollection.h"

/// Returns the calo entry point of a track from the Trk::CaloExtension
/// (caloEntryLayerIntersection) stored in a CaloExtensionCollection.
/// IMPORTANT: assumes the CaloExtensionCollection is index-parallel to the track's container.
class TrackCaloEntryExtrapolationTool : public extends<AthAlgTool, ITrackCaloEntryExtrapolationTool> {
 public:
  using base_class::base_class;

  virtual StatusCode initialize() override;
  virtual std::optional<TrackCaloEntryPoint>
  caloEntryPoint(const EventContext& ctx, const xAOD::TrackParticle* track) const override;

 private:
  SG::ReadHandleKey<CaloExtensionCollection> m_caloExtKey {this, "CaloExtensionName", "", "Input Trk CaloExtensionCollection"};
};

#endif
