/*
 *   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */

#ifndef GLOBALSIM_EGAMMA1_ONLINEMAPNBHOOD_H
#define GLOBALSIM_EGAMMA1_ONLINEMAPNBHOOD_H

/*
  This Algorithm finds and outputs GlobalLArCells in the neighbourhood of eFEX
  RoIs. These neighhoods are used to run various Egamma1 Algorithms in GlobalSim.
  This method uses the Athena offline calo map to form the neighbourhood.
  It is seeded from the GlobalLArCells.
*/

#include "../FEX_Unpacker/eFexRoIAlgTool.h"

#include "../IO/GlobalLArCell.h"
#include "../IO/GlobalLArCellContainer.h"
#include "../IO/LArStripNeighborhood.h"
#include "../IO/eEmNbhoodTOB.h"

#include "Identifier/Identifier.h"

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "xAODEventInfo/EventInfo.h"

#include <vector>

//forward declarations for calorimenter ID helpers
class CaloCell_ID;
class LArEM_ID;

namespace GlobalSim {

  class Egamma1_OnlineMapNbhood: public AthReentrantAlgorithm {
  public:
    
    Egamma1_OnlineMapNbhood(const std::string& name, ISvcLocator* pSvcLocator);

    /** @brief initialize function running before the first event */
    virtual StatusCode  initialize() override;
    /** @brief execute function running for every event */
    virtual StatusCode  execute(const EventContext& ) const override;    

  private:

    const CaloCell_ID*     m_calocell_id{};
    const LArEM_ID*        m_larem_id{};

    /** @brief ReadHandle Key for the EventInfo object */
    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey{
      this,
	"EventInfo",
	"EventInfo",
	"EventInfo name"};
    
    /** @brief ToolHandle for the eFexRoI AlgTool*/
    ToolHandle<eFexRoIAlgTool>
    m_roiAlgTool{
      this,
      "roiAlgTool",
      "GlobalSim::eFexRoIAlgTool",
      "AlgTool to provide a vector<const xAOD::eFexEMRoI*>"};
    
    /** @brief ReadHandle Key to the GlobalLArCellContainer */
    SG::ReadHandleKey<GlobalSim::GlobalLArCellContainer>
    m_gblLArCellContainerKey {
      this,
      "GlobalLArCellsKey",
      "GlobalLArCells",
      "Key for the container of the LAr cells sent to Global"}; 

    /** @brief WriteHandle Key for the eFexRoI's eta (for valid windows, for DEBUG) */
    SG::WriteHandleKey<std::vector<float>>
    m_eFEXetaKey {
      this,
      "eFEXetaKey",
      "L1_eFEXeta"};

    /** @brief WriteHandle Key for the eFexRoI's phi (for valid windows, for DEBUG) */
    SG::WriteHandleKey<std::vector<float>>
    m_eFEXphiKey {
      this,
      "eFEXphiKey",
      "L1_eFEXphi"};

    /** @brief WriteHandle Key for the eFexRoI's eta (for invalid windows, for DEBUG) */
    SG::WriteHandleKey<std::vector<float>>
    m_FailedeFEXetaKey {
      this,
      "FailedeFEXetaKey",
      "L1_FailedeFEXeta"};

    /** @brief WriteHandle Key for the eFexRoI's phi (for invalid windows, for DEBUG) */
    SG::WriteHandleKey<std::vector<float>>
    m_FailedeFEXphiKey {
      this,
      "FailedeFEXphiKey",
      "L1_FailedeFEXphi"};

    /** @brief WriteHandle Key for the resulting TOBs */
    SG::WriteHandleKey<IOBitwise::eEmNbhoodTOBContainer>
    m_neighKey {
      this,
      "stripNeighborhoodTOBKey",
      "stripNeighborhoodTOBContainer",
      "location to write strip neighborhoods of EFex RoIs, with the associated TOBs"};

    
    /** @brief Parameter to toggle dumping of full information window information */
    Gaudi::Property<bool> m_dump {
      this,
      "dump",
      false,
      "flag to enable dumps"};

    /** @brief Parameter to toggle dumping of brief information window information */
    Gaudi::Property<bool> m_dumpTerse {
      this,
      "dumpTerse",
      false,
      "flag to enable terse dumps"};

    /** @brief Function to produce mutliple windows from a vector of incoming eFeX RoIs */
    StatusCode
    findNeighborhoods_OnlineMap(const std::vector<const xAOD::eFexEMRoI*>&,
				const GlobalSim::GlobalLArCellContainer&,
				std::vector<bool>&,
				IOBitwise::eEmNbhoodTOBContainer&) const;

    /** @brief Function to produce a window from an incoming eFeX RoI */
    StatusCode
    findNeighborhood_OnlineMap(const xAOD::eFexEMRoI*,
			       const GlobalSim::GlobalLArCellContainer&,
			       bool&,
			       IOBitwise::eEmNbhoodTOBContainer&) const;    

    /** @brief Function to find the seed cell for a window from the eFeX RoI position */
    bool
    findSeedCell(float,
		 float,
		 const GlobalSim::GlobalLArCellContainer&,
		 Identifier&) const;

    /** @brief Function to search for seed cells according to local granularity */
    bool
    findHalfStrips(int,
		  float,
		  float,
		  const GlobalSim::GlobalLArCellContainer&,
		  Identifier&) const;

    /** @brief Function to produce a window around an input seed cell */
    StatusCode
    findWindow(IdentifierHash,
	       const GlobalSim::GlobalLArCellContainer&,
	       std::vector<std::vector<std::shared_ptr<const GlobalSim::GlobalLArCell>> >&) const;    

    /** @brief Function to find the maximum energy cell in a window */
    StatusCode
    findMaxima(IdentifierHash&,
	       std::vector<std::vector<std::shared_ptr<const GlobalSim::GlobalLArCell>> >&) const;    

    /** @brief Function to find the maximum energy cell in a vector of cells */
    std::shared_ptr<const GlobalLArCell> findMax(std::vector<std::shared_ptr<const GlobalLArCell>>&) const;
  };

}
#endif




