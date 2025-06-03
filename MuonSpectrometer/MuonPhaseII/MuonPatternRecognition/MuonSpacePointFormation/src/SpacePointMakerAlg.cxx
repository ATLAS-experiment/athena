/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
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
namespace {
    inline std::vector<std::shared_ptr<unsigned>> matchCountVec(unsigned int n) {
        std::vector<std::shared_ptr<unsigned>> out{};
        out.reserve(n);
        for (unsigned int p = 0; p < n ;++p) {
            out.emplace_back(std::make_shared<unsigned>(0));
        }
        return out;
    }
}

namespace MuonR4 {

template<class MeasType>
Amg::Transform3D SpacePointMakerAlg::toChamberTransform(const ActsGeometryContext& gctx, 
                                                        const MeasType* meas) const{
    IdentifierHash hash = {};
    if constexpr(std::is_same_v<MeasType, xAOD::MdtDriftCircle>) {
        hash = meas->measurementHash();
    } else {
        hash = meas->layerHash();
    }
    const MuonGMR4::MuonReadoutElement* reEle{meas->readoutElement()};
    return reEle->msSector()->globalToLocalTrans(gctx) * reEle->localToGlobalTrans(gctx, hash);
}

template<class MeasType>
Amg::Vector3D SpacePointMakerAlg::positionInChamber(const MeasType* meas,
                                                    const Amg::Transform3D& toChamberTrans) const{
    if constexpr (std::is_same_v<MeasType, xAOD::MdtDriftCircle>){
        return toChamberTrans * meas->localCirclePosition();
    }
    else if constexpr (std::is_same_v<MeasType, xAOD::RpcMeasurement>){
        return toChamberTrans * meas->localMeasurementPos();
    }
    else if constexpr (std::is_same_v<MeasType, xAOD::TgcStrip> or std::is_same_v<MeasType, xAOD::MMCluster>){
        return toChamberTrans * (meas->template localPosition<1>()[Trk::locX] * Amg::Vector3D::UnitX());
    }
    else if constexpr (std::is_same_v<MeasType, xAOD::sTgcMeasurement>){
        if (meas->channelType() == sTgcIdHelper::sTgcChannelTypes::Strip ||
            meas->channelType() == sTgcIdHelper::sTgcChannelTypes::Wire) {
                return toChamberTrans * (meas->template localPosition<1>()[Trk::locX] * Amg::Vector3D::UnitX());
        }
        Amg::Vector3D locPos{Amg::Vector3D::Zero()};
        locPos.block<2,1>(0,0) = xAOD::toEigen(meas->template localPosition<2>());
        return toChamberTrans * locPos;
    }
    else static_assert(std::false_type::value, "Unsupported measurement type.");
    return Amg::Vector3D::Zero();
}

template<class MeasType> 
Amg::Vector3D SpacePointMakerAlg::channelDirInChamber(const MeasType* meas,
                                    const Amg::Transform3D& toChamberTrans) const{       
    if constexpr (std::is_same_v<MeasType, xAOD::MdtDriftCircle>){
        return toChamberTrans.linear() * Amg::Vector3D::UnitZ();
    }
    else if constexpr (std::is_same_v<MeasType, xAOD::RpcMeasurement> or 
                       std::is_same_v<MeasType, xAOD::MMCluster> or 
                       std::is_same_v<MeasType, xAOD::sTgcMeasurement>){
        return toChamberTrans.linear() * Amg::Vector3D::UnitY();
    }
    else if constexpr (std::is_same_v<MeasType, xAOD::TgcStrip>){           
        Amg::Vector3D dir{Amg::Vector3D::UnitY()};
        if (meas->measuresPhi()) {
            dir.block<2,1>(0,0) = meas->readoutElement()->stripLayout(meas->gasGap()).stripDir(meas->channelNumber());
        } 
        return toChamberTrans.linear() * dir;
    }
    else static_assert(std::false_type::value, "Unsupported measurement type.");
    return Amg::Vector3D::Zero();      
}
        
template<class MeasType> 
Amg::Vector3D SpacePointMakerAlg::channelNormalInChamber(const MeasType* meas,
                                                         const Amg::Transform3D& toChamberTrans) const{
    if constexpr (std::is_same_v<MeasType, xAOD::MdtDriftCircle>){
        return toChamberTrans.linear() * Amg::Vector3D::UnitY();
    }
    else if constexpr (std::is_same_v<MeasType, xAOD::RpcMeasurement> or 
                       std::is_same_v<MeasType, xAOD::MMCluster> or 
                       std::is_same_v<MeasType, xAOD::sTgcMeasurement>){
        return toChamberTrans.linear() * Amg::Vector3D::UnitX();
    }
    else if constexpr (std::is_same_v<MeasType, xAOD::TgcStrip>){                     
        Amg::Vector3D dir{Amg::Vector3D::UnitX()};
        if (meas->measuresPhi()) {
            dir.block<2,1>(0,0) = meas->readoutElement()->stripLayout(meas->gasGap()).stripNormal(meas->channelNumber());
        } 
        return toChamberTrans.linear() * dir;
    }
    else static_assert(std::false_type::value, "Unsupported measurement type.");
    return Amg::Vector3D::Zero();     
}

template<class MeasType>
AmgSymMatrix(2) SpacePointMakerAlg::computeCov(const MeasType* primaryMeas,
                                               const Amg::Vector3D& dir,
                                               const Amg::Vector3D& nor) const {
    AmgSymMatrix(2) Jac{AmgSymMatrix(2)::Identity()}, uvcov {AmgSymMatrix(2)::Identity()};
        
    if (primaryMeas->numDimensions() == 1) {
        uvcov(0,0) = primaryMeas->template localCovariance<1>()[0];

        if constexpr (std::is_same_v<MeasType, xAOD::MdtDriftCircle>) {
            uvcov(1,1) = 0.5* primaryMeas->readoutElement()->activeTubeLength(primaryMeas->measurementHash());
        }
        else if constexpr (std::is_same_v<MeasType, xAOD::RpcMeasurement>) {
            if (primaryMeas->numDimensions() == 1) {
                const xAOD::RpcStrip* strip = static_cast<const xAOD::RpcStrip*>(primaryMeas);
                uvcov(1,1) = strip->measuresPhi() ? 0.5* strip->readoutElement()->stripPhiLength(): 0.5* strip->readoutElement()->stripEtaLength();
            }
        }
        else if constexpr (std::is_same_v<MeasType, xAOD::TgcStrip>){
            if (primaryMeas->measuresPhi()) {
                uvcov(1,1) = 0.5 * primaryMeas->readoutElement()->stripLayout(primaryMeas->gasGap()).stripLength(primaryMeas->channelNumber());
            } else {
                uvcov(1,1) = 0.5 * primaryMeas->readoutElement()->wireGangLayout(primaryMeas->gasGap()).stripLength(primaryMeas->channelNumber());
            }
        }
        else if constexpr (std::is_same_v<MeasType, xAOD::MMCluster>){
            uvcov(1,1) = 0.5 * primaryMeas->readoutElement()->stripLayer(primaryMeas->measurementHash()).design().stripLength(primaryMeas->channelNumber());
        }
        else if constexpr (std::is_same_v<MeasType, xAOD::sTgcMeasurement>){
            switch (primaryMeas->channelType()) {
                case sTgcIdHelper::sTgcChannelTypes::Strip:
                    uvcov(1,1) = 0.5 *  primaryMeas->readoutElement()->stripDesign(primaryMeas->measurementHash()).stripLength(primaryMeas->channelNumber());
                    break;
                case sTgcIdHelper::sTgcChannelTypes::Wire:
                    uvcov(1,1) = 0.5 *  primaryMeas->readoutElement()->wireDesign(primaryMeas->measurementHash()).stripLength(primaryMeas->channelNumber());
                    break;
                /// Do nothing for the pads, the covariance is calculated in the 2D measurement case
                case sTgcIdHelper::sTgcChannelTypes::Pad:
                    break;
            }
        }    
        else static_assert(std::false_type::value, "Unsupported measurement type.");

        uvcov(1,1) = std::pow(uvcov(1,1), 2);
    }
    else if (primaryMeas->numDimensions() == 2){
        /// In case of 2D measurements like sTgc-pads or BI-RPC strips we can directly take the covariance
        /// from the measurement itself. To indicate that the space point measures both, eta & phi coordinate
        /// set the secondary measurement to be the primary one
        uvcov = xAOD::toEigen(primaryMeas->template localCovariance<2>());
    }
    else THROW_EXCEPTION("Unexpected numDimension for PRD: " << m_idHelperSvc->toString(primaryMeas->identify()));

    Jac.col(0)  = nor.block<2,1>(0,0).unit();
    Jac.col(1) = dir.block<2,1>(0,0).unit();
    
    return Jac * uvcov * Jac.inverse();
}

AmgSymMatrix(2) SpacePointMakerAlg::computeCov(const xAOD::UncalibratedMeasurement* primaryMeas,
                                               const xAOD::UncalibratedMeasurement* secondaryMeas,
                                               const Amg::Vector3D& nor1,
                                               const Amg::Vector3D& nor2) const {
    AmgSymMatrix(2) Jac{AmgSymMatrix(2)::Identity()}, uvcov {AmgSymMatrix(2)::Identity()};

    if (primaryMeas->numDimensions() != 1) THROW_EXCEPTION("Unexpected numDimension");

    uvcov(0,0) = primaryMeas->localCovariance<1>()[0];
    uvcov(1,1) = secondaryMeas->localCovariance<1>()[0]; 

    Jac.col(0)  = nor1.block<2,1>(0,0).unit();
    Jac.col(1)  = nor2.block<2,1>(0,0).unit();

    return Jac * uvcov * Jac.inverse();
}

template<class MeasType>
void SpacePointMakerAlg::fillSpacePoint (std::vector<SpacePoint>& pointColl,
                                        const MeasType* primaryMeas,
                                        const Amg::Transform3D& toChamberTrans) const {
    pointColl.emplace_back(primaryMeas);
    SpacePoint& sp {pointColl.back()};

    Amg::Vector3D pos {positionInChamber(primaryMeas, toChamberTrans)};
    Amg::Vector3D dir {channelDirInChamber(primaryMeas, toChamberTrans)};
    Amg::Vector3D nor {channelNormalInChamber(primaryMeas, toChamberTrans)};
    AmgSymMatrix(2) cov {computeCov(primaryMeas,dir,nor)};

    sp.setDirection(dir);
    sp.setPosition(pos);
    sp.setNormal(nor);
    sp.setCovariance(cov);
}

template<class MeasType>
void SpacePointMakerAlg::fillSpacePoint (std::vector<SpacePoint>& pointColl,
                                        const MeasType* primaryMeas,
                                        const MeasType* secondaryMeas,
                                        const Amg::Transform3D& toChamberTrans_eta, 
                                        const Amg::Transform3D& toChamberTrans_phi) const{
    pointColl.emplace_back(primaryMeas, secondaryMeas);
    SpacePoint& sp {pointColl.back()};

    Amg::Vector3D dir1 {channelDirInChamber(primaryMeas, toChamberTrans_eta)};
    Amg::Vector3D nor1 {channelNormalInChamber(primaryMeas, toChamberTrans_eta)};
    Amg::Vector3D pos1 {positionInChamber(primaryMeas, toChamberTrans_eta)};

    Amg::Vector3D dir2 {channelDirInChamber(secondaryMeas, toChamberTrans_phi)};
    Amg::Vector3D nor2 {channelNormalInChamber(secondaryMeas, toChamberTrans_phi)};
    Amg::Vector3D pos2 {positionInChamber(secondaryMeas, toChamberTrans_phi)};
    
    AmgSymMatrix(2) cov {computeCov(primaryMeas,secondaryMeas,nor1,nor2)};

    sp.setDirection(dir1);
    sp.setPosition(pos1 + Amg::intersect<3>(pos2,dir2, pos1, dir1).value_or(0) * dir1);
    sp.setNormal(nor1);
    sp.setCovariance(cov);
}

bool SpacePointMakerAlg::SpacePointStatistics::FieldKey::operator<(const FieldKey& other) const{
    if (techIdx != other.techIdx) {
        return static_cast<int>(techIdx) < static_cast<int>(other.techIdx);
    }
    if (stIdx != other.stIdx) {
        return static_cast<int>(stIdx) < static_cast<int>(other.stIdx);
    }
    return eta < other.eta;
}
unsigned int SpacePointMakerAlg::SpacePointStatistics::StatField::allHits() const {
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
    if (m_doStat) m_statCounter = std::make_unique<SpacePointStatistics>(m_idHelperSvc.get());
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
      ATH_MSG_DEBUG("Fill space points for chamber "<<m_idHelperSvc->toStringDetEl(viewer.at(0)->identify()));
      if constexpr( std::is_same_v<ContType, xAOD::MdtDriftCircleContainer> ||
                    std::is_same_v<ContType, xAOD::MMClusterContainer>) {
 
            pointsInChamb.etaHits.reserve(pointsInChamb.etaHits.capacity() + viewer.size());
            for (const PrdType prd: viewer) {
                ATH_MSG_VERBOSE("Create space point from "<<m_idHelperSvc->toString(prd->identify())
                              <<", hash: "<<prd->identifierHash());

                Amg::Transform3D toChamberTrans{ toChamberTransform(*gctx, prd)};
                fillSpacePoint(pointsInChamb.etaHits, prd, toChamberTrans);
            }
       } else {
            /// Loop over the chamber hits to split the hits per gasGap
            using EtaPhiHits = std::array<PrdVec, 2>;
            std::vector<EtaPhiHits> hitsPerGasGap{};
            for (const PrdType prd : viewer) {
                ATH_MSG_VERBOSE("Create space point from "<<m_idHelperSvc->toString(prd->identify())<<", hash: "<<prd->identifierHash());
                
                unsigned int gapIdx = prd->gasGap() -1;
                if constexpr (std::is_same_v<ContType, xAOD::RpcMeasurementContainer>) {
                    gapIdx = prd->readoutElement()->createHash(0, prd->gasGap(), prd->doubletPhi(), false);
                }

                bool measPhi{false};
                if constexpr(std::is_same_v<ContType, xAOD::sTgcMeasContainer>) {
                    /// Make directly to a space point
                    if (prd->channelType() == sTgcIdHelper::sTgcChannelTypes::Pad) {
                        Amg::Transform3D toChamberTrans{ toChamberTransform(*gctx, prd)};
                        fillSpacePoint(pointsInChamb.etaHits, prd, toChamberTrans);
                        continue;
                    }
                    /// Wires measure the phi coordinate
                    measPhi = prd->channelType() == sTgcIdHelper::sTgcChannelTypes::Wire;
                } else {
                    /// Tgc & Rpcs have the measuresPhi property
                    measPhi = prd->measuresPhi();
                }

                if (hitsPerGasGap.size() <= gapIdx) {
                    hitsPerGasGap.resize(gapIdx + 1);
                }
                /// Sort in the hit
                PrdVec& toPush = hitsPerGasGap[gapIdx][measPhi];
                if (toPush.capacity() == toPush.size()) {
                    toPush.reserve(toPush.size() + m_capacityBucket);
                }
                toPush.push_back(prd);
            }
            /// Create the space points
            for (auto& [etaHits, phiHits] : hitsPerGasGap) {

                Amg::Transform3D toChamberTrans_eta {}, toChamberTrans_phi {};
                if (!etaHits.empty()){
                    toChamberTrans_eta = toChamberTransform(*gctx, etaHits.at(0));
                }
                if (!phiHits.empty()){
                    toChamberTrans_phi = toChamberTransform(*gctx, phiHits.at(0));
                }

                if (!passOccupancy2D(etaHits, phiHits)) {
                    ATH_MSG_VERBOSE("Occupancy cut not passed "<<etaHits.size()<<", "<<phiHits.size());
                    pointsInChamb.etaHits.reserve(pointsInChamb.etaHits.size() + etaHits.size());
                    pointsInChamb.phiHits.reserve(pointsInChamb.phiHits.size() + phiHits.size());
                    for (const PrdType etaPrd : etaHits) {

                        fillSpacePoint(pointsInChamb.etaHits, etaPrd, toChamberTrans_eta);
                        ATH_MSG_VERBOSE("Add new eta hit "<<m_idHelperSvc->toString(pointsInChamb.etaHits.back().identify())
                                <<" "<<Amg::toString(pointsInChamb.etaHits.back().positionInChamber()));

                    }
                    for (const PrdType phiPrd : phiHits) {    
                        fillSpacePoint(pointsInChamb.phiHits, phiPrd, toChamberTrans_phi);
                        ATH_MSG_VERBOSE("Add new phi hit "<<m_idHelperSvc->toString(pointsInChamb.phiHits.back().identify())
                                <<" "<<Amg::toString(pointsInChamb.phiHits.back().positionInChamber()));
                    }
                    continue;
                }
                std::vector<std::shared_ptr<unsigned>> etaCounts{matchCountVec(etaHits.size())}, 
                                                       phiCounts{matchCountVec(phiHits.size())};
                pointsInChamb.etaHits.reserve(etaHits.size()*phiHits.size());
                /// Simple combination by taking the cross-product
                for (unsigned int etaP = 0; etaP < etaHits.size(); ++etaP) {
                    /// There's no valid combination with another phi hit
                    for (unsigned int phiP = 0; phiP < phiHits.size(); ++ phiP) {
                        /** Tgc measurements with different bunch crossing tags cannot be combined */
                        if constexpr(std::is_same<xAOD::TgcStripContainer, ContType>::value) {
                            if (!(etaHits[etaP]->bcBitMap() & phiHits[phiP]->bcBitMap())){
                                continue;
                            }
                        }
                        fillSpacePoint(pointsInChamb.etaHits, etaHits[etaP], phiHits[phiP], toChamberTrans_eta, toChamberTrans_phi);

                        SpacePoint& sp {pointsInChamb.etaHits.back()};
                        ATH_MSG_VERBOSE("Create new spacepoint from "<<m_idHelperSvc->toString(etaHits[etaP]->identify())
                        <<" & "<<m_idHelperSvc->toString(phiHits[phiP]->identify())<<" at "<<Amg::toString(sp.positionInChamber()));
                        sp.setInstanceCounts(etaCounts[etaP], phiCounts[phiP]);
                    }
                    if (!(*etaCounts[etaP])) {
                        fillSpacePoint(pointsInChamb.etaHits, etaHits[etaP], toChamberTrans_eta);
                        continue;
                    }
                }
                /// If there's a phi measuremnt which cannot be combined with the others 
                /// or no eta measurement is suitable, then manually push_back the phi hits
                for (unsigned int phiP = 0; phiP < phiHits.size(); ++ phiP){
                    if (!(*phiCounts[phiP])) {
                        fillSpacePoint(pointsInChamb.phiHits, phiHits[phiP], toChamberTrans_phi);
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
        ATH_MSG_DEBUG("Fill space points for chamber "<<chamber->identString());
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
        if (msgLvl(MSG::VERBOSE)){
            std::stringstream spStr{};
            for (const std::shared_ptr<SpacePoint>& sp : bucket){
                spStr<<"SpacePoint: PrimaryMeas: " << m_idHelperSvc->toString(sp->identify()) << " SecondaryMeas: " 
                 <<(sp->secondaryMeasurement() ? m_idHelperSvc->toString(xAOD::identify(sp->secondaryMeasurement())) : " None ") 
                 << " Pos: " <<  Amg::toString(sp->positionInChamber())<<std::endl;
            }
            ATH_MSG_VERBOSE("Created a bucket, printing all spacepoints..."<<std::endl<<spStr.str());
        }

        std::ranges::sort(bucket, MuonR4::SpacePointPerLayerSorter{m_idHelperSvc.get()});
        bucket.populateChamberLocations();
        finalContainer.push_back(std::make_unique<SpacePointBucket>(std::move(bucket)));
    }

}
void SpacePointMakerAlg::distributePhiPoints(std::vector<SpacePoint>&& spacePoints,
                                             SpacePointBucketVec& splittedContainer) const{
    for (SpacePoint& sp : spacePoints) {
        auto phiPoint = std::make_shared<SpacePoint>(std::move(sp));
        const double minY = phiPoint->positionInChamber().y() - phiPoint->uncertainty().y();
        const double maxY = phiPoint->positionInChamber().y() + phiPoint->uncertainty().y();
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
    const double spY = spacePoint.positionInChamber().y();
    if (spY - firstSpPos > m_maxBucketLength){
        return true;
    }
    
    if (sortedPoints.empty() || sortedPoints.back().empty()) {
        return false;
    }
    return spY - sortedPoints.back().back()->positionInChamber().y() > m_spacePointWindow;
}
void SpacePointMakerAlg::newBucket(const SpacePoint& refSpacePoint,
                                   SpacePointBucketVec& sortedPoints) const {
    SpacePointBucket& newContainer = sortedPoints.emplace_back();
    newContainer.setBucketId(sortedPoints.size() -1);

    ///Set the boundaries from the previous bucket
    SpacePointBucket& overlap{sortedPoints[sortedPoints.size() - 2]};
    overlap.setCoveredRange(overlap.front()->positionInChamber().y(), 
                            overlap.back()->positionInChamber().y());
    
    const double refBound = refSpacePoint.positionInChamber().y();
                                
    /** Copy space points that could be within the overlap region to the next bucket */
    for (const std::shared_ptr<SpacePoint>& pointInBucket : overlap | std::views::reverse) {
        const double overlapPos = pointInBucket->positionInChamber().y() + pointInBucket->uncertainty()[Amg::y];
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
                        return a.positionInChamber().y() < b.positionInChamber().y();
                      });

    double firstPointPos = spacePoints.front().positionInChamber().y();
    
    for (SpacePoint& toSort : spacePoints) {        
        ATH_MSG_VERBOSE("Add new primary space point "<<m_idHelperSvc->toString(toSort.identify())<<", "
                     <<" @ "<<Amg::toString(toSort.positionInChamber()));

        if (splitBucket(toSort, firstPointPos, splittedHits)){
            newBucket(toSort, splittedHits);
            firstPointPos = splittedHits.back().empty() ? toSort.positionInChamber().y() : splittedHits.back().front()->positionInChamber().y();
            ATH_MSG_VERBOSE("New bucket: id " << splittedHits.back().bucketId() << " Coverage: " << firstPointPos);
        }
        std::shared_ptr<SpacePoint> spacePoint = std::make_shared<SpacePoint>(std::move(toSort));
        splittedHits.back().emplace_back(spacePoint);
    }
    SpacePointBucket& lastBucket{splittedHits.back()};
    lastBucket.setCoveredRange(lastBucket.front()->positionInChamber().y(), 
                               lastBucket.back()->positionInChamber().y());
}

}