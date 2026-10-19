/*
 *   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 *   */

#include "TrackOverlayRoISiRDOMergerAlg.h"

#include "AthAllocators/DataPool.h"
#include "FourMomUtils/P4Helpers.h"
#include "InDetIdentifier/PixelID.h"
#include "InDetIdentifier/SCT_ID.h"
#include "InDetRawData/Pixel1RawData.h"
#include "InDetRawData/SCT1_RawData.h"
#include "InDetRawData/SCT3_RawData.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"
#include "TrigSteeringEvent/TrigRoiDescriptor.h"

#include <algorithm>
#include <stdexcept>
#include <vector>

namespace {

// Copies of RDO collections. The collections are views; the RDO objects are owned by
// per-event DataPools (as in PixelOverlay/SCTOverlay), avoiding one heap allocation per hit.
struct RDOPools {
  DataPool<Pixel1RawData>& pixel;
  DataPool<SCT3_RawData>& sct3;
  DataPool<SCT1_RawData>& sct1;
};

std::unique_ptr<PixelRDO_Collection> copyCollection(const PixelRDO_Collection& in, RDOPools& pools) {
  auto out = std::make_unique<PixelRDO_Collection>(in.identifyHash());
  out->setIdentifier(in.identify());
  out->clear(SG::VIEW_ELEMENTS);
  out->reserve(in.size());
  for (const PixelRDORawData* rdo : in) {
    Pixel1RawData* copy = pools.pixel.nextElementPtr();
    *copy = Pixel1RawData(rdo->identify(), rdo->getWord());
    out->push_back(copy);
  }
  return out;
}

std::unique_ptr<SCT_RDO_Collection> copyCollection(const SCT_RDO_Collection& in, RDOPools& pools) {
  auto out = std::make_unique<SCT_RDO_Collection>(in.identifyHash());
  out->setIdentifier(in.identify());
  out->clear(SG::VIEW_ELEMENTS);
  out->reserve(in.size());
  for (const SCT_RDORawData* rdo : in) {
    if (const auto* rdo3 = dynamic_cast<const SCT3_RawData*>(rdo)) {
      SCT3_RawData* copy = pools.sct3.nextElementPtr();
      *copy = *rdo3;
      out->push_back(copy);
    } else if (const auto* rdo1 = dynamic_cast<const SCT1_RawData*>(rdo)) {
      SCT1_RawData* copy = pools.sct1.nextElementPtr();
      *copy = *rdo1;
      out->push_back(copy);
    } else {
      throw std::runtime_error("TrackOverlayRoISiRDOMergerAlg: unknown SCT RDO type");
    }
  }
  return out;
}

// Fill output with the merged collection for hashes in the RoI mask, signal-only otherwise
template <class CONT>
StatusCode fillContainer(const CONT& sig, const CONT& merged, const std::vector<bool>& inRoI, CONT& out, RDOPools& pools) {
  for (const auto& [hash, coll] : sig.GetAllHashPtrPair()) {
    if (hash < inRoI.size() && inRoI[hash]) continue;
    if (out.addOrDelete(copyCollection(*coll, pools), hash).isFailure()) return StatusCode::FAILURE;
  }
  for (size_t i = 0; i < inRoI.size(); ++i) {
    if (!inRoI[i]) continue;
    const IdentifierHash hash(i);
    if (const auto* coll = merged.indexFindPtr(hash)) {
      if (out.addOrDelete(copyCollection(*coll, pools), hash).isFailure()) return StatusCode::FAILURE;
    }
  }
  return StatusCode::SUCCESS;
}

} // namespace

namespace TrackOverlay {

StatusCode TrackOverlayRoISiRDOMergerAlg::initialize()
{
  ATH_CHECK(m_clusterKey.initialize());
  ATH_CHECK(m_sigPixelKey.initialize());
  ATH_CHECK(m_fullPixelKey.initialize());
  ATH_CHECK(m_outPixelKey.initialize());
  ATH_CHECK(m_sigSCTKey.initialize());
  ATH_CHECK(m_fullSCTKey.initialize());
  ATH_CHECK(m_outSCTKey.initialize());
  ATH_CHECK(m_regSelPixel.retrieve());
  ATH_CHECK(m_regSelSCT.retrieve());
  ATH_CHECK(detStore()->retrieve(m_pixelID, "PixelID"));
  ATH_CHECK(detStore()->retrieve(m_sctID, "SCT_ID"));
  ATH_MSG_INFO("RoI-limited Si overlay: clusters " << m_clusterKey.key() << " with ET > " << m_minClusterEt
               << " MeV, RoI half widths deta=" << m_etaHalfWidth << " dphi=" << m_phiHalfWidth
               << " dz=" << m_zHalfWidth << " mm");
  return StatusCode::SUCCESS;
}

StatusCode TrackOverlayRoISiRDOMergerAlg::execute(const EventContext& ctx) const
{
  SG::ReadHandle<xAOD::CaloClusterContainer> clusters(m_clusterKey, ctx);
  if (!clusters.isValid()) {
    ATH_MSG_ERROR("Could not retrieve " << m_clusterKey.key());
    return StatusCode::FAILURE;
  }

  std::vector<bool> pixelInRoI(m_pixelID->wafer_hash_max(), false);
  std::vector<bool> sctInRoI(m_sctID->wafer_hash_max(), false);
  std::vector<IdentifierHash> hashes;
  unsigned int nRoIs = 0;

  for (const xAOD::CaloCluster* cl : *clusters) {
    if (cl->et() < m_minClusterEt.value()) continue;
    const double eta = cl->eta();
    const double phi = cl->phi();
    const double dEta = m_etaHalfWidth.value();
    const double dPhi = m_phiHalfWidth.value();
    const double dZ = m_zHalfWidth.value();
    const TrigRoiDescriptor roi(eta, eta - dEta, eta + dEta,
                                phi, P4Helpers::deltaPhi(phi - dPhi, 0.), P4Helpers::deltaPhi(phi + dPhi, 0.),
                                0., -dZ, dZ);
    ++nRoIs;
    hashes.clear();
    m_regSelPixel->lookup(ctx)->HashIDList(roi, hashes);
    for (IdentifierHash h : hashes) if (h < pixelInRoI.size()) pixelInRoI[h] = true;
    hashes.clear();
    m_regSelSCT->lookup(ctx)->HashIDList(roi, hashes);
    for (IdentifierHash h : hashes) if (h < sctInRoI.size()) sctInRoI[h] = true;
  }

  SG::ReadHandle<PixelRDO_Container> sigPixel(m_sigPixelKey, ctx);
  SG::ReadHandle<PixelRDO_Container> fullPixel(m_fullPixelKey, ctx);
  SG::ReadHandle<SCT_RDO_Container> sigSCT(m_sigSCTKey, ctx);
  SG::ReadHandle<SCT_RDO_Container> fullSCT(m_fullSCTKey, ctx);
  if (!sigPixel.isValid() || !fullPixel.isValid() || !sigSCT.isValid() || !fullSCT.isValid()) {
    ATH_MSG_ERROR("Missing input Pixel/SCT RDO containers");
    return StatusCode::FAILURE;
  }

  // The DataPools own the copied RDOs for the lifetime of the event; the containers are views
  DataPool<Pixel1RawData> pixelPool(ctx);
  DataPool<SCT3_RawData> sct3Pool(ctx);
  DataPool<SCT1_RawData> sct1Pool(ctx);
  pixelPool.prepareToAdd(50000);
  sct3Pool.prepareToAdd(50000);
  RDOPools pools{pixelPool, sct3Pool, sct1Pool};

  SG::WriteHandle<PixelRDO_Container> outPixel(m_outPixelKey, ctx);
  ATH_CHECK(outPixel.record(std::make_unique<PixelRDO_Container>(m_pixelID->wafer_hash_max())));
  ATH_CHECK(fillContainer(*sigPixel, *fullPixel, pixelInRoI, *outPixel, pools));

  SG::WriteHandle<SCT_RDO_Container> outSCT(m_outSCTKey, ctx);
  ATH_CHECK(outSCT.record(std::make_unique<SCT_RDO_Container>(m_sctID->wafer_hash_max())));
  ATH_CHECK(fillContainer(*sigSCT, *fullSCT, sctInRoI, *outSCT, pools));

  const unsigned long nPix = std::count(pixelInRoI.begin(), pixelInRoI.end(), true);
  const unsigned long nSCT = std::count(sctInRoI.begin(), sctInRoI.end(), true);
  ATH_MSG_DEBUG(nRoIs << " RoIs, " << nPix << " Pixel and " << nSCT << " SCT modules with merged RDOs");
  ++m_nEvents;
  m_nRoIs += nRoIs;
  m_nPixelRoIModules += nPix;
  m_nSCTRoIModules += nSCT;
  return StatusCode::SUCCESS;
}

StatusCode TrackOverlayRoISiRDOMergerAlg::finalize()
{
  const double n = std::max<unsigned long>(m_nEvents.load(), 1);
  ATH_MSG_INFO("Events: " << m_nEvents << ", RoIs/event: " << m_nRoIs.load() / n
               << ", merged Pixel modules/event: " << m_nPixelRoIModules.load() / n << " of " << m_pixelID->wafer_hash_max()
               << ", merged SCT modules/event: " << m_nSCTRoIModules.load() / n << " of " << m_sctID->wafer_hash_max());
  return StatusCode::SUCCESS;
}

} // namespace TrackOverlay
