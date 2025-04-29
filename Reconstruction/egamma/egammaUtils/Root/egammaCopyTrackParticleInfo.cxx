/*
 Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#include "egammaUtils/egammaCopyTrackParticleInfo.h"

#include "xAODEgamma/EgammaxAODHelpers.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTruth/TruthParticleContainer.h"
namespace {
void
copySummaryValue(const xAOD::TrackParticle& src,
                 xAOD::TrackParticle& dest,
                 const xAOD::SummaryType& information) {
  uint8_t value = xAOD::EgammaHelpers::summaryValueInt(src, information, 0);
  dest.setSummaryValue(value, information);
}
}  // namespace

void
egammaCopyTrackParticleInfo::copy(xAOD::TrackParticle& created,
                                  const xAOD::TrackParticle& original,
                                  const egammaCopyTrackParticleInfo::ToCopy& toCopy) {
  // Add Truth decorations. Copy from the original.
  if (toCopy.doTruth) {
    static const SG::AuxElement::Accessor<
        ElementLink<xAOD::TruthParticleContainer>>
        tPL("truthParticleLink");
    if (tPL.isAvailable(original)) {
      tPL(created) = tPL(original);
    }
    static const SG::AuxElement::Accessor<float> tMP("truthMatchProbability");
    if (tMP.isAvailable(original)) {
      tMP(created) = tMP(original);
    }
    static const SG::AuxElement::Accessor<int> tT("truthType");
    if (tT.isAvailable(original)) {
      tT(created) = tT(original);
    }
    static const SG::AuxElement::Accessor<int> tO("truthOrigin");
    if (tO.isAvailable(original)) {
      tO(created) = tO(original);
    }
  }

  copySummaryValue(original, created, xAOD::numberOfPixelSplitHits);
  copySummaryValue(original, created,
                   xAOD::numberOfInnermostPixelLayerSplitHits);
  copySummaryValue(original, created,
                   xAOD::numberOfNextToInnermostPixelLayerSplitHits);
  copySummaryValue(original, created, xAOD::numberOfPixelSharedHits);
  copySummaryValue(original, created,
                   xAOD::numberOfInnermostPixelLayerSharedHits);
  copySummaryValue(original, created,
                   xAOD::numberOfNextToInnermostPixelLayerSharedHits);
  copySummaryValue(original, created, xAOD::numberOfSCTSharedHits);
  copySummaryValue(original, created, xAOD::numberOfTRTSharedHits);

  if (toCopy.doHGTD) {
    created.setHasValidTime(original.hasValidTime());
    created.setTime(original.time());
  }

  if (toCopy.isRefitted) {
    if (toCopy.doPix) {
      // copy over dead sensors
      copySummaryValue(original, created, xAOD::numberOfPixelDeadSensors);

      // Figure the new number of holes
      uint8_t nPixHolesRefitted =
          -xAOD::EgammaHelpers::summaryValueInt(created, xAOD::numberOfPixelHits, -1) -
          xAOD::EgammaHelpers::summaryValueInt(created, xAOD::numberOfPixelOutliers, -1) +
          xAOD::EgammaHelpers::summaryValueInt(original, xAOD::numberOfPixelHits, -1) +
          xAOD::EgammaHelpers::summaryValueInt(original, xAOD::numberOfPixelOutliers, -1) +
          xAOD::EgammaHelpers::summaryValueInt(original, xAOD::numberOfPixelHoles, -1);

      created.setSummaryValue(nPixHolesRefitted, xAOD::numberOfPixelHoles);
    }
    if (toCopy.doSCT) {
      // Copy over dead and double holes
      copySummaryValue(original, created, xAOD::numberOfSCTDeadSensors);
      copySummaryValue(original, created, xAOD::numberOfSCTDoubleHoles);

      uint8_t nSCTHolesRefitted =
          -xAOD::EgammaHelpers::summaryValueInt(created, xAOD::numberOfSCTHits, -1) -
          xAOD::EgammaHelpers::summaryValueInt(created, xAOD::numberOfSCTOutliers, -1) +
          xAOD::EgammaHelpers::summaryValueInt(original, xAOD::numberOfSCTHits, -1) +
          xAOD::EgammaHelpers::summaryValueInt(original, xAOD::numberOfSCTHoles, -1) +
          xAOD::EgammaHelpers::summaryValueInt(original, xAOD::numberOfSCTOutliers, -1);

      created.setSummaryValue(nSCTHolesRefitted, xAOD::numberOfSCTHoles);
    }
    if (toCopy.doTRT) {
      uint8_t nTRTHolesRefitted =
          -xAOD::EgammaHelpers::summaryValueInt(created, xAOD::numberOfTRTHits, -1) -
          xAOD::EgammaHelpers::summaryValueInt(created, xAOD::numberOfTRTOutliers, -1) +
          xAOD::EgammaHelpers::summaryValueInt(original, xAOD::numberOfTRTHits, -1) +
          xAOD::EgammaHelpers::summaryValueInt(original, xAOD::numberOfTRTHoles, -1) +
          xAOD::EgammaHelpers::summaryValueInt(original, xAOD::numberOfTRTOutliers, -1);

      created.setSummaryValue(nTRTHolesRefitted, xAOD::numberOfTRTHoles);
    }
  }
}

