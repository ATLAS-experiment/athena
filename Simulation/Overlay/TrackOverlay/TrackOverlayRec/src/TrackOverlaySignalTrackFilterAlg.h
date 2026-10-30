/*
 *   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 *   */

#ifndef TRACKOVERLAYREC_TRACKOVERLAYSIGNALTRACKFILTERALG_H
#define TRACKOVERLAYREC_TRACKOVERLAYSIGNALTRACKFILTERALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

#include "InDetRawData/PixelRDO_Container.h"
#include "InDetRawData/SCT_RDO_Container.h"
#include "AthContainers/ConstDataVector.h"
#include "TrkTrack/TrackCollection.h"

#include <atomic>

class SCT_ID;

namespace TrackOverlay {

class TrackOverlaySignalTrackFilterAlg : public AthReentrantAlgorithm
{
public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;
  virtual StatusCode finalize() override;

private:
  SG::ReadHandleKey<TrackCollection> m_inputKey{this, "InputTracks", "Sig_ResolvedTracks", "Primary-pass resolved tracks"};
  SG::WriteHandleKey<ConstDataVector<TrackCollection>> m_outputKey{this, "OutputTracks", "Sig_ResolvedTracksFiltered", "Tracks with enough signal clusters"};
  // Removed (re-found pileup) tracks: not written out, but used to mark their clusters as used
  // for the later tracking passes, as the corresponding pileup tracks would in MC overlay
  SG::WriteHandleKey<ConstDataVector<TrackCollection>> m_removedKey{this, "RemovedTracks", "Sig_ResolvedTracksRemovedPileup", "Re-found pileup tracks"};
  SG::ReadHandleKey<PixelRDO_Container> m_sigPixelKey{this, "SignalPixelRDOKey", "Sig_PixelRDOs", "Signal-only Pixel RDOs"};
  SG::ReadHandleKey<SCT_RDO_Container> m_sigSCTKey{this, "SignalSCT_RDOKey", "Sig_SCT_RDOs", "Signal-only SCT RDOs"};

  Gaudi::Property<float> m_minSignalFraction{this, "MinSignalClusterFraction", 0.5, "Minimum fraction of Si clusters with a signal RDO"};

  const SCT_ID* m_sctID{nullptr};

  mutable std::atomic<unsigned long> m_nIn{0};
  mutable std::atomic<unsigned long> m_nOut{0};
};

} // namespace TrackOverlay

#endif
