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

#include "xAODMuonViews/FillContainer.h"
#include "MuonTrackEvent/ExpandedSector.h"
#include "MuonPatternEvent/SegmentFitterEventData.h"
#include "MuonReadoutGeometryR4/SpectrometerSector.h"
#include "MuonTrackEvent/TrackingHelpers.h"

using namespace MuonR4::SegmentFit;

namespace {
    inline double eta(const Acts::BoundTrackParameters& pars) {
        return -std::log(std::tan(pars.theta() / 2.));
    }
    inline std::string print(const xAOD::TrackParticle& idTrack) {
        std::stringstream ostr{};
        ostr<<"track with pt: "<<(idTrack.pt() / Gaudi::Units::GeV)
            <<" [GeV], eta: "<<idTrack.eta()<<", phi: "<<idTrack.phi()<<", q: "<<idTrack.charge()<<
            ", chi2 (nDoF): "<<(idTrack.chiSquared() / idTrack.numberDoF())<<"("<<idTrack.numberDoF()<<") ";
        return ostr.str();
    }
    /** @brief Enum value to retrieve the inner cylinder surface from the portals */
    constexpr auto cylinderFace = Acts::toUnderlying(Acts::CylinderVolumeBounds::Face::OuterCylinder);
    /** @brie Enume value to retrieve the two endcap discs from the entrance portals */
    constexpr auto faceSideA = Acts::toUnderlying(Acts::CylinderVolumeBounds::Face::PositiveDisc);
    constexpr auto faceSideC = Acts::toUnderlying(Acts::CylinderVolumeBounds::Face::NegativeDisc);

    using IdCandidateCont_t = xAOD::FillContainer<MuonR4::MuonTagContainer, void*>;

}

using namespace ActsTrk;

namespace MuonCombinedR4 {

    StatusCode InDetTrackSelectionAlg::initialize() {
        ATH_CHECK(m_idTrkKey.initialize());
        ATH_CHECK(m_msTrkKey.initialize());
        ATH_CHECK(m_selectionTool.retrieve(EnableTool{!m_selectionTool.empty()}));
        ATH_CHECK(m_trackingGeometryTool.retrieve());
        ATH_CHECK(m_extrapolationTool.retrieve());
        ATH_CHECK(m_writeKey.initialize());
        ATH_CHECK(m_segmentKey.initialize());
        ATH_CHECK(detStore()->retrieve(m_detMgr));
        if (m_trackingGeometryTool->trackingGeometry()->geometryVersion() !=
            Acts::TrackingGeometry::GeometryVersion::Gen3){
            ATH_MSG_ERROR("The ID track selection alg requires the Gen 3 geometry format");
            return StatusCode::FAILURE;
        }
        return StatusCode::SUCCESS;
    }
    StatusCode InDetTrackSelectionAlg::execute(const EventContext& ctx) const {
        const xAOD::TrackParticleContainer* idTracks{nullptr};
        const MuonR4::MuonTagContainer* msTags{nullptr};
        const xAOD::MuonSegmentContainer* msSegments{nullptr};
        ATH_CHECK(SG::get(idTracks, m_idTrkKey, ctx));
        ATH_CHECK(SG::get(msSegments, m_segmentKey, ctx));
        ATH_CHECK(SG::get(msTags, m_msTrkKey, ctx));

        std::vector<const xAOD::MuonSegment*> uncombinedSegments{};
        uncombinedSegments.reserve(msSegments->size());
        std::copy_if(msSegments->begin(), msSegments->end(), std::back_inserter(uncombinedSegments),
                    [msTags](const xAOD::MuonSegment* segment){
                        return std::none_of(msTags->begin(), msTags->end(),
                                            [&segment](const MuonR4::MuonTag* tag){
                                                return !Acts::rangeContainsValue(tag->segments(), segment);
                                            });
                    });

        ATH_MSG_DEBUG(__func__<<"() "<<__LINE__
            <<" - Select track candidates suitable for combined reconstruction amongst "
            <<idTracks->size()<<" ID tracks.");
        IdCandidateCont_t idCandidates{};

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
                if (compatibleWithMsTrk(*parsAtEntrance, *msTags) ||
                    compatibleWithSegment(ctx, *parsAtEntrance, uncombinedSegments)) {
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

        const auto trackingGeometry = m_trackingGeometryTool->trackingGeometry();
    
        const Acts::GeometryContext tgContext = m_trackingGeometryTool->getGeometryContext(ctx).context();
        const Acts::TrackingVolume* msEntrance = m_trackingGeometryTool->getEnvelope(ActsTrk::SystemEnvelope::CaloExit);
        ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Extrapolate ID "<<print(idTrack)<<"\n to the calorimeter exit.\n"
                        <<msEntrance->volumeBounds()<<", id: "<<msEntrance->geometryId());
        /** Retrieve the last state of the track to extrapolate into the MS  */
        auto lastState = lastMeasurementState(idTrack);
        if (!lastState) {
            ATH_MSG_WARNING(__func__<<"() "<<__LINE__<<" - The ID track has no valid last state");
            return std::nullopt;
        }
        /** Construct the track parameter from it  */
        const Acts::BoundTrackParameters idExitPars = getActsTrack(idTrack)->createParametersFromState(*lastState);
        /** @todo We need to check whether we can retrieve the track parameters from the 
                 calo extension provided by the Egamma / Jet ETmiss group. */
        const Acts::GeometryIdentifier barrelId = msEntrance->geometryId().withBoundary(1 + cylinderFace);
        const Acts::GeometryIdentifier endcapId = msEntrance->geometryId().withBoundary(1 + (idTrack.eta() > 0 ? faceSideA : faceSideC));        
        const Acts::Surface* barrelEntance = trackingGeometry->findSurface(barrelId);
        const Acts::Surface* endcapDisc = trackingGeometry->findSurface(endcapId);
        
        const Acts::Surface* target = std::abs(idTrack.eta()) < 1.1 ? barrelEntance : endcapDisc;

        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Target: "<<target->toString(tgContext)
            <<", "<<target->geometryId()<<", alignable: "<<target->isAlignable()<<".");
        auto caloPars = m_extrapolationTool->propagate(ctx, idExitPars, *target);
        if (caloPars.ok()) {
            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Extrapolation successful "<<(*caloPars));
            return *caloPars;
        }
        if (std::abs(idTrack.eta()) > 0.9 && (target == barrelEntance)){
            ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" -  Track is in the transition region. Try the endcap as target.");
            caloPars = m_extrapolationTool->propagate(ctx, idExitPars, *endcapDisc);
        } else if (std::abs(idTrack.eta()) < 1.2 && (target == endcapDisc)) {
            ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" -  Track is in the transition region. Try the barrel as target.");
            caloPars = m_extrapolationTool->propagate(ctx, idExitPars, *barrelEntance);
        }
        if (caloPars.ok()) {
            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Transition recovery succeeded.");
            return *caloPars;
        }
        ATH_MSG_WARNING(__func__<<"() "<<__LINE__<<" - Failed to extrapolate ID "<<print(idTrack)<<".");
        return std::nullopt;
    }

    bool InDetTrackSelectionAlg::compatibleWithMsTrk(const Acts::BoundTrackParameters& itkParameters,
                                                     const MuonR4::MuonTagContainer& msTrks) const {

        ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Check whether the "<<itkParameters
            <<" are compatible with one of the "<<msTrks.size()<<" MS tracks.");
        if (std::any_of(msTrks.begin(), msTrks.end(), 
                [&](const MuonR4::MuonTag* saTag) {
                    const xAOD::TrackParticle* msTrack = saTag->msTrack();
                    ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Check MS "<<print(*msTrack));
                    return std::abs(msTrack->eta() - eta(itkParameters)) < m_dEtaCutMsTrk &&
                           std::abs(xAOD::P4Helpers::deltaPhi(msTrack->phi(),
                                                              itkParameters.phi())) < m_dPhiCutMsTrk;
                })) {
            ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" Found a matching track.");
            return true;
        }
        return false;
    }
    bool InDetTrackSelectionAlg::compatibleWithSegment(const EventContext& ctx,
                                                       const Acts::BoundTrackParameters& caloExitPars,
                                                       const std::span<const xAOD::MuonSegment*> candidateSegs) const {
        
        const ActsTrk::GeometryContext& gctx{m_trackingGeometryTool->getGeometryContext(ctx)};
        const Amg::Vector3D exitPos = caloExitPars.position(gctx.context());
        const Amg::Vector3D exitDir = caloExitPars.direction();
        MuonR4::ExpandedSector exitSector{exitPos.phi()};
        const double caloEta = eta(caloExitPars);

        const MuonGMR4::SpectrometerSector* lastSector{nullptr};
        Amg::Vector3D extPosOnSector{Amg::Vector3D::Zero()};

        for (const xAOD::MuonSegment* segment : candidateSegs) {

            MuonR4::ExpandedSector segSector{segment->position().phi()};
            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Check compatibility with "<<MuonR4::printID(*segment)
                        <<", sector: "<<segSector);
            if (!segSector.isNeighbour(exitSector)) {
                ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Sector requirement failed");
                continue;
            }
            // Reject segments on oposite sides
            if (std::abs(segment->direction().eta() - caloEta) > m_dEtaCutMsSeg) {
                ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Too large separation in dEta: "
                    <<std::abs(segment->direction().eta() - caloEta)<<" cut: "<<m_dEtaCutMsSeg);
                continue;
            }
            /** Straight line extrapolation onto the surface */
            const MuonGMR4::SpectrometerSector* msSector = m_detMgr->getSectorEnvelope(segment->chamberIndex(), 
                                                                                       segment->sector(), 
                                                                                       segment->etaIndex());
            /** Segment parameters expressed on the envelope surface */
            Parameters segPars = localSegmentPars(*segment);

            if (lastSector == msSector) {
                if (std::abs(segPars[Acts::toUnderlying(ParamDefs::y0)] - extPosOnSector.y()) < m_dY0CutMsSeg) {
                    ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" Segment is close enough: "
                                <<std::abs(segPars[Acts::toUnderlying(ParamDefs::y0)] - extPosOnSector.y()));
                    return true;
                }
                continue;
            }
            lastSector = msSector;
                                                    
            const Amg::Transform3D toLocal = msSector->globalToLocalTransform(gctx);

            const Amg::Vector3D locExitPos = toLocal * exitPos;
            const Amg::Vector3D locExitDir = toLocal.linear()* exitDir;

            using namespace Acts::PlanarHelper;
            auto iSect = intersectPlane(locExitPos, locExitDir, Amg::Vector3D::Zero(), 0.);
            extPosOnSector = iSect.position();
            if (std::abs(segPars[Acts::toUnderlying(ParamDefs::y0)] - extPosOnSector.y()) < m_dY0CutMsSeg) {
                ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" Segment is close enough: "
                                <<std::abs(segPars[Acts::toUnderlying(ParamDefs::y0)] - extPosOnSector.y()));
                return true;
            }
        }
        return false;
    }
}