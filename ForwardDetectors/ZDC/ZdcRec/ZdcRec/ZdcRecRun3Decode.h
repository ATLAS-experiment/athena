/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// ZdcRecRun3Decode.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef ZDCRECRun3DECODE_H
#define ZDCRECRun3DECODE_H

#include <string>
#include <map>

#include "AthenaBaseComps/AthAlgorithm.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"

class ISvcLocator;
class StatusCode;
class Identifier;
class StoreGateSvc;

class ZdcRecChannelToolLucrod;
#include "ZdcByteStream/ZdcLucrodDataContainer.h"
#include "xAODForward/ZdcModuleContainer.h"
#include "xAODForward/ZdcModuleAuxContainer.h"
#include "ZdcAnalysis/ZdcAnalysisTool.h"
#include "xAODEventInfo/EventInfo.h"

/** @class ZdcRecRun3Decode

    Class definition for the ZDC decoder class for Run 3
    This class is responsible for taking the ZdcLucrodData and rendering them as ZdcModules for subsequent steps, and for defining the two output ZdcModuleContainers.

    @author  Brian Cole and Peter Steinberg, bcole@cern.ch, steinberg@bnl.gov
*/


class ZdcRecRun3Decode : public AthAlgorithm
{

public:

	ZdcRecRun3Decode(const std::string& name, ISvcLocator* pSvcLocator);
	~ZdcRecRun3Decode();

	StatusCode initialize() override;
	StatusCode execute() override;
	StatusCode finalize() override;

private:
	
	int m_ownPolicy;
	Gaudi::Property<unsigned int> m_nFragments{this,"NFragments",6,"Number of expected LUCROD fragments"};

	SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey{this,"EventInfoKey","EventInfo"};
	SG::WriteDecorHandleKey<xAOD::EventInfo> m_eventInfoDecorKey{this, "eventInfoDecorKey", m_eventInfoKey, "forwardDetFlags", "Key for EventInfo decoration object"};

	SG::ReadHandleKey<ZdcLucrodDataContainer> m_zldContainerName
          { this, "ZdcLucrodDataContainerKey", "ZdcLucrodDataContainer", "" };

	SG::WriteHandleKey<xAOD::ZdcModuleContainer> m_zdcModuleContainerName
	  { this, "ZdcModuleContainerName", "ZdcModules", "" };

	SG::WriteHandleKey<xAOD::ZdcModuleContainer> m_zdcSumContainerName
	  { this, "ZdcSumContainerName", "ZdcSums", "" };

	ToolHandle<ZdcRecChannelToolLucrod> m_ChannelTool
	  { this, "ChannelTool", "ZdcRecChannelToolLucrod", "" };


};

#endif
