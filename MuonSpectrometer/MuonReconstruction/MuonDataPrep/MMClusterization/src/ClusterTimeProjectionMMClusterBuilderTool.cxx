/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "ClusterTimeProjectionMMClusterBuilderTool.h"

#include <algorithm>
#include <numeric>

#include "EventPrimitives/EventPrimitivesHelpers.h"
#include "GaudiKernel/PhysicalConstants.h"
#include "GaudiKernel/SystemOfUnits.h"

#define DEFINE_VECTOR(dType, varName, nEles)  \
    std::vector<dType> varName{};             \
    varName.reserve(nEles); 

namespace {
    constexpr double halfGapWidth = 2.52;
}
namespace Muon{
using RIO_Author = IMMClusterBuilderTool::RIO_Author;

ClusterTimeProjectionMMClusterBuilderTool::ClusterTimeProjectionMMClusterBuilderTool(const std::string& t, 
                                                                                     const std::string& n,
                                                                                     const IInterface* p) :
    AthAlgTool(t, n, p) {
    declareInterface<IMMClusterBuilderTool>(this);
}

StatusCode ClusterTimeProjectionMMClusterBuilderTool::initialize() {
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_uncertCalibKey.initialize());
    return StatusCode::SUCCESS;
}

StatusCode ClusterTimeProjectionMMClusterBuilderTool::getClusters(const EventContext& ctx,
                                                                  std::vector<MMPrepData>&& MMprds,
                                                                  std::vector<std::unique_ptr<MMPrepData>>& clustersVec) const  {
    LaySortedPrds prdsPerLayer = sortHitsToLayer(std::move(MMprds));


    for (std::vector<MMPrepData>& prdsOfLayer : prdsPerLayer) {
        if (prdsOfLayer.size() < 2) continue;        // require at least two strips per layer
        
        std::vector<std::vector<uint>> idxClusters = clusterLayer(prdsOfLayer);

        for (std::vector<uint>& clustIds : idxClusters) {
            DEFINE_VECTOR(MMPrepData, clustConstituents, clustIds.size());
            DEFINE_VECTOR(NSWCalib::CalibratedStrip, stripFeatures, clustIds.size());
            
            Amg::Vector3D clustDir{Amg::Vector3D::Zero()};
            for (unsigned idx : clustIds) {
                MMPrepData& toMerge{prdsOfLayer[idx]};
                
                const Amg::Vector3D lDir{NswClustering::toLocal(toMerge)};                
                const double q = toMerge.charge();

                clustDir += q * lDir;
                NSWCalib::CalibratedStrip clusFeat{};
                clusFeat.identifier = toMerge.identify();
                clusFeat.locPos = toMerge.localPosition();
                clusFeat.distDrift = toMerge.driftDist();
                clusFeat.resTransDistDrift = toMerge.localCovariance()(0, 0);
                clusFeat.resLongDistDrift = toMerge.localCovariance()(1, 1);
                clusFeat.charge = q;
                
                stripFeatures.emplace_back(std::move(clusFeat));
                clustConstituents.emplace_back(std::move(toMerge));
            }

            SG::ReadCondHandle<NswErrorCalibData> errorCalibDB{m_uncertCalibKey, ctx};
            if (!errorCalibDB.isValid()) {
                ATH_MSG_FATAL("Failed to retrieve the parameterized errors "<<m_uncertCalibKey.fullKey());
                return StatusCode::FAILURE;
            }

            constexpr uint8_t toolAuthor = static_cast<uint8_t>(MMPrepData::Author::ClusterTimeProjectionClusterBuilder);
            std::pair<double, double> posAndErrorSq = getClusterPositionPRD(**errorCalibDB, 
                                                                            toolAuthor, 
                                                                            stripFeatures, 
                                                                            clustDir.unit());

            ATH_CHECK(writeClusterPrd(ctx, clustConstituents, posAndErrorSq.first, posAndErrorSq.second, clustersVec));

        }
    }
    return StatusCode::SUCCESS;
}

ClusterTimeProjectionMMClusterBuilderTool::LaySortedPrds 
    ClusterTimeProjectionMMClusterBuilderTool::sortHitsToLayer(std::vector<MMPrepData>&& MMprds) const {
    
    LaySortedPrds prdsPerLayer{};
    const MmIdHelper& idHelper{m_idHelperSvc->mmIdHelper()};
    // sorting hits by gas gap
    for (MMPrepData& prd : MMprds) {
        const Identifier id = prd.identify();
        int layer = 4 * (idHelper.multilayer(id) - 1) + (idHelper.gasGap(id) - 1);
        prdsPerLayer.at(layer).push_back(std::move(prd));
    }
    ATH_MSG_DEBUG("sorted hist");
    // sort MMPrds by channel
    for (std::vector<MMPrepData>& prdsInLay : prdsPerLayer) {
        std::sort(prdsInLay.begin(), prdsInLay.end(), [&idHelper](const MMPrepData& a, const MMPrepData& b) { 
                                                        return idHelper.channel(a.identify()) < 
                                                               idHelper.channel(b.identify()); 
                                                        });
    }
    return prdsPerLayer;
}

 std::vector<std::vector<uint>> 
        ClusterTimeProjectionMMClusterBuilderTool::clusterLayer(const std::vector<MMPrepData>& MMPrdsPerLayer) const {
    
    const MmIdHelper& idHelper{m_idHelperSvc->mmIdHelper()};
    // get gas gap for later correction of the sign of the slope
    int gasGap = idHelper.gasGap(MMPrdsPerLayer.at(0).identify());

    ATH_MSG_DEBUG("Scanning gas gap " << gasGap);
    if (msgLvl(MSG::VERBOSE)) {
        for (const auto& prd : MMPrdsPerLayer) {
            ATH_MSG_DEBUG("Hit channel "  << "  " << m_idHelperSvc->toStringDetEl(prd.identify()) 
                       << " local positionX " << prd.localPosition().x() << " time " << prd.time()
                       << " corrected time " << prd.time() << " angle to IP "<< prd.globalPosition().theta() << " "
                       << prd.globalPosition().theta() / Gaudi::Units::degree);
        }
    }
    std::vector<std::vector<uint>> idxClusters{};
    // simple grouping of strips using the fact that the strips where ordered by channel
    idxClusters.push_back(std::vector<uint>{0});  // assumes that mmPrdsPerLayer always has at least one strip
    for (uint i_strip = 1; i_strip < MMPrdsPerLayer.size(); i_strip++) {
        if (channel(MMPrdsPerLayer.at(i_strip)) - channel(MMPrdsPerLayer.at(i_strip - 1)) <= m_maxHoleSize + 1) {
            idxClusters.back().push_back(i_strip);
        } else {
            idxClusters.push_back(std::vector<uint>{i_strip});
        }
    }
    if (msgLvl(MSG::VERBOSE)) {
        ATH_MSG_DEBUG("Found " << idxClusters.size() << " clusters");
        for (const auto& idxCluster : idxClusters) { ATH_MSG_DEBUG("cluster: " << idxCluster); }
    }
    return idxClusters;
}  // end of cluster layer

std::pair<double, double>
    ClusterTimeProjectionMMClusterBuilderTool::getClusterPositionPRD(const NswErrorCalibData& errorCalibDB,
                                                                     uint8_t author, 
                                                                     const std::vector<NSWCalib::CalibratedStrip>& constituents,
                                                                     const Amg::Vector3D& dirEstimate) const {
                                                                      
    
    double clusterPosition{0}, clusterPositionErrorSq{0.};
    
    double qtot{0}, meanDriftDist{0.}, meanDriftDistError{0.};
    double meanPosX{0.}, meanPosXError{0.};
    
    for (const NSWCalib::CalibratedStrip& clustFeat : constituents) {
        const double driftDist = clustFeat.distDrift;
        ///  divide by 1000 to avoid overflow of variables
        const double charge = clustFeat.charge * Gaudi::Units::perThousand;
        const double chargeSq = std::pow(charge, 2); 
        qtot += charge;
        meanPosX += clustFeat.locPos.x() * charge;
        meanPosXError += clustFeat.resLongDistDrift * chargeSq;
        meanDriftDist += driftDist * charge;
        meanDriftDistError += clustFeat.resTransDistDrift * chargeSq;
        ATH_MSG_VERBOSE("Strip:"<<m_idHelperSvc->toString(clustFeat.identifier)
                        << " drift dist " << driftDist << " +- " << std::sqrt(clustFeat.resTransDistDrift) 
                        << " xpos: "<< clustFeat.locPos.x() << " +- " << std::sqrt(clustFeat.resLongDistDrift) 
                        << " xMeanPos " << meanPosX / qtot << " +- " << std::sqrt(meanPosXError) / qtot 
                        << " meanPosXError " << meanPosXError << " meanDriftDist " << meanDriftDist / qtot 
                        << " meanDriftDist Error " << std::sqrt(meanDriftDistError) / qtot
                        << " charge " << charge << " qtot " << qtot);
    }
    meanPosX /= qtot;
    double meanPosXErrorSq = meanPosXError / (qtot * qtot);
    meanDriftDist /= qtot;
    double meanDriftDistErrorSq = meanDriftDistError / (qtot * qtot);
    
    const double tanTheta = std::tan(dirEstimate.theta());
    double correction = tanTheta * (meanDriftDist - halfGapWidth);
    if (m_idHelperSvc->mmIdHelper().gasGap(constituents[0].identifier) % 2 == 0) {
        correction = -1. * correction;  // take care of inverted drif direction for even gaps
    }
    double correctionErrorSq = tanTheta * tanTheta * meanDriftDistErrorSq;

    clusterPosition = meanPosX + correction;
    clusterPositionErrorSq = correctionErrorSq + meanPosXErrorSq;

    
    NswErrorCalibData::Input errorCalibIn{};
    errorCalibIn.stripId = constituents[0].identifier;
    errorCalibIn.clusterAuthor = author;
    errorCalibIn.locPhi = dirEstimate.phi();
    errorCalibIn.locTheta = dirEstimate.theta();
    errorCalibIn.localPos = Amg::Vector2D{clusterPosition, 0.};
    errorCalibIn.clusterSize = constituents.size();
     
    const double localUncertainty = errorCalibDB.clusterUncertainty(errorCalibIn);

    ATH_MSG_DEBUG("Cluster Properties"
                  << " meanX " << meanPosX << " +-" << std::sqrt(meanPosXErrorSq) << "  mean drift dist " << meanDriftDist << " +- "
                  << std::sqrt(meanDriftDistErrorSq) << " correction " << correction << " +- " << std::sqrt(correctionErrorSq)
                  << " final position " << clusterPosition << " +- " << std::sqrt(clusterPositionErrorSq));
    return std::make_pair(clusterPosition, localUncertainty*localUncertainty);
}

StatusCode ClusterTimeProjectionMMClusterBuilderTool::writeClusterPrd(const EventContext& ,
                                                                      const std::vector<Muon::MMPrepData>& constituents, 
                                                                      const double clusterPosition, 
                                                                      const double clusterPositionErrorSq,
                                                                      std::vector<std::unique_ptr<Muon::MMPrepData>>& mergedClust) const { 
    
    DEFINE_VECTOR(Identifier, rdoList, constituents.size());
    DEFINE_VECTOR(int, stripCharges, constituents.size());
    DEFINE_VECTOR(short int, stripTimes, constituents.size());
    DEFINE_VECTOR(uint16_t, stripNumbers, constituents.size());
    DEFINE_VECTOR(float, stripDriftDists, constituents.size());
    DEFINE_VECTOR(AmgVector(2), stripDriftDistErrors, constituents.size());

    int totalCharge{0};
    for (const Muon::MMPrepData& clustFeat : constituents) {
        rdoList.push_back(clustFeat.identify());
        stripNumbers.push_back(channel(clustFeat));
        stripTimes.push_back(clustFeat.time());
        stripCharges.push_back(clustFeat.charge());
        stripDriftDists.push_back(clustFeat.driftDist());
        const Amg::MatrixX cov{clustFeat.localCovariance()};
        stripDriftDistErrors.emplace_back(cov(0,0), cov(1,1));
        totalCharge += clustFeat.charge();
    }
    if (!m_writeStripProperties) { stripNumbers.clear(); }
    
    auto covN = Amg::MatrixX(1, 1);
    covN.coeffRef(0, 0) = clusterPositionErrorSq;
    Amg::Vector2D localClusterPositionV(clusterPosition, constituents[0].localPosition().y());
    Identifier idStrip0 = constituents[0].identify();

    std::unique_ptr<MMPrepData> prdN = std::make_unique<MMPrepData>(idStrip0, 
                                                                    constituents[0].collectionHash(), 
                                                                    std::move(localClusterPositionV), 
                                                                    std::move(rdoList), 
                                                                    std::move(covN),
                                                                    constituents[0].detectorElement(),
                                                                    0,  // drift dist
                                                                    totalCharge, 
                                                                    0.0 /*drift dist*/, 
                                                                    std::move(stripNumbers), 
                                                                    std::move(stripTimes), 
                                                                    std::move(stripCharges));

    prdN->setDriftDist(std::move(stripDriftDists), std::move(stripDriftDistErrors));
    prdN->setAuthor(MMPrepData::Author::ClusterTimeProjectionClusterBuilder);

    mergedClust.push_back(std::move(prdN));
    return StatusCode::SUCCESS;
}

RIO_Author ClusterTimeProjectionMMClusterBuilderTool::getCalibratedClusterPosition(const EventContext& ctx, 
                                                                                   const std::vector<NSWCalib::CalibratedStrip>& calibratedStrips,
                                                                                   const Amg::Vector3D& directionEstimate, 
                                                                                   Amg::Vector2D& clusterLocalPosition,
                                                                                   Amg::MatrixX& covMatrix) const{

    SG::ReadCondHandle<NswErrorCalibData> errorCalibDB{m_uncertCalibKey, ctx};
    if (!errorCalibDB.isValid()) {
        ATH_MSG_FATAL("Failed to retrieve the parameterized errors "<<m_uncertCalibKey.fullKey());
        return RIO_Author::unKnownAuthor;
    }

    constexpr uint8_t toolAuthor = static_cast<uint8_t>(RIO_Author::ClusterTimeProjectionClusterBuilder);
    std::pair<double, double> posAndErrorSq = getClusterPositionPRD(*errorCalibDB.cptr(), toolAuthor, calibratedStrips, directionEstimate);

    clusterLocalPosition[Trk::locX] = posAndErrorSq.first;
    Amg::MatrixX covN(1, 1);
    covN.coeffRef(0, 0) = posAndErrorSq.second;
    covMatrix = covN;

    return RIO_Author::ClusterTimeProjectionClusterBuilder;
}
}
#undef DEFINE_VECTOR
