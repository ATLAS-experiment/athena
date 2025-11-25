/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_GLOBALCELLTOWERALGTOOL_H
#define GLOBALSIM_GLOBALCELLTOWERALGTOOL_H

/*
  This algorithm simulates the cell towers for the Global Trigger. Input is taken only from LAr cells contained in the
  GlobalLArCellContainer. Tile cells are not included for now. Cells are placed in towers based on their eta and phi
  positions and written out as a GenericTOB.
*/

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"

#include "../GlobalSimComponents/IGlobalSimAlgTool.h"
#include "../IO/CommonTOB.h"
#include "../IO/ICommonTOB.h"
#include "GlobalLArCellContainer.h"
#include <bitset>
#include <string>
#include <cassert>

namespace GlobalSim {

  class GlobalLArCell;
  

  class GlobalCellTowerAlgTool: public extends<AthAlgTool, IGlobalSimAlgTool> {
    
  public:

    /** @brief Main constructor */    
    GlobalCellTowerAlgTool(const std::string& type, const std::string& name, const IInterface* parent);
    
    /** @brief Main destructor (explicitly defaulted) */
    ~GlobalCellTowerAlgTool() override = default;
    
    /** @brief Initialize function running before first event */
    virtual StatusCode initialize() override;

    /** @brief Main functional block running for each event */
    virtual StatusCode run(const EventContext& ctx) const override;

    /** @brief Overriding toString function from base class */
    virtual std::string toString() const override;

  private:

    /** @brief Key to the GlobalLArCellContainer */
    SG::ReadHandleKey<GlobalSim::GlobalLArCellContainer> m_gblLArCellContainerKey {this, "GlobalLArCellsKey", "GlobalLArCells", "Key for the output container of the LAr cells sent to Global"}; 
 
    /** @brief Write key for the output cell towers as a GenericTobContainer */
    SG::WriteHandleKey<IOBitwise::ICommonTOBContainer> m_gblCellTowers {this, "GlobalCellTowersKey", "GlobalCellTowers", "Key to the container of generic TOBS containing the cell towers"};

  };
  
} // namespace GlobalSim
    
#endif
