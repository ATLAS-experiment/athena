/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// ZdcRec.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef ZDCRECV2_H
#define ZDCRECV2_H

#include <string>
#include <map>



//which one ???
#include "AthenaBaseComps/AthAlgorithm.h"
//#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"
#include "Gaudi/Property.h"
#include "AthContainers/OwnershipPolicy.h"



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


class ZdcRecV2 : public AthAlgorithm
{

public:

	ZdcRecV2(const std::string& name, ISvcLocator* pSvcLocator);
	~ZdcRecV2();

	StatusCode initialize();
	StatusCode execute(const EventContext& ctx);
	StatusCode finalize();

private:

  /** class member version of retrieving StoreGate */
	//StoreGateSvc* m_storeGate;
	 ServiceHandle<StoreGateSvc> m_storeGate;


	/** Does the collection own it's objects ? **/
	Gaudi::Property<int> m_ownPolicy{this, "OwnPolicy", static_cast<int> (SG::OWN_ELEMENTS)};


	/** Digits data container name */
	Gaudi::Property<std::string> m_ttContainerName{this, "DigitsContainerName", "ZdcTriggerTowers", "ZdcTriggerTowers"};

	/** Raw data object name */
	Gaudi::Property<std::string> m_zdcModuleContainerName{this, "ZdcModuleContainerName", "ZdcModules", "ZdcModules"};
	Gaudi::Property<std::string> m_zdcModuleAuxContainerName{this, "ZdcModuleAuxContainerName", "ZdcModulesAux.", "ZdcModulesAux."};


	/** Pointer to Zdc input "digits" data */
	const xAOD::TriggerTowerContainer* m_ttContainer{nullptr};

	int m_eventCount{0};
	bool m_complainContain{true};
	bool m_complainRetrieve{true};

	//Include here all tools to do the job. They will be called by the algorithm execute method
	//Another option is to use ToolHandleArray<IZdcRecTool>, where IZdcRecTool is the factory for
	//the tools
	ToolHandle<ZdcRecChannelToolV2> m_ChannelTool{"ZdcRecChannelToolV2"};

};

#endif
