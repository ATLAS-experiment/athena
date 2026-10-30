/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 TileInfoDump
 -----------------------------------------
 Test algorithm to dump out calibration constants from the TileInfoSvc


 2007-05-14      nils.gollub@cern.ch
 2008-06-06      lukas.pribyl@cern.ch
 ***************************************************************************/

#ifndef TILECALIBALGS_TILEINFODUMP_H
#define TILECALIBALGS_TILEINFODUMP_H

// Gaudi includes
#include "GaudiKernel/ITHistSvc.h"
#include "GaudiKernel/ToolHandle.h"

// Athena includes
#include "AthenaBaseComps/AthAlgorithm.h"
#include "xAODEventInfo/EventInfo.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "TileConditions/ITileBadChanTool.h"
#include "TileConditions/TileCondToolEmscale.h"
#include "TileConditions/TileCondToolIntegrator.h"
#include "TileConditions/TileCondToolTiming.h"
#include "TileConditions/TileCondToolPulseShape.h"
#include "TileConditions/TileCondToolNoiseSample.h"
#include "TileConditions/TileCondToolNoiseRawChn.h"
#include "TileConditions/TileCondToolOfcCool.h"
#include "TileConditions/TileCondToolMuID.h"
#include "TileConditions/TileCondToolAutoCr.h"
#include "TileConditions/TileInfo.h"
#include "CaloDetDescr/CaloDetDescrManager.h"

class TileHWID;
class TileID;
class TH2F;

/**
 ** Algorithm to dump out calibration constants from the TileInfoSvc
 **/

class TileInfoDump: public AthAlgorithm {
  public:
    TileInfoDump(const std::string& name, ISvcLocator* pSvcLocator);
    ~TileInfoDump();

    StatusCode initialize();
    StatusCode execute(const EventContext& ctx);
    StatusCode finalize();

  private:
    Gaudi::Property<bool> m_isOnline{this, "AthenaIsOnline", false, "Availability of COOL folders depends on Athena mode"};

    void printEmscale();
    Gaudi::Property<bool> m_printEmscale{this, "PrintEmscale", false, "Switch on calibration chain printout"};

    void printEmscaleOnl();
    Gaudi::Property<bool> m_printEmscaleOnl{this, "PrintEmscaleOnl", false, "Switch on online calibration chain printout"};

    void printLaser();
    Gaudi::Property<bool> m_printLaser{this, "PrintLaser", false, "Switch on all laser constants printout"};

    void printIntegrator();
    Gaudi::Property<bool> m_printIntegrator{this, "PrintIntegrator", false, "Switch on integrator printout"};

    void printPedestals();
    Gaudi::Property<bool> m_printPedestals{this, "PrintPedestals", false, "Switch on Pedstals print out"};

    void printTimingCorrections();
    Gaudi::Property<bool> m_printTimingCorrections{this, "PrintTimingCorrections", false, "Switch on Timing Corrections print out"};

    void printPulseShapes();
    Gaudi::Property<bool> m_printPulseShapes{this, "PrintPulseShapes", false, "Switch on Pulse Shape print out"};
    Gaudi::Property<float> m_printPulseShapesStep{this, "PrintPulseShapesStep", 0.5, "Time step (ns) of PulseShapes print out"};

    void printBadChannels(const EventContext& ctx);
    Gaudi::Property<bool> m_printBadChannels{this, "PrintBadChannels", false, "Switch on Bad Channels print out"};

    void printBadCells(const EventContext& ctx);
    Gaudi::Property<bool> m_printBadCells{this, "PrintBadCells", false, "Switch on Bad Cells print out"};

    void printOfcs();
    Gaudi::Property<bool> m_printOfcsCool{this, "PrintOfcsCool", false, "Switch on OFC print out"};

    void printMuID();
    Gaudi::Property<bool> m_printMuID{this, "PrintMuID", false, "Switch on MuID threshold print out"};

    void print1gNoise();
    Gaudi::Property<bool> m_print1gNoise{this, "Print1gNoise", false, "Switch on 1g noise print out"};

    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey{this,
      "EventInfo", "EventInfo", "EventInfo key"};

    //=== Non-property members
    ToolHandle<ITileBadChanTool> m_tileBadChanTool{this,
      "TileBadChanTool", "TileBadChanTool", "Tile bad channel tool"};
    ToolHandle<TileCondToolEmscale> m_tileToolEmscale{this,
      "TileCondToolEmscale", "TileCondToolEmscale", "Tile em scale tool"};
    ToolHandle<TileCondToolIntegrator> m_tileToolIntegrator{this,
      "TileCondToolIntegrator", "TileCondToolIntegrator", "Tile integrator tool"};
    ToolHandle<TileCondToolTiming> m_tileToolTiming{this,
      "TileCondToolTiming", "TileCondToolTiming", "Tile timing tool"};
    ToolHandle<TileCondToolPulseShape> m_tileToolPulseShape{this,
      "TileCondToolPulseShape", "TileCondToolPulseShape", "Tile pulse shape tool"};
    ToolHandle<TileCondToolNoiseSample> m_tileToolNoiseSample{this,
      "TileCondToolNoiseSample", "TileCondToolNoiseSample", "Tile noise sample tool"};
    ToolHandle<TileCondToolAutoCr> m_tileToolAutoCr{this,
      "TileCondToolAutoCr", "TileCondToolAutoCr", "Tile auto cr tool"};
    ToolHandle<TileCondToolNoiseRawChn> m_tileToolNoiseRawChn{this,
      "TileCondToolNoiseRawChn", "TileCondToolNoiseRawChn", "Tile raw channel noise tool"};
    ToolHandle<TileCondToolOfcCool> m_tileToolOfcCool{this,
      "TileCondToolOfcCool", "TileCondToolOfcCool", "Tile OfcCool tool"};
    ToolHandle<TileCondToolMuID> m_tileToolMuID{this,
      "TileCondToolMuID", "TileCondToolMuID", "Tile MuID tool"};

    //=== a handle on the Hist/TTree registration service 
    ServiceHandle<ITHistSvc> m_thistSvc;

    const TileHWID* m_tileHWID{nullptr};
    const TileID* m_tileID{nullptr};
    SG::ReadCondHandleKey<CaloDetDescrManager> m_caloMgrKey{this,
	"CaloDetDescrManager", "CaloDetDescrManager", "SG Key for CaloDetDescrManager in the Condition Store" };

    TH2F* m_h_badCellA{nullptr};
    TH2F* m_h_badCellBC{nullptr};
    TH2F* m_h_badCellD{nullptr};
    TH2F* m_h_badCellGap{nullptr};
    TH2F* m_h_badCell{nullptr};

    Gaudi::Property<unsigned int> m_printOfcRos{this, "PrintOfcRos", 0, "Print OFC for this ros (0 by default)"};
    Gaudi::Property<unsigned int> m_printOfcDrawer{this, "PrintOfcDrawer", 0, "Print OFC for this drawer (0 by default)"};
    Gaudi::Property<unsigned int> m_printOfcChannel{this, "PrintOfcChannel", 0, "Print OFC for this channel (0 by default)"};

    //=== TileInfo
    Gaudi::Property<std::string> m_infoName{this, "TileInfoName", "TileInfo"};
    const TileInfo* m_tileInfo{nullptr};
    int m_i_ADCmax = 0;
};

#endif 
