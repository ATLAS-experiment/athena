/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "FPGATrackSimReportingAlg.h"



FPGATrackSim::FPGATrackSimReportingAlg::FPGATrackSimReportingAlg(const std::string& name, ISvcLocator* pSvcLocator) : AthReentrantAlgorithm(name, pSvcLocator) {
}

StatusCode FPGATrackSim::FPGATrackSimReportingAlg::initialize()
{
    ATH_CHECK(m_xAODPixelClusterContainerKeys.initialize());
    ATH_CHECK(m_xAODStripClusterContainerKeys.initialize());
    ATH_CHECK(m_FPGARoadsKey.initialize());
    ATH_CHECK(m_FPGATracksKey.initialize());
    ATH_CHECK(m_FPGAProtoTrackCollection.initialize());
    return StatusCode::SUCCESS;
}

StatusCode FPGATrackSim::FPGATrackSimReportingAlg::execute(const EventContext& ctx) const
{
    // Process xAOD Pixel Clusters
    std::vector<SG::ReadHandle<xAOD::PixelClusterContainer>> xAODPixelClusterContainers = m_xAODPixelClusterContainerKeys.makeHandles(ctx);
    for (SG::ReadHandle<xAOD::PixelClusterContainer>& clusterContainer : xAODPixelClusterContainers)
    {
        if (!clusterContainer.isValid()) {
            ATH_MSG_ERROR("Invalid SG key " << clusterContainer.key());
            return StatusCode::FAILURE;
        }
        processxAODClusters<xAOD::PixelCluster>(clusterContainer);
    }

    // Process xAOD Strip Clusters
    std::vector<SG::ReadHandle<xAOD::StripClusterContainer>> xAODStripClusterContainers = m_xAODStripClusterContainerKeys.makeHandles(ctx);
    for (SG::ReadHandle<xAOD::StripClusterContainer>& clusterContainer : xAODStripClusterContainers)
    {
        if (!clusterContainer.isValid()) {
            ATH_MSG_ERROR("Invalid SG key " << clusterContainer.key());
            return StatusCode::FAILURE;
        }
        processxAODClusters<xAOD::StripCluster>(clusterContainer);
    }

    // Process FPGATrackSim Roads
    SG::ReadHandle<FPGATrackSimRoadCollection> FPGATrackSimRoads(m_FPGARoadsKey, ctx);
    if (!FPGATrackSimRoads.isValid()) {
        ATH_MSG_ERROR("Could not find FPGA Roads Collection with key " << FPGATrackSimRoads.key());
        return StatusCode::FAILURE;
    }
    processFPGARoads(FPGATrackSimRoads);

    // Process FPGATrackSim Tracks
    SG::ReadHandle<FPGATrackSimTrackCollection> FPGATrackSimTracks(m_FPGATracksKey, ctx);
    if (!FPGATrackSimTracks.isValid()) {
        ATH_MSG_ERROR("Could not find FPGA Track Collection with key " << FPGATrackSimTracks.key());
        return StatusCode::FAILURE;
    }
    processFPGATracks(FPGATrackSimTracks);

    // Process FPGATrackSim Prototracks
    SG::ReadHandle<ActsTrk::ProtoTrackCollection> FPGATrackSimProtoTracks(m_FPGAProtoTrackCollection, ctx);
    if (!FPGATrackSimProtoTracks.isValid()) {
        ATH_MSG_ERROR("Could not find FPGA Prototrack Collection with key " << FPGATrackSimProtoTracks.key());
        return StatusCode::FAILURE;
    }
    processFPGAPrototracks(FPGATrackSimProtoTracks);

    return StatusCode::SUCCESS;
}

StatusCode FPGATrackSim::FPGATrackSimReportingAlg::finalize()
{
    ATH_MSG_INFO("Printing statistics for FPGA objects");

    // Printing summary for clusters/FPGATracks 
    std::string header = "\n"
        "Number of measurements for FPGA Tracks\n"
        "|-----------------------------------|\n"
        "|        |  min |  max |      Avg   |\n"
        "|-----------------------------------|\n";
    std::string tableEnd = "|-----------------------------------|";

    std::string pixelsPerFPGATrack = std::format("| Pixels | {:>4} | {:>4} | {:>10} |\n",
        *std::min_element(m_pixelClustersPerFPGATrack.begin(), m_pixelClustersPerFPGATrack.end()),
        *std::max_element(m_pixelClustersPerFPGATrack.begin(), m_pixelClustersPerFPGATrack.end()),
        m_pixelClustersPerFPGATrack.size() == 0 ? "inf" : std::format("{:>.2f}",std::accumulate(m_pixelClustersPerFPGATrack.begin(), m_pixelClustersPerFPGATrack.end(), 0.0) / m_pixelClustersPerFPGATrack.size()));

    std::string stripsPerFPGATrack = std::format("| Strips | {:>4} | {:>4} | {:>10} |\n",
    *std::min_element(m_stripClustersPerFPGATrack.begin(), m_stripClustersPerFPGATrack.end()),
    *std::max_element(m_stripClustersPerFPGATrack.begin(), m_stripClustersPerFPGATrack.end()),
    m_stripClustersPerFPGATrack.size() == 0 ? "inf" : std::format("{:>.2f}",std::accumulate(m_stripClustersPerFPGATrack.begin(), m_stripClustersPerFPGATrack.end(), 0.0) / m_stripClustersPerFPGATrack.size()));

    ATH_MSG_INFO(header << pixelsPerFPGATrack << stripsPerFPGATrack << tableEnd);

    // Printing summary for clusters/prototracks
    header = std::format("\n"
        "Number of measurements for FPGA Prototracks\n"
        "|-----------------------------------|\n"
        "|        |  min |  max |      Avg   |\n"
        "|-----------------------------------|\n");

    std::string pixelsPerPrototrack = std::format("| Pixels | {:>4} | {:>4} | {:>10} |\n",
        *std::min_element(m_pixelClustersPerPrototrack.begin(), m_pixelClustersPerPrototrack.end()),
        *std::max_element(m_pixelClustersPerPrototrack.begin(), m_pixelClustersPerPrototrack.end()),
        m_pixelClustersPerPrototrack.size() == 0 ? "inf" : std::format("{:>.2f}",std::accumulate(m_pixelClustersPerPrototrack.begin(), m_pixelClustersPerPrototrack.end(), 0.0) / m_pixelClustersPerPrototrack.size()));

    std::string stripsPerPrototrack = std::format("| Strips | {:>4} | {:>4} | {:>10} |\n",
    *std::min_element(m_stripClustersPerPrototrack.begin(), m_stripClustersPerPrototrack.end()),
    *std::max_element(m_stripClustersPerPrototrack.begin(), m_stripClustersPerPrototrack.end()),
    m_stripClustersPerPrototrack.size() == 0 ? "inf" : std::format("{:>.2f}",std::accumulate(m_stripClustersPerPrototrack.begin(), m_stripClustersPerPrototrack.end(), 0.0) / m_stripClustersPerPrototrack.size()));

    ATH_MSG_INFO(header << pixelsPerPrototrack << stripsPerPrototrack << tableEnd);

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
    ATH_MSG_DEBUG("Printout of xAOD clusters coming from " << clusterContainer.key() << mainTable );
}

void FPGATrackSim::FPGATrackSimReportingAlg::processFPGARoads(SG::ReadHandle<FPGATrackSimRoadCollection>& FPGARoads) const
{
    if (m_printoutForEveryEvent) printFPGARoads(FPGARoads);
}

void FPGATrackSim::FPGATrackSimReportingAlg::printFPGARoads(SG::ReadHandle<FPGATrackSimRoadCollection>& FPGARoads) const
{
    std::string mainTable = "\n"
        "|--------------------------------------------------------------------------------------------------|\n"
        "|      # |  SubRegion  |    xBin    |    yBin    |       X      |       Y      |       RoadID      |\n"
        "|--------------------------------------------------------------------------------------------------|\n";
    
    unsigned int roadCounter = 0, hitCounter = 0;
    for (const FPGATrackSimRoad& road : *FPGARoads)
    {
        ++roadCounter;
        mainTable += std::format("| {:>6} | {:>11} | {:>10} | {:>10} | {:>12} | {:>12} | {:>12} |\n",
        roadCounter,
        road.getSubRegion(),
        road.getXBin(),
        road.getYBin(),
        road.getX(),
        road.getY(),
        road.getRoadID());
        mainTable +=
            "|        __________________________________________________________________________________________|\n"
            "|        |   layer   |    ##  |   type   |             Global coordinates                |  isReal |\n"
            "|        |           |        |          |        x      |        y      |        z      |         |\n"
            "|        |.........................................................................................|\n";
        for (unsigned int i = 0; i < road.getNLayers(); ++i)
        {
            hitCounter = 0;
            const std::vector <std::shared_ptr<const FPGATrackSimHit>>& hits = road.getHits(i);
            for (auto const& hit : hits)
            {
                ++hitCounter;
                mainTable += std::format("|        | {:>9} | {:>6} | {:>8} | {:>13} | {:>13} | {:>13} | {:>7} |\n",
                    (hit->isMapped() ? hit->getLayer() : 9999),
                    hitCounter,
                    (hit->isPixel() ? "Pixel" : hit->isStrip() ? "Strip" : "FAILED"),
                    hit->getX(),
                    hit->getY(),
                    hit->getZ(),
                    hit->isReal());
            }
        }
        mainTable += "|--------------------------------------------------------------------------------------------------|\n";
    }
    ATH_MSG_DEBUG("List of FPGA roads for " << FPGARoads.key() << mainTable);
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

        maintable += std::format("| {:>6} | {:>12.8f} | {:>12.8f} | {:>12.8f} | {:>12.5f} | {:>12.9f} | {:>12.8f} |\n",
        trackCounter,
        track.getEta(),
        track.getPhi(),
        track.getD0(),
        track.getZ0(),
        track.getQOverPt(),
        track.getChi2ndof());

        maintable += "|__________________________________________________________________________________________________|\n"
                     "|        |    ##  |   type   | layer |             Global coordinates    | isReal |     HashID     |\n"
                     "|        |        |          |       |      x    |      y    |      z    |        |                |\n"
                     "|        |.........................................................................................|\n";
        const std::vector <FPGATrackSimHit>& hits = track.getFPGATrackSimHits();
        unsigned int hitCounter = 0;
        for (auto const& hit : hits)
        {
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
        }
        maintable += "|--------------------------------------------------------------------------------------------------|\n";
    }
    ATH_MSG_DEBUG("List of FPGA tracks for " << FPGATracks.key() << maintable);
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
    ATH_MSG_DEBUG("Printing out prototracks coming from " << FPGAPrototracks.key() << mainTable);
}