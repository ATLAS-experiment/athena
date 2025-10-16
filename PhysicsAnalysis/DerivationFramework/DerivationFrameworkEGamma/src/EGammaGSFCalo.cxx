/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
  Contact: Raphael Haberle <raphael.julien.haberle@cern.ch>
*/

#include "DerivationFrameworkEGamma/EGammaGSFCalo.h"

#include "StoreGate/WriteDecorHandle.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODTracking/TrackParticleAuxContainer.h"
#include "xAODTruth/TruthParticleContainer.h"
// ========================================================================
namespace DerivationFramework {
EGammaGSFCalo::EGammaGSFCalo(const std::string& t, const std::string& n,
                             const IInterface* p)
    : AthAlgTool(t, n, p) {
  declareInterface<DerivationFramework::IAugmentationTool>(this);
}

EGammaGSFCalo::~EGammaGSFCalo() {
  // destructor
}

// ========================================================================

// ========================================================================

StatusCode EGammaGSFCalo::initialize() {
  ATH_MSG_INFO("Initialize...");

  ATH_CHECK(m_trkRefitTool.retrieve());
  ATH_CHECK(m_particleCreatorTool.retrieve());
  ATH_CHECK(m_trackSummaryTool.retrieve());
  ATH_CHECK(m_electronCollectionKey.initialize());
  ATH_CHECK(m_gsfCaloTrackLinkKey.initialize());
  ATH_CHECK(m_OutputTrkPartContainerKey.initialize());

  return StatusCode::SUCCESS;
}

StatusCode EGammaGSFCalo::finalize() {
  ATH_MSG_INFO("Finalize...");

  ATH_MSG_INFO("Electrons:                          " << m_allElectrons);
  ATH_MSG_INFO("Electrons w/o xAOD::TrackParticle:  " << m_noTP);
  ATH_MSG_INFO("Electrons w/ TRT-only track:        " << m_onlyTRT);
  ATH_MSG_INFO("Electrons w/o Trk::Track:           " << m_noTrk);
  ATH_MSG_INFO("Number of failed fits:              " << m_failedFits);
  ATH_MSG_INFO("Number of successful fits:          " << m_successfulFits);
  ATH_MSG_INFO("Number of successful copyInfos():   " << m_successfulCopyInfos);
  ATH_MSG_INFO("Number of failed createParticle():  " << m_noRefTP);
  ATH_MSG_INFO("New container size:                 " << m_allNewTP);
  ATH_MSG_INFO("TSOS:                               " << m_tsos);

  return StatusCode::SUCCESS;
}

// ========================================================================

StatusCode EGammaGSFCalo::addBranches( const EventContext& ctx ) const {

  // input electron decoration handle
  SG::WriteDecorHandle<xAOD::ElectronContainer,
                       ElementLink<xAOD::TrackParticleContainer>>
      dec_gsfCaloTrackLink(m_gsfCaloTrackLinkKey, ctx);

  // gsf calo electrons decorations
  static const SG::AuxElement::Accessor<int> acc_gsfCaloStatus("gsfCaloStatus");
  static const SG::AuxElement::Accessor<float> acc_gsfChi2oNDF(
      "gsfChiSquareOverNDOF");
  static const SG::AuxElement::Accessor<float> acc_gsfCaloChi2oNDF(
      "gsfCaloChiSquareOverNDOF");
  static const SG::AuxElement::Accessor<ElementLink<xAOD::ElectronContainer>>
      acc_usedElectronLink("usedElectronLink");
  static const SG::AuxElement::ConstAccessor<
      ElementLink<xAOD::TrackParticleContainer>>
      read_originalTP("originalTrackParticle");
  static const SG::AuxElement::Accessor<
      ElementLink<xAOD::TrackParticleContainer>>
      write_originalTP("originalTrackParticle");

  // Create the final containers to be written out
  SG::WriteHandle<xAOD::TrackParticleContainer> finalTrkPartContainer(
      m_OutputTrkPartContainerKey, ctx);

  ATH_CHECK(finalTrkPartContainer.record(
      std::make_unique<xAOD::TrackParticleContainer>(),
      std::make_unique<xAOD::TrackParticleAuxContainer>()));

  xAOD::TrackParticleContainer* cPtrTrkPart = finalTrkPartContainer.ptr();

  // Get the input electron container from StoreGate
  SG::ReadHandle<xAOD::ElectronContainer> ehandle(m_electronCollectionKey, ctx);
  ATH_CHECK(ehandle.isValid());
  const xAOD::ElectronContainer* electronContainer = ehandle.cptr();
  m_allElectrons += electronContainer->size();

  for (const auto* electron : *electronContainer) {
    const xAOD::TrackParticle* electronTP = electron->trackParticle();
    if (!electronTP) {
      ATH_MSG_WARNING("Can't get electron #" << electron->index()
                                             << " TrackParticle!");
      m_noTP++;
      continue;
    }
    // Electron has a xAOD::TrackParticle, proceed...

    xAOD::TrackParticle* electronTPRefit = nullptr;
    int gsfCaloStatus = -1;
    float gsfChiSquareOverNDOF = -1;
    float gsfCaloChiSquareOverNDOF = -1;

    ATH_CHECK(electronTP->trackLink().isValid());
    // We have a Trk::Track to re-fit...
    uint8_t dummy(0);
    int nSiHits = 0;

    if (electronTP->summaryValue(dummy, xAOD::numberOfSCTHits))
      nSiHits += dummy;
    if (electronTP->summaryValue(dummy, xAOD::numberOfPixelHits))
      nSiHits += dummy;
    if (electronTP->summaryValue(dummy, xAOD::numberOfSCTOutliers))
      nSiHits += dummy;
    if (electronTP->summaryValue(dummy, xAOD::numberOfPixelOutliers))
      nSiHits += dummy;

    const Trk::Track* track = electronTP->track();

    // Skip TRT-only tracks
    if (nSiHits >= m_minNSiHits) {
      // We want to re-fit only "silicon" Trk::Tracks
      auto electronTrackQuality = electronTP->track()->fitQuality();
      gsfChiSquareOverNDOF = electronTrackQuality->chiSquared() /
                             electronTrackQuality->numberDoF();

      auto electronTrackRefit = std::make_unique<Trk::Track>();
      IegammaTrkRefitterTool::Cache cache{};
      // Add electron pointer to cache for the refitting
      cache.electron = electron;
      StatusCode sc = m_trkRefitTool->refitTrack(ctx, track, cache);

      if (sc == StatusCode::SUCCESS) {
        electronTrackRefit.reset(cache.refittedTrack.release());
        // move
        m_trackSummaryTool->updateTrack(ctx, *electronTrackRefit);

        gsfCaloStatus = 1;
        m_successfulFits++;

        const xAOD::Vertex* electronTrackVertex = nullptr;
        electronTPRefit = m_particleCreatorTool->createParticle(
            ctx, *electronTrackRefit, cPtrTrkPart, electronTrackVertex,
            xAOD::electron);
        if (electronTPRefit) {
          copyInfo(*electronTP, *electronTPRefit, !m_isAOD);
          auto electronTrackRefitQuality = electronTrackRefit->fitQuality();
          gsfCaloChiSquareOverNDOF = electronTrackRefitQuality->chiSquared() /
                                     electronTrackRefitQuality->numberDoF();
        } else {
          ATH_MSG_DEBUG(
              "Can't create a new xAOD::TrackParticle from the re-fitted "
              "Trk::Track! Using the original xAOD::TrackParticle!");
          gsfCaloStatus = 0;
          m_noRefTP++;
        }
      } else {
        ATH_MSG_DEBUG(
            "CALO-improved Trk::Track re-fit failed! Using the original "
            "xAOD::TrackParticle!");
        m_failedFits++;
        gsfCaloStatus = 0;
      }
    } else {
      ATH_MSG_DEBUG("Electron "
                    << electron->index()
                    << " has a valid track particle but is a TRT-only track");
      gsfCaloStatus = 0;
      m_onlyTRT++;
    }

    if (gsfCaloStatus != 1) {
      electronTPRefit = new xAOD::TrackParticle();
      cPtrTrkPart->push_back(electronTPRefit);
      *electronTPRefit = *electronTP;
    }
    // Debug flags
    acc_gsfCaloStatus(*electronTPRefit) = gsfCaloStatus;
    acc_gsfChi2oNDF(*electronTPRefit) = gsfChiSquareOverNDOF;
    acc_gsfCaloChi2oNDF(*electronTPRefit) = gsfCaloChiSquareOverNDOF;

    electronTPRefit->setTrackLink(electronTP->trackLink());

    // Add link to refitted TP to electron container
    ElementLink<xAOD::TrackParticleContainer> linkToRefittedTrackParticle(
        electronTPRefit, *cPtrTrkPart);
    dec_gsfCaloTrackLink(*electron) = linkToRefittedTrackParticle;

    // Add link to initial electron to the refitted TP container
    ElementLink<xAOD::ElectronContainer> linkToElectron;
    linkToElectron.toIndexedElement(*electronContainer, electron->index());
    acc_usedElectronLink(*electronTPRefit) = linkToElectron;

    ElementLink<xAOD::TrackParticleContainer> linkToOriginal;
    if (read_originalTP.isAvailable(*electronTP)) {
      linkToOriginal = read_originalTP(*electronTP);
    }
    write_originalTP(*electronTPRefit) = linkToOriginal;
  }  // end of the electron container loop

  m_allNewTP += cPtrTrkPart->size();
  return StatusCode::SUCCESS;
}

void EGammaGSFCalo::copyInfo(const xAOD::TrackParticle& original,
                             xAOD::TrackParticle& created,
                             bool isRefitted) const {
  // Add Truth decorations. Copy from the original.
  if (m_doTruth) {
    static const SG::AuxElement::Accessor<
        ElementLink<xAOD::TruthParticleContainer>>
        tPL("truthParticleLink");
    if (tPL.isAvailable(original)) {
      const ElementLink<xAOD::TruthParticleContainer>& linkToTruth =
          tPL(original);
      tPL(created) = linkToTruth;
    }
    static const SG::AuxElement::Accessor<float> tMP("truthMatchProbability");
    if (tMP.isAvailable(original)) {
      float originalProbability = tMP(original);
      tMP(created) = originalProbability;
    }
    static const SG::AuxElement::Accessor<int> tT("truthType");
    if (tT.isAvailable(original)) {
      int truthType = tT(original);
      tT(created) = truthType;
    }
    static const SG::AuxElement::Accessor<int> tO("truthOrigin");
    if (tO.isAvailable(original)) {
      int truthOrigin = tO(original);
      tO(created) = truthOrigin;
    }
  }

  // It's apparently not possible to update Trk::Track when running on AOD; copy
  // the values from the original xAOD::TrackParticle
  if (m_isAOD) {
    copySummaryValue(original, created, xAOD::numberOfBLayerHits);
    copySummaryValue(original, created, xAOD::numberOfPixelHits);
    copySummaryValue(original, created, xAOD::numberOfPixelDeadSensors);
    copySummaryValue(original, created, xAOD::numberOfSCTHits);
    copySummaryValue(original, created, xAOD::numberOfPixelHoles);
    copySummaryValue(original, created, xAOD::numberOfSCTHoles);
    copySummaryValue(original, created, xAOD::numberOfSCTDoubleHoles);
    copySummaryValue(original, created, xAOD::numberOfSCTDeadSensors);
    copySummaryValue(original, created, xAOD::numberOfTRTHits);
    copySummaryValue(original, created, xAOD::numberOfTRTOutliers);
    copySummaryValue(original, created, xAOD::numberOfTRTHighThresholdHits);
  }

  // Copy shared hit content from the original xAOD::TrackParticle
  // Taken from
  // https://gitlab.cern.ch/atlas/athena/-/blob/master/Reconstruction/egamma/egammaAlgs/src/EMBremCollectionBuilder.cxx?ref_type=heads#L329
  copySummaryValue(original, created, xAOD::numberOfPixelSplitHits);
  copySummaryValue(original, created,
                   xAOD::numberOfInnermostPixelLayerSplitHits);
  copySummaryValue(original, created,
                   xAOD::numberOfNextToInnermostPixelLayerSplitHits);
  copySummaryValue(original, created,
                   xAOD::numberOfInnermostPixelLayerSharedHits);
  copySummaryValue(original, created,
                   xAOD::numberOfNextToInnermostPixelLayerSharedHits);
  copySummaryValue(original, created, xAOD::numberOfBLayerSharedHits);
  copySummaryValue(original, created, xAOD::numberOfPixelSharedHits);
  copySummaryValue(original, created, xAOD::numberOfSCTSharedHits);
  copySummaryValue(original, created, xAOD::numberOfTRTSharedHits);

  if (isRefitted) {
    // Figure the new number of holes
    uint8_t dummy(0);
    if (m_doPix) {
      int nPixHitsRefitted =
          created.summaryValue(dummy, xAOD::numberOfPixelHits) ? dummy : -1;
      int nPixOutliersRefitted =
          created.summaryValue(dummy, xAOD::numberOfPixelOutliers) ? dummy : -1;

      int nPixHitsOriginal =
          original.summaryValue(dummy, xAOD::numberOfPixelHits) ? dummy : -1;
      int nPixOutliersOriginal =
          original.summaryValue(dummy, xAOD::numberOfPixelOutliers) ? dummy
                                                                    : -1;
      int nPixHolesOriginal =
          original.summaryValue(dummy, xAOD::numberOfPixelHoles) ? dummy : -1;

      uint8_t nPixHolesRefitted = nPixHitsOriginal + nPixHolesOriginal +
                                  nPixOutliersOriginal - nPixOutliersRefitted -
                                  nPixHitsRefitted;

      created.setSummaryValue(nPixHolesRefitted, xAOD::numberOfPixelHoles);
    }

    if (m_doSCT) {
      int nSCTHitsRefitted =
          created.summaryValue(dummy, xAOD::numberOfSCTHits) ? dummy : -1;
      int nSCTOutliersRefitted =
          created.summaryValue(dummy, xAOD::numberOfSCTOutliers) ? dummy : -1;

      int nSCTHitsOriginal =
          original.summaryValue(dummy, xAOD::numberOfSCTHits) ? dummy : -1;
      int nSCTHolesOriginal =
          original.summaryValue(dummy, xAOD::numberOfSCTHoles) ? dummy : -1;
      int nSCTOutliersOriginal =
          original.summaryValue(dummy, xAOD::numberOfSCTOutliers) ? dummy : -1;

      uint8_t nSCTHolesRefitted = nSCTHitsOriginal + nSCTHolesOriginal +
                                  nSCTOutliersOriginal - nSCTOutliersRefitted -
                                  nSCTHitsRefitted;

      created.setSummaryValue(nSCTHolesRefitted, xAOD::numberOfSCTHoles);
    }

    if (m_doTRT) {
      int nTRTHitsRefitted =
          created.summaryValue(dummy, xAOD::numberOfTRTHits) ? dummy : -1;
      int nTRTOutliersRefitted =
          created.summaryValue(dummy, xAOD::numberOfTRTOutliers) ? dummy : -1;

      int nTRTHitsOriginal =
          original.summaryValue(dummy, xAOD::numberOfTRTHits) ? dummy : -1;
      int nTRTHolesOriginal =
          original.summaryValue(dummy, xAOD::numberOfTRTHoles) ? dummy : -1;
      int nTRTOutliersOriginal =
          original.summaryValue(dummy, xAOD::numberOfTRTOutliers) ? dummy : -1;

      uint8_t nTRTHolesRefitted = nTRTHitsOriginal + nTRTHolesOriginal +
                                  nTRTOutliersOriginal - nTRTOutliersRefitted -
                                  nTRTHitsRefitted;

      created.setSummaryValue(nTRTHolesRefitted, xAOD::numberOfTRTHoles);
    }
  }
}
}  // namespace DerivationFramework
