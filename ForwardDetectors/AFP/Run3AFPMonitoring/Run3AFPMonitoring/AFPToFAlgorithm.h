/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#ifndef AFPTOFALGORITHM_H
#define AFPTOFALGORITHM_H

#include "AthenaMonitoring/AthMonitorAlgorithm.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "StoreGate/ReadHandleKey.h"
#include "xAODForward/AFPToFHitContainer.h"
#include "xAODForward/AFPToFHit.h"
#include "xAODForward/AFPTrackContainer.h"
#include "xAODForward/AFPTrack.h"
#include "LumiBlockData/BunchCrossingCondData.h"

#include "TRandom3.h"

class AFPToFAlgorithm : public AthMonitorAlgorithm {
public:
	AFPToFAlgorithm( const std::string& name, ISvcLocator* pSvcLocator );
	virtual ~AFPToFAlgorithm();
	virtual StatusCode initialize() override;
	virtual StatusCode fillHistograms( const EventContext& ctx ) const override;
	virtual StatusCode fillHistograms_crossBarDeltaT( const xAOD::AFPTrackContainer&, const xAOD::AFPToFHitContainer& ) const;

private:
	std::map<std::string,int> m_StationNamesGroup;
	std::map<std::string,int> m_TrainsToFGroup;
	std::map<std::string,std::map<std::string,int>> m_BarsInTrainsA;
	std::map<std::string,std::map<std::string,int>> m_BarsInTrainsC;
	std::map<std::string,int> m_GroupChanCombDeltaT;
	SG::ReadHandleKey<xAOD::AFPToFHitContainer> m_afpToFHitContainerKey;
	SG::ReadHandleKey<xAOD::AFPTrackContainer> m_afpTrackContainerKey;
	SG::ReadCondHandleKey<BunchCrossingCondData> m_bunchCrossingKeyToF{this, "BunchCrossingKey", "BunchCrossingData", "Key BunchCrossing CDO" };

protected:
	// Only 0 and 3 are ToF stations (farAside and farCside)
	std::vector<std::string> m_stationNamesToF = { "farAside", "nearAside" , "nearCside" , "farCside" };
	std::vector<std::string> m_trainsToF = { "train0", "train1" , "train2" , "train3" };
	
	std::vector<std::string> m_trainsToFA = { "T0", "T1" , "T2" , "T3" };
	std::vector<std::string> m_trainsToFC = { "T0", "T1" , "T2" , "T3" };
	std::vector<std::string> m_barsToF = { "A", "B" , "C" , "D" };

	std::vector<std::string> m_chanComb = {
		"0AB", "0AC", "0AD", "0BC", "0BD", "0CD",
		"1AB", "1AC", "1AD", "1BC", "1BD", "1CD",
		"2AB", "2AC", "2AD", "2BC", "2BD", "2CD",
		"3AB", "3AC", "3AD", "3BC", "3BD", "3CD"};
};
#endif

