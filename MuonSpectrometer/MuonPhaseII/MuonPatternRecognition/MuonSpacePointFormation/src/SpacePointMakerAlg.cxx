/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "SpacePointMakerAlg.h"

#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "EventPrimitives/EventPrimitivesToStringConverter.h"
#include "GeoPrimitives/GeoPrimitives.h"
#include "GeoPrimitives/GeoPrimitivesToStringConverter.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"
#include <GaudiKernel/IMessageSvc.h>
#include <memory>
#include <type_traits>
#include <xAODMuonViews/ChamberViewer.h>
#include "MuonSpacePoint/SpacePointPerLayerSorter.h"
#include "xAODMeasurementBase/MeasurementDefs.h"
#include "xAODMeasurementBase/UncalibratedMeasurement.h"
#include "xAODMuonPrepData/RpcStrip.h"
#include "xAODMuonPrepData/MdtTwinDriftCircle.h"


#include "Acts/Surfaces/detail/LineHelper.hpp"
namespace {
    inline std::vector<std::shared_ptr<unsigned>> matchCountVec(unsigned n) {
        std::vector<std::shared_ptr<unsigned>> out{};
        out.reserve(n);
        for (unsigned p = 0; p < n ;++p) {
            out.emplace_back(std::make_shared<unsigned>(0));
        }
        return out;
    }
    /** @brief Construct the transform to go from the measurement's local frame into
     *         the sector frame
     *  @param gctx: Geometry context to align the measurement within ATLAS
     *  @param sectorTrans: Reference to the transform from global -> sector frame
     *  @param meas: Reference to the measurement of interest */
    template<class MeasType>
        Amg::Transform3D toChamberTransform(const ActsGeometryContext& gctx, 
                                            const Amg::Transform3D& sectorTrans,
                                            const MeasType& meas) {
        const MuonGMR4::MuonReadoutElement* reEle{meas.readoutElement()};
        if constexpr(std::is_same_v<MeasType, xAOD::MdtDriftCircle>) {
            return sectorTrans * reEle->localToGlobalTrans(gctx, meas.measurementHash());
        } else {
            return sectorTrans * reEle->localToGlobalTrans(gctx, meas.layerHash());
        }
    }
    /** @brief Express the position of an uncalibrated measurement in the sector frame.
     *  @param meas: Reference to the measurement of interest
     *  @param toChamberTrans: Transformation to go from the local frame of the measurement
     *                         into the sector frame */
    template<typename MeasType>
        Amg::Vector3D positionInChamber(const MeasType& meas,
                                        const Amg::Transform3D& toChamberTrans) {
        if constexpr (std::is_same_v<MeasType, xAOD::MdtDriftCircle>){
            return toChamberTrans * meas.localCirclePosition();
        } else if constexpr (std::is_same_v<MeasType, xAOD::RpcMeasurement>){
            return toChamberTrans * meas.localMeasurementPos();
        } else if constexpr (std::is_same_v<MeasType, xAOD::TgcStrip>){
            const auto& stripLay = meas.readoutElement()->sensorLayout(meas.layerHash());
            return toChamberTrans * stripLay->to3D(meas.template localPosition<1>()[0]*Amg::Vector2D::UnitX(),
                                                meas.measuresPhi());
        } else if constexpr (std::is_same_v<MeasType, xAOD::MMCluster>){
            return toChamberTrans * (meas.template localPosition<1>()[Trk::locX] * Amg::Vector3D::UnitX());
        } else if constexpr (std::is_same_v<MeasType, xAOD::sTgcMeasurement>){
            if (meas.channelType() == sTgcIdHelper::sTgcChannelTypes::Strip ||
                meas.channelType() == sTgcIdHelper::sTgcChannelTypes::Wire) {
                    return toChamberTrans * (meas.template localPosition<1>()[Trk::locX] * Amg::Vector3D::UnitX());
            }
            Amg::Vector3D locPos{Amg::Vector3D::Zero()};
            locPos.block<2,1>(0,0) = xAOD::toEigen(meas.template localPosition<2>());
            return toChamberTrans * locPos;
        }  else {
            static_assert(std::false_type::value, "Unsupported measurement type.");
        }
        return Amg::Vector3D::Zero();
    }
    /** @brief Estimates the half-length of a tube or a readout-strip 
     *  @param prd: Reference to the measurement for which the half-length is to be estimated */
    template <typename PrdType>
        double sensorHalfLength(const PrdType& prd) {
            const auto* re = prd.readoutElement();
            if constexpr(std::is_same_v<PrdType, xAOD::MdtDriftCircle>) {
                return 0.5 * re->activeTubeLength(prd.measurementHash());
            } else if constexpr(std::is_same_v<PrdType, xAOD::RpcMeasurement>) {
                return 0.5*(prd.measuresPhi() ? re->stripPhiLength() : re->stripEtaLength());
            } else if constexpr(std::is_same_v<PrdType, xAOD::TgcStrip>) {
                return 0.5 * re->sensorLayout(prd.layerHash())->design().stripLength(prd.channelNumber());
            } else if constexpr(std::is_same_v<PrdType, xAOD::MMCluster>) {
                return 0.5* re->stripLayer(prd.layerHash()).design().stripLength(prd.channelNumber());
            } else if constexpr(std::is_same_v<PrdType, xAOD::sTgcMeasurement>) {
                return 0.5* re->stripLayer(prd.layerHash()).design().stripLength(prd.channelNumber());
            }
            return 0.;
        }
}

namespace MuonR4 {

///################################################################
///                 SpacePointStatistics
///################################################################
bool SpacePointMakerAlg::SpacePointStatistics::FieldKey::operator<(const FieldKey& other) const{
    if (techIdx != other.techIdx) {
        return static_cast<int>(techIdx) < static_cast<int>(other.techIdx);
    }
    if (stIdx != other.stIdx) {
        return static_cast<int>(stIdx) < static_cast<int>(other.stIdx);
    }
    return eta < other.eta;
}
unsigned SpacePointMakerAlg::SpacePointStatistics::StatField::allHits() const {
    return measEta + measPhi + measEtaPhi;
}
SpacePointMakerAlg::SpacePointStatistics::SpacePointStatistics(const Muon::IMuonIdHelperSvc* idHelperSvc):
    m_idHelperSvc{idHelperSvc}{}

void SpacePointMakerAlg::SpacePointStatistics::addToStat(const std::vector<SpacePoint>& spacePoints){
    std::lock_guard guard{m_mutex};
    for (const SpacePoint& sp : spacePoints){
        FieldKey key{};
        key.stIdx = m_idHelperSvc->stationIndex(sp.identify());
        key.techIdx = m_idHelperSvc->technologyIndex(sp.identify());
        key.eta = m_idHelperSvc->stationEta(sp.identify());
        StatField & stats = m_map[key];
        if (sp.measuresEta() && sp.measuresPhi()) {
            ++stats.measEtaPhi;
        } else {
            stats.measEta += sp.measuresEta();
            stats.measPhi += sp.measuresPhi();
        }               
    }
}
void SpacePointMakerAlg::SpacePointStatistics::dumpStatisics(MsgStream& msg) const {
    using KeyVal = std::pair<FieldKey, StatField>; 
    std::vector<KeyVal> sortedstats{};
    sortedstats.reserve(m_map.size());
    /// Sort statistics from largest to smallest
    for (const auto & [key, stats] : m_map){
        sortedstats.emplace_back(std::make_pair(key, stats));
    }
    std::stable_sort(sortedstats.begin(), sortedstats.end(), [](const KeyVal& a, const KeyVal&b) {
        return a.second.allHits() > b.second.allHits();
    });
    msg<<MSG::ALWAYS<<"###########################################################################"<<endmsg;
    for (const auto & [key, stats] : sortedstats) {
        msg<<MSG::ALWAYS<<" "<<Muon::MuonStationIndex::technologyName(key.techIdx)
                        <<" "<<Muon::MuonStationIndex::stName(key.stIdx)
                        <<" "<<std::abs(key.eta)<<(key.eta < 0 ? "A" : "C")
                        <<" "<<std::setw(8)<<stats.measEtaPhi
                        <<" "<<std::setw(8)<<stats.measEta
                        <<" "<<std::setw(8)<<stats.measPhi<<endmsg;
    }
    msg<<MSG::ALWAYS<<"###########################################################################"<<endmsg;
    
}
///##########################################
///         SpacePointMakerAlg
///##########################################
using CovIdx = SpacePoint::CovIdx;


StatusCode SpacePointMakerAlg::finalize() {
    if (m_statCounter) {
        m_statCounter->dumpStatisics(msgStream());
    }
    return StatusCode::SUCCESS;
}
StatusCode SpacePointMakerAlg::initialize() {
    ATH_CHECK(m_geoCtxKey.initialize());
    ATH_CHECK(m_mdtKey.initialize(!m_mdtKey.empty()));
    ATH_CHECK(m_rpcKey.initialize(!m_rpcKey.empty()));
    ATH_CHECK(m_tgcKey.initialize(!m_tgcKey.empty()));
    ATH_CHECK(m_mmKey.initialize(!m_mmKey.empty()));
    ATH_CHECK(m_stgcKey.initialize(!m_stgcKey.empty()));
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_writeKey.initialize());
    if (m_doStat) {
        m_statCounter = std::make_unique<SpacePointStatistics>(m_idHelperSvc.get());
    }
    return StatusCode::SUCCESS;
}

template <> 
    bool SpacePointMakerAlg::passOccupancy2D(const std::vector<const xAOD::TgcStrip*>& etaHits,
                                             const std::vector<const xAOD::TgcStrip*>& phiHits) const {
        if (etaHits.empty() || phiHits.empty()) {
            return false;
        }
        const MuonGMR4::TgcReadoutElement* re = etaHits[0]->readoutElement();
        ATH_MSG_VERBOSE("Collected "<<etaHits.size()<<"/"<<phiHits.size()<<" hits in "<<m_idHelperSvc->toStringGasGap(etaHits[0]->identify()));
        return ((1.*etaHits.size()) / ((1.*re->numChannels(etaHits[0]->measurementHash())))) < m_maxOccTgcEta &&
               ((1.*phiHits.size()) / ((1.*re->numChannels(phiHits[0]->measurementHash())))) < m_maxOccTgcPhi;
    }
template <> 
    bool SpacePointMakerAlg::passOccupancy2D(const std::vector<const xAOD::RpcMeasurement*>& etaHits,
                                             const std::vector<const xAOD::RpcMeasurement*>& phiHits) const {
        if (etaHits.empty() || phiHits.empty()) {
            return false;
        }
        const MuonGMR4::RpcReadoutElement* re = etaHits[0]->readoutElement();
        ATH_MSG_VERBOSE("Collected "<<etaHits.size()<<"/"<<phiHits.size()<<" hits in "<<m_idHelperSvc->toStringGasGap(etaHits[0]->identify()));
        return ((1.*etaHits.size()) / (1.*re->nEtaStrips())) < m_maxOccRpcEta &&
               ((1.*phiHits.size()) / (1.*re->nPhiStrips())) < m_maxOccRpcPhi;
    }

template <> 
    bool SpacePointMakerAlg::passOccupancy2D(const std::vector<const xAOD::sTgcMeasurement*>& etaHits,
                                             const std::vector<const xAOD::sTgcMeasurement*>& phiHits) const {
        if (etaHits.empty() || phiHits.empty()) {
            return false;
        }
        const MuonGMR4::sTgcReadoutElement* re = etaHits[0]->readoutElement();
        return ((1.*etaHits.size()) / (1.*re->numChannels(etaHits[0]->measurementHash()))) < m_maxOccStgcEta &&
               ((1.*phiHits.size()) / (1.*re->numChannels(phiHits[0]->measurementHash()))) < m_maxOccStgcPhi;
    }
template <>
    bool SpacePointMakerAlg::passOccupancy2D(const std::vector<const xAOD::MMCluster*>& /*etaHits*/,
                                             const std::vector<const xAOD::MMCluster*>& /*phiHits*/) const {
       return false;
    }
template <typename PrdType>
    void SpacePointMakerAlg::fillUncombinedSpacePoints(const ActsGeometryContext& gctx,
                                                       const Amg::Transform3D& sectorTrans,
                                                       const std::vector<const PrdType*>& prdsToFill,
                                                       std::vector<SpacePoint>& outColl) const {
    if (prdsToFill.empty()) {
        return;
    }
    const Amg::Transform3D toSectorTrans = toChamberTransform(gctx, sectorTrans, *prdsToFill.front());
    /// Local coordinate system aligned such that the strips point along local y
    Amg::Vector3D sensorDir = toSectorTrans.rotation().col(Amg::y);
    Amg::Vector3D toNextSen = toSectorTrans.rotation().col(Amg::x);
    outColl.reserve(outColl.size() + prdsToFill.size());
    for (const PrdType* prd: prdsToFill) {
        SpacePoint& newSp = outColl.emplace_back(prd);
        if constexpr (std::is_same_v<PrdType, xAOD::TgcStrip>) {
            const bool isStrip = prd->measuresPhi();
            const auto& stripLay = prd->readoutElement()->sensorLayout(prd->layerHash());
            if (isStrip) {                
                const auto& radialDesign = static_cast<const MuonGMR4::RadialStripDesign&>(stripLay->design(isStrip));
                toNextSen = toSectorTrans.rotation() * stripLay->to3D(radialDesign.stripNormal(prd->channelNumber()), isStrip);
                sensorDir = toSectorTrans.rotation() * stripLay->to3D(radialDesign.stripDir(prd->channelNumber()), isStrip);
            } else {
                toNextSen = toSectorTrans.rotation() * stripLay->to3D(Amg::Vector2D::UnitX(), isStrip);
                sensorDir = toSectorTrans.rotation() * stripLay->to3D(Amg::Vector2D::UnitY(), isStrip);
            }
        }
        newSp.setPosition(positionInChamber(*prd, toSectorTrans));
        newSp.setDirection(sensorDir, toNextSen);
        auto cov = Acts::filledArray<double,3>(0.);        
        if (prd->numDimensions() == 2) {
            if constexpr(std::is_same_v<PrdType, xAOD::RpcMeasurement>) {
                cov[Acts::toUnderlying(CovIdx::etaCov)] = prd->template localCovariance<2>()(0,0);
                cov[Acts::toUnderlying(CovIdx::phiCov)] = prd->template localCovariance<2>()(1,1);
            } else if constexpr(std::is_same_v<PrdType, xAOD::sTgcMeasurement>) {
                cov[Acts::toUnderlying(CovIdx::phiCov)] = prd->template localCovariance<2>()(0,0);
                cov[Acts::toUnderlying(CovIdx::etaCov)] = prd->template localCovariance<2>()(1,1);
            } else {
                ATH_MSG_WARNING("Unsupported measurement type. "<<typeid(PrdType).name());
                // Prevent division by zero later on.
                cov[Acts::toUnderlying(CovIdx::phiCov)] = 1;
                cov[Acts::toUnderlying(CovIdx::etaCov)] = 1;
            }
        } else {
            /// 
            if (newSp.measuresEta()) {
                cov[Acts::toUnderlying(CovIdx::etaCov)] = prd->template localCovariance<1>()[0];
                cov[Acts::toUnderlying(CovIdx::phiCov)] = Acts::square(sensorHalfLength(*prd));
            } else {
                cov[Acts::toUnderlying(CovIdx::phiCov)] = prd->template localCovariance<1>()[0];
                cov[Acts::toUnderlying(CovIdx::etaCov)] = Acts::square(sensorHalfLength(*prd));
            }
        }
        newSp.setCovariance(std::move(cov));
    }
}
template <class ContType>
    StatusCode SpacePointMakerAlg::loadContainerAndSort(const EventContext& ctx,
                                                        const SG::ReadHandleKey<ContType>& key,
                                                        PreSortedSpacePointMap& fillContainer) const {
    const ContType* measurementCont{nullptr};
    ATH_CHECK(SG::get(measurementCont, key, ctx));
    if (!measurementCont || measurementCont->empty()){
        ATH_MSG_DEBUG("nothing to do"); 
        return StatusCode::SUCCESS;
    }
    const ActsGeometryContext* gctx{nullptr};
    ATH_CHECK(SG::get(gctx, m_geoCtxKey, ctx));
    
    using PrdType = typename ContType::const_value_type;
    using PrdVec = std::vector<PrdType>;
    xAOD::ChamberViewer viewer{*measurementCont};
    do {
        SpacePointsPerChamber& pointsInChamb = fillContainer[viewer.at(0)->readoutElement()->msSector()];
        const Amg::Transform3D sectorTrans = viewer.at(0)->readoutElement()->msSector()->globalToLocalTrans(*gctx);
        ATH_MSG_DEBUG("Fill space points for chamber "<<m_idHelperSvc->toStringDetEl(viewer.at(0)->identify()));
        if constexpr( std::is_same_v<ContType, xAOD::MdtDriftCircleContainer>){
            pointsInChamb.etaHits.reserve(pointsInChamb.etaHits.capacity() + viewer.size());       
            for (const PrdType prd : viewer) {
                Amg::Transform3D toChamberTrans{toChamberTransform(*gctx, sectorTrans, *prd)};
                SpacePoint& sp{pointsInChamb.etaHits.emplace_back(prd)};
                sp.setPosition(positionInChamber(*prd, toChamberTrans));
                sp.setDirection(toChamberTrans.rotation().col(Amg::z),
                                toChamberTrans.rotation().col(Amg::y));
                std::array<double, 3> cov{Acts::filledArray<double,3>(0.)};
                cov[Acts::toUnderlying(CovIdx::etaCov)] = prd->driftRadiusCov();
                cov[Acts::toUnderlying(CovIdx::phiCov)] = Acts::square(sensorHalfLength(*prd));
                if  (ATH_UNLIKELY(prd->numDimensions() == 2)){
                    cov[Acts::toUnderlying(CovIdx::phiCov)] = static_cast<const xAOD::MdtTwinDriftCircle*>(prd)->posAlongWireCov();
                }
                sp.setCovariance(std::move(cov));
            }
        } else {
            /// Loop over the chamber hits to split the hits per gasGap
            using EtaPhi2DHits = std::array<PrdVec, 3>;
            std::vector<EtaPhi2DHits> hitsPerGasGap{};
            for (const PrdType prd : viewer) {
                ATH_MSG_VERBOSE("Create space point from "<<m_idHelperSvc->toString(prd->identify())
                              <<", hash: "<<prd->identifierHash());
                
                unsigned gapIdx = prd->gasGap() -1;
                if constexpr (std::is_same_v<ContType, xAOD::RpcMeasurementContainer>) {
                    gapIdx = prd->readoutElement()->createHash(0, prd->gasGap(), prd->doubletPhi(), false);
                }
                if (hitsPerGasGap.size() <= gapIdx) {
                    hitsPerGasGap.resize(gapIdx + 1);
                }
                bool measPhi{false};
                if constexpr(std::is_same_v<ContType, xAOD::sTgcMeasContainer>) {
                    /// Wires measure the phi coordinate
                    measPhi = prd->channelType() == sTgcIdHelper::sTgcChannelTypes::Wire;
                } else if constexpr(!std::is_same_v<ContType, xAOD::MMClusterContainer>) {
                    /// Tgc & Rpcs have the measuresPhi property
                    measPhi = prd->measuresPhi();
                }

                if (prd->numDimensions() == 2) {
                    hitsPerGasGap[gapIdx][2].push_back(prd);
                    continue;
                }
                /// Sort in the hit
                PrdVec& toPush = hitsPerGasGap[gapIdx][measPhi];
                if (toPush.capacity() == toPush.size()) {
                    toPush.reserve(toPush.size() + m_capacityBucket);
                }
                toPush.push_back(prd);
            }
       
            for (auto& [etaHits, phiHits, two2DHits] : hitsPerGasGap) {
                ATH_MSG_DEBUG("Found "<<etaHits.size()<<"/"<<phiHits.size()<<" 1D and "<<two2DHits.size()<<" 2D hits in chamber "<<m_idHelperSvc->toStringDetEl(viewer.at(0)->identify()));
                /// Fill in the 2D measurements BIL Rpc / sTGC pad
                fillUncombinedSpacePoints(*gctx, sectorTrans, two2DHits, pointsInChamb.etaHits);
                /// Only one dimensional space points can be built
                if (!passOccupancy2D(etaHits, phiHits)) {
                    fillUncombinedSpacePoints(*gctx, sectorTrans, etaHits, pointsInChamb.etaHits);
                    fillUncombinedSpacePoints(*gctx, sectorTrans, phiHits, pointsInChamb.phiHits);
                    continue;
                }

                std::vector<std::shared_ptr<unsigned>> etaCounts{matchCountVec(etaHits.size())}, 
                                                       phiCounts{matchCountVec(phiHits.size())};
                pointsInChamb.etaHits.reserve(etaHits.size()*phiHits.size());
                /// Simple combination by taking the cross-product
                const Amg::Transform3D toSectorTransEta = toChamberTransform(*gctx,sectorTrans, *etaHits.front());
                const Amg::Transform3D toSectorTransPhi = toChamberTransform(*gctx,sectorTrans, *phiHits.front());
            
                Amg::Vector3D toNextDir = toSectorTransEta.rotation().col(Amg::x);
                Amg::Vector3D sensorDir = toSectorTransPhi.rotation().col(Amg::x);
                using namespace Acts::detail::LineHelper;
                for (unsigned etaP = 0; etaP < etaHits.size(); ++etaP) {
                    /// There's no valid combination with another phi hit
                    for (unsigned phiP = 0; phiP < phiHits.size(); ++ phiP) {
                        /** Tgc measurements with different bunch crossing tags cannot be combined */
                        if constexpr(std::is_same<xAOD::TgcStripContainer, ContType>::value) {
                            if (!(etaHits[etaP]->bcBitMap() & phiHits[phiP]->bcBitMap())){
                                continue;
                            }
                            const auto& stripLay = phiHits[phiP]->readoutElement()->sensorLayout(phiHits[phiP]->layerHash());
                            const auto& radialDesign = static_cast<const MuonGMR4::RadialStripDesign&>(stripLay->design(true));
                            toNextDir = toSectorTransPhi.rotation() * stripLay->to3D(radialDesign.stripDir(phiHits[phiP]->channelNumber()), true);
                        }
                        SpacePoint& newSp = pointsInChamb.etaHits.emplace_back(etaHits[etaP], phiHits[phiP]);
                        newSp.setInstanceCounts(etaCounts[etaP], phiCounts[phiP]);
          
                        auto spIsect = lineIntersect(positionInChamber(*etaHits[etaP], toSectorTransEta), sensorDir, 
                                                     positionInChamber(*phiHits[phiP], toSectorTransPhi), toNextDir); 
                        newSp.setPosition(spIsect.position());
                        newSp.setDirection(sensorDir, toNextDir);
                        auto cov = Acts::filledArray<double, 3>(0.);
                        cov[Acts::toUnderlying(CovIdx::etaCov)] = etaHits[etaP]->template localCovariance<1>()[0];
                        cov[Acts::toUnderlying(CovIdx::phiCov)] = phiHits[phiP]->template localCovariance<1>()[0];
                        newSp.setCovariance(std::move(cov));
                    }
                }
            }
        }
    } while (viewer.next());
    return StatusCode::SUCCESS;
}


StatusCode SpacePointMakerAlg::execute(const EventContext& ctx) const {
    PreSortedSpacePointMap preSortedContainer{};
    ATH_CHECK(loadContainerAndSort(ctx, m_mdtKey, preSortedContainer));
    ATH_CHECK(loadContainerAndSort(ctx, m_rpcKey, preSortedContainer));
    ATH_CHECK(loadContainerAndSort(ctx, m_tgcKey, preSortedContainer));
    ATH_CHECK(loadContainerAndSort(ctx, m_mmKey, preSortedContainer));
    ATH_CHECK(loadContainerAndSort(ctx, m_stgcKey, preSortedContainer));
    std::unique_ptr<SpacePointContainer> outContainer = std::make_unique<SpacePointContainer>();
    
    for (auto &[chamber, hitsPerChamber] : preSortedContainer){
        ATH_MSG_DEBUG("Fill space points for chamber "<<chamber->identString() << " with "<<hitsPerChamber.etaHits.size()
                        <<" primary and "<<hitsPerChamber.phiHits.size()<<" phi space points.");
        distributePointsAndStore(std::move(hitsPerChamber), *outContainer);
    }
    SG::WriteHandle<SpacePointContainer> writeHandle{m_writeKey, ctx};
    ATH_CHECK(writeHandle.record(std::move(outContainer)));
    return StatusCode::SUCCESS;
}

void SpacePointMakerAlg::distributePointsAndStore(SpacePointsPerChamber&& hitsPerChamber,
                                                  SpacePointContainer& finalContainer) const {
    SpacePointBucketVec splittedHits{};
    splittedHits.emplace_back();
    if (m_statCounter){
        m_statCounter->addToStat(hitsPerChamber.etaHits);
        m_statCounter->addToStat(hitsPerChamber.phiHits);

    }
    distributePrimaryPoints(std::move(hitsPerChamber.etaHits), splittedHits);
    splittedHits.erase(std::remove_if(splittedHits.begin(), splittedHits.end(),
                       [](const SpacePointBucket& bucket) {
                           return bucket.size() <= 1;
                       }), splittedHits.end());
    distributePhiPoints(std::move(hitsPerChamber.phiHits), splittedHits);
    
    for (SpacePointBucket& bucket : splittedHits) {

        std::ranges::sort(bucket, MuonR4::SpacePointPerLayerSorter{});

        if (msgLvl(MSG::VERBOSE)){
            std::stringstream spStr{};
            for (const std::shared_ptr<SpacePoint>& sp : bucket){
                spStr<<"SpacePoint: PrimaryMeas: " <<(*sp)<<std::endl;
            }
            ATH_MSG_VERBOSE("Created a bucket, printing all spacepoints..."<<std::endl<<spStr.str());
        }
        bucket.populateChamberLocations();
        finalContainer.push_back(std::make_unique<SpacePointBucket>(std::move(bucket)));
    }

}
void SpacePointMakerAlg::distributePhiPoints(std::vector<SpacePoint>&& spacePoints,
                                             SpacePointBucketVec& splittedContainer) const{
    for (SpacePoint& sp : spacePoints) {
        auto phiPoint = std::make_shared<SpacePoint>(std::move(sp));
        const double dY = std::sqrt(phiPoint->covariance()[Acts::toUnderlying(CovIdx::etaCov)]);
        const double minY = phiPoint->localPosition().y() - dY;
        const double maxY = phiPoint->localPosition().y() + dY;
        for (SpacePointBucket& bucket : splittedContainer){
            /// If maxY is smaller than the lower cov boundary or minY is bigger than the 
            /// other boundary, there's definetely no overlap
            if (! (maxY < bucket.coveredMin() || bucket.coveredMax() < minY) ) {
                bucket.emplace_back(phiPoint);
            }
        }
    }
}
bool SpacePointMakerAlg::splitBucket(const SpacePoint& spacePoint,
                                     const double firstSpPos,
                                     const SpacePointBucketVec& sortedPoints) const {
    /// Distance between this point and the first one exceeds the maximum length
    const double spY = spacePoint.localPosition().y();
    if (spY - firstSpPos > m_maxBucketLength){
        return true;
    }
    
    if (sortedPoints.empty() || sortedPoints.back().empty()) {
        return false;
    }
    return spY - sortedPoints.back().back()->localPosition().y() > m_spacePointWindow;
}
void SpacePointMakerAlg::newBucket(const SpacePoint& refSpacePoint,
                                   SpacePointBucketVec& sortedPoints) const {
    SpacePointBucket& newContainer = sortedPoints.emplace_back();
    newContainer.setBucketId(sortedPoints.size() -1);

    ///Set the boundaries from the previous bucket
    SpacePointBucket& overlap{sortedPoints[sortedPoints.size() - 2]};
    overlap.setCoveredRange(overlap.front()->localPosition().y(), 
                            overlap.back()->localPosition().y());
    
    const double refBound = refSpacePoint.localPosition().y();
                                
    /** Copy space points that could be within the overlap region to the next bucket */
    for (const std::shared_ptr<SpacePoint>& pointInBucket : overlap | std::views::reverse) {
        const double overlapPos = pointInBucket->localPosition().y() + 
                                  std::sqrt(pointInBucket->covariance()[Acts::toUnderlying(CovIdx::etaCov)]);
        if (refBound - overlapPos < m_spacePointOverlap) {    
            newContainer.insert(newContainer.begin(), pointInBucket);
        } else {
            break;
        }
    }    

}

void SpacePointMakerAlg::distributePrimaryPoints(std::vector<SpacePoint>&& spacePoints, 
                                                 SpacePointBucketVec& splittedHits) const {

    if (spacePoints.empty()) return;
   
    /** Order the space points by local chamber y which is along the tube-plane. */
    std::ranges::sort(spacePoints, 
                      [] (const SpacePoint& a, const SpacePoint& b) {
                        return a.localPosition().y() < b.localPosition().y();
                      });

    double firstPointPos = spacePoints.front().localPosition().y();
    
    for (SpacePoint& toSort : spacePoints) {        
        ATH_MSG_VERBOSE("Add new primary space point "<<toSort);

        if (splitBucket(toSort, firstPointPos, splittedHits)){
            newBucket(toSort, splittedHits);
            firstPointPos = splittedHits.back().empty() ? toSort.localPosition().y() : splittedHits.back().front()->localPosition().y();
            ATH_MSG_VERBOSE("New bucket: id " << splittedHits.back().bucketId() << " Coverage: " << firstPointPos);
        }
        std::shared_ptr<SpacePoint> spacePoint = std::make_shared<SpacePoint>(std::move(toSort));
        splittedHits.back().emplace_back(spacePoint);
    }
    SpacePointBucket& lastBucket{splittedHits.back()};
    lastBucket.setCoveredRange(lastBucket.front()->localPosition().y(), 
                               lastBucket.back()->localPosition().y());
}

}
