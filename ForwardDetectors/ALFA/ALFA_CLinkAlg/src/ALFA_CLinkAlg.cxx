/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "AthenaKernel/errorcheck.h"
#include "StoreGate/ReadCondHandle.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

#include "xAODForward/ALFAData.h"
#include "xAODForward/ALFADataContainer.h"
#include "xAODForward/ALFADataAuxContainer.h"
#include "ALFA_CLinkAlg.h"

using namespace std;

ALFA_CLinkAlg::ALFA_CLinkAlg(const std::string& name, ISvcLocator* pSvcLocator)
	: AthAlgorithm(name, pSvcLocator)
{
	// data type using in the local reconstruction
	// for the simulation data the value is 0, for the real data the value is 1. Unset value is -1
	declareProperty("DataType", m_nDataType=1, "data type using in the local reconstruction");
	declareProperty("ProcessingMode", m_nProcessingMode=2, "Processing mode, 1=offline, 2=online");

	m_nMaxTrackCnt=0;
}


ALFA_CLinkAlg::~ALFA_CLinkAlg()
{

}

StatusCode ALFA_CLinkAlg::initialize()
{
	ATH_MSG_DEBUG ("ALFA_CLinkAlg::initialize()");

        ATH_CHECK( m_BLMKey.initialize (m_nDataType==1) );
        ATH_CHECK( m_HVChannelKey.initialize (m_nDataType==1) );
        ATH_CHECK( m_localMonitoringKey.initialize (m_nDataType==1) );
        ATH_CHECK( m_movementKey.initialize (m_nDataType==1) );
        ATH_CHECK( m_radmonKey.initialize (m_nDataType==1) );
        ATH_CHECK( m_triggerRatesKey.initialize (m_nDataType==1) );
        ATH_CHECK( m_FEConfigurationKey.initialize (m_nDataType==1) );
        ATH_CHECK( m_triggerSettingsKey.initialize (m_nDataType==1) );

        ATH_CHECK( m_rawDataContKey.initialize (m_nDataType==1) );
        ATH_CHECK( m_digitCollKey.initialize() );
        ATH_CHECK( m_ODDigitCollKey.initialize() );
        ATH_CHECK( m_locRecEvCollKey.initialize() );
        ATH_CHECK( m_locRecODEvCollKey.initialize() );
        ATH_CHECK( m_locRecCorrEvCollKey.initialize() );
        ATH_CHECK( m_locRecCorrODEvCollKey.initialize() );

        ATH_CHECK( m_clinkEventKey.initialize() );
        ATH_CHECK( m_xaodDataKey.initialize() );

	return StatusCode::SUCCESS;
}

StatusCode ALFA_CLinkAlg::execute(const EventContext& ctx)
{
	ATH_MSG_DEBUG ("ALFA_CLinkAlg::execute()");


	auto pDataEvent = std::make_unique<ALFA_CLinkEvent>();

        ATH_CHECK( LoadAllEventData(ctx, *pDataEvent) );
        if (m_nDataType==1) {
          DCSID DCSIds;
          ATH_CHECK( CalcAllDCSIds (ctx, DCSIds) );
          pDataEvent->SetDCSFolderIDs(&DCSIds);
        }
        SG::WriteHandle<ALFA_CLinkEvent> clinkEventH (m_clinkEventKey, ctx);
        ATH_CHECK( clinkEventH.record (std::move (pDataEvent)) );
        ATH_CHECK( GenerateXAOD (ctx) );

        return StatusCode::SUCCESS;
}

StatusCode ALFA_CLinkAlg::finalize()
{
	ATH_MSG_DEBUG ("ALFA_CLinkAlg::finalize()");

	return StatusCode::SUCCESS;
}

StatusCode ALFA_CLinkAlg::LoadAllEventData(const EventContext& ctx,
                                           ALFA_CLinkEvent& dataEvent) const
{
	//RawDataContainer
	if (m_nDataType==1)
	{
                SG::ReadHandle<ALFA_RawDataContainer> rawDataCont (m_rawDataContKey, ctx);
		if(!rawDataCont.isValid())
		{
                        ATH_MSG_WARNING( "Container '"<<EVCOLLNAME_RAWDATA<<"' not found" );
			//return StatusCode::FAILURE;
		}
		else ATH_CHECK(dataEvent.AddLink(EDVT_RAWDATAEVCOLLECTION, rawDataCont.cptr()));
	}

	//DigitCollection
        SG::ReadHandle<ALFA_DigitCollection> digitColl (m_digitCollKey, ctx);
	if(!digitColl.isValid()) {
                ATH_MSG_WARNING( "Container '"<<EVCOLLNAME_DIGIT<<"' not found" );
		//return StatusCode::FAILURE;
	}
	else ATH_CHECK(dataEvent.AddLink(EDVT_DIGITCOLLECTION, digitColl.cptr()));

	//ODDigitCollection
        SG::ReadHandle<ALFA_ODDigitCollection> odDigitColl (m_ODDigitCollKey, ctx);
	if(!odDigitColl.isValid()) {
                ATH_MSG_WARNING( "Container '"<<EVCOLLNAME_ODDIGIT<<"' not found" );
		//return StatusCode::FAILURE;
	}
	else ATH_CHECK(dataEvent.AddLink(EDVT_ODDIGITCOLLECTION, odDigitColl.cptr()));

	//LocRecEvCollection
        SG::ReadHandle<ALFA_LocRecEvCollection> locRecEvColl (m_locRecEvCollKey, ctx);
	if(!locRecEvColl.isValid()) {
                ATH_MSG_WARNING( "Container '"<<EVCOLLNAME_LOCREC<<"' not found" );
		//return StatusCode::FAILURE;
	}
	else ATH_CHECK(dataEvent.AddLink(EDVT_LOCRECEVCOLLECTION, locRecEvColl.cptr()));

	//LocRecODEvCollection
        SG::ReadHandle<ALFA_LocRecODEvCollection> locRecODEvColl (m_locRecODEvCollKey, ctx);
	if(!locRecODEvColl.isValid()) {
                ATH_MSG_WARNING( "Container '"<<EVCOLLNAME_LOCRECOD<<"' not found" );
		//return StatusCode::FAILURE;
	}
	else CHECK(dataEvent.AddLink(EDVT_LOCRECODEVCOLLECTION, locRecODEvColl.cptr()));

	//LocRecCorrEvCollection
        SG::ReadHandle<ALFA_LocRecCorrEvCollection> locRecCorrEvColl (m_locRecCorrEvCollKey, ctx);
	if(!locRecCorrEvColl.isValid()) {
                ATH_MSG_WARNING( "Container '"<<EVCOLLNAME_LOCRECCORR<<"' not found" );
		//return StatusCode::FAILURE;
	}
	else CHECK(dataEvent.AddLink(EDVT_LOCRECCORREVCOLLECTION, locRecCorrEvColl.cptr()));

	//LocRecCorrODEvCollection
        SG::ReadHandle<ALFA_LocRecCorrODEvCollection> locRecCorrODEvColl (m_locRecCorrODEvCollKey, ctx);
	if(!locRecCorrODEvColl.isValid()) {
                ATH_MSG_WARNING("Container '"<<EVCOLLNAME_LOCRECCORROD<<"' not found" );
		//return StatusCode::FAILURE;
	}
	else CHECK(dataEvent.AddLink(EDVT_LOCRECCORRODEVCOLLECTION, locRecCorrODEvColl.cptr()));

	return StatusCode::SUCCESS;

}

unsigned long long
ALFA_CLinkAlg::CalcDCSId (const EventContext& ctx,
                          const SG::ReadCondHandleKey<CondAttrListCollection>& key) const
{
	unsigned long long ullID;
        std::string Folder;

        SG::ReadCondHandle<CondAttrListCollection> h (key, ctx);
        EventIDRange range;
        if (!h.range (range)) return 0;

	// Construct the ID:
        EventIDBase time = range.start();
	if(time.isRunLumi()){
		ullID=static_cast<unsigned long long>(((time.run_number()&0xffff)<<16)|(time.lumi_block()&0xffff));
	}
	else if(time.isTimeStamp()){
                ullID = time.time_stamp();
                ullID <<= 32;
                ullID |= time.time_stamp_ns_offset();
	}
	else{
		ullID=0;
	}

	return ullID;
}

StatusCode ALFA_CLinkAlg::CalcAllDCSIds (const EventContext& ctx,
                                         DCSID& DCSIds) const
{
	bool bRes=true;

        bRes&=(DCSIds.ullBlmID=CalcDCSId(ctx, m_BLMKey))>0;
        bRes&=(DCSIds.ullHVChannelID=CalcDCSId(ctx, m_HVChannelKey))>0;
        bRes&=(DCSIds.ullLocalMonitoringID=CalcDCSId(ctx, m_localMonitoringKey))>0;
        bRes&=(DCSIds.ullMovementID=CalcDCSId(ctx, m_movementKey))>0;
        bRes&=(DCSIds.ullRadMonID=CalcDCSId(ctx, m_radmonKey))>0;
        bRes&=(DCSIds.ullTriggerRatesID=CalcDCSId(ctx, m_triggerRatesKey))>0;
        bRes&=(DCSIds.ullFEConfigurationID=CalcDCSId(ctx, m_FEConfigurationKey))>0;
        bRes&=(DCSIds.ullTriggerSettingsID=CalcDCSId(ctx, m_triggerSettingsKey))>0;
	return bRes? StatusCode::SUCCESS:StatusCode::FAILURE;
}

StatusCode ALFA_CLinkAlg::GenerateXAOD(const EventContext& ctx)
{
	auto pxAODContainer = std::make_unique<xAOD::ALFADataContainer>();
	auto pxAODAuxContainer = std::make_unique<xAOD::ALFADataAuxContainer>();
	pxAODContainer->setStore(pxAODAuxContainer.get());

	CHECK(FillXAOD_TrackingData(ctx, *pxAODContainer));
	CHECK(FillXAOD_HeaderData(ctx, *pxAODContainer));

        SG::WriteHandle<xAOD::ALFADataContainer> xaodData (m_xaodDataKey, ctx);
        ATH_CHECK( xaodData.record (std::move(pxAODContainer),
                                    std::move(pxAODAuxContainer)) );

	return StatusCode::SUCCESS;
}

StatusCode ALFA_CLinkAlg::FillXAOD_TrackingData(const EventContext& ctx,
                                                xAOD::ALFADataContainer& xAODContainer)
{
	unsigned int i;
	int nPotID, nSideID, nODSign;
	int arrTrackCntPerRPot[RPOTSCNT];
	vector<int> vecFiberSel;

	//LocRecEvCollection & LocRecODEvCollection
        SG::ReadHandle<ALFA_LocRecEvCollection> locRecEvColl (m_locRecEvCollKey, ctx);
        SG::ReadHandle<ALFA_LocRecODEvCollection> locRecODEvColl (m_locRecODEvCollKey, ctx);

	if(locRecEvColl.isValid() && locRecODEvColl.isValid())
	{
		m_nMaxTrackCnt=1;
		// resolve max track count from LocRecEvCollection
		memset(&arrTrackCntPerRPot[0],0,sizeof(arrTrackCntPerRPot));
		for(const ALFA_LocRecEvent* locRecEvent : *locRecEvColl)
		{
			nPotID=locRecEvent->getPotNum();
			arrTrackCntPerRPot[nPotID]++;
		}
		for(const ALFA_LocRecODEvent* locRecODEvent : *locRecODEvColl)
		{
			nPotID=locRecODEvent->getPotNum();
			arrTrackCntPerRPot[nPotID]++;
		}
		for(i=0;i<RPOTSCNT;i++){
			if(arrTrackCntPerRPot[i]>m_nMaxTrackCnt) m_nMaxTrackCnt=arrTrackCntPerRPot[i];
		}

		memset(&arrTrackCntPerRPot[0],0,sizeof(arrTrackCntPerRPot));
		ClearXAODTrackingData(m_nMaxTrackCnt,ERC_LOCUNCORRECTED);

		//fill data - LocRecEvCollection
		vecFiberSel.clear();
		for(const ALFA_LocRecEvent* locRecEvent : *locRecEvColl)
		{
			nPotID=locRecEvent->getPotNum();

			(m_vecDetectorPartID)[nPotID*m_nMaxTrackCnt+arrTrackCntPerRPot[nPotID]]=1;
			(m_vecXDetCS)[nPotID*m_nMaxTrackCnt+arrTrackCntPerRPot[nPotID]]=locRecEvent->getXposition();
			(m_vecYDetCS)[nPotID*m_nMaxTrackCnt+arrTrackCntPerRPot[nPotID]]=locRecEvent->getYposition();

			(m_vecOverU)[nPotID*m_nMaxTrackCnt+arrTrackCntPerRPot[nPotID]]=locRecEvent->getOverU();
			(m_vecOverV)[nPotID*m_nMaxTrackCnt+arrTrackCntPerRPot[nPotID]]=locRecEvent->getOverV();
			(m_vecNumU)[nPotID*m_nMaxTrackCnt+arrTrackCntPerRPot[nPotID]]=locRecEvent->getNumU();
			(m_vecNumV)[nPotID*m_nMaxTrackCnt+arrTrackCntPerRPot[nPotID]]=locRecEvent->getNumV();

			vecFiberSel=locRecEvent->getFibSel();
			for(i=0;i<vecFiberSel.size();i++)
			{
				(m_vecMDFibSel)[nPotID*m_nMaxTrackCnt*MDLAYERSCNT*MDPLATESCNT+arrTrackCntPerRPot[nPotID]*MDLAYERSCNT*MDPLATESCNT+i]=vecFiberSel[i];
			}

			arrTrackCntPerRPot[nPotID]++;
		}

		//fill data - LocRecODEvCollection
		vecFiberSel.clear();
		for(const ALFA_LocRecODEvent* locRecODEvent : *locRecODEvColl)
		{
			nPotID=locRecODEvent->getPotNum();
			nSideID=locRecODEvent->getSide();

			nODSign=(nSideID==0)? -1:1;
			(m_vecDetectorPartID)[nPotID*m_nMaxTrackCnt+arrTrackCntPerRPot[nPotID]]=(nSideID==0)? 3:2;
			(m_vecXDetCS)[nPotID*m_nMaxTrackCnt+arrTrackCntPerRPot[nPotID]]=nODSign*22.0;
			(m_vecYDetCS)[nPotID*m_nMaxTrackCnt+arrTrackCntPerRPot[nPotID]]=locRecODEvent->getYposition();
			(m_vecOverY)[nPotID*m_nMaxTrackCnt+arrTrackCntPerRPot[nPotID]]=locRecODEvent->getOverY();
			(m_vecNumY)[nPotID*m_nMaxTrackCnt+arrTrackCntPerRPot[nPotID]]=locRecODEvent->getNumY();

			vecFiberSel=locRecODEvent->getFibSel();
			for(i=0;i<vecFiberSel.size();i++)
			{
				(m_vecODFibSel)[nPotID*m_nMaxTrackCnt*ODPLATESCNT+arrTrackCntPerRPot[nPotID]*ODPLATESCNT+i]=vecFiberSel[i];
			}

			arrTrackCntPerRPot[nPotID]++;
		}

	}
	else
	{
                ATH_MSG_WARNING( "Cannot find '"<< EVCOLLNAME_LOCREC <<"' or '"<<EVCOLLNAME_LOCRECOD<<"' collection" );
		//return StatusCode::FAILURE;
	}

	//LocRecCorrEvCollection && LocRecCorrODEvCollection
        SG::ReadHandle<ALFA_LocRecCorrEvCollection> locRecCorrEvColl (m_locRecCorrEvCollKey, ctx);
        SG::ReadHandle<ALFA_LocRecCorrODEvCollection> locRecCorrODEvColl (m_locRecCorrODEvCollKey, ctx);

	if(locRecCorrEvColl.isValid() && locRecCorrODEvColl.isValid())
	{
		memset(&arrTrackCntPerRPot[0],0,sizeof(arrTrackCntPerRPot));
		ClearXAODTrackingData(m_nMaxTrackCnt,ERC_LOCCORRECTED);

		//fill data - LocRecCorrEvCollection - ONLY DetCS for now (TODO rest)
		for(const ALFA_LocRecCorrEvent* locRecCorrEvent : *locRecCorrEvColl)
		{
			nPotID=locRecCorrEvent->getPotNum();

			(m_vecXLhcCS)[nPotID*m_nMaxTrackCnt+arrTrackCntPerRPot[nPotID]]=locRecCorrEvent->getXpositionLHC();
			(m_vecYLhcCS)[nPotID*m_nMaxTrackCnt+arrTrackCntPerRPot[nPotID]]=locRecCorrEvent->getYpositionLHC();
			(m_vecZLhcCS)[nPotID*m_nMaxTrackCnt+arrTrackCntPerRPot[nPotID]]=locRecCorrEvent->getZpositionLHC();

			(m_vecXRPotCS)[nPotID*m_nMaxTrackCnt+arrTrackCntPerRPot[nPotID]]=locRecCorrEvent->getXpositionPot();
			(m_vecYRPotCS)[nPotID*m_nMaxTrackCnt+arrTrackCntPerRPot[nPotID]]=locRecCorrEvent->getYpositionPot();

			(m_vecXStatCS)[nPotID*m_nMaxTrackCnt+arrTrackCntPerRPot[nPotID]]=locRecCorrEvent->getXpositionStat();
			(m_vecYStatCS)[nPotID*m_nMaxTrackCnt+arrTrackCntPerRPot[nPotID]]=locRecCorrEvent->getYpositionStat();

			(m_vecXBeamCS)[nPotID*m_nMaxTrackCnt+arrTrackCntPerRPot[nPotID]]=locRecCorrEvent->getXpositionBeam();
			(m_vecYBeamCS)[nPotID*m_nMaxTrackCnt+arrTrackCntPerRPot[nPotID]]=locRecCorrEvent->getYpositionBeam();

			arrTrackCntPerRPot[nPotID]++;
		}

		//fill data - LocRecCorrODEvCollection - ONLY DetCS for now (TODO rest)
		for(const ALFA_LocRecCorrODEvent* locRecCorrODEvent : *locRecCorrODEvColl)
		{
			nPotID=locRecCorrODEvent->getPotNum();
			nSideID=locRecCorrODEvent->getSide();

			nODSign=(nSideID==0)? -1:1;
			(m_vecXLhcCS)[nPotID*m_nMaxTrackCnt+arrTrackCntPerRPot[nPotID]]=nODSign*22.0;
			(m_vecYLhcCS)[nPotID*m_nMaxTrackCnt+arrTrackCntPerRPot[nPotID]]=locRecCorrODEvent->getYpositionLHC();
			(m_vecZLhcCS)[nPotID*m_nMaxTrackCnt+arrTrackCntPerRPot[nPotID]]=locRecCorrODEvent->getZpositionLHC();

			(m_vecXRPotCS)[nPotID*m_nMaxTrackCnt+arrTrackCntPerRPot[nPotID]]=nODSign*22.0;
			(m_vecYRPotCS)[nPotID*m_nMaxTrackCnt+arrTrackCntPerRPot[nPotID]]=locRecCorrODEvent->getYpositionPot();

			(m_vecXStatCS)[nPotID*m_nMaxTrackCnt+arrTrackCntPerRPot[nPotID]]=nODSign*22.0;
			(m_vecYStatCS)[nPotID*m_nMaxTrackCnt+arrTrackCntPerRPot[nPotID]]=locRecCorrODEvent->getYpositionStat();

			(m_vecXBeamCS)[nPotID*m_nMaxTrackCnt+arrTrackCntPerRPot[nPotID]]=nODSign*22.0;
			(m_vecYBeamCS)[nPotID*m_nMaxTrackCnt+arrTrackCntPerRPot[nPotID]]=locRecCorrODEvent->getYpositionBeam();

			arrTrackCntPerRPot[nPotID]++;
		}
	}
	else
	{
                ATH_MSG_WARNING( "Cannot find '"<< EVCOLLNAME_LOCRECCORR <<"' or '"<<EVCOLLNAME_LOCRECCORROD<<"' collection" );
		//return StatusCode::FAILURE;
	}

        auto pData =
          xAODContainer.push_back(std::make_unique<xAOD::ALFAData>());

	//LocRecEvCollection & LocRecODEvCollection
	pData->setXDetCS(m_vecXDetCS);
	pData->setYDetCS(m_vecYDetCS);
	pData->setDetectorPartID(m_vecDetectorPartID);
	pData->setMaxTrackCnt(m_nMaxTrackCnt);
	pData->setOverU(m_vecOverU);
	pData->setOverV(m_vecOverV);
	pData->setOverY(m_vecOverY);
	pData->setNumU(m_vecNumU);
	pData->setNumV(m_vecNumV);
	pData->setNumY(m_vecNumY);
	pData->setMDFibSel(m_vecMDFibSel);
	pData->setODFibSel(m_vecODFibSel);

	//LocRecCorrEvCollection & LocRecCorrODEvCollection
	pData->setXLhcCS(m_vecXLhcCS);
	pData->setYLhcCS(m_vecYLhcCS);
	pData->setZLhcCS(m_vecZLhcCS);
	pData->setXRPotCS(m_vecXRPotCS);
	pData->setYRPotCS(m_vecYRPotCS);
	pData->setXStatCS(m_vecXStatCS);
	pData->setYStatCS(m_vecYStatCS);
	pData->setXBeamCS(m_vecXBeamCS);
	pData->setYBeamCS(m_vecYBeamCS);

	return StatusCode::SUCCESS;
}

StatusCode ALFA_CLinkAlg::FillXAOD_HeaderData(const EventContext& ctx,
                                              xAOD::ALFADataContainer& xAODContainer)
{
	unsigned int i;
	int nPotID, nPlateID, nFiberID, nSideID;
	ClearXAODHeaderData();

	if (m_nDataType==1)
	{
		//DCS IDs
		/*
		*m_pullDCSBlmID=DataEvent.GetDCSFolderID(EDCSI_BLM);
		*m_pullDCSHVChannelID=DataEvent.GetDCSFolderID(EDCSI_HVCHANNEL);
		*m_pullDCSLocalMonitoringID=DataEvent.GetDCSFolderID(EDCSI_LOCALMONITORING);
		*m_pullDCSMovementID=DataEvent.GetDCSFolderID(EDCSI_MOVEMENT);
		*m_pullDCSRadMonID=DataEvent.GetDCSFolderID(EDCSI_RADMON);
		*m_pullDCSTriggerRatesID=DataEvent.GetDCSFolderID(EDCSI_TRIGGERRATES);
		*m_pullDCSFEConfigurationID=DataEvent.GetDCSFolderID(EDCSI_FECONFIGURATION);
		*m_pullDCSTriggerSettingsID=DataEvent.GetDCSFolderID(EDCSI_TRIGGERSETTINGS);*/

		//RawDataContainer
                SG::ReadHandle<ALFA_RawDataContainer> rawDataCont (m_rawDataContKey, ctx);
		if(rawDataCont.isValid())
		{
			//m_nTimeStamp=pRawDataColl->GetTimeStamp();
			//m_nTimeStamp_ns=pRawDataColl->GetTimeStampns();
			//m_nBCId=pRawDataColl->GetBCId();

			vector<bool> vecRPPattern;
                        for (const ALFA_RawDataCollection* rawDataColl : *rawDataCont)
			{
				nPotID=rawDataColl->GetMBId_POT();
				(m_vecScaler)[nPotID-1]=rawDataColl->Get_scaler_POT();

				vecRPPattern=rawDataColl->Get_pattern_POT();
				for(i=0;i<vecRPPattern.size();i++){
					if(i<RPOTSCNT*TRIGPATCNT) (m_vecTrigPat)[(nPotID-1)*TRIGPATCNT+i]=vecRPPattern[vecRPPattern.size()-(i+1)];
				}
			}
		}
		else
		{
                        ATH_MSG_WARNING( "Cannot find '"<< EVCOLLNAME_RAWDATA <<"' collection" );
			//return StatusCode::FAILURE;
		}
	}

	//DigitCollection
        SG::ReadHandle<ALFA_DigitCollection> digitColl (m_digitCollKey, ctx);
	if(digitColl.isValid())
	{
                for (const ALFA_Digit* digit : *digitColl)
		{
			nPotID=digit->getStation(); //in range 0-7
			nPlateID=digit->getPlate(); //indexed from 0
			nFiberID=digit->getFiber(); //indexed from 0

			if(nPotID<RPOTSCNT && nPlateID<(MDLAYERSCNT*MDPLATESCNT) && nFiberID<MDFIBERSCNT)
			{
				(m_vecMDFiberHits)[(nPotID*MDLAYERSCNT*MDPLATESCNT*MDFIBERSCNT)+(nPlateID*MDFIBERSCNT)+nFiberID]=1;
				(m_vecMDMultiplicity)[(nPotID*MDLAYERSCNT*MDPLATESCNT)+nPlateID]++;
			}
			else
			{
                                ATH_MSG_ERROR( "Index exceed array size for [RPotID, nPlateID, nFiberID]= ["<<nPotID<<", "<<nPlateID<<", "<<nFiberID<<"]" );
				//return StatusCode::FAILURE;
			}
		}
	}
	else{
                ATH_MSG_WARNING( "Cannot find '"<< EVCOLLNAME_DIGIT <<"' collection" );
		//return StatusCode::FAILURE;
	}

	//ODDigitCollection
        SG::ReadHandle<ALFA_ODDigitCollection> odDigitColl (m_ODDigitCollKey, ctx);
	if(odDigitColl.isValid())
	{
                for (const ALFA_ODDigit* oddigit : *odDigitColl)
		{
			nPotID=oddigit->getStation(); //in range 0-7
			nPlateID=oddigit->getPlate(); //indexed from 0
			nSideID=oddigit->getSide();   //indexed from 0
			nFiberID=oddigit->getFiber(); //indexed from 0

			if(nPotID<RPOTSCNT && nPlateID<(ODPLATESCNT) && nFiberID<ODLAYERSCNT*ODFIBERSCNT)
			{
				if(nSideID==0){ //right side
					(m_vecODFiberHitsNeg)[(nPotID*ODPLATESCNT*ODLAYERSCNT*ODFIBERSCNT)+(nPlateID*ODLAYERSCNT*ODFIBERSCNT)+nFiberID]=1;
					(m_vecODMultiplicityNeg)[(nPotID*ODPLATESCNT)+nPlateID]++;
				}
				else{ //left side
					(m_vecODFiberHitsPos)[(nPotID*ODPLATESCNT*ODLAYERSCNT*ODFIBERSCNT)+(nPlateID*ODLAYERSCNT*ODFIBERSCNT)+nFiberID]=1;
					(m_vecODMultiplicityPos)[(nPotID*ODPLATESCNT)+nPlateID]++;
				}
			}
			else
			{
                                ATH_MSG_ERROR( "Index exceed array size for [RPotID, nPlateID, nFiberID, nSideID]= ["<<nPotID<<", "<<nPlateID<<", "<<nFiberID<<", "<<nSideID<<"]" );
				//return StatusCode::FAILURE;
			}
		}
	}
	else
	{
                ATH_MSG_WARNING( "Cannot find '"<< EVCOLLNAME_ODDIGIT <<"' collection" );
		//return StatusCode::FAILURE;
	}

	auto pData =
          xAODContainer.push_back(std::make_unique<xAOD::ALFAData>());

	//RawDataContainer
	pData->setScaler(m_vecScaler);
	//pData->setBCId(m_nBCId);
	//pData->setTimeStamp(m_nTimeStamp);
	//pData->setTimeStamp_ns(m_nTimeStamp_ns);
	pData->setTrigPat(m_vecTrigPat);

	//DigitCollection
	pData->setMDFiberHits(m_vecMDFiberHits);
	pData->setMDMultiplicity(m_vecMDMultiplicity);

	//ODDigitCollection
	pData->setODFiberHitsPos(m_vecODFiberHitsPos);
	pData->setODFiberHitsNeg(m_vecODFiberHitsNeg);
	pData->setODMultiplicityPos(m_vecODMultiplicityPos);
	pData->setODMultiplicityNeg(m_vecODMultiplicityNeg);

	return StatusCode::SUCCESS;
}

void ALFA_CLinkAlg::ClearXAODTrackingData(const int nMaxTrackCnt, eRecType eType)
{
	if(eType==ERC_LOCUNCORRECTED)
	{
		//LocRecEvCollection & LocRecEvODCollection
		m_vecXDetCS.resize(RPOTSCNT*nMaxTrackCnt);
		fill_n(m_vecXDetCS.begin(),m_vecXDetCS.size(),-9999);
		m_vecYDetCS.resize(RPOTSCNT*nMaxTrackCnt);
		fill_n(m_vecYDetCS.begin(),m_vecYDetCS.size(),-9999);
		m_vecDetectorPartID.resize(RPOTSCNT*nMaxTrackCnt);
		fill_n(m_vecDetectorPartID.begin(),m_vecDetectorPartID.size(),0);
		m_vecOverU.resize(RPOTSCNT*nMaxTrackCnt);
		fill_n(m_vecOverU.begin(),m_vecOverU.size(),-9999);
		m_vecOverV.resize(RPOTSCNT*nMaxTrackCnt);
		fill_n(m_vecOverV.begin(),m_vecOverV.size(),-9999);
		m_vecOverY.resize(RPOTSCNT*nMaxTrackCnt);
		fill_n(m_vecOverY.begin(),m_vecOverY.size(),-9999);
		m_vecNumU.resize(RPOTSCNT*nMaxTrackCnt);
		fill_n(m_vecNumU.begin(),m_vecNumU.size(),-9999);
		m_vecNumV.resize(RPOTSCNT*nMaxTrackCnt);
		fill_n(m_vecNumV.begin(),m_vecNumV.size(),-9999);
		m_vecNumY.resize(RPOTSCNT*nMaxTrackCnt);
		fill_n(m_vecNumY.begin(),m_vecNumY.size(),-9999);
		m_vecMDFibSel.resize(RPOTSCNT*nMaxTrackCnt*MDLAYERSCNT*MDPLATESCNT);
		fill_n(m_vecMDFibSel.begin(),m_vecMDFibSel.size(),-9999);
		m_vecODFibSel.resize(RPOTSCNT*nMaxTrackCnt*ODPLATESCNT);
		fill_n(m_vecODFibSel.begin(),m_vecODFibSel.size(),-9999);
	}
	else if(eType==ERC_LOCCORRECTED)
	{
		m_vecXLhcCS.resize(RPOTSCNT*nMaxTrackCnt);
		fill_n(m_vecXLhcCS.begin(),m_vecXLhcCS.size(),-9999);
		m_vecYLhcCS.resize(RPOTSCNT*nMaxTrackCnt);
		fill_n(m_vecYLhcCS.begin(),m_vecYLhcCS.size(),-9999);
		m_vecZLhcCS.resize(RPOTSCNT*nMaxTrackCnt);
		fill_n(m_vecZLhcCS.begin(),m_vecZLhcCS.size(),-9999);
		m_vecXRPotCS.resize(RPOTSCNT*nMaxTrackCnt);
		fill_n(m_vecXRPotCS.begin(),m_vecXRPotCS.size(),-9999);
		m_vecYRPotCS.resize(RPOTSCNT*nMaxTrackCnt);
		fill_n(m_vecYRPotCS.begin(),m_vecYRPotCS.size(),-9999);
		m_vecXStatCS.resize(RPOTSCNT*nMaxTrackCnt);
		fill_n(m_vecXStatCS.begin(),m_vecXStatCS.size(),-9999);
		m_vecYStatCS.resize(RPOTSCNT*nMaxTrackCnt);
		fill_n(m_vecYStatCS.begin(),m_vecYStatCS.size(),-9999);
		m_vecXBeamCS.resize(RPOTSCNT*nMaxTrackCnt);
		fill_n(m_vecXBeamCS.begin(),m_vecXBeamCS.size(),-9999);
		m_vecYBeamCS.resize(RPOTSCNT*nMaxTrackCnt);
		fill_n(m_vecYBeamCS.begin(),m_vecYBeamCS.size(),-9999);
	}
}

void ALFA_CLinkAlg::ClearXAODHeaderData()
{
	if (m_nDataType==1)
	{
		//DCS IDs
		/*
		*m_pullDCSBlmID=0;
		*m_pullDCSHVChannelID=0;
		*m_pullDCSLocalMonitoringID=0;
		*m_pullDCSMovementID=0;
		*m_pullDCSRadMonID=0;
		*m_pullDCSTriggerRatesID=0;
		*m_pullDCSFEConfigurationID=0;
		*m_pullDCSTriggerSettingsID=0;*/

		//RawDataContainer
		m_vecScaler.resize(RPOTSCNT);
		fill_n(m_vecScaler.begin(),m_vecScaler.size(),-1);
		//m_nBCId=-1;
		//m_nTimeStamp=-1;
		//m_nTimeStamp_ns=-1;
		m_vecTrigPat.resize(RPOTSCNT*TRIGPATCNT);
		fill_n(m_vecTrigPat.begin(),m_vecTrigPat.size(),0);
	}

	//DigitCollection
	m_vecMDFiberHits.resize(RPOTSCNT*MDLAYERSCNT*MDPLATESCNT*MDFIBERSCNT);
	fill_n(m_vecMDFiberHits.begin(),m_vecMDFiberHits.size(),0);
	m_vecMDMultiplicity.resize(RPOTSCNT*MDLAYERSCNT*MDPLATESCNT);
	fill_n(m_vecMDMultiplicity.begin(),m_vecMDMultiplicity.size(),0);

	//ODDigitCollection
	m_vecODFiberHitsPos.resize(RPOTSCNT*ODPLATESCNT*ODLAYERSCNT*ODFIBERSCNT);
	fill_n(m_vecODFiberHitsPos.begin(),m_vecODFiberHitsPos.size(),0);
	m_vecODFiberHitsNeg.resize(RPOTSCNT*ODPLATESCNT*ODLAYERSCNT*ODFIBERSCNT);
	fill_n(m_vecODFiberHitsNeg.begin(),m_vecODFiberHitsNeg.size(),0);

	m_vecODMultiplicityPos.resize(RPOTSCNT*ODPLATESCNT);
	fill_n(m_vecODMultiplicityPos.begin(),m_vecODMultiplicityPos.size(),0);
	m_vecODMultiplicityNeg.resize(RPOTSCNT*ODPLATESCNT);
	fill_n(m_vecODMultiplicityNeg.begin(),m_vecODMultiplicityNeg.size(),0);
}
