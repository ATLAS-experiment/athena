/*
 *   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
 */

#ifndef GLOBALSIM_EGAMMA1_LARSTRIP_FEX_H
#define GLOBALSIM_EGAMMA1_LARSTRIP_FEX_H

/*
  This Algorithm finds and outputs CaloCell in the neighborhoods of eFEX
  RoIs. These neighhoods are used to run various Algorithms in GlobalSim.
*/

#include "ICaloCellsProducer.h"
#include "eFexRoIAlgTool.h"

#include "../IO/LArStripNeighborhoodContainer.h"
#include "../IO/IeEmNbhoodTOBContainer.h"
#include "../IO/IeEmTOB.h"

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "xAODEventInfo/EventInfo.h"

#include <vector>

namespace GlobalSim {


  class Egamma1_LArStrip_Fex: public AthReentrantAlgorithm { 
  public:
    
    
    Egamma1_LArStrip_Fex(const std::string& name, ISvcLocator* pSvcLocator);

    virtual StatusCode  initialize() override;   
    virtual StatusCode  execute(const EventContext& ) const override;    

  private:

    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey{
      this,
	"EventInfo",
	"EventInfo",
	"EventInfo name"};
    
    // tool to get a vector of cal cells
    
    ToolHandle<ICaloCellsProducer> m_cellProducer{this,
	"caloCellProducer",
	"EMB1CellFromCaloCells",
	"AlgTool to provide a vector of CaloCells"
	};

    // tool to get eFexRoIs
    
    ToolHandle<eFexRoIAlgTool>
    m_roiAlgTool{this,
		 "roiAlgTool",
		 "EMB1CellFromCaloCells",
		 "AlgTool to provide a vector<const xAOD::eFexEMRoI*>"};

    Gaudi::Property<bool> m_dump {
      this,
      "dump",
      false,
      "flag to enable dumps"};

    Gaudi::Property<bool> m_dumpTerse {
      this,
      "dumpTerse",
      false,
      "flag to enable terse dumps"};

    SG::WriteHandleKey<IOBitwise::IeEmNbhoodTOBContainer>
    m_neighKey {
      this,
      "stripNeighborhoodTOBKey",
      "stripNeighborhoodTOBContainer",
      "location to write strip neighborhoods of EFex RoIs, with the associated TOBs"};

    StatusCode
    findNeighborhoods(const std::vector<const xAOD::eFexEMRoI*>&,
		      const std::vector<const CaloCell*>&,
		      IOBitwise::IeEmNbhoodTOBContainer&) const;

    StatusCode
    findNeighborhood(const xAOD::eFexEMRoI*,
		     const std::vector<const CaloCell*>&,
		     IOBitwise::IeEmNbhoodTOBContainer&) const;

    StatusCode
    findClosestCellToRoI(const xAOD::eFexEMRoI*,
			 const std::vector<const CaloCell*>&,
			 const CaloCell*&) const;

  };

}
#endif




