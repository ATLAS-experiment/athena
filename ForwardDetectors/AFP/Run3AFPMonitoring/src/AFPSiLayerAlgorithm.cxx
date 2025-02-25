/* 
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*
*
*	AFPSiLayerAlgorithm
*
*
*/

#include "Run3AFPMonitoring/AFPSiLayerAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "xAODForward/AFPStationID.h"


namespace {
	constexpr int reorganizePlanes(const int station, const int layer)
	{
		bool reverse = station == 0 || station == 1;
		return station * 4 + (reverse ? 3 - layer : layer);
	}
}


AFPSiLayerAlgorithm::AFPSiLayerAlgorithm( const std::string& name, ISvcLocator* pSvcLocator )
:AthMonitorAlgorithm(name,pSvcLocator)
, m_afpHitContainerKey("AFPSiHitContainer")
{
	declareProperty("AFPSiHitContainer", m_afpHitContainerKey);
}


AFPSiLayerAlgorithm::~AFPSiLayerAlgorithm() {}


StatusCode AFPSiLayerAlgorithm::initialize() {

	using namespace Monitored;

	m_StationPlaneGroup = buildToolMap<std::map<std::string,int>>(m_tools,"AFPSiLayerTool", m_stationnames, m_pixlayers);
	m_StationGroup = buildToolMap<int>(m_tools, "AFPSiLayerTool", m_stationnames);


	// We must declare to the framework in initialize what SG objects we are going to use:
	SG::ReadHandleKey<xAOD::AFPSiHitContainer> afpHitContainerKey("AFPSiHits");
	ATH_CHECK(m_afpHitContainerKey.initialize());
	
	ATH_MSG_INFO( "BunchCrossingKey initialization (SiT)" );
	ATH_CHECK(m_bunchCrossingKey.initialize());
	ATH_MSG_INFO( "initialization completed (SiT)" );
	return AthMonitorAlgorithm::initialize();
}

StatusCode AFPSiLayerAlgorithm::fillHistograms( const EventContext& ctx ) const {
	using namespace Monitored;
	
	auto bcidAll    = Monitored::Scalar<int>("bcidAll", 0);
	auto bcidFront  = Monitored::Scalar<int>("bcidFront", 0);
	auto bcidMiddle = Monitored::Scalar<int>("bcidMiddle", 0);
	auto bcidEnd    = Monitored::Scalar<int>("bcidEnd", 0);
	
	auto numberOfEventsPerLumiblockFront  = Monitored::Scalar<int>("numberOfEventsPerLumiblockFront", 0);
	auto numberOfEventsPerLumiblockMiddle = Monitored::Scalar<int>("numberOfEventsPerLumiblockMiddle", 0);
	auto numberOfEventsPerLumiblockEnd    = Monitored::Scalar<int>("numberOfEventsPerLumiblockEnd", 0);

	SG::ReadHandle<xAOD::EventInfo> eventInfo = GetEventInfo(ctx);
	numberOfEventsPerLumiblockFront   = eventInfo->lumiBlock();
	numberOfEventsPerLumiblockMiddle  = eventInfo->lumiBlock();
	numberOfEventsPerLumiblockEnd     = eventInfo->lumiBlock();

	// BCX handler
	const unsigned int bcid = eventInfo->bcid();
	SG::ReadCondHandle<BunchCrossingCondData> bcidHdl(m_bunchCrossingKey,ctx);
	if (!bcidHdl.isValid()) {
		ATH_MSG_ERROR( "Unable to retrieve BunchCrossing conditions object (SiT)" );
	}
	const BunchCrossingCondData* bcData{*bcidHdl};

	// Classifying bunches by position in train (Front, Middle, End)
	enum { FRONT, MIDDLE, END, NPOS } position = NPOS;
	if(bcData->isFilled(bcid))
	{
		bcidAll = bcid;
		fill("AFPSiLayerTool", bcidAll);
		if(!bcData->isFilled(bcid-1))
		{
			position = FRONT;
			bcidFront = bcid;
			fill("AFPSiLayerTool", bcidFront);
			fill("AFPSiLayerTool", numberOfEventsPerLumiblockFront);
		}
		else
		{
			if(bcData->isFilled(bcid+1))
			{
				position = MIDDLE;
				bcidMiddle = bcid;
				fill("AFPSiLayerTool", bcidMiddle);
				fill("AFPSiLayerTool", numberOfEventsPerLumiblockMiddle);
			}
			else
			{
				position = END;
				bcidEnd = bcid;
				fill("AFPSiLayerTool", bcidEnd);
				fill("AFPSiLayerTool", numberOfEventsPerLumiblockEnd);
			}
		}
	}
	
		
	// Declare the quantities which should be monitored:
	auto lb       = Monitored::Scalar<int>("lb", 0);
	auto muPerBX  = Monitored::Scalar<float>("muPerBX", 0.0);
	//auto run = Monitored::Scalar<int>("run",0);

	auto nSiHits = Monitored::Scalar<int>("nSiHits", 1);

	auto pixelRowIDChip = Monitored::Scalar<int>("pixelRowIDChip", 0); 
	auto pixelColIDChip = Monitored::Scalar<int>("pixelColIDChip", 0); 

	auto timeOverThreshold = Monitored::Scalar<int>("timeOverThreshold", 0);

	auto clusterX = Monitored::Scalar<float>("clusterX", 0.0);
	auto clusterY = Monitored::Scalar<float>("clusterY", 0.0); 
	auto clustersInPlanes = Monitored::Scalar<int>("clustersInPlanes", 0);

	auto trackX = Monitored::Scalar<float>("trackX", 0.0);
	auto trackY = Monitored::Scalar<float>("trackY", 0.0);
	
	auto planeHits        = Monitored::Scalar<int>("planeHits", 0);
	auto planeHitsAllMU   = Monitored::Scalar<int>("planeHitsAllMU", 0);
	auto weightAllPlanes  = Monitored::Scalar<float>("weightAllPlanes", 1.0);
	
	auto numberOfHitsPerStation = Monitored::Scalar<int>("numberOfHitsPerStation", 0);
	
	auto lbEvents             = Monitored::Scalar<int>("lbEvents", 0);
	auto lbHits               = Monitored::Scalar<int>("lbHits", 0);
	auto lbEventsStations     = Monitored::Scalar<int>("lbEventsStations", 0);
	auto lbEventsStationsAll  = Monitored::Scalar<int>("lbEventsStationsAll", 0);
	
	auto planes = Monitored::Scalar<int>("planes", 0);
	
	auto eventsPerStation = Monitored::Scalar<int>("eventsPerStation", 0);
	
	auto clusterToT = Monitored::Scalar<int>("clusterToT", 0);
	
	lb        = eventInfo->lumiBlock();
	lbEvents  = eventInfo->lumiBlock();
	//muPerBX   = lbAverageInteractionsPerCrossing(ctx);
	muPerBX   = lbInteractionsPerCrossing(ctx);
	if (muPerBX == 0.0) {
		ATH_MSG_DEBUG("AverageInteractionsPerCrossing is 0, forcing to 1.0");
		muPerBX=1.0;
	}
	fill("AFPSiLayerTool", lb, muPerBX);
	fill("AFPSiLayerTool", lbEvents);
	
	
	SG::ReadHandle<xAOD::AFPSiHitContainer> afpHitContainer(m_afpHitContainerKey, ctx);
	if(! afpHitContainer.isValid())
	{
		ATH_MSG_WARNING("evtStore() does not contain hits collection with name " << m_afpHitContainerKey);
		return StatusCode::SUCCESS;
	}

	ATH_CHECK( afpHitContainer.initialize() );

	nSiHits = afpHitContainer->size();
	fill("AFPSiLayerTool", lb, nSiHits);

	int eventsInStations[4] = {};
	int numberOfHitsPerPlane[4][4] = {};

	for(const xAOD::AFPSiHit *hitsItr: *afpHitContainer)
	{
		lb                  = eventInfo->lumiBlock();
		lbHits              = eventInfo->lumiBlock();
		lbEventsStations    = eventInfo->lumiBlock();
		lbEventsStationsAll = eventInfo->lumiBlock();
		pixelRowIDChip      = hitsItr->pixelRowIDChip();
		pixelColIDChip      = hitsItr->pixelColIDChip();
		timeOverThreshold   = hitsItr->timeOverThreshold();
		
		
		if (hitsItr->stationID()<4 && hitsItr->stationID()>=0 && hitsItr->pixelLayerID()<4 && hitsItr->pixelLayerID()>=0) 
		{
			++eventsInStations[hitsItr->stationID()];

			fill(m_tools[m_StationPlaneGroup.at(m_stationnames.at(hitsItr->stationID())).at(m_pixlayers.at(hitsItr->pixelLayerID()))], pixelRowIDChip, pixelColIDChip);
			fill(m_tools[m_StationPlaneGroup.at(m_stationnames.at(hitsItr->stationID())).at(m_pixlayers.at(hitsItr->pixelLayerID()))], pixelRowIDChip);
			fill(m_tools[m_StationPlaneGroup.at(m_stationnames.at(hitsItr->stationID())).at(m_pixlayers.at(hitsItr->pixelLayerID()))], pixelColIDChip);
			fill(m_tools[m_StationPlaneGroup.at(m_stationnames.at(hitsItr->stationID())).at(m_pixlayers.at(hitsItr->pixelLayerID()))], timeOverThreshold);

			planeHits = hitsItr->pixelLayerID();
			fill(m_tools[m_StationGroup.at(m_stationnames.at(hitsItr->stationID()))], planeHits);
			
			++numberOfHitsPerPlane[hitsItr->stationID()][hitsItr->pixelLayerID()];
			planeHitsAllMU = reorganizePlanes(hitsItr->stationID(), hitsItr->pixelLayerID());
			weightAllPlanes = 1 / muPerBX;
			fill("AFPSiLayerTool", planeHitsAllMU, weightAllPlanes);
			weightAllPlanes = 1.0;
			
			numberOfHitsPerStation = hitsItr->stationID();
			fill("AFPSiLayerTool", numberOfHitsPerStation);
			
			fill("AFPSiLayerTool", lbHits);
		}
		else ATH_MSG_WARNING("Unrecognised station index: " << hitsItr->stationID());
	}

	auto hitsPerPlaneProfile          = Monitored::Scalar<float>("hitsPerPlaneProfile", 0.0);
	auto lbhitsPerPlaneProfile       = Monitored::Scalar<int>("lbhitsPerPlaneProfile", 0);

	lbhitsPerPlaneProfile = eventInfo->lumiBlock();
	for(int i_station = 0; i_station < 4; i_station++)
		for(int j_layer = 0; j_layer < 4; j_layer++)
		{
			hitsPerPlaneProfile = numberOfHitsPerPlane[i_station][j_layer]/muPerBX;
			fill(m_tools[m_StationPlaneGroup.at(m_stationnames.at(i_station)).at(m_pixlayers.at(j_layer))], lbhitsPerPlaneProfile, hitsPerPlaneProfile);
		}
	
			
	bool noEventsInStations = true;
	for(int i=0; i<4; i++)
	{
		if(eventsInStations[i]>0) {
			fill(m_tools[m_StationGroup.at(m_stationnames.at(i))], lbEventsStations);
			
			eventsPerStation = i * 4;
			fill("AFPSiLayerTool", eventsPerStation);
			++eventsPerStation;
			fill("AFPSiLayerTool", eventsPerStation);
			++eventsPerStation;
			fill("AFPSiLayerTool", eventsPerStation);
			++eventsPerStation;
			fill("AFPSiLayerTool", eventsPerStation);

			noEventsInStations = false;
		}
	}
	if(!noEventsInStations)
	{
		fill("AFPSiLayerTool", lbEventsStationsAll);
	}
	
	// Filling of cluster and track 2D histograms
	AFPMon::AFPFastReco fast(afpHitContainer.get());
	fast.reco();

	// Track histograms:
	unsigned int totalTracksAll[4] = {};
	unsigned int totalTracksFront[4] = {};
	unsigned int totalTracksMiddle[4] = {};
	unsigned int totalTracksEnd[4] = {};
		
	for (const auto& track : fast.tracks()) 
	{
		trackX = track.x * 1.0;
		trackY = track.y * 1.0;
		fill(m_tools[m_StationGroup.at(m_stationnames.at(track.station))], trackY, trackX);
		
		if (position == FRONT)
		{
			++totalTracksFront[track.station];
			++totalTracksAll[track.station];
		}
		else if (position == MIDDLE)
		{
			++totalTracksMiddle[track.station];
			++totalTracksAll[track.station];
		}
		else if (position == END)
		{
			++totalTracksEnd[track.station];
			++totalTracksAll[track.station];
		}
	}
	
	auto lbTracksAll        = Monitored::Scalar<int>("lbTracksAll", 0);
	auto lbTracksFront      = Monitored::Scalar<int>("lbTracksFront", 0);
	auto lbTracksMiddle     = Monitored::Scalar<int>("lbTracksMiddle", 0);
	auto lbTracksEnd        = Monitored::Scalar<int>("lbTracksEnd", 0);

	auto Total_tracks_All_profile     = Monitored::Scalar<float>("Total_tracks_All_profile", 0.0);
	auto Total_tracks_Front_profile   = Monitored::Scalar<float>("Total_tracks_Front_profile", 0.0);
	auto Total_tracks_Middle_profile  = Monitored::Scalar<float>("Total_tracks_Middle_profile", 0.0);
	auto Total_tracks_End_profile     = Monitored::Scalar<float>("Total_tracks_End_profile", 0.0);
	
	lbTracksAll     = eventInfo->lumiBlock();
	lbTracksFront   = eventInfo->lumiBlock();
	lbTracksMiddle  = eventInfo->lumiBlock();
	lbTracksEnd     = eventInfo->lumiBlock();
	
	for(int i = 0; i < 4; i++)
	{
		Total_tracks_All_profile = totalTracksAll[i] / muPerBX;
		fill(m_tools[m_StationGroup.at(m_stationnames.at(i))], lbTracksAll, Total_tracks_All_profile);
		totalTracksAll[i] = 0;

		Total_tracks_Front_profile = totalTracksFront[i] / muPerBX;
		if (position == FRONT)
			fill(m_tools[m_StationGroup.at(m_stationnames.at(i))], lbTracksFront, Total_tracks_Front_profile);
		totalTracksFront[i] = 0;
		
		Total_tracks_Middle_profile = totalTracksMiddle[i] / muPerBX;
		if (position == MIDDLE)
			fill(m_tools[m_StationGroup.at(m_stationnames.at(i))], lbTracksMiddle, Total_tracks_Middle_profile);
		totalTracksMiddle[i] = 0;
		
		Total_tracks_End_profile = totalTracksEnd[i] / muPerBX;
		if (position == END)
			fill(m_tools[m_StationGroup.at(m_stationnames.at(i))], lbTracksEnd, Total_tracks_End_profile);
		totalTracksEnd[i] = 0;
	}
	
	// Cluster histograms 
	unsigned int totalClustersAll[4][4] = {};
	unsigned int totalClustersFront[4][4] = {};
	unsigned int totalClustersMiddle[4][4] = {};
	unsigned int totalClustersEnd[4][4] = {};

	auto clustersPerPlaneAllPP        = Monitored::Scalar<float>("clustersPerPlaneAllPP", 0.0);
	auto clustersPerPlaneFrontPP        = Monitored::Scalar<float>("clustersPerPlaneFrontPP", 0.0);
	auto clustersPerPlaneMiddlePP       = Monitored::Scalar<float>("clustersPerPlaneMiddlePP", 0.0);
	auto clustersPerPlaneEndPP          = Monitored::Scalar<float>("clustersPerPlaneEndPP", 0.0);

	auto lbClustersPerPlanesAll        = Monitored::Scalar<int>("lbClustersPerPlanesAll", 0);
	auto lbClustersPerPlanesFront      = Monitored::Scalar<int>("lbClustersPerPlanesFront", 0);
	auto lbClustersPerPlanesMiddle     = Monitored::Scalar<int>("lbClustersPerPlanesMiddle", 0);
	auto lbClustersPerPlanesEnd        = Monitored::Scalar<int>("lbClustersPerPlanesEnd", 0);
	
	lbClustersPerPlanesAll     = eventInfo->lumiBlock();
	lbClustersPerPlanesFront   = eventInfo->lumiBlock();
	lbClustersPerPlanesMiddle  = eventInfo->lumiBlock();
	lbClustersPerPlanesEnd     = eventInfo->lumiBlock();

	for(const auto& cluster : fast.clusters()) 
	{
		clusterX = cluster.x * 1.0;
		clusterY = cluster.y * 1.0;
		fill(m_tools[m_StationPlaneGroup.at(m_stationnames.at(cluster.station)).at(m_pixlayers.at(cluster.layer))], clusterY, clusterX);
		if (cluster.station == 0 || cluster.station == 1)
		{
			clustersInPlanes = reorganizePlanes(cluster.station, cluster.layer);
		}
		else
		{
			clustersInPlanes = (cluster.station*4)+cluster.layer;
		}
		fill("AFPSiLayerTool", clustersInPlanes);
		
		clusterToT = cluster.sumToT;
		fill(m_tools[m_StationPlaneGroup.at(m_stationnames.at(cluster.station)).at(m_pixlayers.at(cluster.layer))], clusterToT);

		if (position == FRONT)
		{
			++totalClustersFront[cluster.station][cluster.layer];
			++totalClustersAll[cluster.station][cluster.layer];
		}
		else if (position == MIDDLE)
		{
			++totalClustersMiddle[cluster.station][cluster.layer];
			++totalClustersAll[cluster.station][cluster.layer];
		}
		else if (position == END)
		{
			++totalClustersEnd[cluster.station][cluster.layer];
			++totalClustersAll[cluster.station][cluster.layer];
		}
	}

	for(int i_station = 0; i_station < 4; i_station++)
		for(int j_layer = 0; j_layer < 4; j_layer++)
		{
			clustersPerPlaneAllPP = totalClustersAll[i_station][j_layer] / muPerBX;
			fill(m_tools[m_StationPlaneGroup.at(m_stationnames.at(i_station)).at(m_pixlayers.at(j_layer))], lbClustersPerPlanesAll, clustersPerPlaneAllPP);
			totalClustersAll[i_station][j_layer] = 0;

			clustersPerPlaneFrontPP = totalClustersFront[i_station][j_layer] / muPerBX;
			if (position == FRONT)
				fill(m_tools[m_StationPlaneGroup.at(m_stationnames.at(i_station)).at(m_pixlayers.at(j_layer))], lbClustersPerPlanesFront, clustersPerPlaneFrontPP);
			totalClustersFront[i_station][j_layer] = 0;

			clustersPerPlaneMiddlePP = totalClustersMiddle[i_station][j_layer] / muPerBX;
			if (position == MIDDLE)
				fill(m_tools[m_StationPlaneGroup.at(m_stationnames.at(i_station)).at(m_pixlayers.at(j_layer))], lbClustersPerPlanesMiddle, clustersPerPlaneMiddlePP);
			totalClustersMiddle[i_station][j_layer] = 0;

			clustersPerPlaneEndPP = totalClustersEnd[i_station][j_layer] / muPerBX;
			if (position == END)
				fill(m_tools[m_StationPlaneGroup.at(m_stationnames.at(i_station)).at(m_pixlayers.at(j_layer))], lbClustersPerPlanesEnd, clustersPerPlaneEndPP);
			totalClustersEnd[i_station][j_layer] = 0;
		}

	return StatusCode::SUCCESS;
} // end of fillHistograms


