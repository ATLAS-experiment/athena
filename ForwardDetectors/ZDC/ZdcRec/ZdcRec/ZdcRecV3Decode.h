/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// ZdcRec.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef ZDCRECV3Decode_H
#define ZDCRECV3Decode_H

#include <string>
#include <map>



//which one ???
#include "AthenaBaseComps/AthAlgorithm.h"
//#include "GaudiKernel/Algorithm.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"



//#include "ZdcRec/ZdcRecNoiseTool.h"
//#include "ZdcRec/ZdcRecChannelTool.h"

class ISvcLocator;
class StatusCode;
class Identifier;
class StoreGateSvc;

class ZdcRecChannelToolV2;
#include "xAODTrigL1Calo/TriggerTowerContainer.h"
#include "xAODForward/ZdcModuleContainer.h"
#include "xAODForward/ZdcModuleAuxContainer.h"

/** @class ZdcRec

    Class definition for the ZDC Reconstruction class

    @author  Brian Cole and Peter Steinberg, bcole@cern.ch, peter.steinberg@bnl.gov
             and M. Leite leite@cern.ch
*/


class ZdcRecV3Decode : public AthAlgorithm
{

public:

	ZdcRecV3Decode(const std::string& name, ISvcLocator* pSvcLocator);
	~ZdcRecV3Decode();

	StatusCode initialize() override;
	StatusCode execute(const EventContext& ctx) override;
	StatusCode finalize() override;

private:

	int m_ownPolicy;

	SG::ReadHandleKey<xAOD::TriggerTowerContainer> m_ttContainerName
	  { this, "DigitsContainerName", "ZdcTriggerTowers", "" };

	SG::WriteHandleKey<xAOD::ZdcModuleContainer> m_zdcModuleContainerName
	  { this, "ZdcModuleContainerName", "ZdcModules", "" };

	SG::WriteHandleKey<xAOD::ZdcModuleContainer> m_zdcSumContainerName
	  { this, "ZdcSumContainerName", "ZdcSums", "" };

	//Include here all tools to do the job. They will be called by the algorithm execute method
	//Another option is to use ToolHandleArray<IZdcRecTool>, where IZdcRecTool is the factory for
	//the tools
	ToolHandle<ZdcRecChannelToolV2> m_ChannelTool
	  { this, "ChannelTool", "ZdcRecChannelToolV2", "" };

};

#endif
