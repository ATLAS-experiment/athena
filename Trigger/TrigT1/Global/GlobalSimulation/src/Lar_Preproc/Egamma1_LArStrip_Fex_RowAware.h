/*
 *   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */

#ifndef GLOBALSIM_EGAMMA1_LARSTRIP_FEX_ROWAWARE_H
#define GLOBALSIM_EGAMMA1_LARSTRIP_FEX_ROWAWARE_H

/*
  This Algorithm finds and outputs CaloCell in the neighborhoods of eFEX
  RoIs. These neighhoods are used to run various Algorithms in GlobalSim.
*/

#include "ICaloCellsProducer.h"
#include "../FEX_Unpacker/eFexRoIAlgTool.h"
#include "Egamma1_LArStrip_Fex.h"
#include "CaloEvent/CaloCellContainer.h"
#include "CaloConditions/CaloNoise.h"

#include "../IO/LArStripNeighborhood.h"
#include "../IO/eEmNbhoodTOB.h"

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "xAODEventInfo/EventInfo.h"

#include <vector>

namespace GlobalSim {


  class Egamma1_LArStrip_Fex_RowAware: public AthReentrantAlgorithm {

  public:
    
    Egamma1_LArStrip_Fex_RowAware(const std::string& name, ISvcLocator* pSvcLocator);

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
	"GlobalSim::EMBE1CellsFromCaloCells",
	"AlgTool to provide a vector of CaloCells"
	};

    // tool to get eFexRoIs
    ToolHandle<eFexRoIAlgTool>
    m_roiAlgTool{this,
		 "roiAlgTool",
		 "GlobalSim::eFexRoIAlgTool",
		 "AlgTool to provide a vector<const xAOD::eFexEMRoI*>"};

    /** @brief Key to the total noise used for each CaloCell */
    SG::ReadCondHandleKey<CaloNoise>
    m_totalNoiseKey{
      this,
      "totalNoiseKey",
      "totalNoise",
      "SG Key of CaloNoise data object"};
    
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
    
    SG::WriteHandleKey<IOBitwise::eEmNbhoodTOBContainer>
    m_neighKey {
      this,
      "stripNeighborhoodTOBKey",
      "stripNeighborhoodTOBContainer",
      "location to write strip neighborhoods of EFex RoIs, with the associated TOBs"};

    StatusCode
    findNeighborhoods_RowAware(const std::vector<const xAOD::eFexEMRoI*>&,
			       const std::vector<const CaloCell*>&,
			       IOBitwise::eEmNbhoodTOBContainer&,
			       const CaloNoise&) const;

    StatusCode
    findNeighborhood_RowAware(const xAOD::eFexEMRoI*,
			      const std::vector<const CaloCell*>&,
			      IOBitwise::eEmNbhoodTOBContainer&,
			      const CaloNoise&) const;

    StatusCode
    findClosestCellToRoI(const xAOD::eFexEMRoI*,
			 const std::vector<const CaloCell*>&,
			 const CaloCell*&) const;
  };

}
#endif




