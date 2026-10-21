/*
 *   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 *   */

#include "TrackOverlaySignalTrackFilterAlg.h"

#include "InDetIdentifier/SCT_ID.h"
#include "InDetPrepRawData/SiCluster.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"
#include "TrkRIO_OnTrack/RIO_OnTrack.h"
#include "TrkTrack/Track.h"
#include "TrkTrack/TrackStateOnSurface.h"

#include <unordered_set>

namespace TrackOverlay {

StatusCode TrackOverlaySignalTrackFilterAlg::initialize()
{
  ATH_CHECK(m_inputKey.initialize());
  ATH_CHECK(m_outputKey.initialize());
  ATH_CHECK(m_removedKey.initialize());
  ATH_CHECK(m_sigPixelKey.initialize());
  ATH_CHECK(m_sigSCTKey.initialize());
  ATH_CHECK(detStore()->retrieve(m_sctID, "SCT_ID"));
  return StatusCode::SUCCESS;
}

StatusCode TrackOverlaySignalTrackFilterAlg::execute(const EventContext& ctx) const
{
  SG::ReadHandle<TrackCollection> input(m_inputKey, ctx);
  SG::ReadHandle<PixelRDO_Container> sigPixel(m_sigPixelKey, ctx);
  SG::ReadHandle<SCT_RDO_Container> sigSCT(m_sigSCTKey, ctx);
  if (!input.isValid() || !sigPixel.isValid() || !sigSCT.isValid()) {
    ATH_MSG_ERROR("Missing input tracks or signal RDO containers");
    return StatusCode::FAILURE;
  }

  // Identifiers of all signal pixels and strips (SCT RDOs are expanded over their strip group)
  std::unordered_set<Identifier::value_type> signalIds;
  for (const auto& [hash, coll] : sigPixel->GetAllHashPtrPair()) {
    for (const PixelRDORawData* rdo : *coll) signalIds.insert(rdo->identify().get_compact());
  }
  for (const auto& [hash, coll] : sigSCT->GetAllHashPtrPair()) {
    for (const SCT_RDORawData* rdo : *coll) {
      const Identifier first = rdo->identify();
      const Identifier wafer = m_sctID->wafer_id(first);
      const int strip0 = m_sctID->strip(first);
      for (int i = 0; i < rdo->getGroupSize(); ++i) {
        signalIds.insert(m_sctID->strip_id(wafer, strip0 + i).get_compact());
      }
    }
  }

  auto output = std::make_unique<ConstDataVector<TrackCollection>>(SG::VIEW_ELEMENTS);
  output->reserve(input->size());
  auto removed = std::make_unique<ConstDataVector<TrackCollection>>(SG::VIEW_ELEMENTS);
  for (const Trk::Track* track : *input) {
    unsigned int nSi = 0;
    unsigned int nSignal = 0;
    const Trk::TrackStates* states = track->trackStateOnSurfaces();
    if (!states) { output->push_back(track); continue; }
    for (const Trk::TrackStateOnSurface* tsos : *states) {
      const auto* rot = dynamic_cast<const Trk::RIO_OnTrack*>(tsos->measurementOnTrack());
      if (!rot) continue;
      const auto* cluster = dynamic_cast<const InDet::SiCluster*>(rot->prepRawData());
      if (!cluster) continue;
      ++nSi;
      for (const Identifier& id : cluster->rdoList()) {
        if (signalIds.count(id.get_compact())) {
          ++nSignal;
          break;
        }
      }
    }
    if (nSi == 0 || nSignal >= m_minSignalFraction.value() * nSi) output->push_back(track);
    else removed->push_back(track);
  }

  m_nIn += input->size();
  m_nOut += output->size();
  SG::WriteHandle<ConstDataVector<TrackCollection>> outHandle(m_outputKey, ctx);
  ATH_CHECK(outHandle.record(std::move(output)));
  SG::WriteHandle<ConstDataVector<TrackCollection>> removedHandle(m_removedKey, ctx);
  ATH_CHECK(removedHandle.record(std::move(removed)));
  return StatusCode::SUCCESS;
}

StatusCode TrackOverlaySignalTrackFilterAlg::finalize()
{
  const unsigned long nIn = m_nIn.load();
  const unsigned long nOut = m_nOut.load();
  ATH_MSG_INFO("Kept " << nOut << " of " << nIn << " primary-pass tracks ("
               << (nIn - nOut) << " pileup tracks re-found inside RoIs removed)");
  return StatusCode::SUCCESS;
}

} // namespace TrackOverlay
