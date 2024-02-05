/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "SimpleMMClusterBuilderTool.h"
#include "StoreGate/ReadCondHandle.h"
#include "GeoPrimitives/GeoPrimitivesToStringConverter.h"
#include "GeoPrimitives/GeoPrimitivesHelpers.h"

#define DEFINE_VECTOR(dType, varName, nEles)  \
    std::vector<dType> varName{};             \
    varName.reserve(nEles);                  

namespace Muon{

using RIO_Author = IMMClusterBuilderTool::RIO_Author;
SimpleMMClusterBuilderTool::SimpleMMClusterBuilderTool(const std::string& t, const std::string& n, const IInterface* p) :
    AthAlgTool(t, n, p) {
    declareInterface<IMMClusterBuilderTool>(this);
}

StatusCode SimpleMMClusterBuilderTool::initialize() {
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_uncertCalibKey.initialize());
    return StatusCode::SUCCESS;
}

StatusCode SimpleMMClusterBuilderTool::getClusters(const EventContext& ctx,
                                                   std::vector<MMPrepData>&& MMprds,
                                                   std::vector<std::unique_ptr<MMPrepData>>& clustersVect) const {
    ATH_MSG_DEBUG("Size of the input vector: " << MMprds.size());
    ATH_MSG_DEBUG("Size of the output vector: " << clustersVect.size());
  
    if (MMprds.empty()) {
        ATH_MSG_DEBUG("Empty PRD collection: no clusterization");
        return StatusCode::SUCCESS;
    }
    const MmIdHelper& idHelper{m_idHelperSvc->mmIdHelper()};
    std::sort(MMprds.begin(), MMprds.end(), 
      [&](const MMPrepData& a, const MMPrepData& b){
        const Identifier ida = a.identify(), idb = b.identify();
        const int mla = idHelper.multilayer(ida);
        const int mlb = idHelper.multilayer(idb);
        if (mla!=mlb) return mla<mlb;
        const int gga = idHelper.gasGap(ida);
        const int ggb = idHelper.gasGap(idb);
        if (gga!=ggb) return gga<ggb;
        return idHelper.channel(ida) < idHelper.channel(idb);        
      });

    
    const IdentifierHash hash = MMprds.at(0).collectionHash(); 
    const unsigned int n_input_prds =  MMprds.size();
    for (unsigned int mergeI = 0; mergeI < n_input_prds; ) {       
        const MMPrepData& primary_prd = MMprds[mergeI];
        const Identifier id_prd = primary_prd.identify();
        const Identifier gasGapId = m_idHelperSvc->gasGapId(id_prd);
        
        ATH_MSG_VERBOSE("  PrepData "<<m_idHelperSvc->toString(id_prd)<<" index: "<<mergeI<< " / " << n_input_prds 
                                     << " z " << primary_prd.globalPosition().z());
        /// Find a strip in the same gas gap that's close around our seed
        unsigned mergeJ{mergeI+1};        
        
        while (mergeJ <  n_input_prds &&               
               gasGapId == m_idHelperSvc->gasGapId(MMprds[mergeJ].identify()) &&
               std::abs(idHelper.channel(MMprds[mergeJ].identify()) - 
                        idHelper.channel(MMprds[mergeJ -1].identify()) ) <= static_cast<int>(m_maxHoleSize) + 1) {
            ++mergeJ;
        }
        unsigned int clustSize = mergeJ - mergeI;
        if (clustSize > m_maxClusSize) {
            mergeI = mergeJ;
            continue;
        }
        DEFINE_VECTOR(uint16_t, mergeStrips, clustSize);
        DEFINE_VECTOR(Identifier, rdoList, clustSize);
        DEFINE_VECTOR(short int, mergeStripsTime, clustSize);
        DEFINE_VECTOR(int, mergeStripsCharge, clustSize);
        DEFINE_VECTOR(float, mergeStripsDriftDists, clustSize);
        DEFINE_VECTOR(AmgVector(2), mergeStripsDriftDistErrors, clustSize);
        double totalCharge{0.0};
        /// Merge all information together
        for (unsigned int mergeMe = mergeI; mergeMe < mergeJ; ++mergeMe) {
            const MMPrepData& mergePrd = MMprds[mergeMe];
            const Identifier mergedId = mergePrd.identify();
            rdoList.push_back(mergedId);
            mergeStrips.push_back(idHelper.channel(mergedId));
            mergeStripsTime.push_back(mergePrd.time());
            mergeStripsCharge.push_back(mergePrd.charge());
            mergeStripsDriftDists.push_back(mergePrd.driftDist());
            const Amg::MatrixX& cov{mergePrd.localCovariance()};
            mergeStripsDriftDistErrors.emplace_back(cov(0,0), cov(1,1));

            totalCharge += mergePrd.charge();
        }
        
        ATH_MSG_VERBOSE(" add merged MMprds nmerge " << clustSize );

       

        // start off from strip in the middle
        unsigned int stripSum = 0;
        for (unsigned short strip : mergeStrips) stripSum += strip;
        stripSum /= mergeStrips.size();        
      
        unsigned int centralIdx{mergeI};
        for (unsigned int k = 0; k < mergeStrips.size(); ++k) {
            ++centralIdx;
            if (mergeStrips[k] == stripSum) break;           
        }
        const Identifier clusterId = MMprds[centralIdx < mergeJ ? centralIdx : mergeI].identify();
        ATH_MSG_VERBOSE(" Look for strip nr " << stripSum << " found at index " << centralIdx);

        ///
        /// get the local position from the cluster centroid
        ///       
       
        DEFINE_VECTOR(MMPrepData, stripsVec, clustSize);
        
        stripsVec.insert(stripsVec.begin(),std::make_move_iterator(MMprds.begin() + mergeI),
                                           std::make_move_iterator(MMprds.begin() + mergeJ));
        
        /// This cluster is essentially done. Update the Index to go to the next cluster
        mergeI = mergeJ;
        Amg::MatrixX covMatrix(1,1);
        Amg::Vector2D clusterLocalPosition{Amg::Vector2D::Zero()};
        ATH_CHECK(getClusterPosition(ctx, clusterId, stripsVec, clusterLocalPosition, covMatrix));

        ///
        /// memory allocated dynamically for the PrepRawData is managed by Event Store
        ///
        if (!m_writeStripProperties) mergeStrips.clear();
        std::unique_ptr<MMPrepData> prdN = std::make_unique<MMPrepData>(clusterId, 
                                                                        hash, 
                                                                        std::move(clusterLocalPosition), 
                                                                        std::move(rdoList), 
                                                                        std::move(covMatrix), 
                                                                        stripsVec.front().detectorElement(),
                                                                        0, 
                                                                        totalCharge, 
                                                                        0.f, 
                                                                        std::move(mergeStrips), 
                                                                        std::move(mergeStripsTime), 
                                                                        std::move(mergeStripsCharge));
        
        prdN->setDriftDist(std::move(mergeStripsDriftDists), std::move(mergeStripsDriftDistErrors));
        prdN->setAuthor(MMPrepData::Author::SimpleClusterBuilder);
        clustersVect.push_back(std::move(prdN));
    }  // end loop MMprds[i]
    return StatusCode::SUCCESS;
}

/// get cluster local position and covariance matrix
StatusCode SimpleMMClusterBuilderTool::getClusterPosition(const EventContext& ctx,
                                                          const Identifier& clustId,
                                                          std::vector<MMPrepData>& stripsVec, 
                                                          Amg::Vector2D& clusterLocalPosition,
                                                          Amg::MatrixX& covMatrix) const {
    
    SG::ReadCondHandle<NswErrorCalibData> errorCalibDB{m_uncertCalibKey, ctx};
    if (!errorCalibDB.isValid()) {
        ATH_MSG_FATAL("Failed to retrieve the parameterized errors "<<m_uncertCalibKey.fullKey());
        return StatusCode::FAILURE;
    }
    covMatrix.setIdentity();
    double weightedPosX{0.}, posY{0.}, totalCharge{0.};
    Amg::Vector3D clusDir{Amg::Vector3D::Zero()};

    /// get the Y local position from the first strip ( it's the same for all strips in the cluster)
    posY = stripsVec[0].localPosition().y();
    for (const MMPrepData& strip : stripsVec) {
        const double posX = strip.localPosition().x();
        const double charge = strip.charge();
        weightedPosX += posX * charge;
        totalCharge += charge; 
        const Amg::Vector3D lDir{NswClustering::toLocal(strip)};
        clusDir+= charge * lDir;
        const Amg::Vector3D& globPos{strip.globalPosition()};
        ATH_MSG_VERBOSE("Adding a strip to the centroid calculation "<<m_idHelperSvc->toString(strip.identify())
                     <<", charge=" << charge<<" global position: "<<Amg::toString(globPos, 2)
                     <<", theta: "<<globPos.theta()<<", eta: "<<globPos.eta()<<", phi: "<<globPos.phi()
                     <<" -- local direction  theta: "<<lDir.theta()<<", eta: "<<lDir.eta()
                     <<", phi: "<<lDir.phi());
    }
    weightedPosX = weightedPosX / totalCharge;
    clusterLocalPosition = Amg::Vector2D(weightedPosX, posY);

    NswErrorCalibData::Input errorCalibIn{};
    errorCalibIn.stripId = clustId;
    errorCalibIn.clusterAuthor = static_cast<unsigned>(MMPrepData::Author::SimpleClusterBuilder);
    errorCalibIn.locPhi = clusDir.phi();
    errorCalibIn.locTheta = clusDir.theta();
    errorCalibIn.localPos = clusterLocalPosition;
    errorCalibIn.clusterSize = stripsVec.size();

    ATH_MSG_VERBOSE(m_idHelperSvc->toString(clustId)<< " - local track direction: "<<errorCalibIn.locTheta);
    
    const double localUncertainty = errorCalibDB->clusterUncertainty(errorCalibIn);
    covMatrix(0, 0) = localUncertainty * localUncertainty;
    return StatusCode::SUCCESS;
}

RIO_Author SimpleMMClusterBuilderTool::getCalibratedClusterPosition(const EventContext& ctx, 
                                                                    const std::vector<NSWCalib::CalibratedStrip>& calibratedStrips,
                                                                    const Amg::Vector3D& dirEstimate, 
                                                                    Amg::Vector2D& clusterLocalPosition,
                                                                    Amg::MatrixX& covMatrix) const {
    /// correct the precision coordinate of the local position based on the centroid calibration
    double xPosCalib{0.}, totalCharge{0.};
    for (const NSWCalib::CalibratedStrip& it : calibratedStrips) {
        xPosCalib += it.charge * it.dx;
        totalCharge += it.charge;
    }
    if (std::abs(totalCharge) < std::numeric_limits<float>::epsilon()) {
        return RIO_Author::unKnownAuthor;
    }

    xPosCalib /=  totalCharge;

    ATH_MSG_DEBUG("position before calibration and correction: " << clusterLocalPosition[Trk::locX] << " " << xPosCalib);

    clusterLocalPosition[Trk::locX] = clusterLocalPosition[Trk::locX] + xPosCalib;
    SG::ReadCondHandle<NswErrorCalibData> errorCalibDB{m_uncertCalibKey, ctx};
    if (!errorCalibDB.isValid()) {
        ATH_MSG_FATAL("Failed to retrieve the parameterized errors "<<m_uncertCalibKey.fullKey());
        return RIO_Author::unKnownAuthor;
    }
    
    NswErrorCalibData::Input errorCalibIn{};
    errorCalibIn.clusterSize = calibratedStrips.size();
    errorCalibIn.clusterAuthor = RIO_Author::SimpleClusterBuilder;
    errorCalibIn.locPhi = dirEstimate.phi();
    errorCalibIn.locTheta = dirEstimate.theta();
    errorCalibIn.localPos = clusterLocalPosition;
    errorCalibIn.stripId = calibratedStrips[errorCalibIn.clusterSize/2].identifier;

    const double localUncertainty = errorCalibDB->clusterUncertainty(errorCalibIn);
    
    covMatrix(0, 0) = localUncertainty * localUncertainty;

    
    return RIO_Author::SimpleClusterBuilder;
}
}
#undef DEFINE_VECTOR
