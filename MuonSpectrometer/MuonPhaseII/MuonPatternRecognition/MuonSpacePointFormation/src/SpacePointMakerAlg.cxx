/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "SpacePointMakerAlg.h"

#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "GeoPrimitives/GeoPrimitives.h"
#include "GeoPrimitives/GeoPrimitivesToStringConverter.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"
#include <GaudiKernel/IMessageSvc.h>
#include <memory>
#include <sstream>
#include <algorithm>
#include <ranges>
#include <optional>
#include <type_traits>
#include "MuonSpacePoint/SpacePointPerLayerSorter.h"
#include "MuonReadoutGeometryR4/PadDesign.h"
#include "xAODMeasurementBase/MeasurementDefs.h"
#include "xAODMeasurementBase/UncalibratedMeasurement.h"
#include "xAODMuonPrepData/RpcStrip.h"
#include "xAODMuonPrepData/MdtTwinDriftCircle.h"


#include "Acts/Surfaces/detail/LineHelper.hpp"
namespace {
    using CovIdx = MuonR4::SpacePoint::CovIdx;

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
        #if defined(FLATTEN)
            // We compile this package with optimization, even in debug builds; otherwise,
            // the heavy use of Eigen makes it too slow.  However, from here we may call
            // to out-of-line Eigen code that is linked from other DSOs; in that case,
            // it would not be optimized.  Avoid this by forcing all Eigen code
            // to be inlined here if possible.
            ATH_FLATTEN
        #endif
        Acts::Transform3 toChamberTransform(const ActsTrk::GeometryContext& gctx, 
                                            const Acts::Transform3& sectorTrans,
                                            const MeasType& meas) {
        const MuonGMR4::MuonReadoutElement* reEle{meas.readoutElement()};
        if constexpr(std::is_same_v<MeasType, xAOD::MdtDriftCircle>) {
            return sectorTrans * reEle->localToGlobalTransform(gctx, meas.measurementHash());
        } else {
            return sectorTrans * reEle->localToGlobalTransform(gctx, meas.layerHash());
        }
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
                return 0.5 * re->sensorLayout(prd.layerHash())->design(prd.measuresPhi()).stripLength(prd.channelNumber());
            } else if constexpr(std::is_same_v<PrdType, xAOD::MMCluster>) {
                return 0.5* re->stripLayer(prd.layerHash()).design().stripLength(prd.channelNumber());
            } else if constexpr(std::is_same_v<PrdType, xAOD::sTgcMeasurement>) {
                return 0.5* re->stripLayer(prd.layerHash()).design().stripLength(prd.channelNumber());
            }
            return 0.;
        }
    /** @brief Extracts the covariance element from the sTgc measurement. In case of a
     *         pad measurement, the covariance index is used to select the proper matrix element
     *  @param m: Reference to the measurement of interest
     *  @param covIdx: Requested covariance index*/
    inline double covElement(const xAOD::sTgcMeasurement& m,
                             const CovIdx covIdx) {
        if (m.numDimensions() == 2) {
            const unsigned i = (covIdx != CovIdx::etaCov);
            return m.localCovariance<2>()(i,i);
        }
        return m.localCovariance<1>()[0];
    }

/** @brief Matches a string against a pattern with wildcards
 *  @param pattern: The pattern to match against, may contain '*' as a wildcard
 *  @param value: The string to be matched
 *  @return true if the string matches the pattern, false otherwise */
bool wildcardMatch(const std::string_view pattern,
                   const std::string_view value) {
    std::size_t patternPos = 0;
    std::size_t valuePos = 0;
    std::size_t wildcardPos = std::string_view::npos;
    std::size_t wildcardValuePos = 0;

    while (valuePos < value.size()) {
        if (patternPos < pattern.size() &&
            pattern[patternPos] == value[valuePos]) {
            ++patternPos;
            ++valuePos;
        } else if (patternPos < pattern.size() &&
                   pattern[patternPos] == '*') {
            wildcardPos = patternPos++;
            wildcardValuePos = valuePos;
        } else if (wildcardPos != std::string_view::npos) {
            patternPos = wildcardPos + 1;
            valuePos = ++wildcardValuePos;
        } else {
            return false;
        }
    }

    while (patternPos < pattern.size() &&
           pattern[patternPos] == '*') {
        ++patternPos;
    }
    return patternPos == pattern.size();
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


    //Initialize the chamber-based bucket windows and max bucket lengths
    m_defaultBucketParameters = {
            .spacePointWindow = m_spacePointWindow.value(),
            .maxBucketLength  = m_maxBucketLength.value(),
            .spacePointOverlap= m_spacePointOverlap.value()
    };


    ATH_CHECK(detStore()->retrieve(m_detMgr));
    const auto chambers = m_detMgr->getAllChambers();

    m_bucketParameters.reserve(chambers.size());
    for (const MuonGMR4::Chamber* chamber : chambers) {
        const std::optional<BucketParameters> parameters = resolveBucketParameters(*chamber);

        //Fill only the chambers for which we have at least one match, the rest will be filled with the default parameters on demand
        if (!parameters) {
            continue;
        }

        m_bucketParameters.emplace( chamber, *parameters);
    }

    ATH_MSG_DEBUG( "Configured chamber-dependent bucket parameters for " << m_bucketParameters.size() << " chambers out of " << chambers.size() << " the rest has the following default bucket parameters: "
                   << " maxBucketLength=" << m_defaultBucketParameters.maxBucketLength
                   << " spacePointWindow=" << m_defaultBucketParameters.spacePointWindow
                   << " spacePointOverlap=" << m_defaultBucketParameters.spacePointOverlap);

    return StatusCode::SUCCESS;
}

template <> 
    bool SpacePointMakerAlg::passOccupancy2D(const PrdVec_t<const xAOD::TgcStrip*>& etaHits,
                                             const PrdVec_t<const xAOD::TgcStrip*>& phiHits) const {
        if (etaHits.empty() || phiHits.empty()) {
            return false;
        }
        const MuonGMR4::TgcReadoutElement* re = etaHits[0]->readoutElement();
        ATH_MSG_VERBOSE("Collected "<<etaHits.size()<<"/"<<phiHits.size()<<" hits in "<<m_idHelperSvc->toStringGasGap(etaHits[0]->identify()));
        return ((1.*etaHits.size()) / ((1.*re->numChannels(etaHits[0]->measurementHash())))) < m_maxOccTgcEta &&
               ((1.*phiHits.size()) / ((1.*re->numChannels(phiHits[0]->measurementHash())))) < m_maxOccTgcPhi;
    }
template <> 
    bool SpacePointMakerAlg::passOccupancy2D(const PrdVec_t<const xAOD::RpcMeasurement*>& etaHits,
                                             const PrdVec_t<const xAOD::RpcMeasurement*>& phiHits) const {
        if (etaHits.empty() || phiHits.empty()) {
            return false;
        }
        const MuonGMR4::RpcReadoutElement* re = etaHits[0]->readoutElement();
        ATH_MSG_VERBOSE("Collected "<<etaHits.size()<<"/"<<phiHits.size()<<" hits in "<<m_idHelperSvc->toStringGasGap(etaHits[0]->identify()));
        return ((1.*etaHits.size()) / (1.*re->nEtaStrips())) < m_maxOccRpcEta &&
               ((1.*phiHits.size()) / (1.*re->nPhiStrips())) < m_maxOccRpcPhi;
    }

template <>
    bool SpacePointMakerAlg::passOccupancy2D(const PrdVec_t<const xAOD::MMCluster*>& /*etaHits*/,
                                             const PrdVec_t<const xAOD::MMCluster*>& /*phiHits*/) const {
       return false;
    }
template <typename PrdType>
    void SpacePointMakerAlg::fillUncombinedSpacePoints(const ActsTrk::GeometryContext& gctx,
                                                       const Acts::Transform3& sectorTrans,
                                                       const PrdVec_t<const PrdType*>& prdsToFill,
                                                       std::vector<SpacePoint>& outColl) const {
    if (prdsToFill.empty()) {
        return;
    }
    const PrdType* refMeas = prdsToFill.front();
    bool allSpArePhi{false};

    const Acts::Transform3 toSectorTrans = toChamberTransform(gctx, sectorTrans, *refMeas);
    /// Local coordinate system aligned such that the strips point along local y
    Amg::Vector3D sensorDir{Amg::Vector3D::Zero()}, toNextSen{Amg::Vector3D::Zero()};
    /// The measurement is a phi measurement
    if constexpr(std::is_same_v<PrdType, xAOD::RpcMeasurement> ||
                 std::is_same_v<PrdType, xAOD::TgcStrip>) {
        allSpArePhi = refMeas->measuresPhi();
        const auto& stripLayout = refMeas->readoutElement()->sensorLayout(refMeas->layerHash());
        const auto& design = stripLayout->design(allSpArePhi);
        sensorDir = toSectorTrans.linear() * stripLayout->to3D(design.stripDir(), allSpArePhi);
        toNextSen = toSectorTrans.linear() * stripLayout->to3D(design.stripNormal(), allSpArePhi);
        ATH_MSG_VERBOSE("Fill space points for "<<m_idHelperSvc->toString(refMeas->identify())
            <<" -> sensor: "<<Amg::toString(sensorDir)<<", "<<Amg::toString(toNextSen));
    } else if constexpr (std::is_same_v<PrdType, xAOD::sTgcMeasurement>){
        allSpArePhi  = refMeas->channelType() == xAOD::sTgcMeasurement::sTgcChannelTypes::Wire;
        const auto& stripLayout = refMeas->readoutElement()->stripLayer(refMeas->measurementHash());
        const auto& design = stripLayout.design(allSpArePhi);
        sensorDir = toSectorTrans.linear() * stripLayout.to3D(design.stripDir(), allSpArePhi);
        toNextSen = toSectorTrans.linear() * stripLayout.to3D(design.stripNormal(), allSpArePhi);
    } else {
        sensorDir = toSectorTrans.linear().col(Amg::y);
        toNextSen = toSectorTrans.linear().col(Amg::x); 
    }
    outColl.reserve(outColl.size() + prdsToFill.size());
    for (const PrdType* prd: prdsToFill) {
        SpacePoint& newSp = outColl.emplace_back(prd);
        if constexpr (std::is_same_v<PrdType, xAOD::TgcStrip>) {
            if (allSpArePhi) {
                const auto& stripLayout = refMeas->readoutElement()->sensorLayout(refMeas->layerHash());
                const auto& radialDesign = static_cast<const MuonGMR4::RadialStripDesign&>(stripLayout->design(allSpArePhi));
                toNextSen = toSectorTrans.linear() * stripLayout->to3D(radialDesign.stripNormal(prd->channelNumber()), allSpArePhi);
                sensorDir = toSectorTrans.linear() * stripLayout->to3D(radialDesign.stripDir(prd->channelNumber()), allSpArePhi);
            }
        }
        newSp.setPosition(toSectorTrans * prd->localMeasurementPos());
        newSp.setDirection(sensorDir, toNextSen);
        auto cov = Acts::filledArray<double,3>(0.);        
        if (prd->numDimensions() == 2) {
            cov[Acts::toUnderlying(CovIdx::etaCov)] = prd->template localCovariance<2>()(0,0);
            cov[Acts::toUnderlying(CovIdx::phiCov)] = prd->template localCovariance<2>()(1,1);
        } else {
            /// 
            auto covIdx{Acts::toUnderlying(CovIdx::etaCov)}, 
                 lenIdx{Acts::toUnderlying(CovIdx::phiCov)};
            if (!newSp.measuresEta()) {
                std::swap(covIdx, lenIdx);
            }
            cov[covIdx] = prd->template localCovariance<1>()[0];
            cov[lenIdx] = Acts::square(sensorHalfLength(*prd));
        }
        newSp.setCovariance(std::move(cov));
    }
}


template <typename ContType>
SpacePointMakerAlg::EtaPhi2DHitsVec<typename ContType::const_value_type>
SpacePointMakerAlg::splitHitsPerGasGap(xAOD::ChamberViewer<ContType>& viewer) const {
    std::vector<EtaPhi2DHits<typename ContType::const_value_type>> hitsPerGasGap{};
    for (const auto& prd : viewer) {
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
        auto& toPush = hitsPerGasGap[gapIdx][measPhi];
        if (toPush.capacity() == toPush.size()) {
            toPush.reserve(toPush.size() + m_capacityBucket);
        }
        toPush.push_back(prd);
    }
    return hitsPerGasGap;
}

template <typename ContType>
    StatusCode SpacePointMakerAlg::loadContainerAndSort(const EventContext& ctx,
                                                        const SG::ReadHandleKey<ContType>& key,
                                                        PreSortedSpacePointMap& fillContainer) const {
    const ContType* measurementCont{nullptr};
    ATH_CHECK(SG::get(measurementCont, key, ctx));
    if (!measurementCont || measurementCont->empty()){
        ATH_MSG_DEBUG("nothing to do"); 
        return StatusCode::SUCCESS;
    }
    const ActsTrk::GeometryContext* gctx{nullptr};
    ATH_CHECK(SG::get(gctx, m_geoCtxKey, ctx));
    
    xAOD::ChamberViewer viewer{*measurementCont};

    do {
        SpacePointsPerChamber& pointsInChamb = fillContainer[viewer.at(0)->readoutElement()->msSector()];
        const Acts::Transform3 sectorTrans = viewer.at(0)->readoutElement()->msSector()->globalToLocalTransform(*gctx);
        ATH_MSG_DEBUG("Fill space points for chamber "<<m_idHelperSvc->toStringDetEl(viewer.at(0)->identify()));
        if constexpr( std::is_same_v<ContType, xAOD::MdtDriftCircleContainer>) {
            pointsInChamb.etaHits.reserve(pointsInChamb.etaHits.capacity() + viewer.size());       
            for (const auto& prd : viewer) {
                Acts::Transform3 toChamberTrans{toChamberTransform(*gctx, sectorTrans, *prd)};
                SpacePoint& sp{pointsInChamb.etaHits.emplace_back(prd)};
                sp.setPosition(toChamberTrans*prd->localMeasurementPos());
                sp.setDirection(toChamberTrans.linear().col(Amg::z),
                                toChamberTrans.linear().col(Amg::y));
                std::array<double, 3> cov{Acts::filledArray<double,3>(0.)};
                cov[Acts::toUnderlying(CovIdx::etaCov)] = prd->driftRadiusCov();
                cov[Acts::toUnderlying(CovIdx::phiCov)] = Acts::square(sensorHalfLength(*prd));
                if  (prd->numDimensions() == 2) [[unlikely]] {
                    cov[Acts::toUnderlying(CovIdx::phiCov)] = static_cast<const xAOD::MdtTwinDriftCircle*>(prd)->posAlongWireCov();
                }
                sp.setCovariance(std::move(cov));
            }
        } else {
            /// Loop over the chamber hits to split the hits per gasGap
            for (auto& [etaHits, phiHits, two2DHits] : splitHitsPerGasGap(viewer)) {
                ATH_MSG_DEBUG("Found "<<etaHits.size()<<"/"<<phiHits.size()
                        <<" 1D and "<<two2DHits.size()<<" 2D hits in chamber "
                        <<m_idHelperSvc->toStringDetEl(viewer.at(0)->identify()));
                /// Fill in the 2D measurements BIL Rpc 
                fillUncombinedSpacePoints(*gctx, sectorTrans, two2DHits, pointsInChamb.etaHits);                
                /// Only one dimensional space points can be built
                // Check if we do not have 2D occupancy (missing phi or eta hits)
                if (!passOccupancy2D(etaHits, phiHits)) {
                    fillUncombinedSpacePoints(*gctx, sectorTrans, etaHits, pointsInChamb.etaHits);
                    fillUncombinedSpacePoints(*gctx, sectorTrans, phiHits, pointsInChamb.phiHits);
                    continue;
                }

                std::vector<std::shared_ptr<unsigned>> etaCounts{matchCountVec(etaHits.size())}, 
                                                       phiCounts{matchCountVec(phiHits.size())};

                pointsInChamb.etaHits.reserve(pointsInChamb.etaHits.size() + etaHits.size()*phiHits.size());               
                /// Simple combination by taking the cross-product
                const auto& firstEta{etaHits.front()};
                const Acts::Transform3 toSectorTrans = toChamberTransform(*gctx, sectorTrans, *firstEta);
               
                Amg::Vector3D toNextDir{Amg::Vector3D::Zero()}, sensorDir{Amg::Vector3D::Zero()};
                if constexpr (std::is_same_v<xAOD::RpcMeasurementContainer, ContType> ||
                              std::is_same_v<xAOD::TgcStripContainer, ContType>) {
                    const auto& stripLayout = firstEta->readoutElement()->sensorLayout(firstEta->layerHash());
                    const auto& design = stripLayout->design();
                    sensorDir = toSectorTrans.linear() * stripLayout->to3D(design.stripDir(), false);
                    toNextDir = toSectorTrans.linear() * stripLayout->to3D(design.stripNormal(), false);
                } else if constexpr (std::is_same_v<xAOD::sTgcMeasContainer, ContType>){
                    const auto& stripLayout = firstEta->readoutElement()->stripLayer(firstEta->measurementHash());
                    const auto& design = stripLayout.design(false);
                    sensorDir = toSectorTrans.linear() * stripLayout.to3D(design.stripDir(), false);
                    toNextDir = toSectorTrans.linear() * stripLayout.to3D(design.stripNormal(), false);
                }  else {
                    ATH_MSG_ERROR("Unsupported container type");
                    return StatusCode::FAILURE;
                }               
                
                using namespace Acts::detail::LineHelper;
                for (unsigned etaP = 0; etaP < etaHits.size(); ++etaP) {
                    /// There's no valid combination with another phi hit
                    for (unsigned phiP = 0; phiP < phiHits.size(); ++ phiP) {
                        /** Tgc measurements with different bunch crossing tags cannot be combined */
                        if constexpr(std::is_same_v<xAOD::TgcStripContainer, ContType>) {
                            if (!(etaHits[etaP]->bcBitMap() & phiHits[phiP]->bcBitMap())){
                                continue;
                            } 
                            const auto& stripLay = phiHits[phiP]->readoutElement()->sensorLayout(phiHits[phiP]->layerHash());
                            const auto& radialDesign = static_cast<const MuonGMR4::RadialStripDesign&>(stripLay->design(true));
                            toNextDir = toSectorTrans.linear() * stripLay->to3D(radialDesign.stripDir(phiHits[phiP]->channelNumber()), true);
                        }
                        
                        SpacePoint& newSp = pointsInChamb.etaHits.emplace_back(etaHits[etaP], phiHits[phiP]);
                        newSp.setInstanceCounts(etaCounts[etaP], phiCounts[phiP]);
            
                        auto spIsect = lineIntersect(toSectorTrans*etaHits[etaP]->localMeasurementPos(), sensorDir, 
                                                     toSectorTrans*phiHits[phiP]->localMeasurementPos(), toNextDir); 
                        newSp.setPosition(spIsect.position());
                        newSp.setDirection(sensorDir, toNextDir);
                        auto cov = Acts::filledArray<double, 3>(0.);
                        cov[Acts::toUnderlying(CovIdx::etaCov)] = etaHits[etaP]->template localCovariance<1>()[0];
                        cov[Acts::toUnderlying(CovIdx::phiCov)] = phiHits[phiP]->template localCovariance<1>()[0];
                        ///
                        if constexpr(std::is_same_v<xAOD::TgcStripContainer, ContType>) {
                            const auto& stripLay = phiHits[phiP]->readoutElement()->sensorLayout(phiHits[phiP]->layerHash());
                            const auto& radialDesign = static_cast<const MuonGMR4::RadialStripDesign&>(stripLay->design(true));
                            const Amg::Vector2D planePos = stripLay->to2D(toSectorTrans.inverse()*spIsect.position(), true);
                            cov[Acts::toUnderlying(CovIdx::phiCov)] =
                                Acts::square(radialDesign.stripPitch(phiHits[phiP]->channelNumber(), planePos)) / 12.;
                        }

                        newSp.setCovariance(std::move(cov));
                        ATH_MSG_VERBOSE("Created new space point "<<newSp);
                    }
                }
            }
        }
    } while (viewer.next());
    return StatusCode::SUCCESS;
}

template<>
StatusCode SpacePointMakerAlg::loadContainerAndSort(const EventContext& ctx,
                                                    const SG::ReadHandleKey<xAOD::sTgcMeasContainer>& key,
                                                    PreSortedSpacePointMap& fillContainer) const {

    const xAOD::sTgcMeasContainer* measurementCont{nullptr};
    ATH_CHECK(SG::get(measurementCont, key, ctx));
    if (!measurementCont || measurementCont->empty()){
        ATH_MSG_DEBUG("nothing to do"); 
        return StatusCode::SUCCESS;
    }
    const ActsTrk::GeometryContext* gctx{nullptr};
    ATH_CHECK(SG::get(gctx, m_geoCtxKey, ctx));
    xAOD::ChamberViewer viewer{*measurementCont};
    using namespace Acts::detail::LineHelper;
    do {
        SpacePointsPerChamber& pointsInChamb = fillContainer[viewer.at(0)->readoutElement()->msSector()];
        const Acts::Transform3 sectorTrans = viewer.at(0)->readoutElement()->msSector()->globalToLocalTransform(*gctx);
        ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Fill space points for multiplet "<<m_idHelperSvc->toStringDetEl(viewer.at(0)->identify()));
        for(auto& HitColls: splitHitsPerGasGap(viewer)){ 
            auto& [etaHits, phiHits, two2DHits] =  HitColls;          
            std::array<std::vector<std::shared_ptr<unsigned>>, 3> instanceCounts{matchCountVec(etaHits.size()), 
                                                                                 matchCountVec(phiHits.size()),
                                                                                 matchCountVec(two2DHits.size())};                                                      
            
            //loop through the Prds and try to combine according` to the hierarchy
            // Strip+Wire
            // Strip+Pad
            // Wire+Pad
            // Pad
            /// @brief Helper function combine an eta and a phi collection. The function takes 
            ///        two indices referrring to the indices of the hit vector inside HitColls
            ///
            /// @param collIdxA: Index of the collection to put as phi
            /// @param collIdxB: Index of the collection to put as eta
            /// @param combFunc: Lambda function that rejects hits which cannot be 
            ///                  combined due to geometry reasons.
            auto combineMe = [&](const std::size_t collIdxA,
                                 const std::size_t collIdxB,
                                 const std::function<bool(const xAOD::sTgcMeasurement*,
                                                          const xAOD::sTgcMeasurement*)>& combFunc) {
                std::vector<char> combinedFlagsA{}, combinedFlagsB{};
                std::ranges::transform(instanceCounts[collIdxA], std::back_inserter(combinedFlagsA),
                                       [](const std::shared_ptr<unsigned>& countPtr){
                                           return (*countPtr) == 0;
                                       });
                std::ranges::transform(instanceCounts[collIdxB], std::back_inserter(combinedFlagsB),
                                       [](const std::shared_ptr<unsigned>& countPtr){
                                           return (*countPtr) == 0;
                                       });
                                       
                const auto& collA = HitColls[collIdxA];
                const auto& collB = HitColls[collIdxB];

                /// Skip if one of collections are empty
                if(collA.empty() || collB.empty()) {
                    ATH_MSG_DEBUG(__func__<<"() "<<__LINE__<<" - Skipping combination: both collections empty");
                    return;
                }
                
                /// Get first hit from the first collection 
                const xAOD::sTgcMeasurement* firstHit = collB.front();
                const Acts::Transform3 toSectorTrans = toChamberTransform(*gctx, sectorTrans, *firstHit);
               
                for(std::size_t idxA = 0; idxA < collA.size(); ++idxA) {
                    /// The hit in the collection has already been used
                    if(!combinedFlagsA[idxA]) {
                        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Hit "<<m_idHelperSvc->toString(collA[idxA]->identify())
                                        <<" has been used in previous iteration");
                        continue;
                    }
                    for(std::size_t idxB = 0; idxB < collB.size(); ++idxB) {
                        if(!combinedFlagsB[idxB] || !combFunc(collA[idxA], collB[idxB])){
                            ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Hit "<<m_idHelperSvc->toString(collB[idxB]->identify())
                                <<" has been used in previous iteration. Or is incompatible with "
                                <<m_idHelperSvc->toString(collA[idxA]->identify()));
                            continue;
                        }
                        //create space point
                        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" - Combine sTgc measurements "
                            <<m_idHelperSvc->toString(collA[idxA]->identify())<<" and "
                            <<m_idHelperSvc->toString(collB[idxB]->identify())<< "with local positions"
                            << Amg::toString(collA[idxA]->localMeasurementPos()) << " and "
                            << Amg::toString(collB[idxB]->localMeasurementPos())
                            <<" to new space point");
                        
                        SpacePoint& newSp = pointsInChamb.etaHits.emplace_back(collB[idxB], collA[idxA]);
                        auto crossPoint = lineIntersect<3>(collA[idxA]->localMeasurementPos(), 
                                                           Amg::Vector3D::UnitX(),
                                                           collB[idxB]->localMeasurementPos(),
                                                           Amg::Vector3D::UnitY());

                        newSp.setPosition(toSectorTrans*crossPoint.position());
                        newSp.setDirection(Amg::Vector3D::UnitX(), Amg::Vector3D::UnitY());
                        auto cov = Acts::filledArray<double, 3>(0.);
                        cov[Acts::toUnderlying(CovIdx::phiCov)] = covElement(*collA[idxA], CovIdx::phiCov);
                        cov[Acts::toUnderlying(CovIdx::etaCov)] = covElement(*collB[idxB], CovIdx::etaCov);
                        newSp.setCovariance(std::move(cov));
                        newSp.setInstanceCounts(instanceCounts[collIdxB][idxB], instanceCounts[collIdxA][idxA]);
                        ATH_MSG_VERBOSE("Created new space point "<<newSp);
                    }
                }
            };

            //try to combine strip with wire measurements first
            combineMe(1, 0, [&](const xAOD::sTgcMeasurement* wire,
                                const xAOD::sTgcMeasurement* strip){
                // do not combine the strips with the wire that are in the etaZero region
                const MuonGMR4::sTgcReadoutElement* readoutElement = strip->readoutElement();
                if(readoutElement->isEtaZero(strip->measurementHash(), 
                                             strip->localMeasurementPos().block<2,1>(0,0))){
                    return false;
                }
                //ignore combinations where the wire and the strip are not crossing
                //check if the projection of the crossing point is within the bounds of the layer
                Amg::Vector3D crossPoint = strip->localMeasurementPos() + wire->localMeasurementPos();
                const Acts::Surface& surf = readoutElement->surface(strip->layerHash());                        
                return surf.insideBounds(crossPoint.block<2,1>(0,0)); 
            });

            //combine strip and pad measurements
            combineMe(2, 0, [&](const xAOD::sTgcMeasurement* pad,
                                const xAOD::sTgcMeasurement* strip){
                // do not combine the strips with the pads that are not overlayed
                const MuonGMR4::sTgcReadoutElement* readoutElement = pad ->readoutElement();
                const MuonGMR4::PadDesign& padDesign = readoutElement->padDesign(pad->measurementHash());
                double padHeight = padDesign.padHeight();
                const Amg::Vector3D padCenter = pad->localMeasurementPos();
               
                return std::abs(strip->localMeasurementPos().x() - padCenter.x()) < 0.5*padHeight;
            });

            //finally combine wire and pad measurements
            combineMe(1, 2, [&](const xAOD::sTgcMeasurement* wire,
                                const xAOD::sTgcMeasurement* pad){
                // do not combine the wires with the pads that are not overlayed
                const MuonGMR4::sTgcReadoutElement* readoutElement = pad ->readoutElement();
                const std::array<Amg::Vector2D, 4> localPadCorners = readoutElement->localPadCorners(pad->measurementHash());
                auto [min,max] = std::ranges::minmax_element(localPadCorners.begin(), localPadCorners.end(),
                                                [](const Amg::Vector2D& a, const Amg::Vector2D& b){
                                                    return a.y() < b.y();
                                                });
                return (wire->localMeasurementPos().y() > min->y() || wire->localMeasurementPos().y() < max->y()); 
        
            });

            //fill uncombined strip, wire and pad measurements that have not been used in combination  
            for(std::size_t collIdx = 0; collIdx < HitColls.size(); ++collIdx){
                const auto& hits = HitColls[collIdx];    
                std::vector<const xAOD::sTgcMeasurement*> unusedHits{};           
                unusedHits.reserve(hits.size());            

                for(std::size_t idx = 0; idx < hits.size(); ++idx){
                    if((*instanceCounts[collIdx][idx]) == 0){
                        unusedHits.push_back(hits[idx]);
                    }
                }
                fillUncombinedSpacePoints(*gctx, sectorTrans, unusedHits, pointsInChamb.etaHits);
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

    ATH_MSG_DEBUG("Created a total of "<<outContainer->size()<<" space points in "<<preSortedContainer.size()<<" chambers");

    SG::WriteHandle writeHandle{m_writeKey, ctx};
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
    
    std::size_t nBuckets{0};
    std::size_t nSpacePointsInBuckets{0};
    std::size_t maxBucketSize{0};
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

        ++nBuckets;
        nSpacePointsInBuckets += bucket.size();
        maxBucketSize = std::max(maxBucketSize, bucket.size());
        finalContainer.push_back(std::make_unique<SpacePointBucket>(std::move(bucket)));
    }

    ATH_MSG_DEBUG("Created "<<nBuckets<<" buckets with a total of "<<nSpacePointsInBuckets
                 <<" space points. Max bucket size: "<<maxBucketSize);

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
                                     const SpacePointBucketVec& sortedPoints, const BucketParameters& bucketParams) const {

    /// Distance between this point and the first one exceeds the maximum length
    const double spY = spacePoint.localPosition().y();

    if (spY - firstSpPos > bucketParams.maxBucketLength){
        ATH_MSG_DEBUG("Splitting bucket based on maxLength. First space point Y=" << firstSpPos
                  << " current space point Y =" << spY
                  << " m_maxBucketLength=" << bucketParams.maxBucketLength );
        return true;
    }
    
    if (sortedPoints.empty() || sortedPoints.back().empty()) {
        return false;
    }

    const double gap = spY - sortedPoints.back().back()->localPosition().y();


    if (gap > bucketParams.spacePointWindow){
         ATH_MSG_DEBUG("Splitting bucket based on gap. Last space point Y=" << sortedPoints.back().back()->localPosition().y()
                   << " current space point Y =" << spY
                   << " gap=" << gap
                   << " window=" << bucketParams.spacePointWindow);
    }

    return gap > bucketParams.spacePointWindow;
}
void SpacePointMakerAlg::newBucket(const SpacePoint& refSpacePoint,
                                   SpacePointBucketVec& sortedPoints,
                                   const BucketParameters& bucketParams) const {
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
        if (refBound - overlapPos < bucketParams.spacePointOverlap) {    
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

   //Base the bucket parameters on the first space point in the chamber. This is a good approximation since all space points in the same chamber should have similar properties with the exception of the edge space points and overlapping chambers
   const BucketParameters& bucketParams = getBucketParameters(spacePoints.front());

   ATH_MSG_DEBUG("Distributing "<<spacePoints.size()<<" primary space points into buckets with parameters: "
                 << "maxBucketLength=" << bucketParams.maxBucketLength
                 << ", spacePointWindow=" << bucketParams.spacePointWindow
                 << ", spacePointOverlap=" << bucketParams.spacePointOverlap);
    
    for (SpacePoint& toSort : spacePoints) {        
        ATH_MSG_VERBOSE("Add new primary space point "<<toSort);

        if (splitBucket(toSort, firstPointPos, splittedHits, bucketParams)){
            newBucket(toSort, splittedHits, bucketParams);
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


std::string SpacePointMakerAlg::chamberConfigKey( const MuonGMR4::Chamber& chamber) const {
    return std::format("{:}_eta{:}_phi{:}", Muon::MuonStationIndex::chName(chamber.chamberIndex()), chamber.stationEta(), chamber.stationPhi());
}

const SpacePointMakerAlg::BucketParameters& SpacePointMakerAlg::getBucketParameters( const SpacePoint& spacePoint) const {
    const MuonGMR4::Chamber* chamber = spacePoint.chamber();

    ATH_MSG_DEBUG("Resolving bucket parameters for chamber "<<chamber->identString());

    if (const auto itr = m_bucketParameters.find(chamber);
        itr != m_bucketParameters.end()) {
        ATH_MSG_DEBUG("FOUNDPARAMS!" );
        return itr->second;
    }

    return m_defaultBucketParameters;
}

SpacePointMakerAlg::ResolvedParameter SpacePointMakerAlg::resolveParameter(
    const std::string_view chamberKey,
    const double defaultValue,
    const BucketPatternMap& patterns) const {

    // Exact match (e.g. BIL_eta-3_phi3) has the highest priority.
    if (const auto exactItr = patterns.find(chamberKey);
        exactItr != patterns.end()) {
        return {
            .value = exactItr->second,
            .matched = true
        };
    }

    double resolvedValue = defaultValue;
    std::size_t bestSpecificity = 0;
    bool matched = false;

    // Otherwise select the most-specific matching wildcard (e.g. BIL_eta-3* or BIL* or BIL_eta*_phi3).
    // If there are multiple patterns matching, the one with the most non-wildcard characters is selected.
    for (const auto& [pattern, value] : patterns) {
        if (pattern.find('*') == std::string::npos) {
            continue;
        }

        if (!wildcardMatch(pattern, chamberKey)) {
            continue;
        }

        // More non-wildcard characters means a more specific pattern.
        const std::size_t specificity =
            std::ranges::count_if(pattern, [](const char c) {
                return c != '*';
            });

        if (!matched || specificity > bestSpecificity) {
            bestSpecificity = specificity;
            resolvedValue = value;
            matched = true;
        }
    }

    return {
        .value = resolvedValue,
        .matched = matched
    };
}



std::optional<SpacePointMakerAlg::BucketParameters> SpacePointMakerAlg::resolveBucketParameters(
    const MuonGMR4::Chamber& chamber) const {

    const std::string chamberKey = chamberConfigKey(chamber);

    const ResolvedParameter maxLength = resolveParameter(
        chamberKey,
        m_maxBucketLength.value(),
        m_maxBucketLengthPatterns.value());

    const ResolvedParameter hitWindow = resolveParameter(
        chamberKey,
        m_spacePointWindow.value(),
        m_spacePointWindowPatterns.value());

    const ResolvedParameter overlap = resolveParameter(
        chamberKey,
        m_spacePointOverlap.value(),
        m_spacePointOverlapPatterns.value());

    // No chamber-dependent setting matched.
    if (!maxLength.matched &&
        !hitWindow.matched &&
        !overlap.matched) {
        return std::nullopt;
    }

    BucketParameters parameters{
        .spacePointWindow  = hitWindow.value,
        .maxBucketLength   = maxLength.value,
        .spacePointOverlap = overlap.value
    };


    ATH_MSG_DEBUG(
        "Resolved bucket override for " << chamberKey
        << ": maxBucketLength=" << parameters.maxBucketLength
        << (maxLength.matched ? " [override]" : " [default]")
        << ", spacePointWindow=" << parameters.spacePointWindow
        << (hitWindow.matched ? " [override]" : " [default]")
        << ", spacePointOverlap=" << parameters.spacePointOverlap
        << (overlap.matched ? " [override]" : " [default]"));

    return parameters;
}


}
