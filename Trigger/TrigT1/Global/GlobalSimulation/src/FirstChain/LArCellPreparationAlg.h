/*
 *   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#ifndef GLOBALSIM_LARCELLPREPARATIONALG_H
#define GLOBALSIM_LARCELLPREPARATIONALG_H

/*
  This Algorithm simulates the energy encoding of all LAr cells for Global and simulated the truncation of cells
  from overflowing FEB2s. The hardware-accurate cells are then stored in a GlobalLArCellContainer object within 
  StoreGate
*/

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "xAODEventInfo/EventInfo.h"
#include "GaudiKernel/ToolHandle.h"
#include "CaloEvent/CaloCellContainer.h"
#include "CaloConditions/CaloNoise.h"

#include "GlobalLArCell.h"
#include "GlobalLArCellContainer.h"

#include <vector>
#include <boost/dynamic_bitset.hpp>

namespace GlobalSim {

  class LArCellPreparationAlg : public AthReentrantAlgorithm { 
  public:

    using AthReentrantAlgorithm::AthReentrantAlgorithm;

    /** @brief initialize function running before first event */
    virtual StatusCode  initialize() override;   
    /** @brief execute function running for every event */
    virtual StatusCode  execute(const EventContext& ) const override;    

  private:

    /** @brief Function to simulate the cell energy as seen by Global */
    std::pair<float,boost::dynamic_bitset<>> encodeEnergy(float energy) const;
    /** @brief Function to simulate the truncation of overflowing FEB2s */
    StatusCode removeCellsFromOverloadedFEB(std::vector<GlobalSim::GlobalLArCell> &cells) const;
    
    /** @brief array holding the energy edges of the multilinear encoding */
    int m_readoutRanges[5] = {-1,-1,-1,-1,-1};
    /** @brief number of discrete values per multilinear energy encoding range */
    int m_stepsPerRange = -1;
    /** @brief maximum number of cells that can be send to Global for each FEB2 */
    unsigned m_maxCellsPerFEB = -1;

    /** @brief LAr cell map where the key is the offline cell ID */
    std::map<int,GlobalSim::GlobalLArCell> m_gblLArCellMap = {};
    /** @brief GlobalLArCellContainer template which is constructed in initialize and used in execute */
    std::unique_ptr<GlobalSim::GlobalLArCellContainer> m_gblLArCellContainerTemplate;

    /** @brief Key for the EventInfo object */
    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfo{this, "EventInfo", "EventInfo", "Key for the EventInfo container"};

    /** @brief Parameters defining the multilinear energy encoding scheme */
    Gaudi::Property<int> m_numberOfEnergyBits {this, "numberOfEnergyBits", 6, "Number of bits reserved for the multilinear energy encoding"};
    Gaudi::Property<int> m_valueLSB {this, "valueLeastSignificantBit", 40, "Value of the least significant bit in MeV"};
    Gaudi::Property<int> m_valueGainFactor {this, "valueGainFactor", 4, "Value of the gain factor of the multilinear energey encoding"};

    /** @brief Path to the LAr cell map in the CVMFS GroupData space */
    Gaudi::Property<std::string> m_LArCellMap {this, "LArCellMapFile", "UpgradePerformanceFunctions/LAr_Cell_Map_offlineID_1.csv",
      "File associating LAr cells with readout FEBs and connection technology"};

    /** @brief Key to the CaloCell container */
    SG::ReadHandleKey<CaloCellContainer> m_caloCellsKey {this, "caloCells", "AllCalo", "key to read in a CaloCell container"};

    /** @brief Key to the total noise used for each CaloCell */
    SG::ReadCondHandleKey<CaloNoise> m_totalNoiseKey{this, "totalNoiseKey", "totalNoise", "SG Key of CaloNoise data object"};

    /** @brief Key to writing the GlobalLArCellContainer to StoreGate */
    SG::WriteHandleKey<GlobalSim::GlobalLArCellContainer> m_LArCellContainerKey{this, "GlobalLArCellsKey", "GlobalLArCells", "Key for the output container of the LAr cells sent to Global"};

  };

}
#endif




