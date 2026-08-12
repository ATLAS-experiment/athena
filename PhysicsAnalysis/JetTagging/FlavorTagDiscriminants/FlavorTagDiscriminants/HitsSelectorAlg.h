/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef HITS_SELECTOR_ALG_HH
#define HITS_SELECTOR_ALG_HH

// FrameWork includes
#include "AthenaBaseComps/AthReentrantAlgorithm.h"

// Containers
#include "xAODJet/JetContainer.h"
#include "xAODTracking/TrackMeasurementValidationContainer.h"

// Read and write handle keys
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadDecorHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"

#include "AthLinks/ElementLink.h"

#include <string>
#include <vector>

namespace FlavorTagDiscriminants {

  class HitsSelectorAlg : public AthReentrantAlgorithm {
    /** @name HitsSelectorAlg
     *  @brief Merge the per-jet Pixel and SCT hit associations into a single
     *         collection, keeping only hits flagged as clean (isCleanHit),
     *         sorting them by |dphi| to the jet axis and truncating to maxHits.
     *
     *         Either subdetector can be dropped by setting its association
     *         property to an empty string, e.g. SCTAssociation="" to run on
     *         Pixel hits alone.
     */

    public:
      HitsSelectorAlg(const std::string& name, ISvcLocator* svcLoc);

      virtual StatusCode initialize() override;
      virtual StatusCode execute(const EventContext& ctx) const override;

    private:
      using HitContainer = xAOD::TrackMeasurementValidationContainer;
      using HitLink = ElementLink<HitContainer>;

      struct SortedHit {
        HitLink link;
        float dphi;
      };

      // Jet input
      SG::ReadHandleKey<xAOD::JetContainer> m_jetsKey{
        this, "jetContainer", "AntiKt4EMPFlowJets", "Input jets"};

      // Per-jet input associations, empty to skip that subdetector
      SG::ReadDecorHandleKey<xAOD::JetContainer> m_pixelAssocKey{
        this, "PixelAssociation", m_jetsKey, "hitsForBTaggingPixel", "Name of Pixel association key, empty to skip Pixel hits"};

      SG::ReadDecorHandleKey<xAOD::JetContainer> m_sctAssocKey{
        this, "SCTAssociation", m_jetsKey, "hitsForBTaggingSCT", "Name of SCT association key, empty to skip SCT hits"};

      // Hit containers the associations point into
      SG::ReadHandleKey<HitContainer> m_pixelHitsKey{
        this, "pixelHitContainer", "PixelClusters", "Pixel hit container"};

      SG::ReadHandleKey<HitContainer> m_sctHitsKey{
        this, "SCTHitContainer", "SCT_Clusters", "SCT hit container"};

      // Per-hit decorations, read via the links above
      SG::ReadDecorHandleKey<HitContainer> m_pixelHitXKey{
        this, "pixelHitX", m_pixelHitsKey, "HitsXRelToBeamspot", "Pixel hit x coordinate"};

      SG::ReadDecorHandleKey<HitContainer> m_pixelHitYKey{
        this, "pixelHitY", m_pixelHitsKey, "HitsYRelToBeamspot", "Pixel hit y coordinate"};

      SG::ReadDecorHandleKey<HitContainer> m_pixelCleanKey{
        this, "pixelCleanHit", m_pixelHitsKey, "isCleanHit", "Pixel clean hit flag"};

      SG::ReadDecorHandleKey<HitContainer> m_sctHitXKey{
        this, "SCTHitX", m_sctHitsKey, "HitsXRelToBeamspot", "SCT hit x coordinate"};

      SG::ReadDecorHandleKey<HitContainer> m_sctHitYKey{
        this, "SCTHitY", m_sctHitsKey, "HitsYRelToBeamspot", "SCT hit y coordinate"};

      SG::ReadDecorHandleKey<HitContainer> m_sctCleanKey{
        this, "SCTCleanHit", m_sctHitsKey, "isCleanHit", "SCT clean hit flag"};

      // Output decoration
      SG::WriteDecorHandleKey<xAOD::JetContainer> m_hitAssocKey{
        this, "hitAssociation", m_jetsKey, "hitsForBTagging", "Name of combined association key"};

      // Parameters
      Gaudi::Property<int> m_maxHits{
        this, "maxHits", 200, "Max hits to keep"};

      bool m_usePixel{true};
      bool m_useSCT{true};

      StatusCode collectHits(
        const EventContext& ctx,
        const SG::ReadDecorHandleKey<xAOD::JetContainer>& assocKey,
        const SG::ReadDecorHandleKey<HitContainer>& xKey,
        const SG::ReadDecorHandleKey<HitContainer>& yKey,
        const SG::ReadDecorHandleKey<HitContainer>& cleanKey,
        const xAOD::JetContainer& jets,
        std::vector<std::vector<SortedHit>>& hitsPerJet) const;
  };

}

#endif
