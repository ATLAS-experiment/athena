/*
    Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file PassThroughTool.cxx
 * @author zhaoyuan.cui@cern.ch
 * @date Oct. 31, 2024
 */

#include "PassThroughTool.h"


StatusCode PassThroughTool::initialize()
{
    ATH_MSG_INFO("Initializing PassThroughTool tool");
    ATH_CHECK(m_stripClustersKey.initialize());
    ATH_CHECK(m_pixelClustersKey.initialize());
    ATH_CHECK(m_spacePointsKey.initialize());

    return StatusCode::SUCCESS;
}

StatusCode PassThroughTool::runPassThrough(EFTrackingTransient::StripClusterAuxInput &scAux,
                                           EFTrackingTransient::PixelClusterAuxInput &pxAux,
                                           EFTrackingTransient::SpacePointAuxInput &stripSpAux,
                                           EFTrackingTransient::SpacePointAuxInput &pixelSpAux,
                                           EFTrackingTransient::Metadata *metadata,
                                           const EventContext &ctx) const
{
    // Retrieve the strip and pixel cluster container from the event store
    SG::ReadHandle<xAOD::StripClusterContainer> inputStripClusters(
        m_stripClustersKey, ctx);
    SG::ReadHandle<xAOD::PixelClusterContainer> inputPixelClusters(
        m_pixelClustersKey, ctx);
    
    // Retrieve the space point container from the event store
    SG::ReadHandle<xAOD::SpacePointContainer> inputSpacePoints(
        m_spacePointsKey, ctx);

    // Check if the strip cluster container is valid
    if (!inputStripClusters.isValid())
    {
        ATH_MSG_ERROR("Failed to retrieve: " << m_stripClustersKey);
        return StatusCode::FAILURE;
    }

    // Check if the pixel cluster container is valid
    if (!inputPixelClusters.isValid())
    {
        ATH_MSG_ERROR("Failed to retrieve: " << m_pixelClustersKey);
        return StatusCode::FAILURE;
    }

    // Check if the space point container is valid
    if (!inputSpacePoints.isValid())
    {   
        ATH_MSG_ERROR("Failed to retrieve: " << m_spacePointsKey);
        return StatusCode::FAILURE;
    }
    
    if (msgLvl(MSG::DEBUG))
    {
        ATH_MSG_DEBUG("StripClusterContainer is valid");
        ATH_MSG_DEBUG("PixelClusterContainer is valid");
        ATH_MSG_DEBUG("SpacePointContainer is valid");
        ATH_MSG_DEBUG("Size of pixel clusters is : " << inputPixelClusters->size());
        ATH_MSG_DEBUG("Size of strip clusters is : " << inputStripClusters->size());
        ATH_MSG_DEBUG("Size of space points is : " << inputSpacePoints->size());
    }

    // Prepare the input data for the kernel
    std::vector<EFTrackingTransient::StripCluster> ef_stripClusters;
    std::vector<EFTrackingTransient::PixelCluster> ef_pixelClusters;
    std::vector<EFTrackingTransient::SpacePoint> ef_pixelSpacePoints;
    std::vector<EFTrackingTransient::SpacePoint> ef_stripSpacePoints;
    
    // Store measurement pointers for space points
    std::vector<std::vector<const xAOD::UncalibratedMeasurement*>> pixelSpMeasurements;
    std::vector<std::vector<const xAOD::UncalibratedMeasurement*>> stripSpMeasurements;

    // Get the input cluster data
    ATH_CHECK(getInputClusterData(inputStripClusters.get(), ef_stripClusters, inputStripClusters->size()));
    ATH_CHECK(getInputClusterData(inputPixelClusters.get(), ef_pixelClusters, inputPixelClusters->size()));
    
    // Get the input space point data
    ATH_CHECK(getInputSpacePointData(inputSpacePoints.get(), ef_pixelSpacePoints, pixelSpMeasurements, 
                                    inputSpacePoints->size(), false));
    
    if (msgLvl(MSG::DEBUG))
    {   // add a guard to prevent out of bounds access
        const int maxToPrintStripCl = std::min<int>(3, ef_stripClusters.size());
        // Print a few clusters to verify the conversion
        for (int i = 0; i < maxToPrintStripCl; i++)
        {
            ATH_MSG_DEBUG("StripCluster["
                          << i << "]: " << ef_stripClusters.at(i).localPosition
                          << ", " << ef_stripClusters.at(i).localCovariance << ", "
                          << ef_stripClusters.at(i).idHash << ", "
                          << ef_stripClusters.at(i).id << ", "
                          << ef_stripClusters.at(i).globalPosition[0] << ", "
                          << ef_stripClusters.at(i).globalPosition[1] << ", "
                          << ef_stripClusters.at(i).globalPosition[2] << ", "
                          << ef_stripClusters.at(i).rdoList[0] << ", "
                          << ef_stripClusters.at(i).channelsInPhi);
        }
        // Pixel clusters
        const int maxToPrintPixelCl = std::min<int>(3, ef_pixelClusters.size());
        for (int i = 0; i < maxToPrintPixelCl; i++)
        {
            ATH_MSG_DEBUG("PixelCluster["
                          << i << "]: " << ef_pixelClusters.at(i).id << ", "
                          << ef_pixelClusters.at(i).idHash << ", "
                          << ef_pixelClusters.at(i).localPosition[0] << ", "
                          << ef_pixelClusters.at(i).localPosition[1] << ", "
                          << ef_pixelClusters.at(i).localCovariance[0] << ", "
                          << ef_pixelClusters.at(i).localCovariance[1] << ", "
                          << ef_pixelClusters.at(i).globalPosition[0] << ", "
                          << ef_pixelClusters.at(i).globalPosition[1] << ", "
                          << ef_pixelClusters.at(i).globalPosition[2] << ", "
                          << ef_pixelClusters.at(i).rdoList[0] << ", "
                          << ef_pixelClusters.at(i).channelsInPhi << ", "
                          << ef_pixelClusters.at(i).channelsInEta << ", "
                          << ef_pixelClusters.at(i).widthInEta << ", "
                          << ef_pixelClusters.at(i).omegaX << ", "
                          << ef_pixelClusters.at(i).omegaY << ", "
                          << ef_pixelClusters.at(i).totList[0] << ", "
                          << ef_pixelClusters.at(i).totalToT << ", "
                          << ef_pixelClusters.at(i).chargeList[0] << ", "
                          << ef_pixelClusters.at(i).totalCharge << ", "
                          << ef_pixelClusters.at(i).energyLoss << ", "
                          << ef_pixelClusters.at(i).isSplit << ", "
                          << ef_pixelClusters.at(i).splitProbability1 << ", "
                          << ef_pixelClusters.at(i).splitProbability2 << ", "
                          << ef_pixelClusters.at(i).lvl1a);
        }
    }

    // Determine if we are running sw or hw
    if (m_runSW)
    {
        ATH_MSG_INFO("Running the sw ver of the pass-through kernel");
        // Strip cluster
        std::vector<float> scLocalPosition(m_maxClusterNum);
        std::vector<float> scLocalCovariance(m_maxClusterNum);
        std::vector<unsigned int> scIdHash(m_maxClusterNum);
        std::vector<long unsigned int> scId(m_maxClusterNum);
        std::vector<float> scGlobalPosition(m_maxClusterNum * 3);
        std::vector<unsigned long long> scRdoList(m_maxClusterNum * 5000);
        std::vector<int> scChannelsInPhi(m_maxClusterNum);

        EFTrackingTransient::StripClusterOutput ef_scOutput;

        ef_scOutput.scLocalPosition = scLocalPosition.data();
        ef_scOutput.scLocalCovariance = scLocalCovariance.data();
        ef_scOutput.scIdHash = scIdHash.data();
        ef_scOutput.scId = scId.data();
        ef_scOutput.scGlobalPosition = scGlobalPosition.data();
        ef_scOutput.scRdoList = scRdoList.data();
        ef_scOutput.scChannelsInPhi = scChannelsInPhi.data();

        // Pixel cluster
        std::vector<float> pcLocalPosition(m_maxClusterNum * 2);
        std::vector<float> pcLocalCovariance(m_maxClusterNum * 2);
        std::vector<unsigned int> pcIdHash(m_maxClusterNum);
        std::vector<long unsigned int> pcId(m_maxClusterNum);
        std::vector<float> pcGlobalPosition(m_maxClusterNum * 3);
        std::vector<unsigned long long> pcRdoList(m_maxClusterNum * 5000);
        std::vector<int> pcChannelsInPhi(m_maxClusterNum);
        std::vector<int> pcChannelsInEta(m_maxClusterNum);
        std::vector<float> pcWidthInEta(m_maxClusterNum);
        std::vector<float> pcOmegaX(m_maxClusterNum);
        std::vector<float> pcOmegaY(m_maxClusterNum);   
        std::vector<int> pcTotList(m_maxClusterNum * 5000);
        std::vector<int> pcTotalToT(m_maxClusterNum);
        std::vector<float> pcChargeList(m_maxClusterNum * 5000);
        std::vector<float> pcTotalCharge(m_maxClusterNum);
        std::vector<float> pcEnergyLoss(m_maxClusterNum);
        std::vector<char> pcIsSplit(m_maxClusterNum);   
        std::vector<float> pcSplitProbability1(m_maxClusterNum);
        std::vector<float> pcSplitProbability2(m_maxClusterNum);
        std::vector<int> pcLvl1a(m_maxClusterNum);

        EFTrackingTransient::PixelClusterOutput ef_pcOutput;

        ef_pcOutput.pcLocalPosition = pcLocalPosition.data();
        ef_pcOutput.pcLocalCovariance = pcLocalCovariance.data();
        ef_pcOutput.pcIdHash = pcIdHash.data();
        ef_pcOutput.pcId = pcId.data();
        ef_pcOutput.pcGlobalPosition = pcGlobalPosition.data();
        ef_pcOutput.pcRdoList = pcRdoList.data();
        ef_pcOutput.pcChannelsInPhi = pcChannelsInPhi.data();
        ef_pcOutput.pcChannelsInEta = pcChannelsInEta.data();
        ef_pcOutput.pcWidthInEta = pcWidthInEta.data();
        ef_pcOutput.pcOmegaX = pcOmegaX.data();
        ef_pcOutput.pcOmegaY = pcOmegaY.data();
        ef_pcOutput.pcTotList = pcTotList.data();
        ef_pcOutput.pcTotalToT = pcTotalToT.data();
        ef_pcOutput.pcChargeList = pcChargeList.data();
        ef_pcOutput.pcTotalCharge = pcTotalCharge.data();
        ef_pcOutput.pcEnergyLoss = pcEnergyLoss.data();
        ef_pcOutput.pcIsSplit = pcIsSplit.data();
        ef_pcOutput.pcSplitProbability1 = pcSplitProbability1.data();
        ef_pcOutput.pcSplitProbability2 = pcSplitProbability2.data();
        ef_pcOutput.pcLvl1a = pcLvl1a.data();

        // Space point output structures
        std::vector<unsigned int> pspIdHash(m_maxSpacePointNum);
        std::vector<float> pspGlobalPosition(m_maxSpacePointNum * 3);
        std::vector<float> pspRadius(m_maxSpacePointNum);
        std::vector<float> pspVarianceR(m_maxSpacePointNum);
        std::vector<float> pspVarianceZ(m_maxSpacePointNum);
        std::vector<int> pspMeasurementIndexes(m_maxSpacePointNum);
        std::vector<unsigned int> pspElementIdList(m_maxSpacePointNum);

        EFTrackingTransient::SpacePointOutput ef_pspOutput;
        ef_pspOutput.spIdHash = pspIdHash.data();
        ef_pspOutput.spGlobalPosition = pspGlobalPosition.data();
        ef_pspOutput.spRadius = pspRadius.data();
        ef_pspOutput.spVarianceR = pspVarianceR.data();
        ef_pspOutput.spVarianceZ = pspVarianceZ.data();
        ef_pspOutput.spMeasurementIndexes = pspMeasurementIndexes.data();
        ef_pspOutput.spElementIdList = pspElementIdList.data();

        // Strip space point
        std::vector<unsigned int> sspIdHash(m_maxSpacePointNum);
        std::vector<float> sspGlobalPosition(m_maxSpacePointNum * 3);
        std::vector<float> sspRadius(m_maxSpacePointNum);
        std::vector<float> sspVarianceR(m_maxSpacePointNum);
        std::vector<float> sspVarianceZ(m_maxSpacePointNum);
        std::vector<int> sspMeasurementIndexes(m_maxSpacePointNum);
        std::vector<unsigned int> sspElementIdList(m_maxSpacePointNum);

        EFTrackingTransient::SpacePointOutput ef_sspOutput;
        ef_sspOutput.spIdHash = sspIdHash.data();
        ef_sspOutput.spGlobalPosition = sspGlobalPosition.data();
        ef_sspOutput.spRadius = sspRadius.data();
        ef_sspOutput.spVarianceR = sspVarianceR.data();
        ef_sspOutput.spVarianceZ = sspVarianceZ.data();
        ef_sspOutput.spMeasurementIndexes = sspMeasurementIndexes.data();
        ef_sspOutput.spElementIdList = sspElementIdList.data();

        // Process clusters and optionally space points if enabled
        if (m_clusterOnlyPassThrough) {
            ATH_CHECK(passThroughSW_clusterOnly(ef_stripClusters, ef_scOutput, 
                                              ef_pixelClusters, ef_pcOutput, 
                                              metadata));
        } else {
            ATH_CHECK(passThroughSW(ef_stripClusters, ef_scOutput,
                                   ef_pixelClusters, ef_pcOutput,
                                   ef_stripSpacePoints, ef_sspOutput,
                                   ef_pixelSpacePoints, ef_pspOutput,
                                   metadata));
        }

        // resize the vector to be the length of the cluster
        scLocalPosition.resize(metadata->numOfStripClusters);
        scLocalCovariance.resize(metadata->numOfStripClusters);
        scIdHash.resize(metadata->numOfStripClusters);
        scId.resize(metadata->numOfStripClusters);
        scGlobalPosition.resize(metadata->numOfStripClusters * 3);
        scRdoList.resize(metadata->scRdoIndexSize);
        scChannelsInPhi.resize(metadata->numOfStripClusters);

        pcLocalPosition.resize(metadata->numOfPixelClusters * 2);
        pcLocalCovariance.resize(metadata->numOfPixelClusters * 2);
        pcIdHash.resize(metadata->numOfPixelClusters);
        pcId.resize(metadata->numOfPixelClusters);
        pcGlobalPosition.resize(metadata->numOfPixelClusters * 3);
        pcRdoList.resize(metadata->pcRdoIndexSize);
        pcChannelsInPhi.resize(metadata->numOfPixelClusters);
        pcChannelsInEta.resize(metadata->numOfPixelClusters);
        pcWidthInEta.resize(metadata->numOfPixelClusters);
        pcOmegaX.resize(metadata->numOfPixelClusters);
        pcOmegaY.resize(metadata->numOfPixelClusters);
        pcTotList.resize(metadata->pcTotIndexSize);
        pcTotalToT.resize(metadata->numOfPixelClusters);
        pcChargeList.resize(metadata->pcChargeIndexSize);
        pcTotalCharge.resize(metadata->numOfPixelClusters);
        pcEnergyLoss.resize(metadata->numOfPixelClusters);
        pcIsSplit.resize(metadata->numOfPixelClusters);
        pcSplitProbability1.resize(metadata->numOfPixelClusters);
        pcSplitProbability2.resize(metadata->numOfPixelClusters);
        pcLvl1a.resize(metadata->numOfPixelClusters);

        if (msgLvl(MSG::DEBUG)) {
            // add guard to prevent out of bounds access
            // in instances of not having any/enough stip clusters
            const unsigned int nStripClustersToPrint = std::min(3u, 
                                                      static_cast<unsigned int>(scLocalPosition.size()));
            // print 3 strip clusters
            for (unsigned i = 0; i < nStripClustersToPrint; i++)
            {
                ATH_MSG_DEBUG("scLocalPosition["
                              << i << "] = " << ef_scOutput.scLocalPosition[i]);
                ATH_MSG_DEBUG("scLocalCovariance["
                              << i << "] = " << ef_scOutput.scLocalCovariance[i]);
                ATH_MSG_DEBUG("scIdHash[" << i << "] = " << ef_scOutput.scIdHash[i]);
                ATH_MSG_DEBUG("scId[" << i << "] = " << ef_scOutput.scId[i]);
                ATH_MSG_DEBUG("scGlobalPosition["
                              << i << "] = " << ef_scOutput.scGlobalPosition[i * 3] << ", "
                              << ef_scOutput.scGlobalPosition[i * 3 + 1] << ", "
                              << ef_scOutput.scGlobalPosition[i * 3 + 2]);
                ATH_MSG_DEBUG("scRdoList[" << i << "] = " << ef_scOutput.scRdoList[i]);
                ATH_MSG_DEBUG("scChannelsInPhi["
                              << i << "] = " << ef_scOutput.scChannelsInPhi[i]);
            }

            // print 3 pixel clusters
            const unsigned int nPixelClustersToPrint = std::min(3u, 
                                                      static_cast<unsigned int>(pcLocalPosition.size()));
            // add guard to prevent out of bounds access; same logic as above
            for (unsigned i = 0; i < nPixelClustersToPrint; i++)
            {
                ATH_MSG_DEBUG("pcLocalPosition["
                              << i << "] = " << ef_pcOutput.pcLocalPosition[i * 2] << ", "
                              << ef_pcOutput.pcLocalPosition[i * 2 + 1]);
                ATH_MSG_DEBUG("pcLocalCovariance["
                              << i << "] = " << ef_pcOutput.pcLocalCovariance[i * 2] << ", "
                              << ef_pcOutput.pcLocalCovariance[i * 2 + 1]);
                ATH_MSG_DEBUG("pcIdHash[" << i << "] = " << ef_pcOutput.pcIdHash[i]);
                ATH_MSG_DEBUG("pcId[" << i << "] = " << ef_pcOutput.pcId[i]);
                ATH_MSG_DEBUG("pcGlobalPosition["
                              << i << "] = " << ef_pcOutput.pcGlobalPosition[i * 3] << ", "
                              << ef_pcOutput.pcGlobalPosition[i * 3 + 1] << ", "
                              << ef_pcOutput.pcGlobalPosition[i * 3 + 2]);
                ATH_MSG_DEBUG("pcRdoList[" << i << "] = " << ef_pcOutput.pcRdoList[i]);
                ATH_MSG_DEBUG("pcChannelsInPhi["
                              << i << "] = " << ef_pcOutput.pcChannelsInPhi[i]);
                ATH_MSG_DEBUG("pcChannelsInEta["
                              << i << "] = " << ef_pcOutput.pcChannelsInEta[i]);
                ATH_MSG_DEBUG("pcWidthInEta["
                              << i << "] = " << ef_pcOutput.pcWidthInEta[i]);
                ATH_MSG_DEBUG("pcOmegaX[" << i << "] = " << ef_pcOutput.pcOmegaX[i]);
                ATH_MSG_DEBUG("pcOmegaY[" << i << "] = " << ef_pcOutput.pcOmegaY[i]);
                ATH_MSG_DEBUG("pcTotList[" << i << "] = " << ef_pcOutput.pcTotList[i]);
                ATH_MSG_DEBUG("pcTotalToT["
                              << i << "] = " << ef_pcOutput.pcTotalToT[i]);
                ATH_MSG_DEBUG("pcChargeList["
                              << i << "] = " << ef_pcOutput.pcChargeList[i]);
                ATH_MSG_DEBUG("pcTotalCharge["
                              << i << "] = " << ef_pcOutput.pcTotalCharge[i]);
                ATH_MSG_DEBUG("pcEnergyLoss["
                              << i << "] = " << ef_pcOutput.pcEnergyLoss[i]);
                ATH_MSG_DEBUG("pcIsSplit[" << i << "] = " << ef_pcOutput.pcIsSplit[i]);
                ATH_MSG_DEBUG("pcSplitProbability1["
                              << i << "] = " << ef_pcOutput.pcSplitProbability1[i]);
                ATH_MSG_DEBUG("pcSplitProbability2["
                              << i << "] = " << ef_pcOutput.pcSplitProbability2[i]);
                ATH_MSG_DEBUG("pcLvl1a[" << i << "] = " << ef_pcOutput.pcLvl1a[i]);
            }
        }

        // Group data to make the strip cluster container
        scAux.localPosition = std::move(scLocalPosition);
        scAux.localCovariance = std::move(scLocalCovariance);
        scAux.idHash = std::move(scIdHash);
        scAux.id = std::move(scId);
        scAux.globalPosition = std::move(scGlobalPosition);
        scAux.rdoList = std::move(scRdoList);
        scAux.channelsInPhi = std::move(scChannelsInPhi);

        // Group data to make the pixel cluster container
        pxAux.id = std::move(pcId);
        pxAux.idHash = std::move(pcIdHash);
        pxAux.localPosition = std::move(pcLocalPosition);
        pxAux.localCovariance = std::move(pcLocalCovariance);
        pxAux.globalPosition = std::move(pcGlobalPosition);
        pxAux.rdoList = std::move(pcRdoList);
        pxAux.channelsInPhi = std::move(pcChannelsInPhi);
        pxAux.channelsInEta = std::move(pcChannelsInEta);
        pxAux.widthInEta = std::move(pcWidthInEta);
        pxAux.omegaX = std::move(pcOmegaX);
        pxAux.omegaY = std::move(pcOmegaY);
        pxAux.totList = std::move(pcTotList);
        pxAux.totalToT = std::move(pcTotalToT);
        pxAux.chargeList = std::move(pcChargeList);
        pxAux.totalCharge = std::move(pcTotalCharge);
        pxAux.energyLoss = std::move(pcEnergyLoss);
        pxAux.isSplit = std::move(pcIsSplit);
        pxAux.splitProbability1 = std::move(pcSplitProbability1);
        pxAux.splitProbability2 = std::move(pcSplitProbability2);
        pxAux.lvl1a = std::move(pcLvl1a);

        // Transfer pixel space point data to pixelSpAux
        for (unsigned int i = 0; i < metadata->numOfPixelSpacePoints; i++) {
            pixelSpAux.elementIdList.push_back(ef_pspOutput.spElementIdList[i]);
            
            // Copy x,y,z position
            pixelSpAux.globalPosition.push_back(ef_pspOutput.spGlobalPosition[i*3]);
            pixelSpAux.globalPosition.push_back(ef_pspOutput.spGlobalPosition[i*3+1]);
            pixelSpAux.globalPosition.push_back(ef_pspOutput.spGlobalPosition[i*3+2]);
            
            pixelSpAux.measurementIndexes.push_back(ef_pspOutput.spMeasurementIndexes[i]);
            pixelSpAux.varianceR.push_back(ef_pspOutput.spVarianceR[i]);
            pixelSpAux.varianceZ.push_back(ef_pspOutput.spVarianceZ[i]);
        }
    
        // Transfer strip space point data to stripSpAux
        for (unsigned int i = 0; i < metadata->numOfStripSpacePoints; i++) {
            // For strip space points, add both element IDs
            stripSpAux.elementIdList.push_back(ef_sspOutput.spElementIdList[i*2]);
            stripSpAux.elementIdList.push_back(ef_sspOutput.spElementIdList[i*2+1]);
            
            // Add position data
            stripSpAux.globalPosition.push_back(ef_sspOutput.spGlobalPosition[i*3]);
            stripSpAux.globalPosition.push_back(ef_sspOutput.spGlobalPosition[i*3+1]);
            stripSpAux.globalPosition.push_back(ef_sspOutput.spGlobalPosition[i*3+2]);
            
            // Add measurement indexes
            stripSpAux.measurementIndexes.push_back(ef_sspOutput.spMeasurementIndexes[i*2]);
            stripSpAux.measurementIndexes.push_back(ef_sspOutput.spMeasurementIndexes[i*2+1]);
            
            // Add variance data
            stripSpAux.varianceR.push_back(ef_sspOutput.spVarianceR[i]);
            stripSpAux.varianceZ.push_back(ef_sspOutput.spVarianceZ[i]);
        }


        if (msgLvl(MSG::DEBUG)) { 
            // add guard to prevent out of bounds access
            // in instances of not having any/enough strip space points
            const int maxToPrintStripSp = std::min<int>(3, 
                                                      static_cast<int>(stripSpAux.elementIdList.size()));
             // Print strip space points
            ATH_MSG_DEBUG("Printing first few strip space points:");
            for (int i = 0; i < maxToPrintStripSp; i++) {
                ATH_MSG_DEBUG("StripSpacePoint[" << i << "]: "
                              << "position=(" << stripSpAux.globalPosition[i*3] << ", "
                              << stripSpAux.globalPosition[i*3+1] << ", " 
                              << stripSpAux.globalPosition[i*3+2] << ")"
                              << ", varianceR=" << stripSpAux.varianceR[i]
                              << ", varianceZ=" << stripSpAux.varianceZ[i]);
            }
            // Print pixel space points
            ATH_MSG_DEBUG("Printing first few pixel space points:");
            // add guard to prevent out of bounds access
            // in instances of not having any/enough pixel space points
            const int maxToPrintPixelSp = std::min<int>(3, 
                                                      static_cast<int>(pixelSpAux.elementIdList.size()));
            for (int i = 0; i < maxToPrintPixelSp; i++) {
                ATH_MSG_DEBUG("PixelSpacePoint[" << i << "]: " 
                              << "position=(" << pixelSpAux.globalPosition[i*3] << ", "
                              << pixelSpAux.globalPosition[i*3+1] << ", " 
                              << pixelSpAux.globalPosition[i*3+2] << ")"
                              << ", varianceR=" << pixelSpAux.varianceR[i]
                              << ", varianceZ=" << pixelSpAux.varianceZ[i]);
            }
        
        }
    }
    else
    {
        ATH_MSG_INFO("FPGA mode is not implemented yet");
        return StatusCode::SUCCESS;
    }

    return StatusCode::SUCCESS;
}

StatusCode PassThroughTool::getInputClusterData(
    const xAOD::StripClusterContainer *sc,
    std::vector<EFTrackingTransient::StripCluster> &ef_sc,
    unsigned long N) const
{
    if (N > sc->size())
    {
        ATH_MSG_ERROR("You want to get the "
                      << N << "th strip cluster, but there are only " << sc->size()
                      << " strip clusters in the container.");
        return StatusCode::FAILURE;
    }
    ATH_MSG_DEBUG("Making vector of strip clusters...");
    for (unsigned long i = 0; i < N; i++)
    {
        EFTrackingTransient::StripCluster cache;
        // Get the data from the input xAOD::StripClusterContainer and set it to the
        // cache
        cache.localPosition = sc->at(i)->localPosition<1>()(0, 0);
        cache.localCovariance = sc->at(i)->localCovariance<1>()(0, 0);
        cache.idHash = sc->at(i)->identifierHash();
        cache.id = sc->at(i)->identifier();
        cache.globalPosition[0] = sc->at(i)->globalPosition()[0];
        cache.globalPosition[1] = sc->at(i)->globalPosition()[1];
        cache.globalPosition[2] = sc->at(i)->globalPosition()[2];

        for (unsigned long j = 0; j < sc->at(i)->rdoList().size(); j++)
        {
            cache.rdoList[j] = sc->at(i)->rdoList().at(j).get_compact();
        }

        cache.channelsInPhi = sc->at(i)->channelsInPhi();
        cache.sizeOfRDOList = sc->at(i)->rdoList().size();

        ef_sc.push_back(cache);
    }

    ATH_MSG_DEBUG("Made " << ef_sc.size() << " strip clusters in the vector");
    return StatusCode::SUCCESS;
}

StatusCode PassThroughTool::getInputClusterData(
    const xAOD::PixelClusterContainer *pc,
    std::vector<EFTrackingTransient::PixelCluster> &ef_pc,
    unsigned long N) const
{
    if (N > pc->size())
    {
        ATH_MSG_ERROR("You want to get the "
                      << N << "th pixel cluster, but there are only " << pc->size()
                      << " pixel clusters in the container.");
        return StatusCode::FAILURE;
    }
    ATH_MSG_DEBUG("Making vector of pixel clusters...");
    for (unsigned long i = 0; i < N; i++)
    {
        EFTrackingTransient::PixelCluster cache;
        // Get the data from the input xAOD::PixelClusterContainer and set it to the
        // cache
        cache.id = pc->at(i)->identifier();
        cache.idHash = pc->at(i)->identifierHash();
        cache.localPosition[0] = pc->at(i)->localPosition<2>()(0, 0);
        cache.localPosition[1] = pc->at(i)->localPosition<2>()(1, 0);
        cache.localCovariance[0] = pc->at(i)->localCovariance<2>()(0, 0);
        cache.localCovariance[1] = pc->at(i)->localCovariance<2>()(1, 1);
        cache.globalPosition[0] = pc->at(i)->globalPosition()[0];
        cache.globalPosition[1] = pc->at(i)->globalPosition()[1];
        cache.globalPosition[2] = pc->at(i)->globalPosition()[2];

        for (long unsigned int j = 0; j < pc->at(i)->rdoList().size(); j++)
        {
            cache.rdoList[j] = pc->at(i)->rdoList().at(j).get_compact();
        }

        cache.channelsInPhi = pc->at(i)->channelsInPhi();
        cache.channelsInEta = pc->at(i)->channelsInEta();
        cache.widthInEta = pc->at(i)->widthInEta();
        cache.omegaX = pc->at(i)->omegaX();
        cache.omegaY = pc->at(i)->omegaY();

        for (long unsigned int j = 0; j < pc->at(i)->totList().size(); j++)
        {
            cache.totList[j] = pc->at(i)->totList().at(j);
        }

        cache.totalToT = pc->at(i)->totalToT();

        for (long unsigned int j = 0; j < pc->at(i)->chargeList().size(); j++)
        {
            cache.chargeList[j] = pc->at(i)->chargeList().at(j);
        }

        cache.totalCharge = pc->at(i)->totalCharge();
        cache.energyLoss = pc->at(i)->energyLoss();
        cache.isSplit = pc->at(i)->isSplit();
        cache.splitProbability1 = pc->at(i)->splitProbability1();
        cache.splitProbability2 = pc->at(i)->splitProbability2();
        cache.lvl1a = pc->at(i)->lvl1a();
        cache.sizeOfRDOList = pc->at(i)->rdoList().size();
        cache.sizeOfTotList = pc->at(i)->totList().size();
        cache.sizeOfChargeList = pc->at(i)->chargeList().size();

        ef_pc.push_back(cache);
    }

    ATH_MSG_DEBUG("Made " << ef_pc.size() << " pixel clusters in the vector");
    return StatusCode::SUCCESS;
}

StatusCode PassThroughTool::getInputSpacePointData(
    const xAOD::SpacePointContainer *sp,
    std::vector<EFTrackingTransient::SpacePoint> &ef_sp,
    std::vector<std::vector<const xAOD::UncalibratedMeasurement *>> &sp_meas,
    unsigned long N, bool isStrip) const
{
    if (N > sp->size())
    {
        ATH_MSG_ERROR("You want to get the "
                      << N << "th space point, but there are only " << sp->size()
                      << " SpacePoint in the container.");
        return StatusCode::FAILURE;
    }
    ATH_MSG_DEBUG("Making vector of space point...");
    for (unsigned long i = 0; i < N; i++)
    {
        EFTrackingTransient::SpacePoint cache;
        // Get the data from the input xAOD::SpacePointContainer and set it to the
        // cache
        cache.idHash[0] = sp->at(i)->elementIdList()[0];
        cache.globalPosition[0] = sp->at(i)->x();
        cache.globalPosition[1] = sp->at(i)->y();
        cache.globalPosition[2] = sp->at(i)->z();
        cache.radius = sp->at(i)->radius();
        cache.cov_r = sp->at(i)->varianceR();
        cache.cov_z = sp->at(i)->varianceZ();

        // Pass the uncalibrated measurement for later stage of xAOD container
        // creation for spacepoint
        std::vector<const xAOD::UncalibratedMeasurement *> temp_vec(
            sp->at(i)->measurements().size());
        std::copy(sp->at(i)->measurements().begin(),
                  sp->at(i)->measurements().end(), temp_vec.begin());

        sp_meas.push_back(temp_vec);

        if (isStrip)
        {
            cache.idHash[1] = sp->at(i)->elementIdList()[1];
            cache.topHalfStripLength = sp->at(i)->topHalfStripLength();
            cache.bottomHalfStripLength = sp->at(i)->bottomHalfStripLength();
            std::copy(sp->at(i)->topStripDirection().data(),
                      sp->at(i)->topStripDirection().data() +
                          sp->at(i)->topStripDirection().size(),
                      cache.topStripDirection);
            std::copy(sp->at(i)->bottomStripDirection().data(),
                      sp->at(i)->bottomStripDirection().data() +
                          sp->at(i)->bottomStripDirection().size(),
                      cache.bottomStripDirection);
            std::copy(sp->at(i)->stripCenterDistance().data(),
                      sp->at(i)->stripCenterDistance().data() +
                          sp->at(i)->stripCenterDistance().size(),
                      cache.stripCenterDistance);
            std::copy(sp->at(i)->topStripCenter().data(),
                      sp->at(i)->topStripCenter().data() +
                          sp->at(i)->topStripCenter().size(),
                      cache.topStripCenter);
        }
        ef_sp.push_back(cache);
    }

    ATH_MSG_DEBUG("Made " << ef_sp.size() << " space points in the vector");
    return StatusCode::SUCCESS;
}

// Full pass-throuh kernel, sw ver.
// Including pixel/strip clusters and spcepoints
StatusCode PassThroughTool::passThroughSW(
    const std::vector<EFTrackingTransient::StripCluster> &inputSC,
    EFTrackingTransient::StripClusterOutput &ef_scOutput,
    // PixelCluster
    const std::vector<EFTrackingTransient::PixelCluster> &inputPC,
    EFTrackingTransient::PixelClusterOutput &ef_pcOutput,
    // Strip SpacePoint
    const std::vector<EFTrackingTransient::SpacePoint> &inputSSP,
    EFTrackingTransient::SpacePointOutput &ef_sspOutput,
    // Pixel SpacePoint
    const std::vector<EFTrackingTransient::SpacePoint> &inputPSP,
    EFTrackingTransient::SpacePointOutput &ef_pspOutput,
    EFTrackingTransient::Metadata *metadata) const
{
    // Add error checking at the beginning of the passThroughSW method
    if (inputSC.empty()) {
        ATH_MSG_DEBUG("No strip clusters found.");
        // Set strip-related metadata to 0
        metadata->numOfStripClusters = 0;
        metadata->numOfStripSpacePoints = 0;
        return StatusCode::SUCCESS;
    }

    if (inputPC.empty()) {
        ATH_MSG_DEBUG("No pixel clusters found.");
        // Set pixel-related metadata to 0
        metadata->numOfPixelClusters = 0; 
        metadata->numOfPixelSpacePoints = 0;
        return StatusCode::SUCCESS;
    }

    int rdoIndex_counter = 0;

    unsigned int inputscRdoIndexSize = 0;
    unsigned int inputpcRdoIndexSize = 0;

    // transfer inputStripClusters
    ATH_MSG_DEBUG("Transfering strip clusters...");
    for (size_t i = 0; i < inputSC.size(); i++)
    {
        ef_scOutput.scLocalPosition[i] = inputSC[i].localPosition;
        ef_scOutput.scLocalCovariance[i] = inputSC[i].localCovariance;
        ef_scOutput.scIdHash[i] = inputSC[i].idHash;
        ef_scOutput.scId[i] = inputSC[i].id;
        ef_scOutput.scGlobalPosition[i * 3] = inputSC[i].globalPosition[0];
        ef_scOutput.scGlobalPosition[i * 3 + 1] = inputSC[i].globalPosition[1];
        ef_scOutput.scGlobalPosition[i * 3 + 2] = inputSC[i].globalPosition[2];

        inputscRdoIndexSize += inputSC[i].sizeOfRDOList;

        for (int j = 0; j < inputSC[i].sizeOfRDOList; j++)
        {
            ef_scOutput.scRdoList[rdoIndex_counter + j] = inputSC[i].rdoList[j];
        }
        // update the index counter
        ef_scOutput.scChannelsInPhi[i] = inputSC[i].channelsInPhi;
        rdoIndex_counter += inputSC[i].sizeOfRDOList;
        metadata[0].scRdoIndex[i] = inputSC[i].sizeOfRDOList;
    }

    // transfer inputPC
    rdoIndex_counter = 0;
    int totIndex_counter = 0;
    int chargeIndex_counter = 0;

    unsigned int inputpcTotListsize = 0;
    unsigned int inputpcChargeListsize = 0;

    // trasnsfer_pc:
    for (size_t i = 0; i < inputPC.size(); i++)
    {
        ef_pcOutput.pcLocalPosition[i * 2] = inputPC[i].localPosition[0];
        ef_pcOutput.pcLocalPosition[i * 2 + 1] = inputPC[i].localPosition[1];
        ef_pcOutput.pcLocalCovariance[i * 2] = inputPC[i].localCovariance[0];
        ef_pcOutput.pcLocalCovariance[i * 2 + 1] = inputPC[i].localCovariance[1];
        ef_pcOutput.pcIdHash[i] = inputPC[i].idHash;
        ef_pcOutput.pcId[i] = inputPC[i].id;
        ef_pcOutput.pcGlobalPosition[i * 3] = inputPC[i].globalPosition[0];
        ef_pcOutput.pcGlobalPosition[i * 3 + 1] = inputPC[i].globalPosition[1];
        ef_pcOutput.pcGlobalPosition[i * 3 + 2] = inputPC[i].globalPosition[2];

        inputpcRdoIndexSize += inputPC[i].sizeOfRDOList;

        for (int j = 0; j < inputPC[i].sizeOfRDOList; j++)
        {
            ef_pcOutput.pcRdoList[rdoIndex_counter + j] = inputPC[i].rdoList[j];
        }
        ef_pcOutput.pcChannelsInPhi[i] = inputPC[i].channelsInPhi;
        ef_pcOutput.pcChannelsInEta[i] = inputPC[i].channelsInEta;
        ef_pcOutput.pcWidthInEta[i] = inputPC[i].widthInEta;
        ef_pcOutput.pcOmegaX[i] = inputPC[i].omegaX;
        ef_pcOutput.pcOmegaY[i] = inputPC[i].omegaY;

        inputpcTotListsize += inputPC[i].sizeOfTotList;

        for (int j = 0; j < inputPC[i].sizeOfTotList; j++)
        {
            ef_pcOutput.pcTotList[totIndex_counter + j] = inputPC[i].totList[j];
        }
        ef_pcOutput.pcTotalToT[i] = inputPC[i].totalToT;

        inputpcChargeListsize += inputPC[i].sizeOfChargeList;

        for (int j = 0; j < inputPC[i].sizeOfChargeList; j++)
        {
            ef_pcOutput.pcChargeList[chargeIndex_counter + j] =
                inputPC[i].chargeList[j];
        }
        ef_pcOutput.pcTotalCharge[i] = inputPC[i].totalCharge;
        ef_pcOutput.pcEnergyLoss[i] = inputPC[i].energyLoss;
        ef_pcOutput.pcIsSplit[i] = inputPC[i].isSplit;
        ef_pcOutput.pcSplitProbability1[i] = inputPC[i].splitProbability1;
        ef_pcOutput.pcSplitProbability2[i] = inputPC[i].splitProbability2;
        ef_pcOutput.pcLvl1a[i] = inputPC[i].lvl1a;

        // update the index counter
        rdoIndex_counter += inputPC[i].sizeOfRDOList;
        totIndex_counter += inputPC[i].sizeOfTotList;
        chargeIndex_counter += inputPC[i].sizeOfChargeList;
        metadata[0].pcRdoIndex[i] = inputPC[i].sizeOfRDOList;
        metadata[0].pcTotIndex[i] = inputPC[i].sizeOfTotList;
        metadata[0].pcChargeIndex[i] = inputPC[i].sizeOfChargeList;
    }

    // transfer metadata for clusters
    metadata[0].numOfStripClusters = inputSC.size();
    metadata[0].numOfPixelClusters = inputPC.size();
    metadata[0].scRdoIndexSize = inputscRdoIndexSize;
    metadata[0].pcRdoIndexSize = inputpcRdoIndexSize;
    metadata[0].pcTotIndexSize = inputpcTotListsize;
    metadata[0].pcChargeIndexSize = inputpcChargeListsize;

    // Now handle space points if not in cluster-only mode
    if (!m_clusterOnlyPassThrough) {
        // Process pixel space points
        ATH_MSG_DEBUG("Processing " << inputPSP.size() << " pixel space points");
        
        // loop over the pixel space points and transfer the data to the output
        for (size_t i = 0; i < inputPSP.size(); i++) {
            ef_pspOutput.spElementIdList[i] = inputPSP[i].idHash[0];
            ef_pspOutput.spGlobalPosition[i * 3] = inputPSP[i].globalPosition[0];
            ef_pspOutput.spGlobalPosition[i * 3 + 1] = inputPSP[i].globalPosition[1];
            ef_pspOutput.spGlobalPosition[i * 3 + 2] = inputPSP[i].globalPosition[2];
            ef_pspOutput.spVarianceR[i] = inputPSP[i].cov_r;
            ef_pspOutput.spVarianceZ[i] = inputPSP[i].cov_z;
            ef_pspOutput.spMeasurementIndexes[i] = i; // Index for linking
        }
        
        // Process strip space points
        ATH_MSG_DEBUG("Processing " << inputSSP.size() << " strip space points");
        
        // loop over the strip space points and transfer the data to the output
        for (size_t i = 0; i < inputSSP.size(); i++) {
            // For strip space points, we need two element IDs per space point
            ef_sspOutput.spElementIdList[i * 2] = inputSSP[i].idHash[0];
            ef_sspOutput.spElementIdList[i * 2 + 1] = inputSSP[i].idHash[1];
            
            // Position (x,y,z for each point)
            ef_sspOutput.spGlobalPosition[i * 3] = inputSSP[i].globalPosition[0];
            ef_sspOutput.spGlobalPosition[i * 3 + 1] = inputSSP[i].globalPosition[1];
            ef_sspOutput.spGlobalPosition[i * 3 + 2] = inputSSP[i].globalPosition[2];
            
            // Two measurement indexes for strip space points
            // placeholder values
            ef_sspOutput.spMeasurementIndexes[i * 2] = i * 2;
            ef_sspOutput.spMeasurementIndexes[i * 2 + 1] = i * 2 + 1;
            
            // Variance values
            ef_sspOutput.spVarianceR[i] = inputSSP[i].cov_r;
            ef_sspOutput.spVarianceZ[i] = inputSSP[i].cov_z;
        }
        
        // Update metadata for space points
        metadata[0].numOfPixelSpacePoints = inputPSP.size();
        metadata[0].numOfStripSpacePoints = inputSSP.size();
    } else {
        // If we're processing clusters only, ensure space point counts are zero
        metadata[0].numOfPixelSpacePoints = 0;
        metadata[0].numOfStripSpacePoints = 0;
    }
    
    ATH_MSG_DEBUG("Metadata: ");
    ATH_MSG_DEBUG("numOfStripClusters: " << metadata[0].numOfStripClusters);
    ATH_MSG_DEBUG("numOfPixelClusters: " << metadata[0].numOfPixelClusters);
    ATH_MSG_DEBUG("numOfStripSpacePoints: " << metadata[0].numOfStripSpacePoints);
    ATH_MSG_DEBUG("numOfPixelSpacePoints: " << metadata[0].numOfPixelSpacePoints);

    if (msgLvl(MSG::DEBUG)) {
        // Print pixel space points
        // add guard to prevent out of bounds access
        // in instances of not having any/enough pixel space points
        const int maxToPrintPixelSp = std::min<int>(3, 
                                                      static_cast<int>(inputPSP.size()));
        ATH_MSG_DEBUG("Printing first few pixel space points:");
        for (int i = 0; i < maxToPrintPixelSp; i++) {
            ATH_MSG_DEBUG("PixelSpacePoint[" << i << "]: " 
                          << "position=(" << inputPSP[i].globalPosition[0] << ", "
                          << inputPSP[i].globalPosition[1] << ", " 
                          << inputPSP[i].globalPosition[2] << ")"
                          << ", varianceR=" << inputPSP[i].cov_r
                          << ", varianceZ=" << inputPSP[i].cov_z);
        }
        
        // Print strip space points
        // add guard to prevent out of bounds access
        // in instances of not having any/enough strip space points
        const int maxToPrintStripSp = std::min<int>(3, 
                                                      static_cast<int>(inputSSP.size()));
        ATH_MSG_DEBUG("Printing first few strip space points:");
        for (int i = 0; i < maxToPrintStripSp; i++) {
            ATH_MSG_DEBUG("StripSpacePoint[" << i << "]: "
                          << "position=(" << inputSSP[i].globalPosition[0] << ", "
                          << inputSSP[i].globalPosition[1] << ", " 
                          << inputSSP[i].globalPosition[2] << ")"
                          << ", varianceR=" << inputSSP[i].cov_r
                          << ", varianceZ=" << inputSSP[i].cov_z);
        }
    }

    return StatusCode::SUCCESS;
}

// Cluster-only ver of sw pass-through
StatusCode PassThroughTool::passThroughSW_clusterOnly(
    const std::vector<EFTrackingTransient::StripCluster> &inputSC,
    EFTrackingTransient::StripClusterOutput &ef_scOutput,
    // PixelCluster
    const std::vector<EFTrackingTransient::PixelCluster> &inputPC,
    EFTrackingTransient::PixelClusterOutput &ef_pcOutput,
    // Metadata
    EFTrackingTransient::Metadata *metadata)
    const
{
    int rdoIndex_counter = 0;

    unsigned int inputscRdoIndexSize = 0;

    // transfer inputStripClusters
    ATH_MSG_DEBUG("Transfering strip clusters...");
    for (size_t i = 0; i < inputSC.size(); i++)
    {
        ef_scOutput.scLocalPosition[i] = inputSC[i].localPosition;
        ef_scOutput.scLocalCovariance[i] = inputSC[i].localCovariance;
        ef_scOutput.scIdHash[i] = inputSC[i].idHash;
        ef_scOutput.scId[i] = inputSC[i].id;
        ef_scOutput.scGlobalPosition[i * 3] = inputSC[i].globalPosition[0];
        ef_scOutput.scGlobalPosition[i * 3 + 1] = inputSC[i].globalPosition[1];
        ef_scOutput.scGlobalPosition[i * 3 + 2] = inputSC[i].globalPosition[2];

        inputscRdoIndexSize += inputSC[i].sizeOfRDOList;

        for (int j = 0; j < inputSC[i].sizeOfRDOList; j++)
        {
            ef_scOutput.scRdoList[rdoIndex_counter + j] = inputSC[i].rdoList[j];
        }
        // update the index counter
        ef_scOutput.scChannelsInPhi[i] = inputSC[i].channelsInPhi;
        rdoIndex_counter += inputSC[i].sizeOfRDOList;
        metadata[0].scRdoIndex[i] = inputSC[i].sizeOfRDOList;
    }

    // transfer inputPC
    rdoIndex_counter = 0;
    int totIndex_counter = 0;
    int chargeIndex_counter = 0;

    unsigned int inputpcRdoIndexSize = 0;
    unsigned int inputpcTotListsize = 0;
    unsigned int inputpcChargeListsize = 0;

    // trasnsfer_pc:
    ATH_MSG_DEBUG("Transfering pixel clusters...");
    for (size_t i = 0; i < inputPC.size(); i++)
    {
        ef_pcOutput.pcLocalPosition[i * 2] = inputPC[i].localPosition[0];
        ef_pcOutput.pcLocalPosition[i * 2 + 1] = inputPC[i].localPosition[1];
        ef_pcOutput.pcLocalCovariance[i * 2] = inputPC[i].localCovariance[0];
        ef_pcOutput.pcLocalCovariance[i * 2 + 1] = inputPC[i].localCovariance[1];
        ef_pcOutput.pcIdHash[i] = inputPC[i].idHash;
        ef_pcOutput.pcId[i] = inputPC[i].id;
        ef_pcOutput.pcGlobalPosition[i * 3] = inputPC[i].globalPosition[0];
        ef_pcOutput.pcGlobalPosition[i * 3 + 1] = inputPC[i].globalPosition[1];
        ef_pcOutput.pcGlobalPosition[i * 3 + 2] = inputPC[i].globalPosition[2];

        inputpcRdoIndexSize += inputPC[i].sizeOfRDOList;

        for (int j = 0; j < inputPC[i].sizeOfRDOList; j++)
        {
            ef_pcOutput.pcRdoList[rdoIndex_counter + j] = inputPC[i].rdoList[j];
        }
        ef_pcOutput.pcChannelsInPhi[i] = inputPC[i].channelsInPhi;
        ef_pcOutput.pcChannelsInEta[i] = inputPC[i].channelsInEta;
        ef_pcOutput.pcWidthInEta[i] = inputPC[i].widthInEta;
        ef_pcOutput.pcOmegaX[i] = inputPC[i].omegaX;
        ef_pcOutput.pcOmegaY[i] = inputPC[i].omegaY;

        inputpcTotListsize += inputPC[i].sizeOfTotList;

        for (int j = 0; j < inputPC[i].sizeOfTotList; j++)
        {
            ef_pcOutput.pcTotList[totIndex_counter + j] = inputPC[i].totList[j];
        }
        ef_pcOutput.pcTotalToT[i] = inputPC[i].totalToT;

        inputpcChargeListsize += inputPC[i].sizeOfChargeList;

        for (int j = 0; j < inputPC[i].sizeOfChargeList; j++)
        {
            ef_pcOutput.pcChargeList[chargeIndex_counter + j] =
                inputPC[i].chargeList[j];
        }
        ef_pcOutput.pcTotalCharge[i] = inputPC[i].totalCharge;
        ef_pcOutput.pcEnergyLoss[i] = inputPC[i].energyLoss;
        ef_pcOutput.pcIsSplit[i] = inputPC[i].isSplit;
        ef_pcOutput.pcSplitProbability1[i] = inputPC[i].splitProbability1;
        ef_pcOutput.pcSplitProbability2[i] = inputPC[i].splitProbability2;
        ef_pcOutput.pcLvl1a[i] = inputPC[i].lvl1a;

        // update the index counter
        rdoIndex_counter += inputPC[i].sizeOfRDOList;
        totIndex_counter += inputPC[i].sizeOfTotList;
        chargeIndex_counter += inputPC[i].sizeOfChargeList;
        metadata[0].pcRdoIndex[i] = inputPC[i].sizeOfRDOList;
        metadata[0].pcTotIndex[i] = inputPC[i].sizeOfTotList;
        metadata[0].pcChargeIndex[i] = inputPC[i].sizeOfChargeList;
    }
    // transfer metadata
    metadata[0].numOfStripClusters = inputSC.size();
    metadata[0].numOfPixelClusters = inputPC.size();
    metadata[0].scRdoIndexSize = inputscRdoIndexSize;
    metadata[0].pcRdoIndexSize = inputpcRdoIndexSize;
    metadata[0].pcTotIndexSize = inputpcTotListsize;
    metadata[0].pcChargeIndexSize = inputpcChargeListsize;

    ATH_MSG_DEBUG("Metadata: ");
    ATH_MSG_DEBUG("numOfStripClusters: " << metadata[0].numOfStripClusters);
    ATH_MSG_DEBUG("numOfPixelClusters: " << metadata[0].numOfPixelClusters);
    ATH_MSG_DEBUG("numOfStripSpacePoints: " << metadata[0].numOfStripSpacePoints);
    ATH_MSG_DEBUG("numOfPixelSpacePoints: " << metadata[0].numOfPixelSpacePoints);

    // If we're processing clusters, we should also set up space point metadata
    // This ensures downstream code knows there are no space points yet
    metadata[0].numOfPixelSpacePoints = 0;
    metadata[0].numOfStripSpacePoints = 0;

    ATH_MSG_DEBUG("Updated Metadata: ");
    ATH_MSG_DEBUG("numOfPixelSpacePoints: " << metadata[0].numOfPixelSpacePoints);
    ATH_MSG_DEBUG("numOfStripSpacePoints: " << metadata[0].numOfStripSpacePoints);

    return StatusCode::SUCCESS;
}
