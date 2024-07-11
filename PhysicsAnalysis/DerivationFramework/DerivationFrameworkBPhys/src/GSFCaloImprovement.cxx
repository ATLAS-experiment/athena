/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
  Contact: Tomas Jakoubek <tomas.jakoubek@cern.ch>
*/

#include "DerivationFrameworkBPhys/GSFCaloImprovement.h"

#include "TrkTrack/TrackCollection.h"

#include "xAODTracking/TrackParticleAuxContainer.h"
#include "xAODTracking/VertexContainer.h"
#include "xAODEgamma/ElectronContainer.h"

#include "egammaInterfaces/IegammaTrkRefitterTool.h"
#include "TrkToolInterfaces/ITrackParticleCreatorTool.h"
#include "TrkToolInterfaces/ITrackSummaryTool.h"
#include "xAODTruth/xAODTruthHelpers.h"
#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthParticleContainer.h"

#include "TrkPseudoMeasurementOnTrack/PseudoMeasurementOnTrack.h"

#include "CxxUtils/make_unique.h"

// ========================================================================

namespace DerivationFramework {
    GSFCaloImprovement::GSFCaloImprovement(const std::string& t, const std::string& n, const IInterface* p) : 
            AthAlgTool(t,n,p),
            m_particleCreatorTool("Trk::TrackParticleCreatorTool"),
            m_electronCollectionKey("Electrons") {

        declareInterface<DerivationFramework::IAugmentationTool>(this);

        declareProperty("electronCollectionKey"    , m_electronCollectionKey);
        declareProperty("ImprovedTrackRefitTool"   , m_trkImprovedRefitTool);
        declareProperty("TrackParticleCreatorTool" , m_particleCreatorTool);
        declareProperty("TrackSummaryTool"         , m_summaryTool);
        declareProperty("GSFCaloOutputName"        , m_gsfCaloOutputName = "GSFCaloContainer");
        declareProperty("minNSiHits"               , m_minNSiHits = 4);
        declareProperty("doTruth"                  , m_doTruth = false);
        declareProperty("isAOD"                    , m_isAOD   = true);
        declareProperty("usePixel"                 , m_doPix   = true);
        declareProperty("useSCT"                   , m_doSCT   = true);
        declareProperty("useTRT"                   , m_doTRT   = true);
    }

    GSFCaloImprovement::~GSFCaloImprovement() {
        // destructor
    }

    // ========================================================================

    StatusCode GSFCaloImprovement::RetrieveImprovedRefitTool(){
      if ( m_trkImprovedRefitTool.empty() ) {
        ATH_MSG_ERROR("CALO-improved refitter is empty");
        return StatusCode::FAILURE;
      }
      if ( m_trkImprovedRefitTool.retrieve().isFailure() ) {
        ATH_MSG_ERROR("Unable to retrieve "<< m_trkImprovedRefitTool);
        return StatusCode::FAILURE;
      }
      else ATH_MSG_INFO("Retrieved Tool " << m_trkImprovedRefitTool);
      return StatusCode::SUCCESS;
    }

    StatusCode GSFCaloImprovement::RetrieveParticleCreatorTool(){
      if ( m_particleCreatorTool.empty() ) {
        ATH_MSG_ERROR("ParticleCreator is empty");
        return StatusCode::FAILURE;
      }
      if ( m_particleCreatorTool.retrieve().isFailure() ) {
        ATH_MSG_ERROR("Unable to retrieve "<< m_particleCreatorTool);
        return StatusCode::FAILURE;
      }
      else ATH_MSG_INFO("Retrieved Tool " << m_particleCreatorTool);
      return StatusCode::SUCCESS;
    }

    StatusCode GSFCaloImprovement::RetrieveParticleSummaryTool(){
      if ( m_summaryTool.empty() ) {
        ATH_MSG_ERROR("ParticleSummary is empty");
        return StatusCode::FAILURE;
      }
      if ( m_summaryTool.retrieve().isFailure() ) {
        ATH_MSG_ERROR("Unable to retrieve "<< m_summaryTool);
        return StatusCode::FAILURE;
      }
      else ATH_MSG_INFO("Retrieved Tool " << m_summaryTool);
      return StatusCode::SUCCESS;
    }

    // ========================================================================

    StatusCode GSFCaloImprovement::initialize() {
        ATH_MSG_INFO("Initialize...");

        CHECK( RetrieveImprovedRefitTool() );
        CHECK( RetrieveParticleCreatorTool() );
        CHECK( RetrieveParticleSummaryTool() );

        m_allElectrons   = 0;
        m_noTP           = 0;
        m_onlyTRT        = 0;
        m_noTrk          = 0;
        m_failedFits     = 0;
        m_successfulFits = 0;
        m_noRefTP        = 0;
        m_allNewTP       = 0;

        ATH_MSG_INFO("TRT-only Trk::Track test: N_Si_hits < " << m_minNSiHits);

        return StatusCode::SUCCESS;
    }

    StatusCode GSFCaloImprovement::finalize() {
        ATH_MSG_INFO("Finalize...");

        ATH_MSG_INFO("Electrons:                          " << m_allElectrons);
        ATH_MSG_INFO("Electrons w/o xAOD::TrackParticle:  " << m_noTP);
        ATH_MSG_INFO("Electrons w/ TRT-only track:        " << m_onlyTRT);
        ATH_MSG_INFO("Electrons w/o Trk::Track:           " << m_noTrk);
        ATH_MSG_INFO("Number of failed fits:              " << m_failedFits);
        ATH_MSG_INFO("Number of successful fits:          " << m_successfulFits);
        ATH_MSG_INFO("Number of failed createParticle():  " << m_noRefTP);
        ATH_MSG_INFO("New container size:                 " << m_allNewTP);

        return StatusCode::SUCCESS;
    }

    // ========================================================================

    StatusCode GSFCaloImprovement::addBranches() const {
        // Retrieve input electron container
        const xAOD::ElectronContainer* electronContainer = nullptr;
        CHECK( evtStore()->retrieve(electronContainer, m_electronCollectionKey) );

        // Create the new GSF-CALO-refit TP containers
        auto gsfCaloContainer    = std::make_unique<xAOD::TrackParticleContainer>();
        auto gsfCaloAuxContainer = std::make_unique<xAOD::TrackParticleAuxContainer>();
        gsfCaloContainer->setStore( gsfCaloAuxContainer.get() );

        m_allElectrons += electronContainer->size();

        for ( auto electron : *electronContainer ) {
            const xAOD::TrackParticle* electronTP = electron->trackParticle();
            if ( ! electronTP ) {
                ATH_MSG_DEBUG("Can't get electron TrackParticle!");
                m_noTP++;
                continue;
            }

            // Electron has a xAOD::TrackParticle, proceed...

            xAOD::TrackParticle* electronTPRefit = nullptr;
            int gsfCaloStatus    = -1;
            float gsfChiSquareOverNDOF = -1;
            float gsfCaloChiSquareOverNDOF = -1;

            // Skip TRT-only tracks
            int nSiHits = 0;
            uint8_t dummy(0);
            if ( electronTP->summaryValue(dummy, xAOD::numberOfSCTHits) )       nSiHits += dummy;
            if ( electronTP->summaryValue(dummy, xAOD::numberOfPixelHits) )     nSiHits += dummy;
            if ( electronTP->summaryValue(dummy, xAOD::numberOfSCTOutliers) )   nSiHits += dummy;
            if ( electronTP->summaryValue(dummy, xAOD::numberOfPixelOutliers) ) nSiHits += dummy;

            if ( nSiHits >= m_minNSiHits ) {
                // We want to re-fit only "silicon" Trk::Tracks

                if ( electronTP->trackLink().isValid() ) {
                    // OK, we have a Trk::Track to re-fit...

                    auto electronTrackQuality = electronTP->track()->fitQuality();
                    gsfChiSquareOverNDOF = electronTrackQuality->chiSquared() / electronTrackQuality->numberDoF();
                    
                    auto electronTrackRefit = std::make_unique<Trk::Track>();
                    StatusCode status = m_trkImprovedRefitTool->refitElectronTrack( electron );
                    if ( status == StatusCode::SUCCESS ) {
                        electronTrackRefit.reset( m_trkImprovedRefitTool->refittedTrack() );

                        // TODO: Needed???
                        m_summaryTool->updateTrack( *electronTrackRefit );

                        gsfCaloStatus = 1;
                        m_successfulFits++;

                        // Get the vertex (may be pileup) that this track particle points to
                        const xAOD::Vertex* electronTrackVertex = nullptr;
                        if ( electronTP->vertexLink().isValid() ) {
                            // WIP: Keep it as a nullptr; same as in master; however, save the vertex information
                            // electronTrackVertex = electronTP->vertex();
                        }

                        electronTPRefit = m_particleCreatorTool->createParticle( *electronTrackRefit, gsfCaloContainer.get(), electronTrackVertex, xAOD::electron );
                        if ( electronTPRefit ) {
                            copyInfo(*electronTP, *electronTPRefit, ! m_isAOD);

                            auto electronTrackRefitQuality = electronTrackRefit->fitQuality();
                            gsfCaloChiSquareOverNDOF = electronTrackRefitQuality->chiSquared() / electronTrackRefitQuality->numberDoF();

                            // WIP: Add qOverP for the last measurement. Needed???
                            static const SG::AuxElement::Accessor<float> QoverPLM("QoverPLM");
                            float QoverPLast = 0;
                            auto rtsos = electronTrackRefit->trackStateOnSurfaces()->rbegin();
                            for ( ; rtsos != electronTrackRefit->trackStateOnSurfaces()->rend(); ++rtsos ) {
                                if ( (*rtsos)->type(Trk::TrackStateOnSurface::Measurement) && (*rtsos)->trackParameters() != 0 && (*rtsos)->measurementOnTrack() != 0 && ! dynamic_cast<const Trk::PseudoMeasurementOnTrack*>((*rtsos)->measurementOnTrack()) ) {
                                    QoverPLast = (*rtsos)->trackParameters()->parameters()[Trk::qOverP];
                                    break;
                                }
                            }
                            QoverPLM(*electronTPRefit) = QoverPLast;
                        } else {
                            ATH_MSG_DEBUG("Can't create a new xAOD::TrackParticle from the re-fitted Trk::Track! Using the original xAOD::TrackParticle!");
                            gsfCaloStatus = 0;
                            m_noRefTP++;
                        }
                    } else {
                        ATH_MSG_DEBUG("CALO-improved Trk::Track re-fit failed! Using the original TrackParticle!");
                        gsfCaloStatus = 0;
                        m_failedFits++;
                    }
                } else {
                    ATH_MSG_DEBUG("TrackParticle has no Trk::Track! Using the original TrackParticle!");
                    gsfCaloStatus = 0;
                    m_noTrk++;
                }
            } else {
                ATH_MSG_DEBUG("TrackParticle is TRT-only track! Using the original TrackParticle!");
                gsfCaloStatus = 0;
                m_onlyTRT++;
            }

            if ( gsfCaloStatus != 1 ) {
                electronTPRefit = new xAOD::TrackParticle();
                gsfCaloContainer->push_back( electronTPRefit );
                *electronTPRefit = *electronTP;
            }

            // Set a flag to distinguish between refitted and original TP later in the DAOD/ntuples
            electronTPRefit->auxdata<int>("gsfCaloStatus") = gsfCaloStatus;

            // WIP: Debug flags...
            electronTPRefit->auxdata<float>("gsfChiSquareOverNDOF")     = gsfChiSquareOverNDOF;
            electronTPRefit->auxdata<float>("gsfCaloChiSquareOverNDOF") = gsfCaloChiSquareOverNDOF;

            // TODO: Needed?
            electronTPRefit->setTrackLink( electronTP->trackLink() );
            electronTPRefit->setVertexLink( electronTP->vertexLink() );

            ElementLink<xAOD::TrackParticleContainer> linkToRefittedTrackParticle(electronTPRefit, *gsfCaloContainer);
            electron->auxdecor< ElementLink<xAOD::TrackParticleContainer> >("gsfCaloTrackParticleLink") = linkToRefittedTrackParticle;

            ElementLink<xAOD::ElectronContainer> linkToElectron(electron, *electronContainer);
            static const SG::AuxElement::Accessor<ElementLink<xAOD::ElectronContainer> > electronLink("usedElectronLink");
            electronLink(*electronTPRefit) = linkToElectron;

            static const SG::AuxElement::Accessor<ElementLink<xAOD::TrackParticleContainer> > tP("originalTrackParticle");
            if ( tP.isAvailable(*electronTP) ) {
                ElementLink<xAOD::TrackParticleContainer> linkToOriginal = tP(*electronTP);
                tP(*electronTPRefit) = linkToOriginal;
            }
        } // end of the electron container loop

        m_allNewTP += gsfCaloContainer->size();

        // Record the objects into the event store
        CHECK( evtStore()->record(gsfCaloContainer.release(), m_gsfCaloOutputName) );
        CHECK( evtStore()->record(gsfCaloAuxContainer.release(), m_gsfCaloOutputName + "Aux.") );

        return StatusCode::SUCCESS;
    }

    // ========================================================================

    void GSFCaloImprovement::copyInfo(const xAOD::TrackParticle& original, xAOD::TrackParticle& created, bool isRefitted) const {
        // Add Truth decorations. Copy from the original.
        if ( m_doTruth ) {
            static const SG::AuxElement::Accessor< ElementLink<xAOD::TruthParticleContainer> > tPL("truthParticleLink");
            if ( tPL.isAvailable(original) ) {
                const ElementLink<xAOD::TruthParticleContainer>& linkToTruth = tPL(original);
                tPL(created) = linkToTruth;
            }
            static const SG::AuxElement::Accessor<float> tMP("truthMatchProbability");
            if ( tMP.isAvailable(original) ) {
                float originalProbability = tMP(original);
                tMP(created) = originalProbability;
            }
            static const SG::AuxElement::Accessor<int> tT("truthType");
            if ( tT.isAvailable(original) ) {
                int truthType = tT(original);
                tT(created) = truthType;
            }
            static const SG::AuxElement::Accessor<int> tO("truthOrigin");
            if ( tO.isAvailable(original) ) {
                int truthOrigin = tO(original);
                tO(created) = truthOrigin;
            }
        }

        // It's apparently not possible to update Trk::Track when running on AOD; copy the values from the original xAOD::TrackParticle
        if ( m_isAOD ) {
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
        // Taken from https://gitlab.cern.ch/atlas/athena/-/blob/master/Reconstruction/egamma/egammaAlgs/src/EMBremCollectionBuilder.cxx?ref_type=heads#L329
        copySummaryValue(original, created, xAOD::numberOfPixelSplitHits);
        copySummaryValue(original, created, xAOD::numberOfInnermostPixelLayerSplitHits);
        copySummaryValue(original, created, xAOD::numberOfNextToInnermostPixelLayerSplitHits);
        copySummaryValue(original, created, xAOD::numberOfInnermostPixelLayerSharedHits);
        copySummaryValue(original, created, xAOD::numberOfNextToInnermostPixelLayerSharedHits);
        copySummaryValue(original, created, xAOD::numberOfBLayerSharedHits);
        copySummaryValue(original, created, xAOD::numberOfPixelSharedHits);
        copySummaryValue(original, created, xAOD::numberOfSCTSharedHits);
        copySummaryValue(original, created, xAOD::numberOfTRTSharedHits);

        if ( isRefitted ) {
            // Figure the new number of holes
            uint8_t dummy(0);

            if ( m_doPix ) {
                int nPixHitsRefitted     = created.summaryValue(dummy, xAOD::numberOfPixelHits)     ? dummy : -1;
                int nPixOutliersRefitted = created.summaryValue(dummy, xAOD::numberOfPixelOutliers) ? dummy : -1;

                int nPixHitsOriginal     = original.summaryValue(dummy, xAOD::numberOfPixelHits)     ? dummy : -1;
                int nPixOutliersOriginal = original.summaryValue(dummy, xAOD::numberOfPixelOutliers) ? dummy : -1;
                int nPixHolesOriginal    = original.summaryValue(dummy, xAOD::numberOfPixelHoles)    ? dummy : -1;

                uint8_t nPixHolesRefitted = nPixHitsOriginal + nPixHolesOriginal + nPixOutliersOriginal - nPixOutliersRefitted - nPixHitsRefitted;

                created.setSummaryValue(nPixHolesRefitted, xAOD::numberOfPixelHoles);
            }

            if ( m_doSCT ) {
                int nSCTHitsRefitted     = created.summaryValue(dummy, xAOD::numberOfSCTHits)     ? dummy : -1;
                int nSCTOutliersRefitted = created.summaryValue(dummy, xAOD::numberOfSCTOutliers) ? dummy : -1; 

                int nSCTHitsOriginal     = original.summaryValue(dummy, xAOD::numberOfSCTHits)     ? dummy : -1;
                int nSCTHolesOriginal    = original.summaryValue(dummy, xAOD::numberOfSCTHoles)    ? dummy : -1;
                int nSCTOutliersOriginal = original.summaryValue(dummy, xAOD::numberOfSCTOutliers) ? dummy : -1;

                uint8_t nSCTHolesRefitted = nSCTHitsOriginal + nSCTHolesOriginal + nSCTOutliersOriginal - nSCTOutliersRefitted - nSCTHitsRefitted;

                created.setSummaryValue(nSCTHolesRefitted, xAOD::numberOfSCTHoles);
            }

            if ( m_doTRT ) {
                int nTRTHitsRefitted     = created.summaryValue(dummy, xAOD::numberOfTRTHits)     ? dummy : -1;
                int nTRTOutliersRefitted = created.summaryValue(dummy, xAOD::numberOfTRTOutliers) ? dummy : -1;

                int nTRTHitsOriginal     = original.summaryValue(dummy, xAOD::numberOfTRTHits)     ? dummy : -1;
                int nTRTHolesOriginal    = original.summaryValue(dummy, xAOD::numberOfTRTHoles)    ? dummy : -1;
                int nTRTOutliersOriginal = original.summaryValue(dummy, xAOD::numberOfTRTOutliers) ? dummy : -1;

                uint8_t nTRTHolesRefitted = nTRTHitsOriginal + nTRTHolesOriginal + nTRTOutliersOriginal - nTRTOutliersRefitted - nTRTHitsRefitted;

                created.setSummaryValue(nTRTHolesRefitted, xAOD::numberOfTRTHoles);
            }
        }
    }
}
