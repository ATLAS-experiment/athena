/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "SegmentTaggingAlg.h"

#include "Acts/Utilities/Helpers.hpp"
#include "Acts/Surfaces/PlaneSurface.hpp"
#include "Acts/Utilities/StringHelpers.hpp"

#include "MuonTrackEvent/ExpandedSector.h"
#include "MuonTrackEvent/TrackingHelpers.h"
#include "MuonPatternEvent/SegmentFitterEventData.h"
#include "xAODMuonViews/FillContainer.h"

namespace{
    constexpr Acts::HashedString caloExitParKey = Acts::hashString("@CaloExit");
    using namespace Muon::MuonStationIndex;
    using MuTagCont_t = xAOD::FillContainer<MuonR4::MuonTagContainer, void*>;

}

namespace MuonCombinedR4  {
    StatusCode SegmentTaggingAlg::initialize() {
        ATH_CHECK(m_idTrkKey.initialize());
        ATH_CHECK(m_cmbTrkKey.initialize(SG::AllowEmpty));
        ATH_CHECK(m_segmentKey.initialize());
        ATH_CHECK(m_writeKey.initialize());
        ATH_CHECK(detStore()->retrieve(m_detMgr));
        ATH_CHECK(m_trackingGeometryTool.retrieve());
        ATH_CHECK(m_extrapolationTool.retrieve());
        return StatusCode::SUCCESS;
    }
    std::vector<const xAOD::MuonSegment*> SegmentTaggingAlg::prepareSegments(const EventContext& ctx) const {
        const MuonR4::MuonTagContainer* combinedTrks{nullptr};
        const xAOD::MuonSegmentContainer* segments{nullptr};
 
        if (!SG::get(combinedTrks, m_cmbTrkKey, ctx).isSuccess() ||
            !SG::get(segments, m_segmentKey, ctx).isSuccess()) {
            THROW_EXCEPTION("Failed to retrieve the input.");
        }
        std::vector<const xAOD::MuonSegment*> pickedSegments{};
        pickedSegments.reserve(segments->size());
        std::copy_if(segments->begin(), segments->end(), std::back_inserter(pickedSegments),
                    [&combinedTrks](const xAOD::MuonSegment* segment){
                        return !combinedTrks || std::none_of(combinedTrks->begin(), combinedTrks->end(),
                                            [&segment](const MuonR4::MuonTag* tag){
                                                return !Acts::rangeContainsValue(tag->segments(), segment);
                                            });
                    });
        return pickedSegments;
    }
    const Acts::Surface& SegmentTaggingAlg::getSurface(const xAOD::MuonSegment& segment) const {
        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Retrieve surface on which segment "
                    <<MuonR4::printID(segment)<<" is expressed.");
        return m_detMgr->getSectorEnvelope(segment.chamberIndex(), segment.sector(), 
                                           segment.etaIndex())->surface();
    }   
    
    std::vector<const xAOD::MuonSegment*> 
        SegmentTaggingAlg::findPotentialMatches(const Acts::GeometryContext& tgContext,
                                                const Acts::BoundTrackParameters& caloExitPars,
                                                const std::vector<const xAOD::MuonSegment*>& segmentCont) const {
        std::vector<const xAOD::MuonSegment*> matches{};
        
        const MuonR4::ExpandedSector caloSector{caloExitPars.phi()};
        const Amg::Vector3D globExit = caloExitPars.position(tgContext);
        const Amg::Vector3D globDir = caloExitPars.direction();
        for (const xAOD::MuonSegment* matchMe : segmentCont) {
            /** Check whether the segment is in the same sector */
            if (!caloSector.isNeighbour(MuonR4::ExpandedSector{matchMe->position().phi()})){
                continue;
            }
            if (matchMe->position().z() * globExit.z() < 0.) {
                continue;
            }
            /// Check that the surface is indeed in front of the exit parameters
            if ((getSurface(*matchMe).center(tgContext) - globExit).dot(globDir) < 0.){
                continue;
            }
            /// Check that the straight line roughly matches

            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - "<<MuonR4::printID(*matchMe)<<" is compatible with parameters @"
                            <<Amg::toString(globExit)<<" + "<<Amg::toString(globDir));
            matches.push_back(matchMe);
        }
        std::ranges::sort(matches,[&](const xAOD::MuonSegment* a,
                                      const xAOD::MuonSegment* b) {
            const Acts::Surface& sA = getSurface(*a);
            const Acts::Surface& sB = getSurface(*b);
            if (&sA == &sB){
                using namespace MuonR4::SegmentFit;                
                return localSegmentPars(*a)[Acts::toUnderlying(ParamDefs::y0)] <
                       localSegmentPars(*b)[Acts::toUnderlying(ParamDefs::y0)];
            }
            return (sA.center(tgContext) - globExit).dot(globDir) < 
                   (sB.center(tgContext) - globExit).dot(globDir);
        });
        ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Found "<<matches.size()<<" candidates.");
        return matches; 
    }
    double SegmentTaggingAlg::matchingScore(const xAOD::MuonSegment& segment,
                                            const Acts::BoundTrackParameters& extpIdPars) const  {
        
        Acts::BoundTrackParameters segmentPars{MuonR4::SegmentFit::boundSegmentPars(*m_detMgr, segment)};
        Acts::BoundVector dPars = segmentPars.parameters() - extpIdPars.parameters();
        /** The segment does not measure phi. Reset anything in loc0 and non-precision direction */
        if (!segment.nPhiLayers()) {
            dPars[Acts::eBoundPhi] = dPars[Acts::eBoundLoc0] = 0.;
        }
        dPars[Acts::eBoundQOverP] = dPars[Acts::eBoundTime] = 0.;
        Acts::BoundMatrix covariance{Acts::BoundMatrix::Identity()};
        covariance(Acts::eBoundLoc0, Acts::eBoundLoc0) = Acts::square(m_toleranceX0.value());
        covariance(Acts::eBoundLoc1, Acts::eBoundLoc1) = Acts::square(m_toleranceY0.value());
        covariance(Acts::eBoundTheta, Acts::eBoundTheta) = Acts::square(m_toleranceTheta.value());
        covariance(Acts::eBoundPhi, Acts::eBoundPhi) = Acts::square(m_tolerancePhi.value());
        if (extpIdPars.covariance()) {
            covariance += (*extpIdPars.covariance());
        }
        if (segmentPars.covariance()) {
            covariance += (*segmentPars.covariance());
        }
        const double chi2 = dPars.dot(covariance.inverse()*dPars);
        ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Difference: "<<Acts::toString(dPars)
                        <<", covariance: \n"<<Acts::toString(covariance)<<",\nchi2: "
                        <<chi2);
        return chi2;
    }
    std::unique_ptr<MuonR4::MuonTag> 
            SegmentTaggingAlg::tagSegments(const EventContext& ctx,
                                           std::vector<const xAOD::MuonSegment*>&& selectedCandidates,
                                           std::unique_ptr<MuonR4::MuonTag>&& idTag) const {

        std::optional<Acts::BoundTrackParameters> currentPars = idTag->extrapolatedParsID(caloExitParKey);
        
        const Acts::Surface* currentSurface{nullptr};

        const xAOD::MuonSegment* bestMatch{nullptr};
        double bestChi2{std::numeric_limits<double>::max()};

        /** Loop over the segments and attempt to match them with the ID track */
        for (auto segIter = selectedCandidates.begin(); segIter!= selectedCandidates.end(); ) {
            const xAOD::MuonSegment* matchMe{*segIter};

            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" Try to match the segment "<<MuonR4::printID(*matchMe));

            const Acts::Surface& target = getSurface(*matchMe);
            /// Attempt to extrapolate onto the target
            if (&target != currentSurface) {
                ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Atempt to extrapolate to associated surface "
                    <<target.toString(m_trackingGeometryTool->getGeometryContext(ctx).context()));
                auto surfPars = m_extrapolationTool->propagate(ctx, *currentPars, target);
                /** Extrapolation failed */
                if (!surfPars.ok()) {
                    ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__
                        <<" - Extrapolation failed. Skipp all other segments on the surface.");
                    segIter = std::find_if(segIter, selectedCandidates.end(),
                        [&](const xAOD::MuonSegment* failedSeg) {
                            const Acts::Surface& skipSurface = getSurface(*failedSeg);
                            return &skipSurface != &target;
                        });
                    continue;
                }
                currentSurface = &target;
                currentPars = (*surfPars);
                if (bestChi2 < m_matchChi2) {
                    idTag->setSegments(std::array{bestMatch});
                }
                bestChi2 = std::numeric_limits<double>::max();
                ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Extrapolation succeeded: "<<(*currentPars));
                idTag->setExtrapolatedParsID(Acts::toUnderlying(matchMe->chamberIndex()), std::move(*surfPars));          
            }
            ++segIter;
            /// Now apply the matching
            const double chi2 = matchingScore(*matchMe, *currentPars);
            if (chi2 < bestChi2) {
                bestChi2 = chi2;
                bestMatch = matchMe;
            }
        }
        if (bestChi2 < m_matchChi2) {
            idTag->setSegments(std::array{bestMatch});
        }
        if (idTag->segments().empty()) {
            return nullptr;
        }

        /** Calculate the max dEta and dPhi variables */
        float maxDeta{0.};
        float maxDPhi{0.};
        for (const xAOD::MuonSegment* matched : idTag->segments()) {
            const Amg::Vector3D segDir = matched->direction();
            const Amg::Vector3D parDir = idTag->extrapolatedParsID(Acts::toUnderlying(matched->chamberIndex()))->direction();
            const float dEta = parDir.eta() - segDir.eta();
            const float dPhi = parDir.deltaPhi(segDir) * (matched->nPhiLayers() ? 1. : 0.);
            maxDeta = std::copysign(std::max(std::abs(dEta), maxDeta), dEta);
            maxDPhi = std::copysign(std::max(std::abs(dPhi), maxDPhi), dPhi);
            
        }
        idTag->setParameter(xAOD::Muon::ParamDef::segmentDeltaEta, maxDeta);
        idTag->setParameter(xAOD::Muon::ParamDef::segmentDeltaPhi, maxDPhi);
        return idTag;
    }
    StatusCode SegmentTaggingAlg::execute(const EventContext& ctx) const {
        const MuonR4::MuonTagContainer* idTracks{nullptr};
        const MuonR4::MuonTagContainer* combinedTrks{nullptr};
        ATH_CHECK(SG::get(idTracks, m_idTrkKey, ctx));  
        ATH_CHECK(SG::get(combinedTrks, m_cmbTrkKey, ctx));

        const std::vector<const xAOD::MuonSegment*> candidateSegs = prepareSegments(ctx);
        ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Try to match "<<idTracks->size()
            <<" ID tracks to "<<candidateSegs.size()<<" segments.");
        const Acts::GeometryContext tgContext = m_trackingGeometryTool->getGeometryContext(ctx).context();

        
        MuTagCont_t outContainer{};
        for (const MuonR4::MuonTag* candidate : *idTracks) {
            if (combinedTrks && std::any_of(combinedTrks->begin(), combinedTrks->end(),
                                            [candidate](const MuonR4::MuonTag* cmbTag){
                                                return candidate->idTrack() == cmbTag->idTrack();
                                            })) {
                ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Track already used in combined reconstruction");
                continue;
            }
            auto caloExitPars = candidate->extrapolatedParsID(caloExitParKey);
            if (!caloExitPars) {
                ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Track has no calo exit pars");
                continue;
            }

            std::vector<const xAOD::MuonSegment*> segsToMatch = findPotentialMatches(tgContext, *caloExitPars,
                                                                                     candidateSegs);
            if (segsToMatch.empty()) {
                continue;
            }
            auto segTag = std::make_unique<MuonR4::MuonTag>();
            segTag->setAuthor(xAOD::Muon::Author::MuTagIMO);
            segTag->setIdTrack(candidate->idTrack());
            segTag->setExtrapolatedParsID(caloExitParKey, std::move(*caloExitPars));

            segTag = tagSegments(ctx, std::move(segsToMatch), std::move(segTag));
            if (segTag) {
                ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Segment tagging succeeded with "
                                <<segTag->segments().size());
                outContainer->push_back(std::move(segTag));
            }
        }
        ATH_CHECK(outContainer.record(m_writeKey, ctx));
        return StatusCode::SUCCESS;
    }
}