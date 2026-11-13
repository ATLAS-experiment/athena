/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/*
 * ZdcRecNoiseTool.h
 *
 *  This extracts the pedestal and noise figures from every channel. This should
 *  process "pedestal runs" where no activity on the channels are present.
 *      Author: leite
 */

#ifndef ZDCRECNOISETOOL_H_
#define ZDCRECNOISETOOL_H_



#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "Gaudi/Property.h"

#include "ZdcEvent/ZdcRawChannelCollection.h"

#include <string>
#include <memory>

class IInterface;
class InterfaceID;
class StatusCode;
class ZdcDigitsCollection;


//Interface Id for retrieving the tool
static const InterfaceID IID_IZdcRecNoiseTool("ZdcRecNoiseTool", 1, 1);

class ZdcRecNoiseTool: public AthAlgTool
{
public:
	 ZdcRecNoiseTool(const std::string& type,
					   const std::string& name,
					   const IInterface* parent);

	virtual ~ZdcRecNoiseTool();

	static const InterfaceID& interfaceID();

	virtual StatusCode initialize();
	virtual StatusCode finalize();

	int readPedestals();
	int writePedestals();

private:

	Gaudi::Property<std::string> m_pedestalDir{this, "PedestalsDirectory", "/afs/cern.ch/user/l/leite/public/ZdcData/Pedestals",
						"Pedestal files directory"};
	Gaudi::Property<std::string> m_pedestalFile{this, "PedestalsFile", "zdc_pedestal_0000.data",
						"Pedestal file name"};

	std::unique_ptr<ZdcDigitsCollection> m_pedestalData{};

};

#endif /* ZDCRECNOISETOOL_H_ */
