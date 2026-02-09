/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "FPGATrackSimReportingAlg.h"

#include <format>

FPGATrackSim::FPGATrackSimReportingAlg::FPGATrackSimReportingAlg(const std::string& name, ISvcLocator* pSvcLocator) : AthReentrantAlgorithm(name, pSvcLocator) {
}

StatusCode FPGATrackSim::FPGATrackSimReportingAlg::initialize()
{
    ATH_CHECK(m_xAODPixelClusterContainerKeys.initialize());
    ATH_CHECK(m_xAODStripClusterContainerKeys.initialize());
    ATH_CHECK(m_xAODSpacePointContainerKeys.initialize());
    ATH_CHECK(m_FPGARoadsKey.initialize(!m_isDataPrep));
    ATH_CHECK(m_FPGAProtoTrackCollections.initialize(!m_isDataPrep));
    ATH_CHECK(m_FPGATracksKey.initialize(!m_isDataPrep));
    ATH_CHECK(m_ActsTrackCollections.initialize());
    ATH_CHECK(m_ActsSeedCollections.initialize());
    ATH_CHECK(m_ActsSeedParamCollections.initialize());

    ATH_CHECK(m_ActsInspectionTool.retrieve());
    return StatusCode::SUCCESS;
}

StatusCode FPGATrackSim::FPGATrackSimReportingAlg::execute(const EventContext& ctx) const
{
    // Process xAOD Pixel Clusters
    std::vector<SG::ReadHandle<xAOD::PixelClusterContainer>> xAODPixelClusterContainers = m_xAODPixelClusterContainerKeys.makeHandles(ctx);
    for (SG::ReadHandle<xAOD::PixelClusterContainer>& clusterContainer : xAODPixelClusterContainers)
    {
        if (!clusterContainer.isValid()) {
            ATH_MSG_WARNING("SG key not available " << clusterContainer.key());
            continue;
        }
        processxAODClusters<xAOD::PixelCluster>(clusterContainer);
    }
    // Process xAOD Strip Clusters
    std::vector<SG::ReadHandle<xAOD::StripClusterContainer>> xAODStripClusterContainers = m_xAODStripClusterContainerKeys.makeHandles(ctx);
    for (SG::ReadHandle<xAOD::StripClusterContainer>& clusterContainer : xAODStripClusterContainers)
    {
        if (!clusterContainer.isValid()) {
            ATH_MSG_WARNING("SG key not available  " << clusterContainer.key());
            continue;
        }
        processxAODClusters<xAOD::StripCluster>(clusterContainer);
    }
    // Process xAOD SpacePoints
    std::vector<SG::ReadHandle<xAOD::SpacePointContainer>> xAODSpacePointContainers = m_xAODSpacePointContainerKeys.makeHandles(ctx);
    for (SG::ReadHandle<xAOD::SpacePointContainer>& spContainer : xAODSpacePointContainers)
    {
        if (!spContainer.isValid()) {
            ATH_MSG_WARNING("SG key not available  " << spContainer.key());
        }
        else {processxAODSpacePoints(spContainer);}
    }
    if (!m_isDataPrep.value()) {
        // Process FPGATrackSim Roads
        SG::ReadHandle<FPGATrackSimRoadCollection> FPGATrackSimRoads(m_FPGARoadsKey, ctx);
        if (FPGATrackSimRoads.isValid()) {
        processFPGARoads(FPGATrackSimRoads);
        }
        else ATH_MSG_WARNING("Could not find FPGA Roads Collection with key " << FPGATrackSimRoads.key());


        // Process FPGATrackSim Tracks
        SG::ReadHandle<FPGATrackSimTrackCollection> FPGATrackSimTracks(m_FPGATracksKey, ctx);
        if (FPGATrackSimTracks.isValid()) {
            processFPGATracks(FPGATrackSimTracks);
        }
        else ATH_MSG_WARNING("Could not find FPGA Track Collection with key " << FPGATrackSimTracks.key());

        // Process FPGATrackSim Prototracks
        std::vector<SG::ReadHandle<ActsTrk::ProtoTrackCollection>> FPGATrackSimProtoTracks = m_FPGAProtoTrackCollections.makeHandles(ctx);
        for (SG::ReadHandle<ActsTrk::ProtoTrackCollection>& prototrackContainer : FPGATrackSimProtoTracks)
        {
            if (!prototrackContainer.isValid()) {
                ATH_MSG_WARNING("SG key not available " << prototrackContainer.key());
                continue;
            }
            processFPGAPrototracks(prototrackContainer);
        }
    } // if it's not the data preparation chain
        // Process Acts tracks
    std::vector<SG::ReadHandle<ActsTrk::TrackContainer>> FPGAActsTracks = m_ActsTrackCollections.makeHandles(ctx);
    for (SG::ReadHandle<ActsTrk::TrackContainer>& actsTrackContainer : FPGAActsTracks)
    {
        if (actsTrackContainer.cptr()) ATH_MSG_DEBUG("Proccessing " << actsTrackContainer.key());
        else continue;
        // initialize the ReadHandle pair if necessarys
        m_allActsTracks.try_emplace(actsTrackContainer.key(), std::vector<FPGATrackSimActsEventTracks>{});

        // fetch acts tracks
        m_allActsTracks[actsTrackContainer.key()].push_back(m_ActsInspectionTool->getActsTracks(*(actsTrackContainer.cptr())));

        // initialize ReadHandle stats map for all tracks if necessary
        m_actsTrackStats.try_emplace(actsTrackContainer.key(), std::map<Acts::TrackStateFlag, std::vector<uint32_t>>{});

        m_actsTrackStats[actsTrackContainer.key()].emplace(
            Acts::TrackStateFlag::IsOutlier, std::vector<uint32_t>{});
        m_actsTrackStats[actsTrackContainer.key()].emplace(
            Acts::TrackStateFlag::IsHole, std::vector<uint32_t>{});
        m_actsTrackStats[actsTrackContainer.key()].emplace(
            Acts::TrackStateFlag::HasMeasurement, std::vector<uint32_t>{});
        for (const auto& track : m_allActsTracks[actsTrackContainer.key()].back()) {
            uint32_t t_nOutliers = 0, t_nMeasurements = 0, t_nHoles = 0;
            for (const auto& measurement : track->trackMeasurements)
            {
                if (measurement->outlierFlag) ++t_nOutliers;
                if (measurement->measurementFlag) ++t_nMeasurements;
                if (measurement->holeFlag) ++t_nHoles;
            }
            m_actsTrackStats[actsTrackContainer.key()][Acts::TrackStateFlag::IsOutlier].push_back(t_nOutliers);
            m_actsTrackStats[actsTrackContainer.key()][Acts::TrackStateFlag::IsHole].push_back(t_nHoles);
            m_actsTrackStats[actsTrackContainer.key()][Acts::TrackStateFlag::HasMeasurement].push_back(t_nMeasurements);
        }
        if (m_printoutForEveryEvent) ATH_MSG_INFO("ACTS tracks in " << actsTrackContainer.key() << m_ActsInspectionTool->getPrintoutActsEventTracks(m_allActsTracks[actsTrackContainer.key()].back()));
    }

    std::vector<SG::ReadHandle<ActsTrk::SeedContainer>> FPGAActsSeeds = m_ActsSeedCollections.makeHandles(ctx);

    for (SG::ReadHandle<ActsTrk::SeedContainer>& actsTrackContainer : FPGAActsSeeds)
    {
        if (!actsTrackContainer.isValid()) {
            ATH_MSG_WARNING("SG key not available " << actsTrackContainer.key());
            continue;
        }
        processFPGASeeds(actsTrackContainer);
    }


    std::vector<SG::ReadHandle<ActsTrk::BoundTrackParametersContainer>> FPGAActsSeedsParam = m_ActsSeedParamCollections.makeHandles(ctx);

    for (SG::ReadHandle<ActsTrk::BoundTrackParametersContainer>& actsTrackContainer : FPGAActsSeedsParam)
    {
        if (!actsTrackContainer.isValid()) {
            ATH_MSG_WARNING("SG key not available " << actsTrackContainer.key());
            continue;
        }
        processFPGASeedsParam(actsTrackContainer);
    }
    
    return StatusCode::SUCCESS;
}

StatusCode FPGATrackSim::FPGATrackSimReportingAlg::finalize()
{
    ATH_MSG_INFO("Printing statistics for FPGA objects");

    if (!m_isDataPrep.value()) {
      // Printing summary for clusters/FPGATracks 
      std::string summaryTableFPGATracks = "\n"
        "Number of measurements for FPGA Tracks\n"
        "|-----------------------------------|\n"
        "|        |  min |  max |      Avg   |\n"
        "|-----------------------------------|\n";
      
      if (not m_pixelClustersPerFPGATrack.empty()) {
        summaryTableFPGATracks += std::format("| Pixels | {:>4} | {:>4} | {:>10.2f} |\n",
					      *std::min_element(m_pixelClustersPerFPGATrack.begin(), m_pixelClustersPerFPGATrack.end()),
					      *std::max_element(m_pixelClustersPerFPGATrack.begin(), m_pixelClustersPerFPGATrack.end()),
					      std::accumulate(m_pixelClustersPerFPGATrack.begin(), m_pixelClustersPerFPGATrack.end(), 0.0) / m_pixelClustersPerFPGATrack.size());
      }
      else summaryTableFPGATracks += std::format("| Pixels | ---- | ---- |      inf   |\n");
      
      if (not m_stripClustersPerFPGATrack.empty()) {
        summaryTableFPGATracks += std::format("| Strips | {:>4} | {:>4} | {:>10.2f} |\n",
					      *std::min_element(m_stripClustersPerFPGATrack.begin(), m_stripClustersPerFPGATrack.end()),
					      *std::max_element(m_stripClustersPerFPGATrack.begin(), m_stripClustersPerFPGATrack.end()),
					      std::accumulate(m_stripClustersPerFPGATrack.begin(), m_stripClustersPerFPGATrack.end(), 0.0) / m_stripClustersPerFPGATrack.size());
      }
      else summaryTableFPGATracks += std::format("| Strips | ---- | ---- |      inf   |\n");
      
      summaryTableFPGATracks += "|-----------------------------------|";
      ATH_MSG_INFO( summaryTableFPGATracks );
      
      
      // Printing summary for clusters/prototracks
      std::string summaryTableFPGAPrototracks = std::format("\n"
							    "Number of measurements for FPGA Prototracks\n"
							    "|-----------------------------------|\n"
							    "|        |  min |  max |      Avg   |\n"
							    "|-----------------------------------|\n");
      
      if (not m_pixelClustersPerPrototrack.empty()) {
	summaryTableFPGAPrototracks += std::format("| Pixels | {:>4} | {:>4} | {:>10.2f} |\n",
						   *std::min_element(m_pixelClustersPerPrototrack.begin(), m_pixelClustersPerPrototrack.end()),
						   *std::max_element(m_pixelClustersPerPrototrack.begin(), m_pixelClustersPerPrototrack.end()),
						   std::accumulate(m_pixelClustersPerPrototrack.begin(), m_pixelClustersPerPrototrack.end(), 0.0) / m_pixelClustersPerPrototrack.size());
      }
      else summaryTableFPGAPrototracks += std::format("| Pixels | ---- | ---- |      inf   |\n");
      
      if (not m_stripClustersPerPrototrack.empty()) {
	summaryTableFPGAPrototracks += std::format("| Strips | {:>4} | {:>4} | {:>10.2f} |\n",
						   *std::min_element(m_stripClustersPerPrototrack.begin(), m_stripClustersPerPrototrack.end()),
						   *std::max_element(m_stripClustersPerPrototrack.begin(), m_stripClustersPerPrototrack.end()),
						   std::accumulate(m_stripClustersPerPrototrack.begin(), m_stripClustersPerPrototrack.end(), 0.0) / m_stripClustersPerPrototrack.size());
      }
      else summaryTableFPGAPrototracks += std::format("| Strips | ---- | ---- |      inf   |\n");
      
      summaryTableFPGAPrototracks += "|-----------------------------------|";
      ATH_MSG_INFO( summaryTableFPGAPrototracks );
    }

    ATH_MSG_INFO(m_ActsInspectionTool->getPrintoutStatistics(m_actsTrackStats));
    return StatusCode::SUCCESS;
}

template <class XAOD_CLUSTER>
void FPGATrackSim::FPGATrackSimReportingAlg::processxAODClusters(SG::ReadHandle<DataVector< XAOD_CLUSTER >>& clusterContainer) const
{
    if (m_printoutForEveryEvent) printxAODClusters(clusterContainer);
}

template <class XAOD_CLUSTER>
void FPGATrackSim::FPGATrackSimReportingAlg::printxAODClusters(SG::ReadHandle<DataVector< XAOD_CLUSTER >>& clusterContainer) const
{
    std::string mainTable = "\n"
        "|=========================================================================================|\n"
        "|      # |             Global coordinates             |     Hash ID  |      Identifier    |\n"
        "|        |       x      |       y      |       z      |              |                    |\n"
        "|-----------------------------------------------------------------------------------------|\n";
    unsigned int counter = 0;
    for (const auto& cluster : *clusterContainer)
    {
        ++counter;
        mainTable += std::format("| {:>6} | {:>12} | {:>12} | {:>12} | {:>12} | {:>18} |\n",
            counter,
            cluster->globalPosition().x(),
            cluster->globalPosition().y(),
            cluster->globalPosition().z(),
            cluster->identifierHash(),
            cluster->identifier());
    }
    mainTable += "|=========================================================================================|";
    ATH_MSG_INFO("Printout of xAOD clusters coming from " << clusterContainer.key() << mainTable );
}

template <>
void FPGATrackSim::FPGATrackSimReportingAlg::printxAODClusters(SG::ReadHandle<DataVector< xAOD::PixelCluster >>& clusterContainer) const
{
    std::string mainTable = "\n"
        "|===================================================================================================================================================================================|\n"
        "|      # |             Global coordinates             |      Local coordinates      |      Local Covariance       |             Size            |     Hash ID  |      Identifier    |\n"
        "|        |       x      |       y      |       z      |       0      |       1      |      0,0     |     1,1      |      phi     |     eta      |              |                    |\n"
        "|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|\n";
    unsigned int counter = 0;
    std::map<int, int> counterMap;
    for (const xAOD::PixelCluster* cluster : *clusterContainer)
    {
        ++counter;

            mainTable += std::format("| {:>6} | {:>12} | {:>12} | {:>12} | {:>12} | {:>12} | {:>12} | {:>12} | {:>12} | {:>12} | {:>12} | {:#018x} |\n",
            counter,
            cluster->globalPosition().x(),
            cluster->globalPosition().y(),
            cluster->globalPosition().z(),
            cluster->localPosition<2>()[0],
            cluster->localPosition<2>()[1],
            cluster->localCovariance<2>()(0, 0),
            cluster->localCovariance<2>()(1, 1),
            cluster->channelsInPhi(),
            cluster->channelsInEta(),
            cluster->identifierHash(),
            cluster->identifier());
            
            counterMap[cluster->identifierHash()]++;
        }
    mainTable += "|=========================================================================================|";

    ATH_MSG_INFO("Printout of xAOD clusters coming from " << clusterContainer.key() << mainTable );
}

template <>
void FPGATrackSim::FPGATrackSimReportingAlg::printxAODClusters(SG::ReadHandle<DataVector< xAOD::StripCluster >>& clusterContainer) const
{
    std::string mainTable = "\n"
        "|======================================================================================================================================|\n"
        "|      # |             Global coordinates             |     Local    |    Local     |     Size     |     Hash ID  |      Identifier    |\n"
        "|        |       x      |       y      |       z      |     Pos      |     Cov      |              |              |                    |\n"
        "|--------------------------------------------------------------------------------------------------------------------------------------|\n";
    unsigned int counter = 0;
    std::map<int, int> counterMap;
    for (const xAOD::StripCluster* cluster : *clusterContainer)
    {
        ++counter;

        mainTable += std::format("| {:>6} | {:>12} | {:>12} | {:>12} | {:>12} | {:>12} | {:>12} | {:>12} | {:#018x} |\n",
        counter,
        cluster->globalPosition().x(),
        cluster->globalPosition().y(),
        cluster->globalPosition().z(),
        cluster->localPosition<1>()[0],
        cluster->localCovariance<1>()(0, 0),
        cluster->channelsInPhi(),
        cluster->identifierHash(),
        cluster->identifier());

        counterMap[cluster->identifierHash()]++;
    }
    mainTable += "|=========================================================================================|";

    ATH_MSG_INFO("Printout of xAOD clusters coming from " << clusterContainer.key() << mainTable );
}


void FPGATrackSim::FPGATrackSimReportingAlg::processxAODSpacePoints(SG::ReadHandle<DataVector< xAOD::SpacePoint >>& spContainer) const
{
    if (m_printoutForEveryEvent) printxAODSpacePoints(spContainer);
}

void FPGATrackSim::FPGATrackSimReportingAlg::printxAODSpacePoints(SG::ReadHandle<DataVector< xAOD::SpacePoint >>& spContainer) const
{
    std::string mainTable = "\n"
        "|============================================================================|\n"
        "|      # |             Global coordinates             |     element ID list  |\n"
        "|        |       x      |       y      |       z      |                      |\n"
        "|----------------------------------------------------------------------------|\n";
    unsigned int counter = 0;
    for (const auto sp : *spContainer)
    {
        ++counter;
        mainTable += std::format("| {:>6} | {:>12} | {:>12} | {:>12} | {:>9}, {:>9} |\n",
            counter,
            sp->globalPosition().x(),
            sp->globalPosition().y(),
            sp->globalPosition().z(),
            sp->elementIdList()[0],
            sp->elementIdList().size() == 2 ? sp->elementIdList()[1] : 0);
    }
    mainTable += "|=========================================================================================|";
    ATH_MSG_INFO("Printout of xAOD space points coming from " << spContainer.key() << mainTable );
}


void FPGATrackSim::FPGATrackSimReportingAlg::processFPGASeeds(SG::ReadHandle<ActsTrk::SeedContainer>& spContainer) const
{
    if (m_printoutForEveryEvent) printFPGASeeds(spContainer);
}

void FPGATrackSim::FPGATrackSimReportingAlg::printFPGASeeds(SG::ReadHandle<ActsTrk::SeedContainer>& seedContainer) const
{
    std::string mainTable = "\n"
        "|=====================================================================================|\n"
        "|      # | seed   |             Global coordinates             |     element ID list  |\n"
        "|        | i      |       x      |       y      |       z      |                      |\n"
        "|-------------------------------------------------------------------------------------|\n";
    unsigned int counter = 0;
    for (const auto thisTrack : *seedContainer)
    {
        auto spList = thisTrack.sp();
        ++counter;
        for(int i = 0; i < 3; i++)
        {
            auto sp = spList.at(i);
            mainTable += std::format("| {:>6} | {:>6} | {:>12} | {:>12} | {:>12} | {:>9}, {:>9} |\n",
                counter,
                i,
                sp->globalPosition().x(),
                sp->globalPosition().y(),
                sp->globalPosition().z(),
                sp->elementIdList()[0],
                sp->elementIdList().size() == 2 ? sp->elementIdList()[1] : 0);
        }

    }
    mainTable += "|=========================================================================================|";
    ATH_MSG_INFO("Printout of ACTS seeds coming from " << seedContainer.key() << mainTable );
}


void FPGATrackSim::FPGATrackSimReportingAlg::processFPGASeedsParam(SG::ReadHandle<ActsTrk::BoundTrackParametersContainer>& spContainer) const
{
    if (m_printoutForEveryEvent) printFPGASeedsParam(spContainer);
}

void FPGATrackSim::FPGATrackSimReportingAlg::printFPGASeedsParam(SG::ReadHandle<ActsTrk::BoundTrackParametersContainer>& seedContainer) const
{
    std::string mainTable = "\n"
        "|=============================================================================================|\n"
        "|      # |      QopT      |     Theta      |       Phi      |        d0      |        z0      |\n"
        "|---------------------------------------------------------------------------------------------|\n";
    unsigned int counter = 0;
    for (const auto paramSet : *seedContainer)
    {
        ++counter;
        auto parameters = paramSet->parameters();
        mainTable += std::format("| {:>6} | {:>14.10f} | {:>14.10f} | {:>14.10f} | {:>14.10f} | {:>14.10f} |\n",
        counter,
        parameters[Acts::eBoundQOverP],
        parameters[Acts::eBoundTheta],
        parameters[Acts::eBoundPhi],
        parameters[Acts::eBoundLoc0],
        parameters[Acts::eBoundLoc1]);
    }
    mainTable += "|=========================================================================================|";
    ATH_MSG_INFO("Printout of ACTS seeds Param coming from " << seedContainer.key() << mainTable );
}





void FPGATrackSim::FPGATrackSimReportingAlg::processFPGARoads(SG::ReadHandle<FPGATrackSimRoadCollection>& FPGARoads) const
{
    if (m_printoutForEveryEvent) printFPGARoads(FPGARoads);
}

void FPGATrackSim::FPGATrackSimReportingAlg::printFPGARoads(SG::ReadHandle<FPGATrackSimRoadCollection>& FPGARoads) const
{
    std::string mainTable = "\n"
        "|--------------------------------------------------------------------------------------------------|\n"
        "|      # |  SubRegion  |    xBin  |    yBin  |       X      |       Y      |   RoadID   |  Sector  |\n"
        "|--------------------------------------------------------------------------------------------------|\n";
    
    unsigned int roadCounter = 0, hitCounter = 0;
    for (const FPGATrackSimRoad& road : *FPGARoads)
    {
        ++roadCounter;
        mainTable += std::format("| {:>6} | {:>11} | {:>8} | {:>8} | {:>12} | {:>12} | {:>10} | {:>8} |\n",
        roadCounter,
        road.getSubRegion(),
        road.getXBin(),
        road.getYBin(),
        road.getX(),
        road.getY(),
        road.getRoadID(),
        road.getSector());
        mainTable +=
            "|        __________________________________________________________________________________________|\n"
            "|        |   layer   |    ##  |   type   |             Global coordinates                |  isReal |\n"
            "|        |           |        |          |        x      |        y      |        z      |         |\n"
            "|        |.........................................................................................|\n";
        for (unsigned int i = 0; i < road.getNLayers(); ++i)
        {
            hitCounter = 0;
            const std::vector<std::shared_ptr<const FPGATrackSimHit>>& hits = road.getHits(i);
            for (auto const& hit : hits)
            {
                if (!hit) {
                    ATH_MSG_WARNING("Null pointer for FPGATrackSimHit");
                    continue;
                }
                try {
                    ++hitCounter;
                    mainTable += std::format("|        | {:>9} | {:>6} | {:>8} | {:>13} | {:>13} | {:>13} | {:>7} |\n",
                        (hit->isMapped() ? hit->getLayer() : 9999),
                        hitCounter,
                        (hit->isPixel() ? "Pixel" : hit->isStrip() ? "Strip" : "FAILED"),
                        hit->getX(),
                        hit->getY(),
                        hit->getZ(),
                        hit->isReal());
                } catch (const std::exception& e) {
                    ATH_MSG_ERROR("Exception while processing FPGATrackSimHit: " << e.what());
                }

            }
        }
        mainTable += "|--------------------------------------------------------------------------------------------------|\n";
    }
    ATH_MSG_INFO("List of FPGA roads for " << FPGARoads.key() << mainTable);
}


void FPGATrackSim::FPGATrackSimReportingAlg::processFPGATracks(SG::ReadHandle<FPGATrackSimTrackCollection>& FPGATracks) const
{
    for (auto const& track : *FPGATracks)
    {
        const std::vector <FPGATrackSimHit>& hits = track.getFPGATrackSimHits();
        uint32_t pixelHits = 0, stripHits = 0;
        for (const FPGATrackSimHit& hit : hits)
        {
            if (hit.isPixel()) ++pixelHits;
            if (hit.isStrip()) ++stripHits;
        }
        m_pixelClustersPerFPGATrack.push_back(pixelHits);
        m_stripClustersPerFPGATrack.push_back(stripHits);
    }
    if (m_printoutForEveryEvent) printFPGATracks(FPGATracks);
}

void FPGATrackSim::FPGATrackSimReportingAlg::printFPGATracks(SG::ReadHandle<FPGATrackSimTrackCollection>& FPGATracks) const
{
    std::string maintable = "\n|--------------------------------------------------------------------------------------------------|\n";
    unsigned int trackCounter = 0;
    for (auto const& track : *FPGATracks)
    {
        ++trackCounter;
        maintable += "|      # |     Eta      |     Phi      |       D0     |       Z0     |    QOverPt   |   chi2/ndf   |\n"
                     "|--------------------------------------------------------------------------------------------------|\n";

        try
        {
            maintable += std::format("| {:>6} | {:>12.8f} | {:>12.8f} | {:>12.8f} | {:>12.5f} | {:>12.9f} | {:>12.8f} |\n",
            trackCounter,
            track.getEta(),
            track.getPhi(),
            track.getD0(),
            track.getZ0(),
            track.getQOverPt(),
            track.getChi2ndof());
        }
        catch (const std::exception& e)
        {
            ATH_MSG_ERROR("Exception while processing FPGATrackSimTrack: " << e.what());
            continue;
        }

        maintable += "|__________________________________________________________________________________________________|\n"
                     "|        |    ##  |   type   | layer |             Global coordinates    | isReal |     HashID     |\n"
                     "|        |        |          |       |      x    |      y    |      z    |        |                |\n"
                     "|        |.........................................................................................|\n";
        const std::vector <FPGATrackSimHit>& hits = track.getFPGATrackSimHits();
        unsigned int hitCounter = 0;
        for (auto const& hit : hits)
        {
            try {
                ++hitCounter;
                maintable += std::format("|        | {:>6} | {:>8} | {:>5} | {:>9.3f} | {:>9.3f} | {:>9.3f} | {:>6} | {:>14} |\n",
                hitCounter,
                (hit.isPixel() ? "Pixel" : hit.isStrip() ? "Strip" : "FAILED"),
                hit.getLayer(),
                hit.getX(),
                hit.getY(),
                hit.getZ(),
                hit.isReal(),
                hit.getIdentifierHash());
            } catch (const std::exception& e) {
                ATH_MSG_ERROR("Exception while processing FPGATrackSimHits: " << e.what());
            }
        }
        maintable += "|--------------------------------------------------------------------------------------------------|\n";
    }
    ATH_MSG_INFO("List of FPGA tracks for " << FPGATracks.key() << maintable);
}

void FPGATrackSim::FPGATrackSimReportingAlg::processFPGAPrototracks(SG::ReadHandle<ActsTrk::ProtoTrackCollection>& FPGAPrototracks) const
{
    unsigned int nPixelMeasurements, nStripMeasurements;
    for (auto const& prototrack : *FPGAPrototracks)
    {
        nPixelMeasurements = 0, nStripMeasurements = 0;
        const std::vector<ActsTrk::ATLASUncalibSourceLink>* measurements = &prototrack.measurements;
        for (const ActsTrk::ATLASUncalibSourceLink& measurementLink : *measurements)
        {
            const xAOD::UncalibratedMeasurement& measurement = ActsTrk::getUncalibratedMeasurement(measurementLink);
            if (measurement.type() == xAOD::UncalibMeasType::PixelClusterType) ++nPixelMeasurements;
            else if (measurement.type() == xAOD::UncalibMeasType::StripClusterType) ++nStripMeasurements;
        }
        m_pixelClustersPerPrototrack.push_back(nPixelMeasurements);
        m_stripClustersPerPrototrack.push_back(nStripMeasurements);

    }
    if (m_printoutForEveryEvent) printFPGAPrototracks(FPGAPrototracks);
}

void FPGATrackSim::FPGATrackSimReportingAlg::printFPGAPrototracks(SG::ReadHandle<ActsTrk::ProtoTrackCollection>& FPGAPrototracks) const
{
    std::string mainTable = "\n";
    if(not (*FPGAPrototracks).empty()) mainTable += "|---------------------------------------------------------------------------------------------|\n";
        
    unsigned int prototrackCounter = 0;
    for (auto const& prototrack : *FPGAPrototracks)
    {
        ++prototrackCounter;
        const Acts::BoundTrackParameters& initialParameters = *prototrack.parameters;
        const Acts::BoundVector& parameters = initialParameters.parameters();
        mainTable +=
        "|      # |      QopT      |     Theta      |       Phi      |        d0      |        z0      |\n"
        "|---------------------------------------------------------------------------------------------|\n";
        mainTable += std::format("| {:>6} | {:>14.10f} | {:>14.10f} | {:>14.10f} | {:>14.10f} | {:>14.10f} |\n",
        prototrackCounter,
        parameters[Acts::eBoundQOverP],
        parameters[Acts::eBoundTheta],
        parameters[Acts::eBoundPhi],
        parameters[Acts::eBoundLoc0],
        parameters[Acts::eBoundLoc1]);

        const std::vector<ActsTrk::ATLASUncalibSourceLink>* measurements = &prototrack.measurements;
        unsigned int measurementCounter = 0;
        if (measurements->size())
        {
            mainTable +=
                "|        _____________________________________________________________________________________|\n"
                "|        |        |   type   |       Global coordinates       |  HashID |       Identifier    |\n"
                "|        |    ##  |          |     x    |     y    |     z    |         |                     |\n"
                "|        |        |...........................................................................|\n";

        }
        for (const ActsTrk::ATLASUncalibSourceLink& measurementLink : *measurements)
        {
            ++measurementCounter;
            const xAOD::UncalibratedMeasurement& measurement = ActsTrk::getUncalibratedMeasurement(measurementLink);
            if (measurement.type() == xAOD::UncalibMeasType::PixelClusterType) {
                const xAOD::PixelCluster* pixelCluster = dynamic_cast<const xAOD::PixelCluster*>(&measurement);
                mainTable += std::format("|        | {:>6} |   Pixel  | {:>8.3f} | {:>8.3f} | {:>8.3f} | {:>7} | {:>19} |\n",
                    measurementCounter,
                    pixelCluster->globalPosition().cast<double>().x(),
                    pixelCluster->globalPosition().cast<double>().y(),
                    pixelCluster->globalPosition().cast<double>().z(),
                    pixelCluster->identifierHash(),
                    pixelCluster->identifier());
            }
            else if (measurement.type() == xAOD::UncalibMeasType::StripClusterType) {
                const xAOD::StripCluster* stripCluster = dynamic_cast<const xAOD::StripCluster*>(&measurement);
                mainTable += std::format("|        | {:>6} |   Strip  | {:>8.3f} | {:>8.3f} | {:>8.3f} | {:>7} | {:>19} |\n",
                    measurementCounter,
                    stripCluster->globalPosition().cast<double>().x(),
                    stripCluster->globalPosition().cast<double>().y(),
                    stripCluster->globalPosition().cast<double>().z(),
                    stripCluster->identifierHash(),
                    stripCluster->identifier());
            }
        }
        mainTable +=  "|---------------------------------------------------------------------------------------------|\n";
    }
    ATH_MSG_INFO("Printing out prototracks coming from " << FPGAPrototracks.key() << mainTable);
}
