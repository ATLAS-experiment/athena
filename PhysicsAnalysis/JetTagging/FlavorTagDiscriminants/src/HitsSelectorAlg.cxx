/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "FlavorTagDiscriminants/HitsSelectorAlg.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/ReadDecorHandle.h"
#include "StoreGate/WriteDecorHandle.h"

#include "xAODTracking/TrackMeasurementValidation.h"
#include "xAODJet/Jet.h"

#include "CxxUtils/phihelper.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

namespace FlavorTagDiscriminants {

  HitsSelectorAlg::HitsSelectorAlg(const std::string& name, ISvcLocator* svcLoc)
    : AthReentrantAlgorithm(name, svcLoc) {}

  StatusCode HitsSelectorAlg::initialize() {
    ATH_MSG_DEBUG("Initializing " << name());

    m_usePixel = !m_pixelAssocKey.empty();
    m_useSCT = !m_sctAssocKey.empty();

    if (!m_usePixel && !m_useSCT) {
      ATH_MSG_ERROR("No hits to select: set PixelAssociation, SCTAssociation, or both");
      return StatusCode::FAILURE;
    }
    ATH_MSG_INFO("Selecting hits from" << (m_usePixel ? " Pixel" : "")
                 << (m_useSCT ? " SCT" : "") << ", keeping at most "
                 << m_maxHits.value() << " per jet");

    ATH_CHECK(m_jetsKey.initialize());

    ATH_CHECK(m_pixelAssocKey.initialize(m_usePixel));
    ATH_CHECK(m_pixelHitsKey.initialize(m_usePixel));
    ATH_CHECK(m_pixelHitXKey.initialize(m_usePixel));
    ATH_CHECK(m_pixelHitYKey.initialize(m_usePixel));
    ATH_CHECK(m_pixelCleanKey.initialize(m_usePixel));

    ATH_CHECK(m_sctAssocKey.initialize(m_useSCT));
    ATH_CHECK(m_sctHitsKey.initialize(m_useSCT));
    ATH_CHECK(m_sctHitXKey.initialize(m_useSCT));
    ATH_CHECK(m_sctHitYKey.initialize(m_useSCT));
    ATH_CHECK(m_sctCleanKey.initialize(m_useSCT));

    ATH_CHECK(m_hitAssocKey.initialize());

    return StatusCode::SUCCESS;
  }

  // Append the clean hits of one subdetector to every jet's list, keeping the
  // |dphi| to the jet axis alongside each link so the merged list can be sorted
  StatusCode HitsSelectorAlg::collectHits(
    const EventContext& ctx,
    const SG::ReadDecorHandleKey<xAOD::JetContainer>& assocKey,
    const SG::ReadDecorHandleKey<HitContainer>& xKey,
    const SG::ReadDecorHandleKey<HitContainer>& yKey,
    const SG::ReadDecorHandleKey<HitContainer>& cleanKey,
    const xAOD::JetContainer& jets,
    std::vector<std::vector<SortedHit>>& hitsPerJet) const {

    SG::ReadDecorHandle<xAOD::JetContainer, std::vector<HitLink>> assoc(assocKey, ctx);
    const SG::ReadDecorHandle<HitContainer, float> hitX(xKey, ctx);
    const SG::ReadDecorHandle<HitContainer, float> hitY(yKey, ctx);
    const SG::ReadDecorHandle<HitContainer, int> isClean(cleanKey, ctx);

    for (std::size_t iJet = 0; iJet < jets.size(); ++iJet) {

      const xAOD::Jet& jet = *jets[iJet];
      const float jetPhi = jet.phi();
      std::vector<SortedHit>& out = hitsPerJet[iJet];

      for (const HitLink& el : assoc(jet)) {
        if (!el.isValid()) {
          ATH_MSG_ERROR("Invalid hit link in " << assocKey.key() << " on jet " << iJet);
          return StatusCode::FAILURE;
        }
        const xAOD::TrackMeasurementValidation& hit = **el;
        if (!isClean(hit)) continue;

        const float hitPhi = std::atan2(hitY(hit), hitX(hit));
        out.push_back({el, std::abs(CxxUtils::wrapToPi(jetPhi - hitPhi))});
      }
    }

    return StatusCode::SUCCESS;
  }

  StatusCode HitsSelectorAlg::execute(const EventContext& ctx) const {

    SG::ReadHandle<xAOD::JetContainer> jets(m_jetsKey, ctx);
    if (!jets.isValid()) {
      ATH_MSG_ERROR("Cannot read jets " << m_jetsKey.key());
      return StatusCode::FAILURE;
    }

    std::vector<std::vector<SortedHit>> hitsPerJet(jets->size());

    if (m_usePixel) {
      ATH_CHECK(collectHits(ctx, m_pixelAssocKey, m_pixelHitXKey, m_pixelHitYKey,
                            m_pixelCleanKey, *jets, hitsPerJet));
    }
    if (m_useSCT) {
      ATH_CHECK(collectHits(ctx, m_sctAssocKey, m_sctHitXKey, m_sctHitYKey,
                            m_sctCleanKey, *jets, hitsPerJet));
    }

    SG::WriteDecorHandle<xAOD::JetContainer, std::vector<HitLink>>
        cleanMergedAssoc(m_hitAssocKey, ctx);

    for (std::size_t iJet = 0; iJet < jets->size(); ++iJet) {

      std::vector<SortedHit>& sorted = hitsPerJet[iJet];

      std::sort(sorted.begin(), sorted.end(),
                [](const SortedHit& a, const SortedHit& b) {
                  return a.dphi < b.dphi;
                });

      if (sorted.size() > static_cast<std::size_t>(m_maxHits)) {
        sorted.resize(m_maxHits);
      }

      std::vector<HitLink> outEL;
      outEL.reserve(sorted.size());
      for (const SortedHit& sh : sorted) {
        outEL.push_back(sh.link);
      }

      cleanMergedAssoc(*(*jets)[iJet]) = std::move(outEL);
    }

    return StatusCode::SUCCESS;
  }

}
