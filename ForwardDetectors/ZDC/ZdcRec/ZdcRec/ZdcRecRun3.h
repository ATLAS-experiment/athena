/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// ZdcRecRun3.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef ZDCRECRun3_H
#define ZDCRECRun3_H

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

#include "xAODForward/ZdcModuleContainer.h"
#include "xAODForward/ZdcModuleAuxContainer.h"
#include "xAODEventInfo/EventInfo.h"
#include "ZdcAnalysis/IZdcAnalysisTool.h"
#include "ZdcUtils/ZdcEventInfo.h"


/** @class ZdcRecRun3

    Class definition for the ZDC Reconstruction class for Run 3

    This class is responsible for running the ZDC signal processing and decorating the ZdcModules accordingly.  In principle, this module runs appropriately on simulation and raw.

    @author  Brian Cole and Peter Steinberg, bcole@cern.ch, steinberg@bnl.gov
*/


class ZdcRecRun3 : public AthAlgorithm
{

public:

	ZdcRecRun3(const std::string& name, ISvcLocator* pSvcLocator);
	~ZdcRecRun3();

	StatusCode initialize() override;
	StatusCode execute() override;
	StatusCode finalize() override;


private:
	
	Gaudi::Property<std::string> m_configuration{this, "Configuration", "none", "ZDC reconstruction configuration"};

        Gaudi::Property<unsigned int> m_DAQMode{this, "DAQMode", static_cast<unsigned int>(ZdcEventInfo::CombinedPhysics), "DAQ mode"};

        Gaudi::Property<unsigned int> m_forcedEventType{this, "ForcedEventType", static_cast<unsigned int>(ZdcEventInfo::ZdcEventUnknown), 
	  "if not ZdcEventUnknown, Use forced event type true/false"};

        Gaudi::Property<int> m_ownPolicy{this, "OwnPolicy", static_cast<int> (SG::OWN_ELEMENTS), "Ownership policy"};

	SG::ReadHandleKey<xAOD::ZdcModuleContainer> m_zdcModuleContainerName
	  { this, "ZdcModuleContainerName", "ZdcModules", "" };

	SG::ReadHandleKey<xAOD::ZdcModuleContainer> m_zdcSumContainerName
	  { this, "ZdcSumContainerName", "ZdcSums", "" };

        SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey {this, "EventInfoKey", "EventInfo","Location of the event info."};
	SG::WriteDecorHandleKey<xAOD::EventInfo> m_eventInfoDecorKey{this, "eventInfoDecorKey", m_eventInfoKey, "forwardDetFlags", "Key for EventInfo decoration object"};
  
	SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcEventType{this, "ZdcEventTypeKey", "", "ZDC event Type"};

	SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcDAQMode{this, "ZdcDAQModeKey", "", "ZDC DAQ mode"};

        // Override bad BCID in standalone data using BCID from ROB header(s)
        //
        SG::WriteDecorHandleKey<xAOD::ZdcModuleContainer> m_ZdcBCIDOverride{this, "ZdcBCIDOverride", "", "BCID with override"};

	ToolHandleArray<ZDC::IZdcAnalysisTool> m_zdcTools{ this, "ZdcAnalysisTools",{} };

};

#endif
