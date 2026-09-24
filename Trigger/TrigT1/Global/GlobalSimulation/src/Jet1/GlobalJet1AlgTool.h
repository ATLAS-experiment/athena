/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_GLOBALJet1ALGTOOL_H
#define GLOBALSIM_GLOBALJet1ALGTOOL_H

/*
  This algorithm simulates the WTAConeJet(Jet1) for the Global Trigger.
  It uses the same headers from TrigGepPerf
  Input is taken only from CellTowers produced from the GlobalJet1AlgTool
*/

#include "AthenaBaseComps/AthAlgTool.h"

#include "../GlobalSimComponents/IGlobalSimAlgTool.h"
#include "../IO/CommonTOBContainer.h"
#include "../IO/Jet1TOB.h"
#include "../Utilities/IDataCollector.h"


#include "TrigGepPerf/WTAConeParallelHelper.h"
#include "TrigGepPerf/WTACone2PassMaker.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

#include <string>

namespace GlobalSim {

  class GlobalLArCell;
  
  class GlobalJet1AlgTool: public extends<AthAlgTool, IGlobalSimAlgTool> {
    
  public:

    /** @brief Main constructor */    
    GlobalJet1AlgTool(const std::string& type, const std::string& name, const IInterface* parent);
    
    /** @brief Main destructor (explicitly defaulted) */
    ~GlobalJet1AlgTool() override = default;
    
    /** @brief Initialize function running before first event */
    virtual StatusCode initialize() override;

    /** @brief Main functional block running for each event */
    virtual StatusCode run(const std::unique_ptr<IDataCollector>&,
			   const EventContext& ctx) const override;

    /** @brief Overriding toString function from base class */
    virtual std::string toString() const override;

  private:
 
    /** @brief Read key for the output cell towers as a GenericTobContainer */
    SG::ReadHandleKey<IOBitwise::CommonTOBContainer>
    m_gblCellTowers {
        this,
	"GlobalCellTowersKey",
	"GlobalCellTowers",
	"Key to the container of generic TOBS containing the cell towers"};
    
    /** @brief Write key for the output Jet1Jets as a Jet1TOBContainer */
    SG::WriteHandleKey<IOBitwise::Jet1TOBContainer>
    m_gblJet1JetsContainerKey {
        this,
	"GlobalJet1JetsKey",
	"GlobalJet1Jets",
	"Key to the container of generic TOBS containing the Jet1Jets"}; 

  };
  
} // namespace GlobalSim
    
#endif
