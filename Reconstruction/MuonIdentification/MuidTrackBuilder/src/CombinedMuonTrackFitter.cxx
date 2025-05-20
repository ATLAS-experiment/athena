/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

//////////////////////////////////////////////////////////////////////////////
// CombinedMuonTrackFitter
//  AlgTool gathering  material effects along a combined muon track, in
//  particular the TSOS'es representing the calorimeter energy deposit and
//  Coulomb scattering.
//  The resulting track is fitted at the IP
//
//////////////////////////////////////////////////////////////////////////////

#include "CombinedMuonTrackFitter.h"

#include <cmath>
#include <iomanip>
#include <memory>
#include "AthenaKernel/Units.h"
#include "EventPrimitives/EventPrimitivesCovarianceHelpers.h"
#include "EventPrimitives/EventPrimitivesToStringConverter.h"
#include "FourMomUtils/xAODP4Helpers.h"
#include "MuonRIO_OnTrack/MdtDriftCircleOnTrack.h"
#include "TrkCompetingRIOsOnTrack/CompetingRIOsOnTrack.h"
#include "TrkEventUtils/IdentifierExtractor.h"
#include "TrkExUtils/TrackSurfaceIntersection.h"
#include "TrkGeometry/TrackingGeometry.h"
#include "TrkGeometry/TrackingVolume.h"
#include "TrkMaterialOnTrack/EnergyLoss.h"
#include "TrkMaterialOnTrack/MaterialEffectsOnTrack.h"
#include "TrkMaterialOnTrack/ScatteringAngles.h"
#include "TrkMeasurementBase/MeasurementBase.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkPseudoMeasurementOnTrack/PseudoMeasurementOnTrack.h"
#include "TrkSurfaces/CylinderSurface.h"
#include "TrkSurfaces/PerigeeSurface.h"
#include "TrkSurfaces/PlaneSurface.h"
#include "TrkSurfaces/RotatedTrapezoidBounds.h"
#include "TrkSurfaces/Surface.h"
#include "TrkSurfaces/TrapezoidBounds.h"
#include "TrkTrack/Track.h"
#include "TrkTrackSummary/MuonTrackSummary.h"
#include "TrkTrackSummary/TrackSummary.h"
#include "VxVertex/RecVertex.h"
#include "muonEvent/CaloEnergy.h"

namespace Rec {
    CombinedMuonTrackFitter::~CombinedMuonTrackFitter() = default;
    CombinedMuonTrackFitter::CombinedMuonTrackFitter(const std::string& type, const std::string& name, const IInterface* parent) :
        AthAlgTool(type, name, parent) {
        declareInterface<ICombinedMuonTrackFitter>(this);


    }

    StatusCode CombinedMuonTrackFitter::initialize() {
        ATH_MSG_DEBUG("Initializing CombinedMuonTrackFitter.");
        ATH_MSG_DEBUG(" with options: ");

        if (m_allowCleanerVeto) ATH_MSG_DEBUG(" AllowCleanerVeto");
        if (!m_muonErrorOptimizer.empty()) ATH_MSG_DEBUG(" ErrorOptimisation");

        // fill WARNING messages
        m_messageHelper->setMaxNumberOfMessagesPrinted(m_maxWarnings);
        m_messageHelper->setMessage(0, "combinedFit:: missing MeasuredPerigee for indet track");
        m_messageHelper->setMessage(1, "combinedFit:: fail with MS removed by cleaner");
        m_messageHelper->setMessage(2, "combinedFit:: fail with perigee outside indet");
        m_messageHelper->setMessage(3, "combinedFit:: fail with missing caloEnergy");
        m_messageHelper->setMessage(4, "combinedFit:: final combined track lost, this should not happen");
        m_messageHelper->setMessage(5, "indetExtension:: reject with insufficient MS measurements");
        m_messageHelper->setMessage(6, "standaloneFit:: input vertex fails dynamic_cast");
        m_messageHelper->setMessage(7, "standaloneFit:: missing MeasuredPerigee for spectrometer track");
        m_messageHelper->setMessage(8, "standaloneFit:: missing TrackParameters on prefit");
        m_messageHelper->setMessage(9, "standaloneFit:: prefit fails parameter extrapolation to calo");
        m_messageHelper->setMessage(10, "standaloneFit:: extrapolated track missing TrackParameters at calo scatterer");
        m_messageHelper->setMessage(11, "standaloneFit:: final track lost, this should not happen");
        m_messageHelper->setMessage(12, "standaloneFit:: fail as calo incorrectly described");
        m_messageHelper->setMessage(13, "standaloneRefit:: fail track as no TSOS with type CaloDeposit");
        m_messageHelper->setMessage(14, "standaloneRefit:: no inner material");
        m_messageHelper->setMessage(15, "standaloneRefit:: no inner parameters");
        m_messageHelper->setMessage(16, "standaloneRefit:: innerScattering dynamic_cast failed");
        m_messageHelper->setMessage(17, "standaloneRefit:: no TSOS of type CaloDeposit found");
        m_messageHelper->setMessage(18, "standaloneRefit:: no inner scattering TSOS found");
        m_messageHelper->setMessage(19, "standaloneRefit:: no middle material");
        m_messageHelper->setMessage(20, "standaloneRefit:: no middle parameters");
        m_messageHelper->setMessage(21, "standaloneRefit:: no CaloDeposit TSOS found");
        m_messageHelper->setMessage(22, "standaloneRefit:: no outer material");
        m_messageHelper->setMessage(23, "standaloneRefit:: no outer parameters");
        m_messageHelper->setMessage(24, "standaloneRefit:: outerScattering dynamic_cast failed");
        m_messageHelper->setMessage(25, "standaloneRefit:: no outerScattering or CaloDeposit TSOS found");
        m_messageHelper->setMessage(26, "standaloneRefit:: failed propagation to innerTSOS");
        m_messageHelper->setMessage(27, "standaloneRefit:: failed propagation to middleTSOS");
        m_messageHelper->setMessage(28, "standaloneRefit:: fail as calo incorrectly described");
        m_messageHelper->setMessage(29, "fit:: particle hypothesis must be 0 or 2 (nonInteracting or muon). Requested: ");
        m_messageHelper->setMessage(30, "fit:: about to add the TSOS's describing calorimeter association to a combined muon");
        m_messageHelper->setMessage(31, "fit:: particle hypothesis must be 0 or 2 (nonInteracting or muon). Requested: ");
        m_messageHelper->setMessage(32, "fit:: particle hypothesis must be 0 or 2 (nonInteracting or muon). Requested: ");
        m_messageHelper->setMessage(33, "fit:: combined muon track is missing the TSOS's describing calorimeter association");
        m_messageHelper->setMessage(34, "appendSelectedTSOS:: skip duplicate measurement ");
        m_messageHelper->setMessage(35, "caloEnergyParameters:: muonTrack without caloEnergy association");
        m_messageHelper->setMessage(36, "caloEnergyParameters:: combinedTrack without caloEnergy association");
        m_messageHelper->setMessage(37, "createMuonTrack:: should never happen: FSR caloEnergy delete");
        m_messageHelper->setMessage(38, "createSpectrometerTSOS:: missing MeasuredPerigee for spectrometer track");
        m_messageHelper->setMessage(39, "createSpectrometerTSOS:: skip unrecognized TSOS without TrackParameters. Type: ");
        m_messageHelper->setMessage(40, "createSpectrometerTSOS:: skip duplicate measurement on same Surface. Type: ");
        m_messageHelper->setMessage(41, "entrancePerigee:: missing TrackingGeometrySvc - no perigee will be added at MS entrance");
        m_messageHelper->setMessage(42, "extrapolatedParameters:: missing MeasuredPerigee for spectrometer track");
        m_messageHelper->setMessage(43, "extrapolatedParameters:: missing spectrometer parameters on spectrometer track");
        m_messageHelper->setMessage(44, "final track lost, this should not happen");
        m_messageHelper->setMessage(45, "momentumUpdate:: update failed, keeping original value");
        m_messageHelper->setMessage(46, "reallocateMaterial:: null perigeeStartValue");
        m_messageHelper->setMessage(47, "reallocateMaterial:: refit fails");
        m_messageHelper->setMessage(48, "standaloneFit:: insufficient measurements on input spectrometer track");
        m_messageHelper->setMessage(49, "standaloneFit:: inconsistent TSOS on input spectrometer track");

        ATH_CHECK(m_printer.retrieve());
        ATH_CHECK(m_fieldCacheCondObjInputKey.initialize());
        ATH_MSG_DEBUG("Setup handle for key " << m_fieldCacheCondObjInputKey);
        ATH_CHECK(m_muonErrorOptimizer.retrieve(DisableTool{m_muonErrorOptimizer.empty()}));

        ATH_CHECK(m_caloTSOS.retrieve());
        ATH_MSG_DEBUG("Retrieved tool " << m_caloTSOS);
        ATH_CHECK(m_cleaner.retrieve());
        ATH_MSG_DEBUG("Retrieved tool " << m_cleaner);

        ATH_CHECK(m_fitter.retrieve());
        ATH_CHECK(m_fitterSL.retrieve());
        ATH_CHECK(m_idHelperSvc.retrieve());

        ATH_CHECK(m_trackingVolumesSvc.retrieve());
        ATH_MSG_DEBUG("Retrieved Svc " << m_trackingVolumesSvc);
        m_calorimeterVolume = std::make_unique<Trk::Volume>(m_trackingVolumesSvc->volume(Trk::ITrackingVolumesSvc::MuonSpectrometerEntryLayer));
        m_indetVolume = std::make_unique<Trk::Volume>(m_trackingVolumesSvc->volume(Trk::ITrackingVolumesSvc::CalorimeterEntryLayer));

        ATH_CHECK(m_trackQuery.retrieve());
        ATH_MSG_DEBUG("Retrieved tool " << m_trackQuery);
        ATH_CHECK(m_trackSummary.retrieve());
        ATH_MSG_DEBUG("Retrieved tool " << m_trackSummary);
        ATH_CHECK(m_materialUpdator.retrieve());
        ATH_MSG_DEBUG("Retrieved tool " << m_materialUpdator);

        return StatusCode::SUCCESS;
    }

    StatusCode CombinedMuonTrackFitter::finalize() {
        ATH_MSG_INFO("Finalizing CombinedMuonTrackFitter:"
                        << m_countStandaloneCleanerVeto << " standalone fits with cleaner veto" << endmsg << "     "
                        << m_countExtensionCleanerVeto << " extension fits with cleaner veto" << endmsg << "     "
                        << m_countCombinedCleanerVeto << " combined fits with cleaner veto");
        // // summarize WARNINGs
        m_messageHelper->printSummary();
        return StatusCode::SUCCESS;
    }
    std::unique_ptr<Trk::Track> CombinedMuonTrackFitter::fit(const EventContext& ctx, const Trk::Track& track,
                                                              const Trk::RunOutlierRemoval runOutlier,
                                                              const Trk::ParticleHypothesis particleHypothesis) const {
        ATH_MSG_VERBOSE(" fit() " << m_printer->print(track) << std::endl
                                  << m_printer->printMeasurements(track) << std::endl
                                  << m_printer->printStations(track));
        // check valid particleHypothesis
        if (particleHypothesis != Trk::muon && particleHypothesis != Trk::nonInteracting) {
            // invalid particle hypothesis
            std::stringstream ss;
            ss << particleHypothesis;
            m_messageHelper->printWarning(29, ss.str());
            return nullptr;
        }

        // check if combined or subsystem track
        bool isCombined = m_trackQuery->isCombined(track, ctx);
        // select straightLine fitter when magnets downstream of leading measurement are off
        const Trk::ITrackFitter* fitter = m_fitter.get();
        MagField::AtlasFieldCache fieldCache;
        // Get field cache object

        if (!loadMagneticField(ctx, fieldCache)) return nullptr;

        if (!fieldCache.toroidOn() && !(isCombined && fieldCache.solenoidOn())) {
            fitter = m_fitterSL.get();
            ATH_MSG_VERBOSE(" fit (track refit method): select SL fitter ");
        }

        // redo ROTs:  ID, CROT and MDT specific treatments
        // if (m_redoRots) redoRots(track);

        // perform fit after ensuring calo is associated for combined tracks
        // calo association for combined tracks (WARN if missing from input)
        std::unique_ptr<Trk::Track> fittedTrack = std::make_unique<Trk::Track>(track);
        if (isCombined && particleHypothesis == Trk::muon && !m_trackQuery->isCaloAssociated(*fittedTrack, ctx)) {
            // about to add the TSOS's describing calorimeter association to a combined muon;
            ATH_MSG_VERBOSE( "fit:: about to add the TSOS's describing calorimeter association to a combined muon" );

            auto combinedTSOS = std::make_unique<Trk::TrackStates>();

            combinedTSOS->reserve(fittedTrack->trackStateOnSurfaces()->size() + 3);
            bool caloAssociated = false;

            // run-2 schema, update default eloss with parametrised value
            if (m_useCaloTG) {
                ATH_MSG_VERBOSE("Updating Calorimeter TSOS in Muon Combined (re)Fit ...");
                m_materialUpdator->updateCaloTSOS(*fittedTrack);
                caloAssociated = true;
            }

            for (const Trk::TrackStateOnSurface* in_tsos : *fittedTrack->trackStateOnSurfaces()) {
                if (caloAssociated) {
                    combinedTSOS->push_back(in_tsos->clone());
                } else if ((in_tsos->measurementOnTrack() && m_indetVolume->inside(in_tsos->measurementOnTrack()->globalPosition())) ||
                           (in_tsos->trackParameters() && m_indetVolume->inside(in_tsos->trackParameters()->position()))) {
                    combinedTSOS->push_back(in_tsos->clone());
                } else {
                    std::unique_ptr<const Trk::TrackStateOnSurface> tsos = m_caloTSOS->innerTSOS(ctx, *fittedTrack->perigeeParameters());
                    if (tsos) {
                        combinedTSOS->push_back(std::move(tsos));
                        const Trk::TrackParameters* parameters = combinedTSOS->back()->trackParameters();
                        if (in_tsos->type(Trk::TrackStateOnSurface::CaloDeposit)) {
                            combinedTSOS->push_back(in_tsos->clone());
                            tsos = m_caloTSOS->outerTSOS(ctx, *parameters);
                            if (tsos) combinedTSOS->push_back(std::move(tsos));
                        } else {
                            tsos = m_caloTSOS->middleTSOS(ctx, *parameters);
                            if (tsos) combinedTSOS->push_back(std::move(tsos));
                            tsos = m_caloTSOS->outerTSOS(ctx, *parameters);
                            if (tsos) combinedTSOS->push_back(std::move(tsos));
                            combinedTSOS->push_back(in_tsos->clone());
                        }
                    }
                    caloAssociated = true;
                }
            }

            std::unique_ptr<Trk::Track> combinedTrack = std::make_unique<Trk::Track>(fittedTrack->info(), std::move(combinedTSOS), nullptr);

            if (msgLevel(MSG::DEBUG)) countAEOTs(*combinedTrack, " combinedTrack track before fit ");

            caloAssociated = m_trackQuery->isCaloAssociated(*combinedTrack, ctx);

            // Updates the calo TSOS with the ones from TG+corrections
            if (m_updateWithCaloTG && !m_useCaloTG && particleHypothesis == Trk::muon) {
                ATH_MSG_VERBOSE("Updating Calorimeter TSOS in Muon Combined (re)Fit ...");
                m_materialUpdator->updateCaloTSOS(*combinedTrack);
            }
            // FIT
            fittedTrack = fitter->fit(ctx, *combinedTrack, false, particleHypothesis);
        } else {
            // Updates the calo TSOS with the ones from TG+corrections
            if (m_updateWithCaloTG && !m_useCaloTG && particleHypothesis == Trk::muon) {
                ATH_MSG_VERBOSE("Updating Calorimeter TSOS in Muon Standalone Fit ...");
                m_materialUpdator->updateCaloTSOS(*fittedTrack);
            }

            // FIT
            fittedTrack = fitter->fit(ctx, *fittedTrack, false, particleHypothesis);
        }

        // quit if fit has failed
        if (!fittedTrack) return nullptr;


        if (!checkTrack("fitInterface1", fittedTrack.get())) return nullptr;


        // eventually this whole tool will use unique_ptrs
        // in the meantime, this allows the MuonErrorOptimisationTool and MuonRefitTool to use them
        // track cleaning
        if (runOutlier) {
            // fit with optimized spectrometer errors

            const double chi2BeforeOptimizer = normalizedChi2(*fittedTrack);
            if (!m_muonErrorOptimizer.empty() && !fittedTrack->info().trackProperties(Trk::TrackInfo::StraightTrack) &&
                optimizeErrors(ctx, *fittedTrack)) {
                ATH_MSG_VERBOSE(" perform spectrometer error optimization after cleaning ");
                std::unique_ptr<Trk::Track> optimizedTrack = m_muonErrorOptimizer->optimiseErrors(*fittedTrack, ctx);
                if (checkTrack("fitInterface1Opt", optimizedTrack.get()) && chi2BeforeOptimizer > normalizedChi2(*optimizedTrack)) {
                    fittedTrack.swap(optimizedTrack);
                    if (msgLevel(MSG::DEBUG)) countAEOTs(*fittedTrack, " re fit scaled errors Track ");
                }
            }

            // chi2 before clean
            const double chi2Before = normalizedChi2(*fittedTrack);

            // muon cleaner
            ATH_MSG_VERBOSE(__FILE__<<":"<<__LINE__<<" perform track cleaning... " << m_printer->print(*fittedTrack) << std::endl
                                                          << m_printer->printStations(*fittedTrack));

            if (msgLevel(MSG::DEBUG)) countAEOTs(*fittedTrack, " refit: fitted track before cleaning ");

            std::unique_ptr<Trk::Track> cleanTrack = m_cleaner->clean(*fittedTrack, ctx);

            if (msgLevel(MSG::DEBUG)) countAEOTs(*cleanTrack, " refit: after cleaning");

            if (!checkTrack("fitInterface1Cleaner", cleanTrack.get())) { cleanTrack.reset(); }

            if (!cleanTrack) {
                if (m_allowCleanerVeto && chi2Before > m_badFitChi2) {
                    ATH_MSG_DEBUG(" cleaner veto A "<<chi2Before<<" "<<m_badFitChi2<<" "<<m_printer->printMeasurements(*fittedTrack) );
                    ++m_countStandaloneCleanerVeto;
                    fittedTrack.reset();
                } else {
                    ATH_MSG_DEBUG(" keep original standalone track despite cleaner veto ");
                }
            } else if (!(*cleanTrack->perigeeParameters() == *fittedTrack->perigeeParameters())) {
                double chi2After = normalizedChi2(*cleanTrack);

                if (chi2After < m_badFitChi2 || chi2After < chi2Before) {
                    ATH_MSG_VERBOSE(" found and removed spectrometer outlier(s) ");
                    fittedTrack.swap(cleanTrack);
                } else {
                    ATH_MSG_VERBOSE(" keep original track despite cleaning ");
                }
            }

            // FIXME: provide indet cleaner
            if (fittedTrack) {
                ATH_MSG_VERBOSE(" finished track cleaning... " << m_printer->print(*fittedTrack) << std::endl
                                                               << m_printer->printStations(*fittedTrack));
            }
        }
        return fittedTrack;
    }

    std::unique_ptr<Trk::Track> CombinedMuonTrackFitter::fit(const EventContext& ctx, const Trk::MeasurementSet& measurementSet,
                                                              const Trk::TrackParameters& perigeeStartValue,
                                                              const Trk::RunOutlierRemoval runOutlier,
                                                              const Trk::ParticleHypothesis particleHypothesis) const {
        // check valid particleHypothesis
        if (particleHypothesis != Trk::muon && particleHypothesis != Trk::nonInteracting) {
            // invalid particle hypothesis
            std::stringstream ss;
            ss << particleHypothesis;
            m_messageHelper->printWarning(31, ss.str());
            return nullptr;
        }

        // select straightLine fitter when magnets downstream of leading measurement are off
        MagField::AtlasFieldCache fieldCache;
        // Get field cache object
        if (!loadMagneticField(ctx, fieldCache)) return nullptr;

        const Trk::ITrackFitter* fitter = m_fitter.get();
        if (!fieldCache.toroidOn() || std::abs(perigeeStartValue.position().z()) > m_zECToroid) {
            fitter = m_fitterSL.get();
            ATH_MSG_VERBOSE(" fit (track refit method): select SL fitter ");
        }

        // redo ROTs:  ID, CROT and MDT specific treatments
        // if (m_redoRots) redoRots(track);

        // calo association (if relevant)

        // create Perigee if starting parameters given for a different surface type
        std::unique_ptr<Trk::TrackParameters> perigee = perigeeStartValue.uniqueClone();
        std::unique_ptr<Trk::PerigeeSurface> perigeeSurface;

        if (perigee->surfaceType() != Trk::SurfaceType::Perigee) {
            Amg::Vector3D origin(perigeeStartValue.position());
            perigeeSurface = std::make_unique<Trk::PerigeeSurface>(origin);

            perigee = std::make_unique<Trk::Perigee>(perigeeStartValue.position(), perigeeStartValue.momentum(), perigeeStartValue.charge(),
                                                     *perigeeSurface);
        }

        // FIT
        std::unique_ptr<Trk::Track> fittedTrack(fitter->fit(ctx, measurementSet, *perigee, false, particleHypothesis));

        if (!checkTrack("fitInterface2", fittedTrack.get())) { return nullptr; }

        // eventually this whole tool will use unique_ptrs
        // in the meantime, this allows the MuonErrorOptimisationTool and MuonRefitTool to use them

        // track cleaning
        if (runOutlier) {
            // fit with optimized spectrometer errors

            if (!m_muonErrorOptimizer.empty() && !fittedTrack->info().trackProperties(Trk::TrackInfo::StraightTrack) &&
                optimizeErrors(ctx, *fittedTrack)) {
                ATH_MSG_VERBOSE(" perform spectrometer error optimization after cleaning ");
                std::unique_ptr<Trk::Track> optimizedTrack = m_muonErrorOptimizer->optimiseErrors(*fittedTrack, ctx);
                if (checkTrack("fitInterface2Opt", optimizedTrack.get())) {
                    fittedTrack.swap(optimizedTrack);
                    if (msgLevel(MSG::DEBUG)) countAEOTs(*fittedTrack, " fit mstSet scaled errors Track ");
                }

            }

            // chi2 before clean
            double chi2Before = normalizedChi2(*fittedTrack);

            // muon cleaner
            ATH_MSG_VERBOSE(__FILE__<<":"<<__LINE__<<" perform track cleaning... ");

            if (msgLevel(MSG::DEBUG)) countAEOTs(*fittedTrack, " fit mstSet before cleaning ");

            std::unique_ptr<Trk::Track> cleanTrack = m_cleaner->clean(*fittedTrack, ctx);

            if (msgLevel(MSG::DEBUG)) countAEOTs(*cleanTrack, " fit mstSet clean Track ");

            if (!checkTrack("fitInterface2Cleaner", cleanTrack.get())) { cleanTrack.reset(); }

            if (!cleanTrack) {
                if (m_allowCleanerVeto && chi2Before > m_badFitChi2) {
                    ATH_MSG_DEBUG(" cleaner veto B");
                    ++m_countExtensionCleanerVeto;
                    fittedTrack.reset();
                } else {
                    ATH_MSG_DEBUG(" keep original extension track despite cleaner veto ");
                }
            } else if (!(*cleanTrack->perigeeParameters() == *fittedTrack->perigeeParameters())) {
                double chi2After = normalizedChi2(*cleanTrack);
                if (chi2After < m_badFitChi2 || chi2After < chi2Before) {
                    ATH_MSG_VERBOSE(" found and removed spectrometer outlier(s) ");
                    fittedTrack.swap(cleanTrack);
                } else {
                    ATH_MSG_VERBOSE(" keep original track despite cleaning ");
                }
            }

            // FIXME: provide indet cleaner
            ATH_MSG_VERBOSE(" Finished cleaning");
        }
        // have to use release until the whole code uses unique_ptr
        return fittedTrack;
    }

    /**combined muon fit */
    std::unique_ptr<Trk::Track> CombinedMuonTrackFitter::fit(const EventContext& ctx, const Trk::Track& indetTrack,
                                                              Trk::Track& extrapolatedTrack, const Trk::RunOutlierRemoval runOutlier,
                                                              const Trk::ParticleHypothesis particleHypothesis) const {
        // check valid particleHypothesis
        if (particleHypothesis != Trk::muon && particleHypothesis != Trk::nonInteracting) {
            // invalid particle hypothesis
            std::stringstream ss;
            ss << particleHypothesis;
            m_messageHelper->printWarning(32, ss.str());
            return nullptr;
        }

        // select straightLine fitter when solenoid and toroid are off
        const Trk::ITrackFitter* fitter = m_fitter.get();
        MagField::AtlasFieldCache fieldCache;
        // Get field cache object
        if (!loadMagneticField(ctx, fieldCache)) return nullptr;

        if (!fieldCache.toroidOn() && !fieldCache.solenoidOn()) {
            fitter = m_fitterSL.get();
            ATH_MSG_VERBOSE(" fit (combined muon fit method): select SL fitter ");
        }

        // redo ROTs:  ID, CROT and MDT specific treatments

        // calo association (for now just WARN if missing)
        if (particleHypothesis == Trk::muon && !m_trackQuery->isCaloAssociated(extrapolatedTrack, ctx)) {
            // combined muon track is missing the TSOS's describing calorimeter association
            m_messageHelper->printWarning(33);
        }

        // Updates the calo TSOS with the ones from TG+corrections
        if (m_updateWithCaloTG && !m_useCaloTG && particleHypothesis == Trk::muon) {
            ATH_MSG_VERBOSE("Updating Calorimeter TSOS in Muon Combined Fit ...");
            m_materialUpdator->updateCaloTSOS(indetTrack, extrapolatedTrack);
        }

        // FIT
        ATH_MSG_VERBOSE(" perform combined fit... " << std::endl
                                                    << m_printer->print(indetTrack) << std::endl
                                                    << m_printer->print(extrapolatedTrack));

        std::unique_ptr<Trk::Track> fittedTrack(fitter->fit(ctx, indetTrack, extrapolatedTrack, false, particleHypothesis));

        if (!fittedTrack) return nullptr;
        // track cleaning
        if (runOutlier) {
            // fit with optimized spectrometer errors

            if (!m_muonErrorOptimizer.empty() && !fittedTrack->info().trackProperties(Trk::TrackInfo::StraightTrack) &&
                optimizeErrors(ctx, *fittedTrack)) {
                ATH_MSG_VERBOSE(" perform spectrometer error optimization after cleaning ");
                std::unique_ptr<Trk::Track> optimizedTrack = m_muonErrorOptimizer->optimiseErrors(*fittedTrack, ctx);
                if (checkTrack("Error opt", optimizedTrack.get()) &&
                    normalizedChi2(*optimizedTrack) < normalizedChi2(*fittedTrack)) {
                    fittedTrack.swap(optimizedTrack);
                    if (msgLevel(MSG::DEBUG)) countAEOTs(*fittedTrack, " cbfit scaled errors Track ");
                }
            }

            // chi2 before clean
            double chi2Before = normalizedChi2(*fittedTrack);

            // muon cleaner
            ATH_MSG_VERBOSE(__FILE__<<":"<<__LINE__<<" perform track cleaning... " << m_printer->print(*fittedTrack) << std::endl
                                                          << m_printer->printStations(*fittedTrack));

            if (msgLevel(MSG::DEBUG)) { countAEOTs(*fittedTrack, " cb before clean Track "); }
            std::unique_ptr<Trk::Track> cleanTrack = m_cleaner->clean(*fittedTrack, ctx);
	    if (cleanTrack && msgLevel(MSG::DEBUG)) { countAEOTs(*cleanTrack, " cb after clean Track "); }

            if (!cleanTrack) {
                if (m_allowCleanerVeto && chi2Before > m_badFitChi2) {
                    ATH_MSG_DEBUG("cleaner veto C "<<chi2Before<<" Cut: "<<m_badFitChi2);
                    ++m_countCombinedCleanerVeto;
                    fittedTrack.reset();
                } else {
                    ATH_MSG_DEBUG(" keep original combined track despite cleaner veto ");
                }
            } else if (!(*cleanTrack->perigeeParameters() == *fittedTrack->perigeeParameters())) {
                double chi2After = normalizedChi2(*cleanTrack);
                if (chi2After < m_badFitChi2 || chi2After < chi2Before) {
                    ATH_MSG_VERBOSE(" found and removed spectrometer outlier(s) ");
                    fittedTrack.swap(cleanTrack);
                } else {
                    ATH_MSG_VERBOSE(" keep original track despite cleaning ");
                }
            }

            // FIXME: provide indet cleaner
            ATH_MSG_VERBOSE(" finished cleaning");
        }
        // have to use release until the whole code uses unique_ptr
        return fittedTrack;
    }

    /*   private methods follow */
    double CombinedMuonTrackFitter::normalizedChi2(const Trk::Track& track) const {
        double chi2 = 999999.;
        if (track.fitQuality()) {
            if (track.fitQuality()->numberDoF()) {
                chi2 = track.fitQuality()->chiSquared() / track.fitQuality()->doubleNumberDoF();
            } else {
                chi2 = m_badFitChi2;
            }
        }

        return chi2;
    }

    bool CombinedMuonTrackFitter::loadMagneticField(const EventContext& ctx, MagField::AtlasFieldCache& fieldCache) const {
        SG::ReadCondHandle<AtlasFieldCacheCondObj> fieldCondObj{m_fieldCacheCondObjInputKey, ctx};
        if (!fieldCondObj.isValid()) {
            ATH_MSG_ERROR("Failed to retrieve AtlasFieldCacheCondObj with key " << m_fieldCacheCondObjInputKey.key());
            return false;
        }
        fieldCondObj->getInitializedCache(fieldCache);
        return true;
    }
    bool CombinedMuonTrackFitter::optimizeErrors(const EventContext& ctx, Trk::Track& track) const {
        const Trk::MuonTrackSummary* muonSummary = nullptr;
        const Trk::TrackSummary* summary = track.trackSummary();

        if (summary) {
            muonSummary = summary->muonTrackSummary();
        } else {
            m_trackSummary->updateTrack(ctx, track);
            summary = track.trackSummary();
            muonSummary = summary->muonTrackSummary();
        }

        if (!muonSummary) return false;


        unsigned int optimize{0},nBarrel{0}, nEndcap{0}, nSmall{0}, nLarge{0};

        for (const Trk::MuonTrackSummary::ChamberHitSummary& summary : muonSummary->chamberHitSummary()) {
            const Identifier& id = summary.chamberId();
            bool isMdt = m_idHelperSvc->isMdt(id);
            if (!isMdt) continue;

            Muon::MuonStationIndex::StIndex stIndex = m_idHelperSvc->stationIndex(id);

            if (stIndex == Muon::MuonStationIndex::BE) { optimize = 1; }

            if (stIndex == Muon::MuonStationIndex::BI && m_idHelperSvc->chamberIndex(id) == Muon::MuonStationIndex::BIS &&
                std::abs(m_idHelperSvc->stationEta(id)) > 6) {
                optimize = 2;
            }

            if (stIndex == Muon::MuonStationIndex::BI || stIndex == Muon::MuonStationIndex::BM || stIndex == Muon::MuonStationIndex::BO ||
                stIndex == Muon::MuonStationIndex::BE) {
                nBarrel++;
            }

            if (stIndex == Muon::MuonStationIndex::EI || stIndex == Muon::MuonStationIndex::EM || stIndex == Muon::MuonStationIndex::EO ||
                stIndex == Muon::MuonStationIndex::EE) {
                nEndcap++;
            }

            if (m_idHelperSvc->isSmallChamber(id)) {
                nSmall++;
            } else {
                nLarge++;
            }
        }

        if (nBarrel > 0 && nEndcap > 0) { optimize += 10; }

        if (nSmall > 0 && nLarge > 0) { optimize += 100; }

        if (optimize > 0) { ATH_MSG_DEBUG(" OptimizeErrors with value " << optimize); }

        return optimize > 0;
    }

    bool CombinedMuonTrackFitter::checkTrack(std::string_view txt, const Trk::Track* newTrack) const {
        if (!newTrack) return false;

        const DataVector<const Trk::TrackParameters>* pars = newTrack->trackParameters();
        if (!pars || pars->empty() || !newTrack->fitQuality()) { return false; }
        DataVector<const Trk::TrackParameters>::const_iterator it = pars->end() -1;

        if ((*it)->position().dot((*it)->momentum()) < 0) {
            return false;
            ATH_MSG_DEBUG(txt <<" "<< __FILE__<<":"<<__LINE__<< " ALARM position " << (*it)->position() << " direction " << (*it)->momentum().unit());
        } else {
            ATH_MSG_DEBUG(txt <<" "<< __FILE__<<":"<<__LINE__<< " OK position " << (*it)->position() << " direction " << (*it)->momentum().unit());
        }

        for (const Trk::TrackParameters* par : *pars) {
            if (!par->covariance()) { continue; }
            if (!Amg::hasPositiveDiagElems(*par->covariance())) {
                ATH_MSG_DEBUG(txt<<" "<<__FILE__<<":"<<__LINE__<< "covariance matrix has negative diagonal element, killing track "
                              <<std::endl<<Amg::toString(*par->covariance()));
                return false;
            }
        }
        unsigned int numberMS{0}, numberMSPrec{0};
        /// Check that the combined track contains enough MS measurements
        Trk::TrackStates::const_reverse_iterator r = newTrack->trackStateOnSurfaces()->rbegin();
        Trk::TrackStates::const_reverse_iterator rEnd = newTrack->trackStateOnSurfaces()->rend();
        for (; r != rEnd; ++r) {
            const Trk::TrackStateOnSurface* tsos{*r};
            if (tsos->trackParameters() && m_calorimeterVolume->inside(tsos->trackParameters()->position())) break;

            if (tsos->measurementOnTrack()) {
                ++numberMS;
                const Trk::RIO_OnTrack* rot = dynamic_cast<const Trk::RIO_OnTrack*>(tsos->measurementOnTrack());
                numberMSPrec+= rot && !m_idHelperSvc->measuresPhi(rot->identify());
            }
        }

        ATH_MSG_VERBOSE( txt<< " "<<__FILE__<<":"<<__LINE__<<" "<< numberMS << "/"<< numberMSPrec<< " fitted MS measurements ");
         // reject with insufficient MS measurements
        if (numberMS < 5 || numberMSPrec < 3) {
            return false;
        }

        return true;
    }

    unsigned int CombinedMuonTrackFitter::countAEOTs(const Trk::Track& track, const std::string& txt) const {
        const Trk::TrackStates* trackTSOS = track.trackStateOnSurfaces();
        unsigned int naeots = 0;

	if (!trackTSOS){
	  ATH_MSG_ERROR("No trackStateOnSurfaces");
	  return naeots;
	}

        for (const auto* m : *trackTSOS) {
	  if (m && m->alignmentEffectsOnTrack()) naeots++;
        }

        ATH_MSG_DEBUG(" count AEOTs " << txt << " " << naeots);

        // add VEBOSE for checking TSOS order


        int tsos{0}, nperigee{0};
        for ( const Trk::TrackStateOnSurface* it : *trackTSOS) {
            tsos++;

            if (it->type(Trk::TrackStateOnSurface::Perigee)) {
                ATH_MSG_DEBUG("perigee");
                nperigee++;
            }

            if (it->trackParameters()) {
                ATH_MSG_VERBOSE(" check tsos " << tsos << " TSOS tp "
                                               << " r " << it->trackParameters()->position().perp() << " z "
                                               << it->trackParameters()->position().z() << " momentum "
                                               << it->trackParameters()->momentum().mag());
            } else if (it->measurementOnTrack()) {
                ATH_MSG_VERBOSE(" check tsos " << tsos << " TSOS mst "
                                               << " r " << it->measurementOnTrack()->associatedSurface().center().perp() << " z "
                                               << it->measurementOnTrack()->associatedSurface().center().z());
            } else if (it->materialEffectsOnTrack()) {
                ATH_MSG_VERBOSE(" check tsos " << tsos << " TSOS mat "
                                               << " r "
                                               << it->materialEffectsOnTrack()->associatedSurface().globalReferencePoint().perp()
                                               << " z " << it->materialEffectsOnTrack()->associatedSurface().globalReferencePoint().z());
            } else {
                ATH_MSG_VERBOSE(" check tsos other than above " << tsos);
            }
        }

        ATH_MSG_VERBOSE(" track with number of TSOS perigees " << nperigee);

        return naeots;
    }
}  // namespace Rec
