/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// ZdcRec.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef ZDCREC_H
#define ZDCREC_H

#include <string>
#include <map>



//which one ???
#include "AthenaBaseComps/AthAlgorithm.h"
//#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"
#include "Gaudi/Property.h"
#include "AthContainers/OwnershipPolicy.h"



#include "ZdcRec/ZdcRecNoiseTool.h"
#include "ZdcRec/ZdcRecChannelTool.h"

class ISvcLocator;
class StatusCode;
class Identifier;
class StoreGateSvc;
class ZdcDigitsCollection;
class ZdcCalibEvent;
class ZdcRawChannelCollection;


/** @class ZdcRec

    Class definition for the ZDC Reconstruction class

    @author  Brian Cole and Peter Steinberg, bcole@cern.ch, peter.steinberg@bnl.gov
             and M. Leite leite@cern.ch
*/


class ZdcRec : public AthAlgorithm
{

public:

	ZdcRec(const std::string& name, ISvcLocator* pSvcLocator);
	~ZdcRec();

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
	Gaudi::Property<std::string> m_digitsContainerName{this, "DigitsContainerName", "ZdcDigitsCollection", "ZdcDigitsCollection"};

	/** Raw data object name */
	Gaudi::Property<std::string> m_rawContainerName{this, "RawContainerName", "ZdcRawChannelCollection", "ZdcRawChannelCollection"};

	/** Pointer to Zdc input "digits" data */
	const ZdcDigitsCollection* m_digitsCollection{nullptr};

	/** Pointer to Zdc output "raw" reconstructed data */
	ZdcRawChannelCollection* m_rawCollection{nullptr};

	int m_eventCount{0};
	bool m_complainContain{true};
	bool m_complainRetrieve{true};

	//Include here all tools to do the job. They will be called by the algorithm execute method
	//Another option is to use ToolHandleArray<IZdcRecTool>, where IZdcRecTool is the factory for
	//the tools
	ToolHandle<ZdcRecChannelTool> m_ChannelTool{"ZdcRecChannelTool"};
	ToolHandle<ZdcRecNoiseTool>   m_NoiseTool{"ZdcRecNoiseTool"};
};

#endif
