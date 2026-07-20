/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ALFA_CLINKALG_H
#define ALFA_CLINKALG_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

#include "ALFA_CLinkEv/ALFA_CLinkEvent.h"
#include "xAODForward/ALFADataContainer.h"
#include "ALFA_RawEv/ALFA_RawDataContainer.h"
#include "ALFA_RawEv/ALFA_DigitCollection.h"
#include "ALFA_RawEv/ALFA_ODDigitCollection.h"
#include "ALFA_LocRecEv/ALFA_LocRecEvCollection.h"
#include "ALFA_LocRecEv/ALFA_LocRecODEvCollection.h"
#include "ALFA_LocRecCorrEv/ALFA_LocRecCorrEvCollection.h"
#include "ALFA_LocRecCorrEv/ALFA_LocRecCorrODEvCollection.h"

#define EVCOLLNAME_XAODALFADATACONTAINER "ALFADataContainer"
#define EVCOLLNAME_XAODALFADATAAUXCONTAINER "ALFADataContainerAux."

#define RPOTSCNT 8
#define MDLAYERSCNT	2
#define MDPLATESCNT	10
#define MDFIBERSCNT	64
#define ODLAYERSCNT	2
#define ODPLATESCNT	3
#define ODFIBERSCNT	15

#define TRIGPATCNT 16
#define BLMCNT 6
#define HVCHANNELCNT 216
#define RADMONCNT 4
#define FECNFTHRESHLOLDCNT 3
#define FECNFGAINCNT 5
#define TRIGSETCNT 6
#define TRIGSETLATENCYCNT 3

#define MAXNUMTRACKS 100
#define MAXNUMGLOBTRACKS 100
#define MAXNUMGENPARTICLES 100
#define MAXPILEUP 500

#include  <string>

enum eRecType { ERC_LOCUNCORRECTED, ERC_LOCCORRECTED, ERC_GLOBAL };

class ALFA_CLinkAlg : public AthAlgorithm
{
public:
	ALFA_CLinkAlg (const std::string& name, ISvcLocator* pSvcLocator);
	virtual ~ALFA_CLinkAlg();

private:
	int m_nDataType; //data type (simulation or real data) using in the local reconstruction
	int m_nProcessingMode; //1=offline, 2=online

public:
	virtual StatusCode initialize() override;
	virtual StatusCode execute(const EventContext& ctx) override;
	virtual StatusCode finalize() override;

public:
	StatusCode LoadAllEventData(const EventContext& ctx, ALFA_CLinkEvent& dataEvent) const;
private:
        unsigned long long CalcDCSId (const EventContext& ctx,
                                      const SG::ReadCondHandleKey<CondAttrListCollection>& key) const;
	StatusCode CalcAllDCSIds (const EventContext& ctx,
                                  DCSID& pDCSIds) const;

private:
	StatusCode GenerateXAOD(const EventContext& ctx);
	StatusCode FillXAOD_TrackingData(const EventContext& ctx, xAOD::ALFADataContainer& xAODContainer);
        StatusCode FillXAOD_HeaderData(const EventContext& ctx, xAOD::ALFADataContainer& xAODContainer);
	void ClearXAODTrackingData(const int nMaxTrackCnt, eRecType eType);
	void ClearXAODHeaderData();

private:
	//xAOD variables: LocRecEvCollection & LocRecODEvCollection
	int m_nMaxTrackCnt;
	std::vector<float> m_vecXDetCS;
	std::vector<float> m_vecYDetCS;
	std::vector<int> m_vecDetectorPartID;
	std::vector<float> m_vecOverU;
	std::vector<float> m_vecOverV;
	std::vector<float> m_vecOverY;
	std::vector<int> m_vecNumU;
	std::vector<int> m_vecNumV;
	std::vector<int> m_vecNumY;
	std::vector<int> m_vecMDFibSel;
	std::vector<int> m_vecODFibSel;

	//xAOD variables: LocRecCorrEvCollection & LocRecCorrODEvCollection
	std::vector<float> m_vecXLhcCS;
	std::vector<float> m_vecYLhcCS;
	std::vector<float> m_vecZLhcCS;
	std::vector<float> m_vecXRPotCS;
	std::vector<float> m_vecYRPotCS;
	std::vector<float> m_vecXStatCS;
	std::vector<float> m_vecYStatCS;
	std::vector<float> m_vecXBeamCS;
	std::vector<float> m_vecYBeamCS;

	//RawDataContainer
	std::vector<int> m_vecScaler;
	//int m_nBCId;
	//int m_nTimeStamp;
	//int m_nTimeStamp_ns;
	std::vector<int> m_vecTrigPat;

	//DigitCollection
	std::vector<int> m_vecMDFiberHits;
	std::vector<int> m_vecMDMultiplicity;

	//ODDigitCollection
	std::vector<int> m_vecODFiberHitsPos;
	std::vector<int> m_vecODFiberHitsNeg;
	std::vector<int> m_vecODMultiplicityPos;
	std::vector<int> m_vecODMultiplicityNeg;

        SG::ReadCondHandleKey<CondAttrListCollection> m_BLMKey
          { this, "BLMKey", DCSCOLLNAME_BLM, "BLM conditions key" };
        SG::ReadCondHandleKey<CondAttrListCollection> m_HVChannelKey
          { this, "HVChannelKey", DCSCOLLNAME_HVCHANNEL, "HV channel conditions key" };
        SG::ReadCondHandleKey<CondAttrListCollection> m_localMonitoringKey
          { this, "LocalMonitoringKey", DCSCOLLNAME_LOCALMONITORING, "Local monitoring conditions key" };
        SG::ReadCondHandleKey<CondAttrListCollection> m_movementKey
          { this, "MovementKey", DCSCOLLNAME_LOCALMONITORING, "Movement conditions key" };
        SG::ReadCondHandleKey<CondAttrListCollection> m_radmonKey
          { this, "RadmonKey", DCSCOLLNAME_RADMON, "Radmon conditions key" };
        SG::ReadCondHandleKey<CondAttrListCollection> m_triggerRatesKey
          { this, "TriggerRatesKey", DCSCOLLNAME_TRIGGERRATES, "Trigger rates conditions key" };
        SG::ReadCondHandleKey<CondAttrListCollection> m_FEConfigurationKey
          { this, "FEConfigurationKey", DCSCOLLNAME_FECONFIGURATION, "FE configuration conditions key" };
        SG::ReadCondHandleKey<CondAttrListCollection> m_triggerSettingsKey
          { this, "TriggerSettingsKey", DCSCOLLNAME_TRIGGERSETTINGS, "Trigger settings conditions key" };

        SG::ReadHandleKey<ALFA_RawDataContainer> m_rawDataContKey
          { this, "RawDataContKey", EVCOLLNAME_RAWDATA, "SG key for raw data container" };
        SG::ReadHandleKey<ALFA_DigitCollection> m_digitCollKey
          { this, "DigitCollKey", EVCOLLNAME_DIGIT, "SG key for digit collection" };
        SG::ReadHandleKey<ALFA_ODDigitCollection> m_ODDigitCollKey
          { this, "ODDigitCollKey", EVCOLLNAME_ODDIGIT, "SG key for OD digit collection" };
        SG::ReadHandleKey<ALFA_LocRecEvCollection> m_locRecEvCollKey
          { this, "LocRecEvCollectionKey", EVCOLLNAME_LOCREC, "SG key for LocRecEv collection" };
        SG::ReadHandleKey<ALFA_LocRecODEvCollection> m_locRecODEvCollKey
          { this, "LocRecEvODCollectionKey", EVCOLLNAME_LOCRECOD, "SG key for LocRecEvOD collection" };
        SG::ReadHandleKey<ALFA_LocRecCorrEvCollection> m_locRecCorrEvCollKey
          { this, "LocRecCorrEvCollectionKey", EVCOLLNAME_LOCRECCORR, "SG key for LocRecCorrEv collection" };
        SG::ReadHandleKey<ALFA_LocRecCorrODEvCollection> m_locRecCorrODEvCollKey
          { this, "LocRecCorrODEvCollectionKey", EVCOLLNAME_LOCRECCORROD, "SG key for LocRecCorrODEv collection" };

        SG::WriteHandleKey<ALFA_CLinkEvent> m_clinkEventKey
          { this, "CLinkEventKey", "ALFA_CLinkEvent", "SG key for output CLinkEvent" };
        SG::WriteHandleKey<xAOD::ALFADataContainer> m_xaodDataKey
          { this, "xAODDataKey", EVCOLLNAME_XAODALFADATACONTAINER, "SG key for output xAOD::ALFADataContainer" };
};

#endif // ALFA_CLINKALG_H
