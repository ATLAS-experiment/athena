/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonChamberHoleRecoveryTool.h"

#include <map>

#include "EventPrimitives/EventPrimitivesHelpers.h"
#include "EventPrimitives/EventPrimitivesCovarianceHelpers.h"
#include "GeoPrimitives/GeoPrimitivesToStringConverter.h"
#include "MuonCompetingRIOsOnTrack/CompetingMuonClustersOnTrack.h"
#include "MuonPrepRawData/MuonCluster.h"
#include "MuonRIO_OnTrack/CscClusterOnTrack.h"
#include "MuonRIO_OnTrack/MMClusterOnTrack.h"
#include "MuonRIO_OnTrack/MdtDriftCircleOnTrack.h"
#include "MuonRIO_OnTrack/RpcClusterOnTrack.h"
#include "MuonRIO_OnTrack/TgcClusterOnTrack.h"
#include "MuonRIO_OnTrack/sTgcClusterOnTrack.h"
#include "MuonReadoutGeometry/MdtReadoutElement.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonSegment/MuonSegment.h"
#include "MuonTrackMakerUtils/MuonTSOSHelper.h"
#include "MuonTrackMakerUtils/MuonTrackMakerStlTools.h"
#include "MuonTrackMakerUtils/SortMeasurementsByPosition.h"
#include "TrkEventPrimitives/ResidualPull.h"
#include "TrkGeometry/MagneticFieldProperties.h"
#include "TrkMeasurementBase/MeasurementBase.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkPseudoMeasurementOnTrack/PseudoMeasurementOnTrack.h"
#include "TrkSurfaces/StraightLineSurface.h"
#include "TrkVolumes/Volume.h"

namespace {
    struct PullCluster {
        double pull{std::numeric_limits<double>::max()};
        std::unique_ptr<Trk::TrackParameters> pars{};
        std::unique_ptr<Muon::MuonClusterOnTrack> clus{};
    };
    using ClusterLayerMap = std::map<Identifier, PullCluster>;

}  // namespace
namespace Muon {
    using NewTrackStates = MuonChamberHoleRecoveryTool::NewTrackStates;


    MuonChamberHoleRecoveryTool::RecoveryState::RecoveryState(const Trk::Track& trk): m_trk{trk} {}
    Trk::TrackStates::const_iterator MuonChamberHoleRecoveryTool::RecoveryState::begin() const{
        return m_trk.trackStateOnSurfaces()->begin();
    }
    Trk::TrackStates::const_iterator MuonChamberHoleRecoveryTool::RecoveryState::end() const {
        return m_trk.trackStateOnSurfaces()->end();
    }
    const Trk::TrackStateOnSurface* MuonChamberHoleRecoveryTool::RecoveryState::tsos() const {
        return m_curr_itr != end() ? (*m_curr_itr) : nullptr;
    }
    bool MuonChamberHoleRecoveryTool::RecoveryState::nextState() {
        if (!m_nextCalled) {
            m_nextCalled = true;
            return true;
        }
        if (m_curr_itr == end()) return false;
        return (++m_curr_itr) != end();
    }
    void MuonChamberHoleRecoveryTool::RecoveryState::copyState(const CopyTarget target) {
        if (!m_copiedStates.insert(tsos()).second) return;
        if (target == CopyTarget::GlobalTrkStates) m_newStates.emplace_back(tsos()->clone());
        else chamberStates.emplace_back(tsos()->clone());
    }
    const Trk::TrackParameters* MuonChamberHoleRecoveryTool::RecoveryState::chamberPars() const {
        return chamberStates.empty() ? tsos()->trackParameters() : chamberStates[0]->trackParameters();
    }
    void MuonChamberHoleRecoveryTool::RecoveryState::finalizeChamber() {
        if (chamberStates.empty()) return;
        std::stable_sort(chamberStates.begin(), chamberStates.end(), SortTSOSByDistanceToPars(chamberPars()));
        m_newStates.insert(m_newStates.end(), std::make_move_iterator(chamberStates.begin()),
                                               std::make_move_iterator(chamberStates.end()));
        chamberStates.clear();
    }
    std::unique_ptr<Trk::TrackStates> MuonChamberHoleRecoveryTool::RecoveryState::releaseStates() {
        finalizeChamber();
        std::unique_ptr<Trk::TrackStates> outVec = std::make_unique<Trk::TrackStates>();
        for (std::unique_ptr<const Trk::TrackStateOnSurface>& tsos : m_newStates){
            outVec->push_back(std::move(tsos));
        }
        return outVec;
    }

    MuonChamberHoleRecoveryTool::MuonChamberHoleRecoveryTool(const std::string& ty, const std::string& na, const IInterface* pa) :
        AthAlgTool(ty, na, pa) {
        declareInterface<IMuonHoleRecoveryTool>(this);
    }

    StatusCode MuonChamberHoleRecoveryTool::initialize() {
        ATH_CHECK(m_DetectorManagerKey.initialize());
        ATH_CHECK(m_edmHelperSvc.retrieve());
        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_CHECK(m_trackingVolumesSvc.retrieve());
        ATH_CHECK(m_printer.retrieve());
        ATH_CHECK(m_extrapolator.retrieve());


        ATH_CHECK(m_key_csc.initialize(!m_key_csc.empty()));
        ATH_CHECK(m_key_stgc.initialize(!m_key_stgc.empty()));
        ATH_CHECK(m_key_mm.initialize(!m_key_mm.empty()));

        ATH_CHECK(m_key_mdt.initialize(!m_key_mdt.empty()));
        ATH_CHECK(m_key_tgc.initialize(!m_key_tgc.empty()));
        ATH_CHECK(m_key_rpc.initialize(!m_key_rpc.empty()));

        ATH_CHECK(m_cscRotCreator.retrieve(DisableTool{m_key_csc.empty()}));

        const bool hasClusKey = !m_key_stgc.empty() || !m_key_mm.empty() || !m_key_tgc.empty() || !m_key_rpc.empty();
        ATH_CHECK(m_clusRotCreator.retrieve(EnableTool{hasClusKey}));
        ATH_CHECK(m_mdtRotCreator.retrieve(EnableTool{!m_key_mdt.empty()}));
        ATH_CHECK(m_pullCalculator.retrieve());

        ATH_CHECK(m_chamberGeoKey.initialize());
        return StatusCode::SUCCESS;
    }
    bool MuonChamberHoleRecoveryTool::getNextMuonMeasurement(RecoveryState& trkRecov, RecoveryState::CopyTarget target) const {
        while(trkRecov.nextState()) {
            const Trk::TrackParameters* pars = trkRecov.tsos()->trackParameters();
            ATH_MSG_VERBOSE("Track parameters "<<(*pars));
            /// Copy all track states inside the MS
            const Trk::Volume& msVol{m_trackingVolumesSvc->volume(Trk::ITrackingVolumesSvc::MuonSpectrometerEntryLayer)};
            if (msVol.inside(pars->position())) {
                ATH_MSG_VERBOSE("Tracking parameters are inside the MS");
                trkRecov.copyState(target);
                continue;
            }

            if (trkRecov.tsos()->type(Trk::TrackStateOnSurface::Hole)) {
                ATH_MSG_VERBOSE("Skip hole in MS");
                continue;
            }
            /// Scatteres AEOTs, material etc.
            const Trk::MeasurementBase* meas = trkRecov.tsos()->measurementOnTrack();
            if (!meas) {
                ATH_MSG_VERBOSE("The track state does not have an associated measurement");
                trkRecov.copyState(target);
                continue;
            }
            trkRecov.tsosId = m_edmHelperSvc->getIdentifier(*meas);
            // Potentially a pseudo measurement. Anyway keep it
            if (!trkRecov.tsosId.is_valid() || !m_idHelperSvc->isMuon(trkRecov.tsosId)) {
                ATH_MSG_VERBOSE("No muon measurement");
                trkRecov.copyState(target);
                continue;
            }
            return true;
        }
        return false;
    }

    std::set<Identifier> MuonChamberHoleRecoveryTool::layersOnTrkIds(const Trk::Track& track) const {
        std::set<Identifier> layerIds{};
        for (const Trk::TrackStateOnSurface* tsos: *track.trackStateOnSurfaces()){
            const Trk::MeasurementBase* meas = tsos->measurementOnTrack();
            if (!meas) continue;
            const Identifier measId = m_edmHelperSvc->getIdentifier(*meas);

            if (!m_idHelperSvc->isMuon(measId)) continue;
            layerIds.insert(measId);
            layerIds.insert(m_idHelperSvc->layerId(measId));
            // split competing ROTs into constituents
            const CompetingMuonClustersOnTrack* comp = dynamic_cast<const CompetingMuonClustersOnTrack*>(meas);
            if (!comp) { continue; }

            for (const Muon::MuonClusterOnTrack* clust : comp->containedROTs()) {
                layerIds.insert(m_idHelperSvc->layerId(clust->identify()));
            }
        }
        return layerIds;
    }

    std::unique_ptr<Trk::Track> MuonChamberHoleRecoveryTool::recover(const Trk::Track& track, const EventContext& ctx) const {
        ATH_MSG_DEBUG(" performing hole search track " << std::endl
                                                       << m_printer->print(track) << std::endl
                                                       << m_printer->printMeasurements(track));

        RecoveryState recovState{track};
        recovState.layersOnTrk = layersOnTrkIds(track);

        /// Copy all parameters befor the MS onto the new track state
        getNextMuonMeasurement(recovState, RecoveryState::CopyTarget::GlobalTrkStates);
        /// Start an iterative recovery looping over each chamber
        recoverHitsInChamber(ctx, recovState);
        std::unique_ptr<Trk::Track> newTrack = std::make_unique<Trk::Track>(track.info(), recovState.releaseStates(),
                                                                            track.fitQuality() ? track.fitQuality()->uniqueClone() : nullptr);
        return newTrack;
    }
    void MuonChamberHoleRecoveryTool::recoverHitsInChamber(const EventContext& ctx, RecoveryState& trkRecov) const {
        const Muon::MuonStationIndex::ChIndex currStation = m_idHelperSvc->chamberIndex(trkRecov.tsosId);
        /// Collect all hits in a chamber
        std::set<Identifier> chambInStation{};
        do {
            if (currStation != m_idHelperSvc->chamberIndex(trkRecov.tsosId)) {
                trkRecov.finalizeChamber();
                recoverHitsInChamber(ctx, trkRecov);
                return;
            }
            trkRecov.copyState(RecoveryState::CopyTarget::ChamberTrkStates);
            /// New chamber added
            if (chambInStation.insert(m_idHelperSvc->chamberId(trkRecov.tsosId)).second) {
                if (m_idHelperSvc->isMdt(trkRecov.tsosId)) {
                    recoverMdtHits(ctx, trkRecov.tsosId, *trkRecov.tsos()->trackParameters(),
                                   trkRecov.chamberStates, trkRecov.layersOnTrk);
                } else {
                    recoverClusterHits(ctx, trkRecov.tsosId, *trkRecov.tsos()->trackParameters(),
                                       trkRecov.chamberStates, trkRecov.layersOnTrk);
                }
            }
        } while (getNextMuonMeasurement(trkRecov, RecoveryState::CopyTarget::ChamberTrkStates));
        trkRecov.finalizeChamber();
    }
    void MuonChamberHoleRecoveryTool::recoverMdtHits(const EventContext& ctx,
                                                     const Identifier& chId,
                                                     const Trk::TrackParameters& pars,
                                                     NewTrackStates& newStates, std::set<Identifier>& knownLayers) const {


        std::set<Identifier> chHoles = holesInMdtChamber(ctx, pars.position(), pars.momentum().unit(), chId, knownLayers);
        ATH_MSG_VERBOSE(" chamber " << m_idHelperSvc->toStringChamber(chId) << " has holes " << chHoles.size());
        if (chHoles.empty()) return;

        const std::vector<const MdtPrepData*> prdCandidates{loadPrepDataHits<MdtPrepData>(ctx, m_key_mdt, chHoles)};
        bool addedState{false};
        for (const MdtPrepData* mdtPrd : prdCandidates) {
            const Trk::StraightLineSurface& surf{mdtPrd->detectorElement()->surface(mdtPrd->identify())};
            std::unique_ptr<Trk::TrackParameters> exPars = m_extrapolator->extrapolateDirectly(ctx, pars, surf, Trk::anyDirection, false, Trk::muon);
            if (!exPars) {
                ATH_MSG_VERBOSE("Propagation to "<<m_idHelperSvc->toString(mdtPrd->identify())<<" failed.");
                continue;
            }

            /// calculate Amg::Vector2D using surf to obtain sign
            Amg::Vector2D locPos{Amg::Vector2D::Zero()};
            if (!surf.globalToLocal(exPars->position(), exPars->momentum(), locPos)) {
                ATH_MSG_DEBUG(" failed to calculate drift sign ");
                continue;
            }
            if (!surf.insideBounds(locPos)) {
                chHoles.erase(mdtPrd->identify());
                continue;
            }

            // calibrate Mdt PRD
            const Amg::Vector3D momentum = exPars->momentum();
            std::unique_ptr<MdtDriftCircleOnTrack> mdtROT{m_mdtRotCreator->createRIO_OnTrack(*mdtPrd,
                                                                                             exPars->position(),
                                                                                             &momentum)};
            if (!mdtROT) continue;

            /// calculate side
            Trk::DriftCircleSide side = locPos[Trk::driftRadius] < 0 ? Trk::LEFT : Trk::RIGHT;
            /// update sign
            m_mdtRotCreator->updateSign(*mdtROT, side);

            /// pointer to resPull
            std::optional<const Trk::ResidualPull> resPull{m_pullCalculator->residualPull(mdtROT.get(),
                                                                                          exPars.get(),
                                                                                          Trk::ResidualPull::Unbiased)};
            if (!resPull) { continue; }

            const double pull = resPull->pull().front();
            const double radialResidual = std::abs(mdtROT->localParameters()[Trk::locR]) -
                                          std::abs(exPars->parameters()[Trk::locR]);

            unsigned int hitFlag = 1;
            if (mdtPrd->adc() < m_adcCut || mdtPrd->status() != MdtStatusDriftTime)
                hitFlag = 3;  // noise
            else if (std::abs(pull) < m_associationPullCutEta)
                hitFlag = 0;  // hit on track
            else if (radialResidual > 0.)
                hitFlag = 2;  // out of time

            ATH_MSG_VERBOSE(__func__<<"() - Recover "<<m_printer->print(*mdtROT)<<" pull: "<<pull<<" hitFlag: "<<hitFlag);
            std::unique_ptr<Trk::TrackStateOnSurface> tsos = MuonTSOSHelper::createMeasTSOS(std::move(mdtROT),
                                                                                            std::move(exPars),
                                                                                        (hitFlag != 0 || !m_addMeasurements) ?
                                                                                                Trk::TrackStateOnSurface::Outlier :
                                                                                                Trk::TrackStateOnSurface::Measurement);

            if (!addedState) {
                newStates.emplace_back(std::move(tsos));
                addedState = true;
            } else {
                const MdtDriftCircleOnTrack* prevDC = static_cast<const MdtDriftCircleOnTrack*>(newStates.back()->measurementOnTrack());
                if (prevDC->identify() != mdtPrd->identify()) {
                    newStates.emplace_back(std::move(tsos));
                } else {
                    ATH_MSG_DEBUG("Two hits recorded for the same tube "<<std::endl<<
                                 " *** current: "<<m_printer->print(*tsos->measurementOnTrack())<<std::endl<<
                                 " *** previous: "<<m_printer->print(*prevDC));
                    std::optional<Trk::ResidualPull> prevPullObj{m_pullCalculator->residualPull(prevDC,
                                                                                                tsos->trackParameters(),
                                                                                                Trk::ResidualPull::Unbiased)};
                    const double prevPull = prevPullObj->pull().front();
                    /// Overwrite the previous drift circle if the pull of this one is better or it's an outlier
                    if (std::abs(pull) < std::abs(prevPull) ||
                        (tsos->type(Trk::TrackStateOnSurface::Measurement) &&
                        newStates.back()->type(Trk::TrackStateOnSurface::Outlier))) {
                        newStates.back() = std::move(tsos);
                    }

                }
            }
            chHoles.erase(mdtPrd->identify());
            knownLayers.insert(mdtPrd->identify());
        }

        for (const Identifier& hole : chHoles) {
            const Trk::Surface& surf{getDetectorElement(ctx, hole)->surface(hole)};
            std::unique_ptr<Trk::TrackParameters> exPars = m_extrapolator->extrapolateDirectly(ctx, pars, surf, Trk::anyDirection, false, Trk::muon);
            if (!exPars) {
                ATH_MSG_VERBOSE("Propagation to "<<m_idHelperSvc->toString(hole)<<" failed.");
                continue;
            }
            ATH_MSG_VERBOSE(__func__<<"() - Add hole "<<m_idHelperSvc->toString(hole));
            newStates.emplace_back(MuonTSOSHelper::createHoleTSOS(std::move(exPars)));
        }
    }
    void MuonChamberHoleRecoveryTool::recoverClusterHits(const EventContext& ctx,
                                                         const Identifier& chambId,
                                                         const Trk::TrackParameters& pars,
                                                         NewTrackStates& states, std::set<Identifier>& knownLayers) const {

        NewTrackStates recovered{};
        if (m_idHelperSvc->isRpc(chambId)) {
            recovered = recoverChamberClusters(ctx, m_key_rpc, chambId, pars, knownLayers);
        } else if (m_idHelperSvc->isTgc(chambId)) {
            recovered = recoverChamberClusters(ctx, m_key_tgc, chambId, pars, knownLayers);
        } else if (m_idHelperSvc->isCsc(chambId)) {
            recovered = recoverChamberClusters(ctx, m_key_csc, chambId, pars, knownLayers);
        } else if (m_idHelperSvc->isMM(chambId)) {
            recovered = recoverChamberClusters(ctx, m_key_mm, chambId, pars, knownLayers);
        } else if (m_idHelperSvc->issTgc(chambId)) {
            recovered = recoverChamberClusters(ctx, m_key_stgc, chambId, pars, knownLayers);
        }
        states.insert(states.end(), std::make_move_iterator(recovered.begin()),
                                    std::make_move_iterator(recovered.end()));
    }
    void MuonChamberHoleRecoveryTool::createHoleTSOSsForClusterChamber(const Identifier& detElId,
                                                                      const EventContext& ctx,
                                                                      const Trk::TrackParameters& pars,
                                                                      std::set<Identifier>& layIds,
                                                                      NewTrackStates& states) const {
        ATH_MSG_VERBOSE(" performing holes search in chamber " << m_idHelperSvc->toString(detElId));
        recoverClusterHits(ctx, detElId, pars, states, layIds);
    }


    const Trk::TrkDetElementBase* MuonChamberHoleRecoveryTool::getDetectorElement(const EventContext& ctx,
                                                                                  const Identifier& detElId) const {
        SG::ReadCondHandle<MuonGM::MuonDetectorManager> DetectorManagerHandle{m_DetectorManagerKey, ctx};
        const MuonGM::MuonDetectorManager* MuonDetMgr{*DetectorManagerHandle};
        if (!MuonDetMgr) {
            ATH_MSG_ERROR("Null pointer to the read MuonDetectorManager conditions object");
            return nullptr;
        }
        if (m_idHelperSvc->isMdt(detElId))
            return MuonDetMgr->getMdtReadoutElement(detElId);
        else if (m_idHelperSvc->isTgc(detElId))
            return MuonDetMgr->getTgcReadoutElement(detElId);
        else if (m_idHelperSvc->isRpc(detElId))
            return MuonDetMgr->getRpcReadoutElement(detElId);
        else if (m_idHelperSvc->isCsc(detElId))
            return MuonDetMgr->getCscReadoutElement(detElId);
        // New Small Wheel
        else if (m_idHelperSvc->issTgc(detElId))
            return MuonDetMgr->getsTgcReadoutElement(detElId);
        else if (m_idHelperSvc->isMM(detElId))
            return MuonDetMgr->getMMReadoutElement(detElId);
        return nullptr;
    }

    std::set<Identifier> MuonChamberHoleRecoveryTool::holesInMdtChamber(const EventContext& ctx, const Amg::Vector3D& position, const Amg::Vector3D& direction,
                                                                        const Identifier& chId, const std::set<Identifier>& tubeIds) const {

        SG::ReadCondHandle<Muon::MuonIntersectGeoData> interSectSvc{m_chamberGeoKey,ctx};
        if (!interSectSvc.isValid())   {
            ATH_MSG_ERROR("Failed to retrieve chamber intersection service");
            throw std::runtime_error("No chamber intersection service");
        }

        SG::ReadCondHandle<MuonGM::MuonDetectorManager> detMgr{m_DetectorManagerKey,ctx};
        const MuonGM::MuonDetectorManager* MuonDetMgr = detMgr.cptr();
        const MuonGM::MdtReadoutElement* readoutEle = MuonDetMgr->getMdtReadoutElement(chId);

        MuonStationIntersect intersect = interSectSvc->tubesCrossedByTrack(MuonDetMgr, chId, position, direction);

        // clear hole vector
        std::set<Identifier> holes;
        for (unsigned int ii = 0; ii < intersect.tubeIntersects().size(); ++ii) {
            const MuonTubeIntersect& tint = intersect.tubeIntersects()[ii];

            if (tubeIds.count(tint.tubeId)) { continue; }
            ATH_MSG_VERBOSE(" intersect " << m_idHelperSvc->toString(tint.tubeId) << "  dist wire " << tint.rIntersect
                                          << "  dist to tube end " << tint.xIntersect);

            if (std::abs(tint.rIntersect) > readoutEle->innerTubeRadius() || tint.xIntersect > -10.) {
                ATH_MSG_VERBOSE(" not counted");
                continue;
            }
            // check whether there is a hit in this tube
            ATH_MSG_VERBOSE(" hole tube");
            holes.insert(tint.tubeId);

        }
        return holes;
    }
    template <class Prd> std::vector<const Prd*> MuonChamberHoleRecoveryTool::loadPrepDataHits(const EventContext& ctx,
                                                                                               const SG::ReadHandleKey<MuonPrepDataContainerT<Prd>>& key,
                                                                                               const std::set<Identifier>& gasGapIds) const {
        std::vector<const Prd*> collectedHits{};
        if (key.empty()) {
            ATH_MSG_DEBUG("No container configured for "<<typeid(Prd).name());
            return collectedHits;
        }
        SG::ReadHandle<MuonPrepDataContainerT<Prd>> prdContainer{key, ctx};
        if (!prdContainer.isPresent()) {
            ATH_MSG_FATAL("Failed to load prep data collection "<<key.fullKey());
            throw std::runtime_error("Invalid prepdata container");
        }
        /// Create a subset of Identifiers
        std::set<IdentifierHash> chamberIds{};
        std::transform(gasGapIds.begin(), gasGapIds.end(),
                            std::inserter(chamberIds, chamberIds.end()),
                                [this](const Identifier& id){
                                    return m_idHelperSvc->moduleHash(id);
                                });
        for (const IdentifierHash& moduleHash : chamberIds) {
            const MuonPrepDataCollection<Prd>* prdColl = prdContainer->indexFindPtr(moduleHash);
            if (!prdColl) continue;
            collectedHits.reserve(collectedHits.size() + prdColl->size());
            for (const Prd* prd: *prdColl) {
                bool appendPrd{false};
                if constexpr (std::is_same<Prd, MdtPrepData>::value) {
                    appendPrd = gasGapIds.count(prd->identify());
                } else {
                    appendPrd = gasGapIds.count(m_idHelperSvc->layerId(prd->identify()));
                }
                if (appendPrd) {
                    ATH_MSG_VERBOSE(__func__<<"() - Add prd candidate "<<m_printer->print(*prd));
                    collectedHits.push_back(prd);
                }
            }
        }
        /// Sort the Mdt prepdatas per Identifier as there's the possibily 2 DC can exist for the
        /// same channel
        if constexpr(std::is_same<Prd, MdtPrepData>::value) {
            std::sort(collectedHits.begin(), collectedHits.end(), [](const Prd* a, const Prd* b){
                return a->identify() < b->identify();
            });
        }
        return collectedHits;
    }
    std::set<Identifier> MuonChamberHoleRecoveryTool::getHoleLayerIds(const Identifier& detElId,
                                                                      const std::set<Identifier>& knownLayers) const {
         std::set<Identifier> holeGaps{};
         if (m_idHelperSvc->isMdt(detElId)) {
            const MdtIdHelper& idHelper{m_idHelperSvc->mdtIdHelper()};
            for (int ml = 1; ml <= idHelper.numberOfMultilayers(detElId); ++ml) {
                for (int layer = idHelper.tubeLayerMin(detElId);
                         layer <= idHelper.tubeLayerMax(detElId); ++layer) {
                    const Identifier layerId = idHelper.channelID(detElId, ml, layer, 1);
                    if (!knownLayers.count(layerId)) holeGaps.insert(layerId);
                }
            }
         } else if (m_idHelperSvc->isMM(detElId)) {
            const MmIdHelper& idHelper{m_idHelperSvc->mmIdHelper()};
            for (int ml : {1 ,2}) {
                for (int gap = MmIdHelper::gasGapMin(); gap <= MmIdHelper::gasGapMax(); ++gap) {
                    const Identifier layerId = idHelper.channelID(detElId, ml, gap, 1);
                    if (!knownLayers.count(layerId)) holeGaps.insert(layerId);
                }
            }
         } else if (m_idHelperSvc->issTgc(detElId)) {
            const sTgcIdHelper& idHelper{m_idHelperSvc->stgcIdHelper()};
            using channelType = sTgcIdHelper::sTgcChannelTypes;
            for (int ml : {1, 2}) {
                for (const channelType chType : {channelType::Strip,
                                                 channelType::Pad,
                                                 channelType::Wire}){
                    for (int gap = sTgcIdHelper::gasGapMin();
                             gap <= sTgcIdHelper::gasGapMax(); ++gap) {
                        const Identifier layerId = idHelper.channelID(detElId, ml, gap, chType, 1);
                        if (!knownLayers.count(layerId)) holeGaps.insert(layerId);
                    }
                }
            }
         } else if (m_idHelperSvc->isTgc(detElId)) {
            const TgcIdHelper& idHelper = m_idHelperSvc->tgcIdHelper();
            const int gapMax{idHelper.gasGapMax(detElId)};
            for (int gasgap = idHelper.gasGapMin(detElId);
                     gasgap < gapMax; ++gasgap){
                for (int measPhi: {0,1}) {
                    /// the second gas gap of the three layer stations does not have a phi measurement
                    if (gapMax == 3 && gasgap ==2 && measPhi == 1) continue;
                    const Identifier layerId = idHelper.channelID(detElId, gasgap, measPhi, 1);
                    if (!knownLayers.count(layerId)) holeGaps.insert(layerId);
                }
            }
        } else if (m_idHelperSvc->isRpc(detElId)) {
            const RpcIdHelper& idHelper = m_idHelperSvc->rpcIdHelper();
            const int doubZ{idHelper.doubletZ(detElId)};
            const int gapMax{idHelper.gasGapMax(detElId)};
            for (int phiGap = idHelper.doubletPhi(detElId);
                     phiGap <= idHelper.doubletPhiMax(detElId); ++phiGap) {
                for (int gap = idHelper.gasGapMin(detElId);
                         gap <= gapMax; ++gap) {
                    for (int measPhi: {0, 1}) {
                        const Identifier layerId = idHelper.channelID(detElId, doubZ, phiGap, gap, measPhi, 1);
                        if (!knownLayers.count(layerId)) holeGaps.insert(layerId);	
                    }
                }
            }
        } else if (m_idHelperSvc->isCsc(detElId)) {
            const CscIdHelper& idHelper{m_idHelperSvc->cscIdHelper()};
            for (int layer = 1; layer <= 4; ++ layer) {
                for (bool measPhi: {false, true}) {
                    const Identifier layId = idHelper.channelID(detElId, 2, layer, measPhi, 1);
                    if (!knownLayers.count(layId)) holeGaps.insert(layId);
                }
            }
         }
        return holeGaps;
    }
    template <class Prd> NewTrackStates MuonChamberHoleRecoveryTool::recoverChamberClusters(const EventContext& ctx,
                                                                                            const SG::ReadHandleKey<MuonPrepDataContainerT<Prd>>& prdKey,
                                                                                            const Identifier& detElId,
                                                                                            const Trk::TrackParameters& parsInChamb,
                                                                                            std::set<Identifier>& knownLayers) const {
        NewTrackStates recoveredStates{};
        const std::set<Identifier> missingLayers = getHoleLayerIds(detElId, knownLayers);
        std::vector<const Prd*> prdCandidates = loadPrepDataHits(ctx, prdKey, missingLayers);
        //// Next extrapolate once the TrackParameters onto the surfaces
        using LayerParsMap = std::map<Identifier, std::unique_ptr<Trk::TrackParameters>>;
        LayerParsMap parsAtSurfMap{};
        for (const Identifier& holeId : missingLayers) {
            const Trk::TrkDetElementBase* detEl = getDetectorElement(ctx, holeId);
            const Trk::Surface& surf{detEl->surface(holeId)};
            std::unique_ptr<Trk::TrackParameters> pars = m_extrapolator->extrapolateDirectly(ctx, parsInChamb, surf,
                                                                                             Trk::anyDirection, false, Trk::muon);
            if (!pars) {
                ATH_MSG_VERBOSE("Surface layer "<<m_idHelperSvc->toStringGasGap(holeId)<<" cannot be reached");
                continue;
            }
            if (!Amg::hasPositiveDiagElems(*pars->covariance())) {
                ATH_MSG_DEBUG("Uncertainties of extraploation to "<<m_idHelperSvc->toStringGasGap(holeId)<<" blew up");
                continue;
            }
            const Amg::Vector2D locExPos{pars->parameters()[Trk::locX],
                                         pars->parameters()[Trk::locY]};
            bool inbounds{false};
            if constexpr(std::is_same<Prd, MMPrepData>::value) {
                inbounds = static_cast<const MuonGM::MMReadoutElement*>(detEl)->insideActiveBounds(holeId, locExPos, 10.,10.);
            } else {
                inbounds = surf.insideBounds(locExPos, 10., 10.);
            }
            /// The hit is out of bounds --> continue
            if (!inbounds) {
                ATH_MSG_VERBOSE("Extrapolation to layer "<<m_idHelperSvc->toStringGasGap(holeId)
                              <<" is outside of the chamber "<<Amg::toString(locExPos, 2));
                continue;
            }
            parsAtSurfMap[holeId] = std::move(pars);
        }
        ClusterLayerMap bestClusterInLay;
        /// Loop over all prd candidates
        for (const Prd* hit : prdCandidates) {
            const Identifier layId = m_idHelperSvc->layerId(hit->identify());
            /// The surface cannot be reached. Discard the prd
            LayerParsMap::const_iterator pars_itr = parsAtSurfMap.find(layId);
            if (pars_itr == parsAtSurfMap.end()) {
                continue;
            }
            const std::unique_ptr<Trk::TrackParameters>& parsInLay{pars_itr->second};
            std::unique_ptr<MuonClusterOnTrack> calibClus{};
            if constexpr(std::is_same<Prd, CscPrepData>::value) {
                calibClus.reset(m_cscRotCreator->createRIO_OnTrack(*hit,
                                                                   parsInLay->position(),
                                                                   parsInLay->momentum().unit()));
            } else {
                calibClus.reset(m_clusRotCreator->createRIO_OnTrack(*hit,
                                                                    parsInLay->position(),
                                                                    parsInLay->momentum().unit()));
            }
            if (!calibClus) continue;

            if constexpr (std::is_same<Prd, sTgcPrepData>::value) {
                const sTgcIdHelper& idHelper{m_idHelperSvc->stgcIdHelper()};
                if (idHelper.channelType(hit->identify()) == sTgcIdHelper::sTgcChannelTypes::Pad){
                    /// Compare the pad & local positions
                    const Amg::Vector2D locExPos{parsInLay->parameters()[Trk::locX],
                                                 parsInLay->parameters()[Trk::locY]};

                    const MuonGM::MuonPadDesign* pad = hit->detectorElement()->getPadDesign(hit->identify());
                    const Amg::Vector2D padDist = pad->distanceToPad(locExPos, idHelper.channel(hit->identify()));

                    /// Calculate the pull by hand
                    const double xCov = std::hypot(Amg::error(*parsInLay->covariance(), Trk::locX),
                                                   Amg::error(hit->localCovariance(), Trk::locX));
                    /// For one reason or another, the pads have 1D covariances
                    const double yCov =Amg::error(*parsInLay->covariance(), Trk::locY);

                    const double xPull = padDist.x() / xCov;
                    const double yPull = padDist.y() / yCov;

                    ATH_MSG_VERBOSE(__func__<<"() - check "<<m_printer->print(*calibClus)
                                 <<" diff "<<Amg::toString(padDist, 2)
                                 <<" covariance: ("<<xCov<<", "<<yCov<<")"
                                 <<" pull: ("<<xPull<<","<<yPull<<").");

                    /// Check whether the pull is reasonable
                    if (xPull > m_associationPullCutEta || yPull > m_associationPullCutPhi) continue;
                    const double pull = std::hypot(xPull, yPull);

                    /// Assign the best pad onto the track
                    PullCluster& bestClus = bestClusterInLay[layId];
                    if (bestClus.pull < pull) continue;
                    bestClus.pull = pull;
                    bestClus.clus = std::move(calibClus);
                    bestClus.pars = parsInLay->uniqueClone();
                    continue;
                }
            }

            /// Calculate the track pull
            std::optional<const Trk::ResidualPull> resPull{
                m_pullCalculator->residualPull(calibClus.get(), parsInLay.get(),
                                               Trk::ResidualPull::Unbiased)};
            if (!resPull || resPull->pull().empty()) {
                continue;
            }

            const double pull = std::abs(resPull->pull().front());
            const double pullCut = m_idHelperSvc->measuresPhi(layId) ? m_associationPullCutPhi
                                                                     : m_associationPullCutEta;
            if (pull > pullCut) continue;

            PullCluster& bestClus = bestClusterInLay[layId];
            if (bestClus.pull < pull) continue;
            bestClus.pull = pull;
            bestClus.clus = std::move(calibClus);
            bestClus.pars = parsInLay->uniqueClone();

        }
        /// the best clusters are found. Put them onto the track
        for (auto& [layerId, foundClus]: bestClusterInLay) {
            ATH_MSG_VERBOSE(__func__<<"() recovered hit " << m_printer->print(*foundClus.clus)<<" pull: "<<foundClus.pull);
            std::unique_ptr<Trk::TrackStateOnSurface> tsos = MuonTSOSHelper::createMeasTSOS(std::move(foundClus.clus),
                                                                                            std::move(foundClus.pars),
                                                                                            Trk::TrackStateOnSurface::Measurement);
            recoveredStates.emplace_back(std::move(tsos));
            knownLayers.insert(layerId);
            parsAtSurfMap[layerId].reset();
        }
        /// Next loop over the reamining parameters at surface to create holes
        for (auto& [layerId, exPars] : parsAtSurfMap) {
            if (!exPars) continue;
            ATH_MSG_VERBOSE(__func__<<"() add new hole state "<<m_idHelperSvc->toStringGasGap(layerId));
            recoveredStates.emplace_back(MuonTSOSHelper::createHoleTSOS(std::move(exPars)));
        }
        return recoveredStates;
    }
}  // namespace Muon
