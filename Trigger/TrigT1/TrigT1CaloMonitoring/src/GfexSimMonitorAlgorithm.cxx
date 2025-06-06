/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
   */
#include "GfexSimMonitorAlgorithm.h"

GfexSimMonitorAlgorithm::GfexSimMonitorAlgorithm( const std::string& name, ISvcLocator* pSvcLocator ) : AthMonitorAlgorithm(name,pSvcLocator) {}

StatusCode GfexSimMonitorAlgorithm::initialize() {

	ATH_MSG_DEBUG("Initializing GfexSimMonitorAlgorithm algorithm with name: "<< name());

	ATH_MSG_DEBUG("m_data_key_gFexRho" << m_data_gFexRho);
	ATH_MSG_DEBUG("m_data_gFexBlock" << m_data_gFexBlock );
	ATH_MSG_DEBUG("m_data_gFexJet" << m_data_gFexJet );
	ATH_MSG_DEBUG("m_data_gScalarEJwoj" << m_data_gScalarEJwoj );
	ATH_MSG_DEBUG("m_data_gMETComponentsJwoj" << m_data_gMETComponentsJwoj );
	ATH_MSG_DEBUG("m_data_gMHTComponentsJwoj" <<  m_data_gMHTComponentsJwoj );
	ATH_MSG_DEBUG("m_data_gMSTComponentsJwoj" << m_data_gMSTComponentsJwoj );
	ATH_MSG_DEBUG("m_data_gMETComponentsNoiseCut" << m_data_gMETComponentsNoiseCut );
	ATH_MSG_DEBUG("m_data_gMETComponentsRms" << m_data_gMETComponentsRms );
	ATH_MSG_DEBUG("m_data_gScalarENoiseCut" << m_data_gScalarENoiseCut );
	ATH_MSG_DEBUG("m_data_gScalarERms" << m_data_gScalarERms );


	ATH_MSG_DEBUG("m_simu_key_gFexRho" << m_simu_gFexRho);
	ATH_MSG_DEBUG("m_simu_gFexBlock" << m_simu_gFexBlock );
	ATH_MSG_DEBUG("m_simu_gFexJet" << m_simu_gFexJet );
	ATH_MSG_DEBUG("m_simu_gScalarEJwoj" << m_simu_gScalarEJwoj );
	ATH_MSG_DEBUG("m_simu_gMETComponentsJwoj" << m_simu_gMETComponentsJwoj );
	ATH_MSG_DEBUG("m_simu_gMHTComponentsJwoj" <<  m_simu_gMHTComponentsJwoj );
	ATH_MSG_DEBUG("m_simu_gMSTComponentsJwoj" << m_simu_gMSTComponentsJwoj );
	ATH_MSG_DEBUG("m_simu_gMETComponentsNoiseCut" << m_simu_gMETComponentsNoiseCut );
	ATH_MSG_DEBUG("m_simu_gMETComponentsRms" << m_simu_gMETComponentsRms );
	ATH_MSG_DEBUG("m_simu_gScalarENoiseCut" << m_simu_gScalarENoiseCut );
	ATH_MSG_DEBUG("m_simu_gScalarERms" << m_simu_gScalarERms );


	/*    ATH_MSG_DEBUG("m_data_key_gGlob "  << m_data_key_gGlob  );
	      ATH_MSG_DEBUG("m_data_key_gJ "  << m_data_key_gJ  );

	      ATH_MSG_DEBUG("m_simu_key_gGlob "   << m_simu_key_gGlob   );
	      ATH_MSG_DEBUG("m_simu_key_gJ "  << m_simu_key_gJ  );
	      */


	// we initialise all the containers
	ATH_CHECK ( m_data_gFexRho.initialize() );
	ATH_CHECK ( m_data_gFexBlock.initialize() );
	ATH_CHECK ( m_data_gFexJet.initialize() );
	ATH_CHECK ( m_data_gScalarEJwoj.initialize() );
	ATH_CHECK ( m_data_gMETComponentsJwoj.initialize() );
	ATH_CHECK ( m_data_gMHTComponentsJwoj.initialize() );
	ATH_CHECK ( m_data_gMSTComponentsJwoj.initialize() );
	ATH_CHECK ( m_data_gMETComponentsNoiseCut.initialize() );
	ATH_CHECK ( m_data_gMETComponentsRms.initialize() );
	ATH_CHECK ( m_data_gScalarENoiseCut.initialize() );
	ATH_CHECK ( m_data_gScalarERms.initialize() );


	ATH_CHECK ( m_simu_gFexRho.initialize() );
	ATH_CHECK ( m_simu_gFexBlock.initialize() );
	ATH_CHECK ( m_simu_gFexJet.initialize() );
	ATH_CHECK ( m_simu_gScalarEJwoj.initialize() );
	ATH_CHECK ( m_simu_gMETComponentsJwoj.initialize() );
	ATH_CHECK ( m_simu_gMHTComponentsJwoj.initialize() );
	ATH_CHECK ( m_simu_gMSTComponentsJwoj.initialize() );
	ATH_CHECK ( m_simu_gMETComponentsNoiseCut.initialize() );
	ATH_CHECK ( m_simu_gMETComponentsRms.initialize() );
	ATH_CHECK ( m_simu_gScalarENoiseCut.initialize() );
	ATH_CHECK ( m_simu_gScalarERms.initialize() );

	ATH_CHECK( m_bcContKey.initialize() );



	// TOBs may come from trigger bytestream - renounce from scheduler



	return AthMonitorAlgorithm::initialize();
}

StatusCode GfexSimMonitorAlgorithm::fillHistograms( const EventContext& ctx ) const {

	compareJetRoI("gLJ",m_data_gFexJet,m_simu_gFexJet,ctx,true);
	compareJetRoI("gJ",m_data_gFexBlock,m_simu_gFexBlock,ctx,true);
	compareJetRoI("gLJRho",m_data_gFexRho,m_simu_gFexRho,ctx);
	compareGlobalRoI("gTEJWOJSum",m_data_gScalarEJwoj,m_simu_gScalarEJwoj,ctx,0xff000fff); // wont compare MET value in scalarE tob
	compareGlobalRoI("gXEJWOJMET",m_data_gMETComponentsJwoj,m_simu_gMETComponentsJwoj,ctx);
	compareGlobalRoI("gXEJWOJMHT",m_data_gMHTComponentsJwoj,m_simu_gMHTComponentsJwoj,ctx);
	compareGlobalRoI("gXEJWOJMST",m_data_gMSTComponentsJwoj,m_simu_gMSTComponentsJwoj,ctx);
	compareGlobalRoI("gXENC",m_data_gMETComponentsNoiseCut,m_simu_gMETComponentsNoiseCut,ctx);
	compareGlobalRoI("gXERHO",m_data_gMETComponentsRms,m_simu_gMETComponentsRms,ctx);
	compareGlobalRoI("gTENC",m_data_gScalarENoiseCut,m_simu_gScalarENoiseCut,ctx,0xff000fff);
	compareGlobalRoI("gTERHO",m_data_gScalarERms,m_simu_gScalarERms,ctx,0xff000fff);

	return StatusCode::SUCCESS;
}

bool GfexSimMonitorAlgorithm::compareJetRoI(const std::string& label,
		const SG::ReadHandleKey<xAOD::gFexJetRoIContainer>& tobs1Key,
		const SG::ReadHandleKey<xAOD::gFexJetRoIContainer>& tobs2Key,
		const EventContext& ctx, bool simReadyFlag) const {
	SG::ReadHandle<xAOD::gFexJetRoIContainer> tobs1Cont{tobs1Key, ctx};
	if(!tobs1Cont.isValid()) {
		return false;
	}
	SG::ReadHandle<xAOD::gFexJetRoIContainer> tobs2Cont{tobs2Key, ctx};
	if(!tobs1Cont.isValid()) {
		return false;
	}


	auto eventType = Monitored::Scalar<std::string>("EventType","DataTowers"); // always have data towers
	auto Signature = Monitored::Scalar<std::string>("Signature",label);
	auto tobMismatched = Monitored::Scalar<double>("tobMismatched",0);
	auto simReady = Monitored::Scalar<bool>("SimulationReady",simReadyFlag);

	//sorting mismatched data/sim TOB
    std::set<const xAOD::gFexJetRoI*> mismatchedTOBs;
	std::set<const xAOD::gFexJetRoI*> DataMismatchedTOBs;

	for (auto tob1 : *tobs1Cont) {
		bool isMatched = false;
		//bool isPartMatched = false;
		auto word1 = tob1->word();
		auto gfex1 = tob1->gFexType();
		for (auto tob2 : *tobs2Cont) {
			if(gfex1 == tob2->gFexType()) {
				if(word1 == tob2->word()) {
					isMatched = true;
					break;
				} /*else if( (word1&0x7FFFFF7F) == (tob2->word()&0x7FFFFF7F) ) {
				// matches after ignore saturation bit (31st bit) and status bit (7th bit) (first bit is 0th)
				isPartMatched=true;
				}*/
			}
		}
		if(!isMatched) {
            mismatchedTOBs.insert(tob1);
			DataMismatchedTOBs.insert(tob1); //saving only the mismatched data tob
		}
	}

    // also check for mismatches in tobs2 collection ... since may have extra tobs in tob2 that aren't in tobs1
    for (auto tob2 : *tobs2Cont) {
        bool isMatched = false;
        auto word = tob2->word();auto gfex = tob2->gFexType();
        for (auto tob1 : *tobs1Cont) {
            if(gfex == tob1->gFexType() && word == tob1->word()) {
                isMatched = true;
                break;
            }
        }
        if(!isMatched) {
            mismatchedTOBs.insert(tob2);
        }
    }



    auto lbn = Monitored::Scalar<ULong64_t>("LBN",GetEventInfo(ctx)->lumiBlock());
	if(!mismatchedTOBs.empty()) {
		// fill the debugging tree with all the words for this signature
		auto lbnString = Monitored::Scalar<std::string>("LBNString",std::to_string(GetEventInfo(ctx)->lumiBlock()));
		auto evtNumber = Monitored::Scalar<ULong64_t>("EventNumber",GetEventInfo(ctx)->eventNumber());
		auto l1id = Monitored::Scalar<int>("L1ID",GetEventInfo(ctx)->extendedLevel1ID());
		{
			std::scoped_lock lock(m_firstEventsMutex);
			auto itr = m_firstEvents.find(lbn);
			if(itr==m_firstEvents.end()) {
				m_firstEvents[lbn] = std::to_string(lbn)+":"+std::to_string(evtNumber);
				itr = m_firstEvents.find(lbn);
			}
			lbnString = itr->second;
		}
		std::vector<SortableTob> sortedDataTobs;
		std::vector<SortableTob> sortedSimTobs;

		// Fill your sorted TOB vectors
		fillVectors(tobs1Key, ctx, sortedDataTobs);
		fillVectors(tobs2Key, ctx, sortedSimTobs);

		auto dtobEtas    = Monitored::Collection("dataEtas", sortedDataTobs, [](const auto& t) { return t.eta; });
		auto dtobPhis    = Monitored::Collection("dataPhis", sortedDataTobs, [](const auto& t) { return t.phi; });
		auto dtobEts     = Monitored::Collection("dataEts",  sortedDataTobs, [](const auto& t) { return t.et; });
		auto dtobWord0s  = Monitored::Collection("dataWord0s", sortedDataTobs, [](const auto& t) { return t.word0; });

		auto stobEtas    = Monitored::Collection("simEtas", sortedSimTobs, [](const auto& t) { return t.eta; });
		auto stobPhis    = Monitored::Collection("simPhis", sortedSimTobs, [](const auto& t) { return t.phi; });
		auto stobEts     = Monitored::Collection("simEts",  sortedSimTobs, [](const auto& t) { return t.et; });
		auto stobWord0s  = Monitored::Collection("simWord0s", sortedSimTobs, [](const auto& t) { return t.word0; });

		/*if(msgLvl(MSG::DEBUG)) {
			std::cout << "LBN: " << ULong64_t(lbn) << " EventNumber: " << ULong64_t(evtNumber) << " L1ID: " << GetEventInfo(ctx)->extendedLevel1ID() << " signature: " << label << std::endl;
			std::cout << "  data : " << std::hex;
			for (const auto w: dword0s) std::cout << w << " ";
			std::cout << std::endl << "  sim  : ";
			for (const auto w: sword0s) std::cout << w << " ";
			std::cout << std::endl << std::dec;
		}*/
        tobMismatched=100;
        auto simReadyMismatch = Monitored::Scalar<bool>("SimulationReadyMismatch",simReady);
		fill("mismatches",simReadyMismatch,tobMismatched,lbn,lbnString,l1id,evtNumber,dtobEtas,dtobPhis,dtobEts,dtobWord0s,stobEtas,stobPhis,stobEts,stobWord0s,Signature,simReady,eventType);		
		if (label=="gJ" || label=="gLJ") {
            auto locIdx = Monitored::Scalar<std::string>("locIdx","");
			for(auto tob : DataMismatchedTOBs) {
				locIdx = std::to_string(tob->iEta()) + ":" + std::to_string(tob->iPhi());
                fill("mismatches"+label,lbn,locIdx); //only recording data eta,phi vs LBN
			}
		}
	} else {
        tobMismatched=0;
        fill("mismatches",lbn,Signature,tobMismatched,simReady,eventType);
    }

	return !mismatchedTOBs.empty();

}


bool GfexSimMonitorAlgorithm::compareGlobalRoI(const std::string& label,
		const SG::ReadHandleKey<xAOD::gFexGlobalRoIContainer>& tobs1Key,
		const SG::ReadHandleKey<xAOD::gFexGlobalRoIContainer>& tobs2Key,
		const EventContext& ctx, uint32_t tobMask) const {
	SG::ReadHandle<xAOD::gFexGlobalRoIContainer> tobs1Cont{tobs1Key, ctx};
	if(!tobs1Cont.isValid()) {
		return false;
	}
	SG::ReadHandle<xAOD::gFexGlobalRoIContainer> tobs2Cont{tobs2Key, ctx};
	if(!tobs1Cont.isValid()) {
		return false;
	}

	auto eventType = Monitored::Scalar<std::string>("EventType","DataTowers"); // always have data towers
	auto Signature = Monitored::Scalar<std::string>("Signature",label);
	auto tobMismatched = Monitored::Scalar<double>("tobMismatched",0);


	bool mismatches = (tobs1Cont->size()!=tobs2Cont->size());

	for(const auto tob1 : *tobs1Cont) {
		bool isMatched = false;
		uint32_t word1 = tob1->word()&tobMask;
		auto gfex1 = tob1->globalType();
		for (auto tob2 : *tobs2Cont) {
			if(gfex1 == tob2->globalType()) {
				if(word1 == (tob2->word()&tobMask)) {
					isMatched = true;
					break;
				}
			}
		}
		if(!isMatched) {
			mismatches = true;
		}
	}

    auto lbn = Monitored::Scalar<ULong64_t>("LBN",GetEventInfo(ctx)->lumiBlock());
	if(mismatches) {
		// fill the debugging tree with all the words for this signature
		auto lbnString = Monitored::Scalar<std::string>("LBNString",std::to_string(GetEventInfo(ctx)->lumiBlock()));
		auto evtNumber = Monitored::Scalar<ULong64_t>("EventNumber",GetEventInfo(ctx)->eventNumber());
		auto l1id = Monitored::Scalar<int>("L1ID",GetEventInfo(ctx)->extendedLevel1ID());
		{
			std::scoped_lock lock(m_firstEventsMutex);
			auto itr = m_firstEvents.find(lbn);
			if(itr==m_firstEvents.end()) {
				m_firstEvents[lbn] = std::to_string(lbn)+":"+std::to_string(evtNumber);
				itr = m_firstEvents.find(lbn);
			}
			lbnString = itr->second;
		}
		auto dtobMet1 = Monitored::Collection("dataTOB1", *tobs1Cont,[](const auto& tob) { return tob->METquantityOne(); });
		auto dtobMet2 = Monitored::Collection("dataTOB2", *tobs1Cont,[](const auto& tob) { return tob->METquantityTwo(); });
		auto dtobWord0s = Monitored::Collection("dataWord0s", *tobs1Cont,[](const auto& tob) { return tob->word(); });
		auto stobMet1 = Monitored::Collection("simTOB1", *tobs2Cont,[](const auto& tob) { return tob->METquantityOne(); });
		auto stobMet2 = Monitored::Collection("simTOB2", *tobs2Cont,[](const auto& tob) { return tob->METquantityTwo(); });
		auto stobWord0s = Monitored::Collection("simWord0s", *tobs2Cont,[](const auto& tob) { return tob->word(); });

		/*if(msgLvl(MSG::DEBUG)) {
			std::cout << "LBN: " << ULong64_t(lbn) << " EventNumber: " << ULong64_t(evtNumber) << " L1ID: " << GetEventInfo(ctx)->extendedLevel1ID() << " signature: " << label << std::endl;
			std::cout << "  data : " << std::hex;
			for (const auto w: dword0s) std::cout << w << " ";
			std::cout << std::endl << "  sim  : ";
			for (const auto w: sword0s) std::cout << w << " ";
			std::cout << std::endl << std::dec;
		}*/
        tobMismatched=100;
		
        auto simReadyMismatch = Monitored::Scalar<bool>("SimulationReadyMismatch",false/* global RoI not sim ready yet*/);
        fill("mismatches",simReadyMismatch,lbn,lbnString,evtNumber,l1id,dtobMet1,dtobMet2,dtobWord0s,stobMet1,stobMet2,stobWord0s,Signature,tobMismatched,eventType);
	} else {
        tobMismatched=0;
        fill("mismatches",lbn,Signature,tobMismatched,eventType);
    }


	return !mismatches;

}

