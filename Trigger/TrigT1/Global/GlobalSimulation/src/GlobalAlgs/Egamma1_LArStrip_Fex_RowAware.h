/*
 *   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#ifndef GLOBALSIM_EGAMMA1_LARSTRIP_FEX_ROWAWARE_H
#define GLOBALSIM_EGAMMA1_LARSTRIP_FEX_ROWAWARE_H

/*
  This Algorithm finds and outputs CaloCell in the neighborhoods of eFEX
  RoIs. These neighhoods are used to run various Algorithms in GlobalSim.
*/

#include "ICaloCellsProducer.h"
#include "eFexRoIAlgTool.h"
#include "Egamma1_LArStrip_Fex.h"
#include "CaloEvent/CaloCellContainer.h"

#include "../IO/LArStripNeighborhoodContainer.h"

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

    SG::WriteHandleKey<LArStripNeighborhoodContainer>
    m_neighKey {
      this,
      "stripNeighborhoodKey",
      "stripNeighborhoodContainer"};
      //"location to write strip neighborhoods of EFex RoIs"};

    SG::WriteHandleKey<std::vector<int>>
    m_phimaxKey {
      this,
      "phimaxKey",
      "phimax"};
      //"location to write strip neighborhoods of EFex RoIs"};

    StatusCode
    findNeighborhoods_RowAware(const std::vector<const xAOD::eFexEMRoI*>&,
			       const std::vector<const CaloCell*>&,
			       LArStripNeighborhoodContainer&,
			       std::vector<int>&) const;

    StatusCode
    findNeighborhood_RowAware(const xAOD::eFexEMRoI*,
			      const std::vector<const CaloCell*>&,
			      LArStripNeighborhoodContainer&,
			      std::vector<int>&) const;

    StatusCode
    findClosestCellToRoI(const xAOD::eFexEMRoI*,
			 const std::vector<const CaloCell*>&,
			 const CaloCell*&) const;
  };

}
#endif




