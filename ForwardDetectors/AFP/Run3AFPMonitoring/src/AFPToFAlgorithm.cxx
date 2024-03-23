/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
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
	m_TrainsToFGroup    = buildToolMap<int>(m_tools, "AFPToFTool", m_trainsToF);
	m_BarsInTrainsA     = buildToolMap<std::map<std::string,int>>(m_tools, "AFPToFTool", m_trainsToFA, m_barsToF);
	m_BarsInTrainsC     = buildToolMap<std::map<std::string,int>>(m_tools,"AFPToFTool", m_trainsToFC, m_barsToF);
	m_GroupChanCombDeltaT    = buildToolMap<int>(m_tools, "AFPToFTool", m_chanComb);


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
	enum { FRONT, MIDDLE, END, NPOS } position = NPOS;
		
	auto bcidAllToF     = Monitored::Scalar<int>("bcidAllToF", 0);
	Monitored::Scalar<int> bcidToF[NPOS] =
	  { Monitored::Scalar<int>("bcidFrontToF", 0),
	    Monitored::Scalar<int>("bcidMiddleToF", 0),
	    Monitored::Scalar<int>("bcidEndToF", 0) };

	// Declare the quantities which should be monitored
	auto lb             = Monitored::Scalar<int>("lb", 0);
	auto nTofHits       = Monitored::Scalar<int>("nTofHits", 1);
	auto numberOfHit_S0 = Monitored::Scalar<int>("numberOfHit_S0", 0); 
	auto numberOfHit_S3 = Monitored::Scalar<int>("numberOfHit_S3", 0);
	auto trainID        = Monitored::Scalar<int>("trainID", 0); 
	auto barInTrainID   = Monitored::Scalar<int>("barInTrainID", 0); 
	auto barInTrainAllA = Monitored::Scalar<int>("barInTrainAllA", 0);
	auto barInTrainIDA  = Monitored::Scalar<int>("barInTrainIDA", 0); 
	auto barInTrainAllC = Monitored::Scalar<int>("barInTrainAllC", 0);
	auto barInTrainIDC  = Monitored::Scalar<int>("barInTrainIDC", 0);
	
	auto ToFHits_sideA      = Monitored::Scalar<int>("ToFHits_sideA", 0);
	auto ToFHits_sideC      = Monitored::Scalar<int>("ToFHits_sideC", 0);
	auto ToFHits_MU_Weight  = Monitored::Scalar<float>("ToFHits_MU_Weight", 0.0);
	auto muPerBXToF         = Monitored::Scalar<float>("muPerBXToF", 0.0);
	
	auto lbAToF           = Monitored::Scalar<int>("lbAToF", 0);
	auto lbCToF           = Monitored::Scalar<int>("lbCToF", 0);
	auto lbAToF_Weight    = Monitored::Scalar<float>("lbAToF_Weight", 0.0);
	auto lbCToF_Weight    = Monitored::Scalar<float>("lbCToF_Weight", 0.0);

	auto lbAToFEvents     = Monitored::Scalar<int>("lbAToFEvents", 0);
	auto lbCToFEvents     = Monitored::Scalar<int>("lbCToFEvents", 0);
	auto lbAandCToFEvents = Monitored::Scalar<int>("lbAandCToFEvents", 0);
	
	// FME histograms quantites (side A)

	Monitored::Scalar<int> lbAToF_T[NTRAINS] =
	  { Monitored::Scalar<int>("lbAToF_T0", 0),
	    Monitored::Scalar<int>("lbAToF_T1", 0),
	    Monitored::Scalar<int>("lbAToF_T2", 0),
	    Monitored::Scalar<int>("lbAToF_T3", 0) };
	
	auto lbAToF_TAll_Weight = Monitored::Scalar<float>("lbAToF_TAll_Weight", 1);
	
	Monitored::Scalar<int> lbAToF_TP[NTRAINS][NPOS] =
	  { { Monitored::Scalar<int>("lbAToF_T0_Front", 0),
	      Monitored::Scalar<int>("lbAToF_T0_Middle", 0),
	      Monitored::Scalar<int>("lbAToF_T0_End", 0) },
	    { Monitored::Scalar<int>("lbAToF_T1_Front", 0),
	      Monitored::Scalar<int>("lbAToF_T1_Middle", 0),
	      Monitored::Scalar<int>("lbAToF_T1_End", 0) },
	    { Monitored::Scalar<int>("lbAToF_T2_Front", 0),
	      Monitored::Scalar<int>("lbAToF_T2_Middle", 0),
	      Monitored::Scalar<int>("lbAToF_T2_End", 0) },
	    { Monitored::Scalar<int>("lbAToF_T3_Front", 0),
	      Monitored::Scalar<int>("lbAToF_T3_Middle", 0),
	      Monitored::Scalar<int>("lbAToF_T3_End", 0) } };

	Monitored::Scalar<float> lbAToF_TWeight[NPOS] =
	  { Monitored::Scalar<float>("lbAToF_TFront_Weight", 1),
	    Monitored::Scalar<float>("lbAToF_TMiddle_Weight", 1),
	    Monitored::Scalar<float>("lbAToF_TEnd_Weight", 1) };
	 
	// FME histograms quantites (side C)
	
	Monitored::Scalar<int> lbCToF_T[NTRAINS] =
	  { Monitored::Scalar<int>("lbCToF_T0", 0),
	    Monitored::Scalar<int>("lbCToF_T1", 0),
	    Monitored::Scalar<int>("lbCToF_T2", 0),
	    Monitored::Scalar<int>("lbCToF_T3", 0) };
	
	auto lbCToF_TAll_Weight = Monitored::Scalar<float>("lbCToF_TAll_Weight", 1);
	
	Monitored::Scalar<int> lbCToF_TP[NTRAINS][NPOS] =
	  { { Monitored::Scalar<int>("lbCToF_T0_Front", 0),
	      Monitored::Scalar<int>("lbCToF_T0_Middle", 0),
	      Monitored::Scalar<int>("lbCToF_T0_End", 0) },
	    { Monitored::Scalar<int>("lbCToF_T1_Front", 0),
	      Monitored::Scalar<int>("lbCToF_T1_Middle", 0),
	      Monitored::Scalar<int>("lbCToF_T1_End", 0) },
	    { Monitored::Scalar<int>("lbCToF_T2_Front", 0),
	      Monitored::Scalar<int>("lbCToF_T2_Middle", 0),
	      Monitored::Scalar<int>("lbCToF_T2_End", 0) },
	    { Monitored::Scalar<int>("lbCToF_T3_Front", 0),
	      Monitored::Scalar<int>("lbCToF_T3_Middle", 0),
	      Monitored::Scalar<int>("lbCToF_T3_End", 0) } };

	Monitored::Scalar<float> lbCToF_TWeight[NPOS] =
	  { Monitored::Scalar<float>("lbCToF_TFront_Weight", 1),
	    Monitored::Scalar<float>("lbCToF_TMiddle_Weight", 1),
	    Monitored::Scalar<float>("lbCToF_TEnd_Weight", 1) };

	SG::ReadHandle<xAOD::EventInfo> eventInfo = GetEventInfo(ctx);
	lb                = eventInfo->lumiBlock();
	lbAToF            = eventInfo->lumiBlock();
	lbCToF            = eventInfo->lumiBlock();
	lbAToFEvents      = eventInfo->lumiBlock();
	lbCToFEvents      = eventInfo->lumiBlock();
	lbAandCToFEvents  = eventInfo->lumiBlock();
	muPerBXToF        = lbAverageInteractionsPerCrossing(ctx);

	if (muPerBXToF == 0.0) {
	  ATH_MSG_DEBUG("AverageInteractionsPerCrossing is 0, forcing to 1.0");
	  muPerBXToF=1.0;
	}
	

	ToFHits_MU_Weight       = 1/muPerBXToF;
	lbAToF_Weight           = 1/muPerBXToF;
	lbCToF_Weight           = 1/muPerBXToF;
	lbAToF_TAll_Weight      = 1/muPerBXToF;
	lbCToF_TAll_Weight      = 1/muPerBXToF;
	lbAToF_TWeight[FRONT]   = 1/muPerBXToF;
	lbAToF_TWeight[MIDDLE]  = 1/muPerBXToF;
	lbAToF_TWeight[END]     = 1/muPerBXToF;
	lbCToF_TWeight[FRONT]   = 1/muPerBXToF;
	lbCToF_TWeight[MIDDLE]  = 1/muPerBXToF;
	lbCToF_TWeight[END]     = 1/muPerBXToF;
	
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

	for(const xAOD::AFPToFHit *hitsItr: *afpToFHitContainer)
	{
		trainID = hitsItr->trainID();
		barInTrainID = hitsItr->barInTrainID();
		++eventsInStations[hitsItr->stationID()];

		if(hitsItr->isSideA())
		{
			numberOfHit_S0 = hitsItr->trainID();
			fill("AFPToFTool", numberOfHit_S0);
			
			barInTrainIDA = hitsItr->barInTrainID();
			fill(m_tools[m_TrainsToFGroup.at(m_trainsToF.at(hitsItr->trainID()))], barInTrainIDA);
			barInTrainAllA = (hitsItr->trainID()*4)+barInTrainIDA;
			fill("AFPToFTool", barInTrainAllA);
			
			ToFHits_sideA = eventInfo->lumiBlock();
			fill("AFPToFTool", ToFHits_sideA, ToFHits_MU_Weight);
		}
		else if(hitsItr->isSideC())
		{
			numberOfHit_S3 = hitsItr->trainID();
			fill("AFPToFTool", numberOfHit_S3);
			
			barInTrainIDC = hitsItr->barInTrainID();
			fill(m_tools[m_TrainsToFGroup.at(m_trainsToF.at(hitsItr->trainID()))], barInTrainIDC);
			barInTrainAllC = (hitsItr->trainID()*4)+barInTrainIDC;
			fill("AFPToFTool", barInTrainAllC);
			
			ToFHits_sideC = eventInfo->lumiBlock();
			fill("AFPToFTool", ToFHits_sideC, ToFHits_MU_Weight);
		}

		if(hitsItr->isSideA() || hitsItr->isSideC())
		{
			auto& lbToF_T = hitsItr->isSideA() ? lbAToF_T : lbCToF_T;
			auto& lbToF_TP = hitsItr->isSideA() ? lbAToF_TP : lbCToF_TP;
			auto& lbToF_TAll_Weight = hitsItr->isSideA() ? lbAToF_TAll_Weight : lbCToF_TAll_Weight;
			auto& lbToF_TWeight = hitsItr->isSideA() ? lbAToF_TWeight : lbCToF_TWeight;

			unsigned int train = hitsItr->trainID();
			if(train < NTRAINS)
			{
				lbToF_T[train] = eventInfo->lumiBlock();
				fill("AFPToFTool", lbToF_T[train], lbToF_TAll_Weight);

				if(position != NPOS)
				{
					lbToF_TP[train][position] = eventInfo->lumiBlock();
					fill("AFPToFTool", lbToF_TP[train][position], lbToF_TWeight[position]);
				}
			}
		}

		if (hitsItr->stationID() == 0 || hitsItr->stationID() == 3)
		{
			fill(m_tools[m_StationNamesGroup.at(m_stationNamesToF.at(hitsItr->stationID()))], barInTrainID, trainID);
			
			if(hitsItr->stationID() == 0)	// farAside
			{
				fill(m_tools[m_BarsInTrainsA.at(m_trainsToFA.at(hitsItr->trainID())).at(m_barsToF.at(hitsItr->barInTrainID()))], lbAToF, lbAToF_Weight);
			}
			else	// farCside
			{
				fill(m_tools[m_BarsInTrainsC.at(m_trainsToFC.at(hitsItr->trainID())).at(m_barsToF.at(hitsItr->barInTrainID()))], lbCToF, lbCToF_Weight);
			}
		}
	}
	
	// Events histograms
	if(eventsInStations[0] > 0 || eventsInStations[3] > 0)
	{
		fill("AFPToFTool", lbAandCToFEvents);
		
		if(eventsInStations[0] > 0)
		{
			fill("AFPToFTool", lbAToFEvents);
		}
		if(eventsInStations[3] > 0)
		{
			fill("AFPToFTool", lbCToFEvents);
		}
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
		if (channel >= 16) continue;
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
