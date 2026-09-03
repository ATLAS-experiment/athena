/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "InDetTrackSelectionAlg.h"

#include "ActsEvent/Decoration.h"
#include "ActsEvent/TrackContainer.h"
#include "FourMomUtils/xAODP4Helpers.h"

#include "Acts/Geometry/CylinderVolumeBounds.hpp"
#include "Acts/Geometry/TrackingGeometry.hpp"
#include "Acts/Utilities/HashedString.hpp"
#include "Acts/Surfaces/PlaneSurface.hpp"
#include "Acts/Surfaces/detail/PlanarHelper.hpp"
#include "Acts/Surfaces/CylinderBounds.hpp"
#include "Acts/Surfaces/DiscBounds.hpp"
#include "Acts/Utilities/VectorHelpers.hpp"
#include "Acts/Definitions/Units.hpp"
#include "ActsInterop/Logger.h"


#include "xAODMuonViews/FillContainer.h"
#include "MuonTrackEvent/ExpandedSector.h"
#include "MuonPatternEvent/SegmentFitterEventData.h"
#include "MuonReadoutGeometryR4/SpectrometerSector.h"
#include "MuonTrackEvent/TrackingHelpers.h"
#include "ActsEvent/CaloExtension.h"


#include "MuonTrackEvent/TrackMatchingUtils.h"

using namespace MuonR4::SegmentFit;
using namespace Acts::VectorHelpers;
using namespace Acts::UnitLiterals;

namespace {
    inline std::string print(const xAOD::TrackParticle& idTrack) {
        std::stringstream ostr{};
        ostr<<"track with pt: "<<(idTrack.pt() / Gaudi::Units::GeV)
            <<" [GeV], eta: "<<idTrack.eta()<<", phi: "<<idTrack.phi()<<", q: "<<idTrack.charge()<<
            ", chi2 (nDoF): "<<(idTrack.chiSquared() / idTrack.numberDoF())<<"("<<idTrack.numberDoF()<<") ";
        return ostr.str();
    }
    
    using IdCandidateCont_t = xAOD::FillContainer<MuonR4::MuonTagContainer, void*>;

}

using namespace ActsTrk;

namespace MuonCombinedR4 {

    StatusCode InDetTrackSelectionAlg::initialize() {
        ATH_CHECK(m_idTrkKey.initialize());
        ATH_CHECK(m_extensionDecorKey.initialize(m_useCaloExtension));
        ATH_CHECK(m_msTrkKey.initialize(m_matchWithMsTrk));
        ATH_CHECK(m_ctxProvider.initialize());
        ATH_CHECK(m_selectionTool.retrieve(EnableTool{!m_selectionTool.empty()}));
        ATH_CHECK(m_trackingGeometrySvc.retrieve());
        ATH_CHECK(m_extrapolationTool.retrieve());
        ATH_CHECK(m_writeKey.initialize());
        ATH_CHECK(m_segmentKey.initialize(m_matchWithSegs));
        ATH_CHECK(detStore()->retrieve(m_detMgr));
        if (m_trackingGeometrySvc->trackingGeometry()->geometryVersion() !=
            Acts::TrackingGeometry::GeometryVersion::Gen3){
            ATH_MSG_ERROR("The ID track selection alg requires the Gen 3 geometry format");
            return StatusCode::FAILURE;
        }
        m_logger = makeActsAthenaLogger(this, name());
        return StatusCode::SUCCESS;
    }
    StatusCode InDetTrackSelectionAlg::execute(const EventContext& ctx) const {
        const xAOD::TrackParticleContainer* idTracks{nullptr};
        const xAOD::TrackParticleContainer* msTracks{nullptr};
        const xAOD::MuonSegmentContainer* msSegments{nullptr};
        ATH_CHECK(SG::get(idTracks, m_idTrkKey, ctx));
        ATH_CHECK(SG::get(msSegments, m_segmentKey, ctx));
        ATH_CHECK(SG::get(msTracks, m_msTrkKey, ctx));

        const Acts::GeometryContext tgContext{m_ctxProvider.getGeometryContext(ctx)};

        using SegVec_t = std::vector<const xAOD::MuonSegment*>;
        SegVec_t uncombinedSegments{};
        if (msSegments) {
            uncombinedSegments.reserve(msSegments->size());
            std::copy_if(msSegments->begin(), msSegments->end(), 
                         std::back_inserter(uncombinedSegments),
                [msTracks, this](const xAOD::MuonSegment* segment){
                return !msTracks || std::none_of(msTracks->begin(), msTracks->end(),
                    [&segment, this](const xAOD::TrackParticle* msTrack) {
                    auto actsTrk = ActsTrk::getActsTrack(*msTrack);
                    if (!actsTrk) {
                        ATH_MSG_WARNING(__func__<<"() "<<__LINE__<<" - No acts track");
                        return false;
                    }
                    return Acts::rangeContainsValue(actsTrk->component<SegVec_t>("muonSegLinks"), segment);
                });
            });
        }
        ATH_MSG_DEBUG(__func__<<"() "<<__LINE__
            <<" - Select track candidates suitable for combined reconstruction amongst "
            <<idTracks->size()<<" ID tracks.");
        IdCandidateCont_t idCandidates{};
        std::vector<Acts::BoundTrackParameters> msTrkPars{};
        if (msTracks) {
            msTrkPars.reserve(msTracks->size());
            std::transform(msTracks->begin(), msTracks->end(), std::back_inserter(msTrkPars),
                           [](const xAOD::TrackParticle* trkPart){
                               return ActsTrk::getActsTrack(*trkPart)->createParametersAtReference();
                           });
        }
        for (const xAOD::TrackParticle* idTrk : *idTracks) {
            /* Track does not satisfy the kinematic requirements
             * Or the optional track quality */
            if (idTrk->pt () < m_trackPt || 
                (m_selectionTool.isEnabled() && !m_selectionTool->accept(*idTrk))) {
                continue;
            }
            /** Tracks within the MS eta range are attempted to be
                 extrapolated to the calo exit */
            auto idTag = std::make_unique<MuonR4::MuonTag>();
            idTag->setIdTrack(idTrk);
            if (std::abs(idTrk->eta()) < m_trackEta) {               
                /** Failed extrapolations to the calo exit -> track probably too
                  * low in momentum in order to be considered even for calo tagging  */
                auto parsAtEntrance = extrapolateToMsEntrance(ctx, *idTrk);
                if (!parsAtEntrance) {
                    continue;
                }
                /** Store the parameters at the calorimeter exit if the association to a
                    MS track or a segment is successful -> The tag is then available for
                    the combined fit, STACO, MuTagIMO && inside-> out chain */
                if (compatibleWithMsTrk(tgContext, *parsAtEntrance, msTrkPars) ||
                    compatibleWithSegment(tgContext, *parsAtEntrance, uncombinedSegments)) {
                    idTag->setExtrapolatedParsID(Acts::hashString("@CaloExit"),
                                                 std::move(*parsAtEntrance));
                }
            }
            idCandidates->push_back(std::move(idTag));
        }
        ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Selected "<<idCandidates->size()
            <<" candidates for combined reconstruction downstream");
        ATH_CHECK(idCandidates.record(m_writeKey, ctx));
        return StatusCode::SUCCESS;
    }
    std::optional<Acts::BoundTrackParameters>
        InDetTrackSelectionAlg::extrapolateToMsEntrance(const EventContext& ctx,
                                                        const xAOD::TrackParticle& idTrack) const {

        const auto trackingGeometry = m_trackingGeometrySvc->trackingGeometry();
    
        const Acts::GeometryContext tgContext = m_ctxProvider.getGeometryContext(ctx);
        const Acts::TrackingVolume* msEntrance = m_trackingGeometrySvc->getEnvelope(ActsTrk::SystemEnvelope::CaloExit);
        ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Extrapolate ID "<<print(idTrack)<<"\n to the calorimeter exit.\n"
                        <<msEntrance->volumeBounds()<<", id: "<<msEntrance->geometryId());
        /** Retrieve the last state of the track to extrapolate into the MS  */
        auto idExitPars = lastTrackParameters(idTrack);
        if (!idExitPars) {
            ATH_MSG_WARNING(__func__<<"() "<<__LINE__<<" - The ID track has no valid last state");
            return std::nullopt;
        }
        if (m_useCaloExtension) {
            const ActsTrk::CaloExtension* extension = ActsTrk::getCaloExtension(idTrack);
            if (extension != nullptr){
                auto lastExtensionPars = extension->lastParameters();
                if (lastExtensionPars->referenceSurface().geometryId().withBoundary(0) == 
                    msEntrance->geometryId()) {
                    ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Calo extension is on the MS entrance");
                    return lastExtensionPars;
                }
                idExitPars = lastExtensionPars;
            }
        }
        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Extrapolate ID track to MS entrance "
                        <<msEntrance->volumeBounds());
        auto caloPars = m_extrapolationTool->propagate(ctx, *idExitPars, *msEntrance, 
                            ActsTrk::IExtrapolationTool::VolumeAbort::atExit);
        if (caloPars.ok()) {
            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Extrapolation successful "<<(*caloPars));
            return *caloPars;
        }
        ATH_MSG_WARNING(__func__<<"() "<<__LINE__<<" - Failed to extrapolate ID "<<print(idTrack)<<".");
        return std::nullopt;
    }

    bool InDetTrackSelectionAlg::parametersCompatible(const Acts::GeometryContext& tgContext, 
                                                      const Acts::BoundTrackParameters& caloExitPars,
                                                      const Acts::BoundTrackParameters& msTrackPars) const {
        
        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Check MS track \n@ "<<msTrackPars
                                    <<", eta: "<<eta(msTrackPars));
        const double dPhi = std::abs(xAOD::P4Helpers::deltaPhi(msTrackPars.phi(), caloExitPars.phi())) ;
        const double dEta = std::abs(eta(msTrackPars) - eta(caloExitPars));
        if(dPhi> m_dPhiCutMsTrk || dEta > m_dEtaCutMsTrk) {
            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Angular cone does not match dEta: "
                <<dEta <<" vs "<<m_dEtaCutMsTrk<<" or dPhi: "<<dPhi<<" vs "<<m_dPhiCutMsTrk);
            return false;
        }
        if (!m_trackSameSurf) {
            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Same surface requirement disabled");
            return true;
        }
        if (msTrackPars.referenceSurface().geometryId().withBoundary(0) !=
            caloExitPars.referenceSurface().geometryId().withBoundary(0)) {
            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Reference surfaces do not belong "
                            <<" to the same volume. ");
            return false;
        }
        auto diffTrkPars = makeDiffParameters(tgContext, caloExitPars, 
                                              msTrackPars, logger(), m_dLoc0CutMsTrk);
        if (!diffTrkPars) {
            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Parameters cannot be expressed on same surface");
            return false;
        }
        return std::abs(longitudinalParam(*diffTrkPars, logger())) < m_dLoc0CutMsTrk &&
               std::abs(localPolarAngle(*diffTrkPars, logger())) < m_dLoc1CutMsTrk;
    }

    bool InDetTrackSelectionAlg::compatibleWithMsTrk(const Acts::GeometryContext& tgContext,
                                                     const Acts::BoundTrackParameters& itkParameters,
                                                     std::span<const Acts::BoundTrackParameters> msTrks) const {
        ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Check whether the parameters \n"<<itkParameters
            <<", eta: "<<eta(itkParameters)<<" are compatible with one of the "<<msTrks.size()<<" MS tracks.");
        if (std::ranges::any_of(msTrks, 
                [&](const Acts::BoundTrackParameters& msPars) {
                    return parametersCompatible(tgContext, itkParameters, msPars);
                })) {
            ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" Found a matching track.");
            return true;
        }
        return false;
    }
    bool InDetTrackSelectionAlg::compatibleWithSegment(const Acts::GeometryContext& tgContext,
                                                       const Acts::BoundTrackParameters& caloExitPars,
                                                       const std::span<const xAOD::MuonSegment*> candidateSegs) const {
        

        const Amg::Vector3D exitPos = caloExitPars.position(tgContext);
        const Amg::Vector3D exitDir = caloExitPars.direction();
        MuonR4::ExpandedSector exitSector{exitPos.phi()};
        const double caloEta = eta(caloExitPars);

        const MuonGMR4::SpectrometerSector* lastSector{nullptr};
        Amg::Vector3D extPosOnSector{Amg::Vector3D::Zero()};
        ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Check whether a compatible segment can be matched to \n"
                    <<caloExitPars<<"\n "<<Amg::toString(exitPos)<<" + "<<Amg::toString(exitDir));
        for (const xAOD::MuonSegment* segment : candidateSegs) {
            MuonR4::ExpandedSector segSector{segment->position().phi()};
            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Check compatibility with "<<MuonR4::printID(*segment)
                        <<", sector: "<<segSector);
            if (!segSector.isNeighbour(exitSector)) {
                ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Sector requirement failed");
                continue;
            }
            const double dEta = std::abs(segment->direction().eta() - caloEta);
            // Reject segments on oposite sides
            if (dEta > m_dEtaCutMsSeg) {
                ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Too large separation in dEta: "
                    <<dEta<<" cut: "<<m_dEtaCutMsSeg);
                continue;
            }
            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - eta separation: "<<dEta<<", max: "<<m_dEtaCutMsSeg);
            /** Straight line extrapolation onto the surface */
            const MuonGMR4::SpectrometerSector* msSector = m_detMgr->getSectorEnvelope(segment->chamberIndex(), 
                                                                                       segment->sector(), 
                                                                                       segment->etaIndex());
            if (lastSector != msSector) {
                lastSector = msSector;
                const Amg::Transform3D toLocal = msSector->globalToLocalTransform(tgContext);

                const Amg::Vector3D locExitPos = toLocal * exitPos;
                const Amg::Vector3D locExitDir = toLocal.linear()* exitDir;
                using namespace Acts::PlanarHelper;
                const auto iSect = intersectPlane(locExitPos, locExitDir, Amg::Vector3D::UnitZ(), 0.);
                extPosOnSector = iSect.position();
                ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Extrapolate "<<Amg::toString(locExitPos)<<" + "
                        <<Amg::toString(locExitDir)<<" to centre of "<<msSector->identString()
                        <<" --> "<<iSect.pathLength()<<", "<<Amg::toString(iSect.position()));
            }
            /** Segment parameters expressed on the envelope surface */
            Parameters segPars = localSegmentPars(*segment);
            const double dY = std::abs(segPars[Acts::toUnderlying(ParamDefs::y0)] - extPosOnSector.y());
            if (dY < m_dY0CutMsSeg) {
                ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Segment is close enough: "<<dY);
                return true;
            }
            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" -  Segment outside acceptance "<<dY<<" max: "<<m_dY0CutMsSeg);
        }
        ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - None of the "<<candidateSegs.size()
                     <<" segments is compatible with Id track\n"<<caloExitPars);
        return false;
    }
}
