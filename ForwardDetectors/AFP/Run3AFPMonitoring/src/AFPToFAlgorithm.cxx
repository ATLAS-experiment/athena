/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*
*
*	AFPToFAlgorithm
*
*
*/

#include "Run3AFPMonitoring/AFPToFAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "xAODForward/AFPStationID.h"


AFPToFAlgorithm::AFPToFAlgorithm( const std::string& name, ISvcLocator* pSvcLocator )
:AthMonitorAlgorithm(name,pSvcLocator)
, m_afpToFHitContainerKey("AFPToFHitContainer"), m_afpTrackContainerKey( "AFPTrackContainer" )

{
	declareProperty( "AFPToFHitContainer", m_afpToFHitContainerKey );
	declareProperty( "AFPTrackContainer", m_afpTrackContainerKey );
}


AFPToFAlgorithm::~AFPToFAlgorithm() {}


StatusCode AFPToFAlgorithm::initialize() {
	using namespace Monitored;

	m_StationNamesGroup = buildToolMap<int>(m_tools,"AFPToFTool", m_stationNamesToF);
	m_BarsInTrains      = buildToolMap<std::map<std::string,std::map<std::string,int>>>(m_tools, "AFPToFTool", m_sidesToF, m_trainsToF, m_barsToF);
	m_GroupChanCombDeltaT    = buildToolMap<int>(m_tools, "AFPToFTool", m_chanComb);
	m_SideTrainGroup    = buildToolMap<std::map<std::string,int>>(m_tools, "AFPToFTool", m_sidesToF, m_trainsToF);
	m_SideGroup         = buildToolMap<int>(m_tools, "AFPToFTool", m_sidesToF);

	// We must declare to the framework in initialize what SG objects we are going to use
	SG::ReadHandleKey<xAOD::AFPToFHitContainer> afpToFHitContainerKey("AFPToFHits");
	ATH_CHECK(m_afpToFHitContainerKey.initialize());
	SG::ReadHandleKey<xAOD::AFPTrackContainer> afpTrackContainerKey( "AFPTracks" );
    ATH_CHECK( m_afpTrackContainerKey.initialize() );
	
	ATH_MSG_INFO( "BunchCrossingKey initialization (ToF)" );
	ATH_CHECK(m_bunchCrossingKeyToF.initialize());
	ATH_MSG_INFO( "initialization completed (ToF)" );

	return AthMonitorAlgorithm::initialize();
}


StatusCode AFPToFAlgorithm::fillHistograms( const EventContext& ctx ) const {
	using namespace Monitored;

	const unsigned NTRAINS = 4;
	const unsigned NBARS = 4;
	const unsigned NSIDES = 2;
	enum { FRONT, MIDDLE, END, NPOS } position = NPOS;
		
	auto bcidAllToF     = Monitored::Scalar<int>("bcidAllToF", 0);
	Monitored::Scalar<int> bcidToF[NPOS] =
	  { Monitored::Scalar<int>("bcidFrontToF", 0),
	    Monitored::Scalar<int>("bcidMiddleToF", 0),
	    Monitored::Scalar<int>("bcidEndToF", 0) };

	// Declare the quantities which should be monitored
	auto lb             = Monitored::Scalar<int>("lb", 0);
	auto nTofHits       = Monitored::Scalar<int>("nTofHits", 1);
	auto trainID        = Monitored::Scalar<int>("trainID", 0); 
	auto barInTrainID   = Monitored::Scalar<int>("barInTrainID", 0); 
	
	auto ToFHits_MU_Weight  = Monitored::Scalar<float>("ToFHits_MU_Weight", 0.0);
	auto muPerBXToF         = Monitored::Scalar<float>("muPerBXToF", 0.0);

	auto numberOfHit      = Monitored::Scalar<int>("numberOfHit", 0);
	auto barInTrainIDSide = Monitored::Scalar<int>("barInTrainIDSide", 0);
	auto barInTrainAll    = Monitored::Scalar<int>("barInTrainAll", 0);
	auto ToFHits_side     = Monitored::Scalar<int>("ToFHits_side", 0);

	auto lbAandCToFEvents = Monitored::Scalar<int>("lbAandCToFEvents", 0);
	Monitored::Scalar<int> lbToFEvents[NSIDES] =
	  { Monitored::Scalar<int>("lbAToFEvents", 0),
	    Monitored::Scalar<int>("lbCToFEvents", 0) };

	// per-mu TH1F variables
	auto lbToFTrainAll       = Monitored::Scalar<int>("lbToFTrainAll", 0);
	auto weightToFTrainAll   = Monitored::Scalar<float>("weightToFTrainAll", 0.0);
	auto lbToFTrainFront     = Monitored::Scalar<int>("lbToFTrainFront", 0);
	auto weightToFTrainFront = Monitored::Scalar<float>("weightToFTrainFront", 0.0);
	auto lbToFTrainMiddle    = Monitored::Scalar<int>("lbToFTrainMiddle", 0);
	auto weightToFTrainMiddle = Monitored::Scalar<float>("weightToFTrainMiddle", 0.0);
	auto lbToFTrainEnd       = Monitored::Scalar<int>("lbToFTrainEnd", 0);
	auto weightToFTrainEnd   = Monitored::Scalar<float>("weightToFTrainEnd", 0.0);
	auto lbToFBar            = Monitored::Scalar<int>("lbToFBar", 0);
	auto lbToFBar_Weight     = Monitored::Scalar<float>("lbToFBar_Weight", 0.0);

	SG::ReadHandle<xAOD::EventInfo> eventInfo = GetEventInfo(ctx);
	lb                = eventInfo->lumiBlock();
	lbAandCToFEvents  = eventInfo->lumiBlock();
	muPerBXToF        = lbAverageInteractionsPerCrossing(ctx);

	if (muPerBXToF == 0.0) {
	  ATH_MSG_DEBUG("AverageInteractionsPerCrossing is 0, forcing to 1.0");
	  muPerBXToF=1.0;
	}

	ToFHits_MU_Weight     = 1/muPerBXToF;
	lbToFBar_Weight       = 1/muPerBXToF;
	weightToFTrainAll     = 1/muPerBXToF;
	weightToFTrainFront   = 1/muPerBXToF;
	weightToFTrainMiddle  = 1/muPerBXToF;
	weightToFTrainEnd     = 1/muPerBXToF;

	for(unsigned int s = 0; s < NSIDES; s++)
	{
		lbToFEvents[s] = eventInfo->lumiBlock();
	}

	fill("AFPToFTool", lb, muPerBXToF);

	// BCX handler
	const unsigned int tempBCID = eventInfo->bcid();
	SG::ReadCondHandle<BunchCrossingCondData> bcidHdlToF(m_bunchCrossingKeyToF,ctx);
	if (!bcidHdlToF.isValid()) {
		ATH_MSG_ERROR( "Unable to retrieve BunchCrossing conditions object (ToF)" );
	}
	const BunchCrossingCondData* bcDataToF{*bcidHdlToF};
	
	// Classifying bunches by position in train (Front, Middle, End)
	if(bcDataToF->isFilled(tempBCID))
	{
		bcidAllToF = tempBCID;
		fill("AFPToFTool", bcidAllToF);
		if(!bcDataToF->isFilled(tempBCID-1))
		{
			position = FRONT;
		}
		else if(bcDataToF->isFilled(tempBCID+1))
		{
			position = MIDDLE;
		}
		else
		{
			position = END;
		}
		bcidToF[position] = tempBCID;
		fill("AFPToFTool", bcidToF[position]);
	}


	SG::ReadHandle<xAOD::AFPToFHitContainer> afpToFHitContainer(m_afpToFHitContainerKey, ctx);
	if(! afpToFHitContainer.isValid())
	{
		ATH_MSG_WARNING("evtStore() does not contain hits collection with name " << m_afpToFHitContainerKey);
		return StatusCode::SUCCESS;
	}

	ATH_CHECK( afpToFHitContainer.initialize() );

	SG::ReadHandle<xAOD::AFPTrackContainer> afpTrackContainer( m_afpTrackContainerKey, ctx );
    if ( !afpTrackContainer.isValid() ) {
        ATH_MSG_WARNING( "evtStore() does not contain hits collection with name " << m_afpTrackContainerKey );
        return StatusCode::SUCCESS;
    }
    ATH_CHECK( afpTrackContainer.initialize() );

	nTofHits = afpToFHitContainer->size();
	fill("AFPToFTool", lb, nTofHits);

	int eventsInStations[4] = {};

	// hit counts per event for TProfile 
	unsigned int totalHitsPerTrainAll[NSIDES][NTRAINS] = {};
	unsigned int totalHitsPerTrainFME[NSIDES][NTRAINS][NPOS] = {};
	unsigned int totalHitsPerBar[NSIDES][NTRAINS][NBARS] = {};
	unsigned int totalStationHits[NSIDES] = {};

	for(const xAOD::AFPToFHit *hitsItr: *afpToFHitContainer)
	{
		if (hitsItr->stationID()<4 && hitsItr->stationID()>=0 && hitsItr->trainID()<4 && hitsItr->trainID()>=0 && hitsItr->barInTrainID()<4 && hitsItr->barInTrainID()>=0)
		{
			trainID = hitsItr->trainID();
			barInTrainID = hitsItr->barInTrainID();
			++eventsInStations[hitsItr->stationID()];

			// Only process ToF stations: 0 (farAside) and 3 (farCside)
			if(hitsItr->stationID() != 0 && hitsItr->stationID() != 3)
				continue;

			int side = (hitsItr->stationID() == 3) ? 1 : 0;
			unsigned int train = hitsItr->trainID();
			unsigned int bar = hitsItr->barInTrainID();

			numberOfHit = train;
			fill(m_tools[m_SideGroup.at(m_sidesToF.at(side))], numberOfHit);

			barInTrainIDSide = bar;
			fill(m_tools[m_SideTrainGroup.at(m_sidesToF.at(side)).at(m_trainsToF.at(train))], barInTrainIDSide);

			barInTrainAll = train * 4 + bar;
			fill(m_tools[m_SideGroup.at(m_sidesToF.at(side))], barInTrainAll);

			ToFHits_side = eventInfo->lumiBlock();
			fill(m_tools[m_SideGroup.at(m_sidesToF.at(side))], ToFHits_side, ToFHits_MU_Weight);

			fill(m_tools[m_StationNamesGroup.at(m_stationNamesToF.at(hitsItr->stationID()))], barInTrainID, trainID);

			// per-mu TH1F per-bar
			lbToFBar = eventInfo->lumiBlock();
			fill(m_tools[m_BarsInTrains.at(m_sidesToF.at(side)).at(m_trainsToF.at(train)).at(m_barsToF.at(bar))], lbToFBar, lbToFBar_Weight);

			// Accumulate counts for TProfile
			++totalStationHits[side];
			++totalHitsPerTrainAll[side][train];
			if(bar < NBARS) ++totalHitsPerBar[side][train][bar];
			if(position != NPOS) ++totalHitsPerTrainFME[side][train][position];

			// per-mu TH1F: fill per-train
			lbToFTrainAll = eventInfo->lumiBlock();
			fill(m_tools[m_SideTrainGroup.at(m_sidesToF.at(side)).at(m_trainsToF.at(train))], lbToFTrainAll, weightToFTrainAll);

			if(position == FRONT)
			{
				lbToFTrainFront = eventInfo->lumiBlock();
				fill(m_tools[m_SideTrainGroup.at(m_sidesToF.at(side)).at(m_trainsToF.at(train))], lbToFTrainFront, weightToFTrainFront);
			}
			else if(position == MIDDLE)
			{
				lbToFTrainMiddle = eventInfo->lumiBlock();
				fill(m_tools[m_SideTrainGroup.at(m_sidesToF.at(side)).at(m_trainsToF.at(train))], lbToFTrainMiddle, weightToFTrainMiddle);
			}
			else if(position == END)
			{
				lbToFTrainEnd = eventInfo->lumiBlock();
				fill(m_tools[m_SideTrainGroup.at(m_sidesToF.at(side)).at(m_trainsToF.at(train))], lbToFTrainEnd, weightToFTrainEnd);
			}

		}
	}
	
	// Events histograms
	if(eventsInStations[0] > 0 || eventsInStations[3] > 0)
	{
		fill("AFPToFTool", lbAandCToFEvents);
		
		if(eventsInStations[0] > 0)
		{
			fill("AFPToFTool", lbToFEvents[0]);
		}
		if(eventsInStations[3] > 0)
		{
			fill("AFPToFTool", lbToFEvents[1]);
		}
	}

	// per-event TProfile per-train 
	auto lbToFPerEvent        = Monitored::Scalar<int>("lbToFPerEvent", eventInfo->lumiBlock());
	auto hitsPerTrainAllPP    = Monitored::Scalar<float>("hitsPerTrainAllPP", 0.0);
	auto lbToFPerEventFront   = Monitored::Scalar<int>("lbToFPerEventFront", eventInfo->lumiBlock());
	auto hitsPerTrainFrontPP  = Monitored::Scalar<float>("hitsPerTrainFrontPP", 0.0);
	auto lbToFPerEventMiddle  = Monitored::Scalar<int>("lbToFPerEventMiddle", eventInfo->lumiBlock());
	auto hitsPerTrainMiddlePP = Monitored::Scalar<float>("hitsPerTrainMiddlePP", 0.0);
	auto lbToFPerEventEnd     = Monitored::Scalar<int>("lbToFPerEventEnd", eventInfo->lumiBlock());
	auto hitsPerTrainEndPP    = Monitored::Scalar<float>("hitsPerTrainEndPP", 0.0);

	for(unsigned int side = 0; side < NSIDES; side++)
	{
		for(unsigned int train = 0; train < NTRAINS; train++)
		{
			hitsPerTrainAllPP = totalHitsPerTrainAll[side][train] / muPerBXToF;
			fill(m_tools[m_SideTrainGroup.at(m_sidesToF.at(side)).at(m_trainsToF.at(train))], lbToFPerEvent, hitsPerTrainAllPP);

			hitsPerTrainFrontPP = totalHitsPerTrainFME[side][train][FRONT] / muPerBXToF;
			if(position == FRONT)
				fill(m_tools[m_SideTrainGroup.at(m_sidesToF.at(side)).at(m_trainsToF.at(train))], lbToFPerEventFront, hitsPerTrainFrontPP);

			hitsPerTrainMiddlePP = totalHitsPerTrainFME[side][train][MIDDLE] / muPerBXToF;
			if(position == MIDDLE)
				fill(m_tools[m_SideTrainGroup.at(m_sidesToF.at(side)).at(m_trainsToF.at(train))], lbToFPerEventMiddle, hitsPerTrainMiddlePP);

			hitsPerTrainEndPP = totalHitsPerTrainFME[side][train][END] / muPerBXToF;
			if(position == END)
				fill(m_tools[m_SideTrainGroup.at(m_sidesToF.at(side)).at(m_trainsToF.at(train))], lbToFPerEventEnd, hitsPerTrainEndPP);
		}
	}

	// per-event TProfile per-bar
	auto lbToFBarPerEvent  = Monitored::Scalar<int>("lbToFBarPerEvent", eventInfo->lumiBlock());
	auto hitsPerBarPP      = Monitored::Scalar<float>("hitsPerBarPP", 0.0);

	for(unsigned int side = 0; side < NSIDES; side++)
	{
		for(unsigned int train = 0; train < NTRAINS; train++)
		{
			for(unsigned int bar = 0; bar < NBARS; bar++)
			{
				hitsPerBarPP = totalHitsPerBar[side][train][bar] / muPerBXToF;
				fill(m_tools[m_BarsInTrains.at(m_sidesToF.at(side)).at(m_trainsToF.at(train)).at(m_barsToF.at(bar))], lbToFBarPerEvent, hitsPerBarPP);
			}
		}
	}

	// per-event TProfile station-level
	auto lbToFStationPerEvent  = Monitored::Scalar<int>("lbToFStationPerEvent", eventInfo->lumiBlock());
	auto hitsPerStationPP      = Monitored::Scalar<float>("hitsPerStationPP", 0.0);

	for(unsigned int side = 0; side < NSIDES; side++)
	{
		hitsPerStationPP = totalStationHits[side] / muPerBXToF;
		fill(m_tools[m_SideGroup.at(m_sidesToF.at(side))], lbToFStationPerEvent, hitsPerStationPP);
	}

	return fillHistograms_crossBarDeltaT(*afpTrackContainer, *afpToFHitContainer);
}

StatusCode AFPToFAlgorithm::fillHistograms_crossBarDeltaT(
        const xAOD::AFPTrackContainer& afpTrackContainer,
        const xAOD::AFPToFHitContainer& afpToFHitContainer) const {
    // Initialize monitored variables for histogram filling
    Monitored::Scalar<float> crossBarDeltaT[2] = {
            Monitored::Scalar<float>( "crossBarDeltaT_A", 0.0 ),
            Monitored::Scalar<float>( "crossBarDeltaT_C", 0.0 )
        };

	bool channel_present[2][16] = {};
	bool multihit[2] = {};
	std::size_t track_count[2] = {};
	std::size_t train_count[2][4] = {};

	for (const xAOD::AFPTrack* tracksItr : afpTrackContainer)  
	{ 
		const auto side = tracksItr->stationID() == 3;
		// Ignore tracks that are not from FAR stations
		if (tracksItr->stationID() != 0 && tracksItr->stationID() != 3) 
			continue;
		++track_count[side];
	}

    // Load the necessary information
	auto times = std::vector<std::vector<std::vector<float>>>(2, std::vector<std::vector<float>>(4, std::vector<float>(4, -10000)));
	for (const xAOD::AFPToFHit* hitsItr : afpToFHitContainer)
	{
		const auto side = hitsItr->stationID() == 3;
        const auto train = hitsItr->trainID();
        const auto bar = hitsItr->barInTrainID();
        const auto channel = 4 * train + bar;
		const auto tof_time = hitsItr->time();
		const auto TimePs=(tof_time)*1000;
		//Cut on only 1 SiT track in the monitored station
		if (track_count[side] != 1) continue;
		// Ignore hits with an impossible origin
		if (hitsItr->stationID() != 0 && hitsItr->stationID() != 3)
			continue;
		if (train < 0 || train >= 4 || bar < 0 || bar >= 4) continue;
		if (channel_present[side][channel])
			multihit[side] = true;
		channel_present[side][channel] = true;
		++train_count[side][train];

		times[side][train][bar]=TimePs;

	}

	for (uint8_t side : {0, 1}) 
	{
		// Cut on only 1 SiT track in the monitored station
		if (track_count[side] != 1) continue;
		// Cut on maximum of 1 hit in each ToF channel
		if (multihit[side]) continue;
		//Cut on maximum 1 train per event
		uint8_t multrain[2] = {};
		for (uint8_t train = 0; train < 4; ++train) {
			if (train_count[side][train]>1) {
				++multrain[side];
			}
		}
		if (multrain[side]>1) continue;
		//fill histos
		for (uint8_t train = 0; train < 4; ++train) {
			for (uint8_t bar1 = 0; bar1 < 4; ++bar1) {
				for (uint8_t bar2 = 0; bar2 < 4; ++bar2) {
					if (bar2>bar1) {
						int comb = bar1*bar2+bar2-1;
						if (comb==5) {comb=4;}
						if (comb==8) {comb=5;}
						int global_comb = train*6 + comb;
						if (times[side][train][bar1]>-10000 && times[side][train][bar2]>-10000) {
							crossBarDeltaT[side] = (times[side][train][bar1] - times[side][train][bar2]);
							fill(m_tools[m_GroupChanCombDeltaT.at(m_chanComb.at(global_comb))], crossBarDeltaT[side]);
						}
					}
				}
			}
		}
	}

    return StatusCode::SUCCESS;
}
