/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//****************************************************************************
// Filename : TileCosmicsTrigger.h
// Author   : jose.maneira@cern.ch
// Created  : Nov 2005
// Reviewed : March 2007 remove std.alone ntp, output to TileEvent/TileTrigger
//            May   2007 set 8 boards
//
// DESCRIPTION
// 
//    To emulate the cosmics trigger in several different configurations
//    To create Ntuple a vector with flags for each type of trigger
//
// Properties (JobOption Parameters):
//
//    TileTTL1Container           string   key value of TileTTL1 in TDS 
//
// BUGS:
//  
// History:
//  
//  
//****************************************************************************
#ifndef TILECOSMICALGS_TILECOSMICSTRIGGER_H
#define TILECOSMICALGS_TILECOSMICSTRIGGER_H
//TileCalo include
#include "TileEvent/TileTTL1Container.h"
#include "TileEvent/TileTriggerContainer.h"

// Athena includes
#include "AthenaBaseComps/AthAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "Gaudi/Property.h"

#include <string>
#include <vector>

class CaloLVL1_ID;
class TileTTL1Hash;

#include <string>

#define NMAXTOWERS 1920

/**
 *
 * @class TileCosmicsTrigger
 * @brief Simulates the trigger logic in the Tile standalone board.
 *
 * This class simulates the response of the Tile standalone cosmics
 * trigger board. It reads the TileTTL1 objects created at digitization level
 * and applies the board coincidence logic. The output is not just a flag,
 * but a set of energies per board, so that the trigger logic can be applied
 * for different thresholds at the analysis level.
 */

class TileCosmicsTrigger: public AthAlgorithm {
  public:
    //Constructor
    TileCosmicsTrigger(const std::string& name, ISvcLocator* pSvcLocator);

    //Destructor 
    virtual ~TileCosmicsTrigger();

    //Gaudi Hooks
    StatusCode initialize();
    StatusCode execute(const EventContext& ctx);
    StatusCode finalize();

  private:

// Input variables
//
    std::vector<std::string> m_connectedDrawers[8];
//------------------------------------------------------------------
// Default configuration
// See talk by Christophe Clement March 1, 2007
// -----------------------------------------------------------------
//  Board   Connected to
//   1    bottom LBA
//   2    bottom EBA
//   3    top	 LBA
//   4    top	 EBA
//   5    bottom LBC
//   6    bottom EBC
//   7    top	 LBC
//   8    top	 EBC
//-------------------------------------------------------------------
    Gaudi::Property<std::vector<std::string>> m_connectedDrawers1{this, "ConnectedDrawers1",
        {"LBA43", "LBA44", "LBA45", "LBA46", "LBA47", "LBA48", "LBA49", "LBA50", "LBA51", "LBA52", "LBA53", "LBA54"}};
    Gaudi::Property<std::vector<std::string>> m_connectedDrawers2{this, "ConnectedDrawers2",
        {"EBA43", "EBA44", "EBA45", "EBA46", "EBA47", "EBA48", "EBA49", "EBA50", "EBA51", "EBA52", "EBA53", "EBA54"}};
    Gaudi::Property<std::vector<std::string>> m_connectedDrawers3{this, "ConnectedDrawers3",
        {"LBA11", "LBA12", "LBA13", "LBA14", "LBA15", "LBA16", "LBA17", "LBA18", "LBA19", "LBA20", "LBA21", "LBA22"}};
    Gaudi::Property<std::vector<std::string>> m_connectedDrawers4{this, "ConnectedDrawers4",
        {"EBA11", "EBA12", "EBA13", "EBA14", "EBA15", "EBA16", "EBA17", "EBA18", "EBA19", "EBA20", "EBA21", "EBA22"}};
    Gaudi::Property<std::vector<std::string>> m_connectedDrawers5{this, "ConnectedDrawers5",
        {"LBC43", "LBC44", "LBC45", "LBC46", "LBC47", "LBC48", "LBC49", "LBC50", "LBC51", "LBC52", "LBC53", "LBC54"}};
    Gaudi::Property<std::vector<std::string>> m_connectedDrawers6{this, "ConnectedDrawers6",
        {"EBC43", "EBC44", "EBC45", "EBC46", "EBC47", "EBC48", "EBC49", "EBC50", "EBC51", "EBC52", "EBC53", "EBC54"}};
    Gaudi::Property<std::vector<std::string>> m_connectedDrawers7{this, "ConnectedDrawers7",
        {"LBC11", "LBC12", "LBC13", "LBC14", "LBC15", "LBC16", "LBC17", "LBC18", "LBC19", "LBC20", "LBC21", "LBC22"}};
    Gaudi::Property<std::vector<std::string>> m_connectedDrawers8{this, "ConnectedDrawers8",
        {"EBC11", "EBC12", "EBC13", "EBC14", "EBC15", "EBC16", "EBC17", "EBC18", "EBC19", "EBC20", "EBC21", "EBC22"}};
    Gaudi::Property<float> m_TThreshold{this, "Threshold", 5.0};

// Constants and auxiliary variables
//
    static const int m_NMaxTowers;
    int m_NBOARDS{8};
    int m_NDRAWERSPERBOARD{12};
    //int m_NTOWERSPERDRAWER;

    bool m_connectedTowers[NMAXTOWERS]{};
    bool m_firedTowers[NMAXTOWERS]{};
    float m_towerSum[NMAXTOWERS]{};
    int m_boardOfTower[NMAXTOWERS]{};

// Store Gate & Auxiliary algorithms 
//

    const CaloLVL1_ID* m_TT_ID{nullptr};
    TileTTL1Hash* m_TTHash{nullptr};

    SG::ReadHandleKey<TileTTL1Container> m_ttl1ContainerKey{this, "TileTTL1Container", "TileTTL1Cnt", "Input TileTTL1Container name"};
    SG::WriteHandleKey<TileTriggerContainer> m_triggerContainerKey{this, "TileTriggerContainer","TileTriggerCnt", "Output TileTriggerContainer name"};

};

#endif // TILECOSMICALGS_TILECOSMICSTRIGGER_H
