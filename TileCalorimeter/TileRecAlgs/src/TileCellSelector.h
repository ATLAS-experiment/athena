// -*- C++ -*-

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TILERECALGS_TILESELECTOR_H
#define TILERECALGS_TILESELECTOR_H

// Tile includes
#include "TileEvent/TileDigitsContainer.h"
#include "TileEvent/TileRawChannelContainer.h"
#include "TileEvent/TileDQstatus.h"
#include "TileConditions/ITileDCSTool.h"
#include "TileConditions/ITileBadChanTool.h"

// Calo includes
#include "CaloEvent/CaloCellContainer.h"

// Atlas includes
#include "AthenaBaseComps/AthAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "xAODEventInfo/EventInfo.h"

// Gaudi incldues
#include "GaudiKernel/ToolHandle.h"
#include "Gaudi/Property.h"

#include <string>
#include <vector>

class TileID;
class TileHWID;
class TileCell;
class TileCablingService;
class TileInfo;


/////////////////////////////////////////////////////////////////////////////

class TileCellSelector: public AthAlgorithm {
  public:
    TileCellSelector(const std::string& name, ISvcLocator* pSvcLocator);
    virtual ~TileCellSelector();

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) override;
    virtual StatusCode finalize() override;

  private:

    int Are3FF(std::vector<float> & OptFilterDigits, int OptFilterGain, int ch_type);
    void printCell(const TileCell * cell);

    unsigned int m_counter{0};
    unsigned int m_accept{0};
    unsigned int m_minCell{0};
    unsigned int m_maxCell{0};
    unsigned int m_minChan{0};
    unsigned int m_maxChan{0};
    unsigned int m_jump{0};
    unsigned int m_const{0};
    unsigned int m_overLG{0};
    unsigned int m_overHG{0};
    unsigned int m_underLG{0};
    unsigned int m_underHG{0};
    unsigned int m_dqerr{0};
    unsigned int m_dmuerr{0};
    unsigned int m_warnerr{0};

    const TileID* m_tileID{nullptr};
    const TileHWID* m_tileHWID{nullptr};
    const TileCablingService* m_cabling{nullptr};
    ToolHandle<ITileBadChanTool> m_tileBadChanTool{this, "TileBadChanTool", "TileBadChanTool", "Tile bad channel tool"};
    SG::ReadHandleKey<TileDQstatus> m_dqStatusKey{this, "TileDQstatus", "TileDQstatus", "TileDQstatus key"};
    ToolHandle<ITileDCSTool> m_tileDCS{this, "TileDCSTool", "TileDCSTool", "Tile DCS tool"};

    unsigned int m_runNum{0};
    unsigned int m_lumiBlock{0};
    unsigned int m_evtNum{0};
    unsigned int m_evtBCID{0};
    // flag will contain status of a given event
    // every 4 bits - status of partitions LBA,LBC,EBA,EBC
    // bits 0-3   - there is a signal above threshold in partitions
    // bits 4-7   - there are channels with underflow (sample=0) in partition (since rel 17.2.6.4)
    // bits 8-11  - there are channels with overflow (sample=1023) in partition (since rel 17.2.6.4)
    // bits 12-15 - there are at least 16 drawers with bad quality in partition
    // bits 16-19 - maximal length of consecutive bad area (since rel 17.2.6.5)
    // bits 20-23 - there are at least 16 drawers which are completely masked in partition
    // bits 24-27 - there are at least 16 drawers which do not send data in partition
    // bits 28-31 - reserved for global good/warning/bad status
    // bits 20-27 are also used for module number which gives warning status (since release 17.2.6.5)
    //            in case of warning we are sure that bits which indicates error are not filled
    unsigned int m_tileFlag{0};
    // global status: 0=OK, 1=warning, 2=error
    unsigned int m_tileError{0};

    std::vector<bool> m_chanBad;
    std::vector<float> m_chanEne;
    std::vector<float> m_chanTime;
    std::vector<float> m_chanDsp;
    std::vector<float> m_chanTDsp;
    std::vector<float> m_chanQua;
    std::vector<bool> m_chanSel;
    std::vector<bool> m_chanToSkip;
    std::vector<bool> m_drawerToSkip;

    bool m_readCells{true};
    bool m_readRawChannels{true};
    bool m_readDigits{true};

    SG::ReadHandleKey<CaloCellContainer> m_cellContainerKey{this,"CellContainerName",
                                                            "AllCalo", "Input Calo cell container key"};

    SG::ReadHandleKey<TileDigitsContainer> m_digitsContainerKey{this,"DigitsContainerName",
                                                               "TileDigitsFlt", "Input Tile digits container key"};

    SG::ReadHandleKey<TileRawChannelContainer> m_rawChannelContainerKey{this,"RawChannelContainerName",
                                                                        "TileRawChannelCnt", 
                                                                        "Input Tile raw channel container key"};

    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey{this,"EventInfo",
                                                      "EventInfo", "Input event info key"};

    Gaudi::Property<float> m_minEneCell{this, "MinEnergyCell", -5000., "Cut on cell energy"};
    Gaudi::Property<float> m_maxEneCell{this, "MaxEnergyCell", 1000000., "Cut on cell energy"};
    Gaudi::Property<float> m_minEneChan[3]{{this, "MinEnergyChan", -5000., "Cut on channel energy for normal channels"},
                                           {this, "MinEnergyGap", -10000., "Cut on channel energy for gap/crack scintillators"},
                                           {this, "MinEnergyMBTS", -10000., "Cut on channel energy for MBTS"}};
    Gaudi::Property<float> m_maxEneChan[3]{{this, "MaxEnergyChan", 500000., "Cut on channel energy for normal channels"},
                                           {this, "MaxEnergyGap", 500000., "Cut on channel energy for gap/crack scintillators"},
                                           {this, "MaxEnergyMBTS", 500000., "Cut on channel energy for MBTS"}};
    Gaudi::Property<float> m_minTimeCell{this, "MinTimeCell", -100., "Cut on cell time"};
    Gaudi::Property<float> m_maxTimeCell{this, "MaxTimeCell", 100., "Cut on cell time"};
    Gaudi::Property<float> m_minTimeChan[3]{{this, "MinTimeChan", -100., "Cut on channel time for normal channels"},
                                            {this, "MinTimeGap", -100., "Cut on channel time for gap/crack scintillators"},
                                            {this, "MinTimeMBTS", -100., "Cut on channel time for MBTS"}};
    Gaudi::Property<float> m_maxTimeChan[3]{{this, "MaxTimeChan", 100., "Cut on channel time for normal channels"},
                                            {this, "MaxTimeGap", 100., "Cut on channel time for gap/crack scintillators"},
                                            {this, "MaxTimeMBTS", 100., "Cut on channel time for MBTS"}};
    // pattern - decimal number with up to 5 digits
    // only values 1(=true) and 0(=false) for every digit are used
    // digit 0 set to 1  - accept event if value < min
    // digit 1 set to 1  - accept event if min < value < max
    // digit 2 set to 1  - accept event if value > max
    // digit 3 set to 1  - accept ene only if quality is good
    //                     or accept time if time != 0
    // digit 4 set to 1  - accept ene only if quality is bad
    //                     or accept time if time == 0
    Gaudi::Property<int> m_ptnEneCell{this, "PtnEnergyCell", 101, "Cell energy pattern, accept events only below min (+1), between min-max (+10), above max (+100)"};
    Gaudi::Property<int> m_ptnEneChan[3]{{this, "PtnEnergyChan", 101, "Channel energy pattern for normal channels"},
                                         {this, "PtnEnergyGap", 101, "Channel energy pattern for gap/crack scintillators"},
                                         {this, "PtnEnergyMBTS", 101, "Channel energy pattern for MBTS"}};
    Gaudi::Property<int> m_ptnTimeCell{this, "PtnTimeCell", 10, "Cell time pattern, accept events only below min (+1), between min-max (+10), above max (+100)"};
    Gaudi::Property<int> m_ptnTimeChan[3]{{this, "PtnTimeChan", 10, "Channel time pattern for normal channels"},
                                          {this, "PtnTimeGap", 10, "Channel time pattern for gap/crack scintillators"},
                                          {this, "PtnTimeMBTS", 10, "Channel time pattern for MBTS"}};
    Gaudi::Property<int> m_selectGain{this, "SelectGain", 2, "0 - select LG only,  1 - HG only, 2 - both gains"};
    bool m_skipGain[2]{};
#define ptnlength 5
    bool m_bitEneCell[ptnlength]{};
    bool m_bitTimeCell[ptnlength]{};
    bool m_bitEneChan[3][ptnlength]{};
    bool m_bitTimeChan[3][ptnlength]{};
    Gaudi::Property<float> m_secondMaxLevel{this, "SecondMaxLevel", 0.3, "Sample below max should be above (max-min)*SecondMaxLevel"};
    Gaudi::Property<float> m_jumpDeltaHG{this, "JumpDeltaHG", 50.0, "Minimal jump in high gain"};
    Gaudi::Property<float> m_jumpDeltaLG{this, "JumpDeltaLG", 10.0, "Minimal jump in low gain"};
    Gaudi::Property<float> m_pedDeltaHG{this, "PedDetlaHG", 4.1, "Max variation of \"const\" value in high gain"};
    Gaudi::Property<float> m_pedDeltaLG{this, "PedDetlaLG", 4.1, "Max variation of \"const\" value in low gain"};
    Gaudi::Property<int> m_constLength{this, "ConstLength", 6, "Min number of consecutive samples of the same value"};
    Gaudi::Property<int> m_minBadDMU{this, "MinBadDMU", 4, "Min number of bad DMUs to accept event"};
    Gaudi::Property<int> m_maxBadDMU{this, "MaxBadDMU", 15, "Max number of bad DMUs to accept event"};
    Gaudi::Property<int> m_minBadMB{this, "MinBadMB", 4, "Min number of bad motherboards in a drawer to accept event"};
    Gaudi::Property<bool> m_skipEmpty{this, "SkipEmpty", true, "Ignore empty channels in selection or not"};
    Gaudi::Property<bool> m_skipMasked{this, "SkipMasked", true, "Ignore masked channels in selection or not"};
    Gaudi::Property<bool> m_skipMBTS{this, "SkipMBTS", true, "Ignore MBTS channels in selection or not"};
    Gaudi::Property<bool> m_checkDCS{this, "CheckDCS", true, "Additional check for DCS status"};
    Gaudi::Property<bool> m_checkJumps{this, "CheckJumps", true, "Global flag which allows to switch on/off all checks in digits"};
    Gaudi::Property<bool> m_checkDMUs{this, "CheckDMUs", true, "Global flag which allows to switch on/off DMU checks"};
    Gaudi::Property<bool> m_checkOverLG{this, "CheckOverLG", true, "Select events with overflow in low gain"};
    Gaudi::Property<bool> m_checkOverHG{this, "CheckOverHG", false, "Select events with overflow in high gain"};
    Gaudi::Property<bool> m_checkUnderLG{this, "CheckUnderLG", false, "Select events with underflow in low gain"};
    Gaudi::Property<bool> m_checkUnderHG{this, "CheckUnderHG", false, "Select events with underflow in high gain"};
    Gaudi::Property<float> m_overflowLG{this, "OverflowLG", -0.1, "Threshold for overflow in low gain (smaller than ADCmax by this value)"};
    Gaudi::Property<float> m_overflowHG{this, "OverflowHG", -1.1, "Threshold for overflow in high gain (smaller than ADCmax by this value)"};
    Gaudi::Property<float> m_underflowLG{this, "UnderflowLG", 0.1, "Threshold for underflow in low gain"};
    Gaudi::Property<float> m_underflowHG{this, "UnderflowHG", 2.1, "Threshold for underflow in high gain"};
    Gaudi::Property<bool> m_checkWarning{this, "CheckWarning", false, "Select events with warning status in TileCal status word"};
    Gaudi::Property<bool> m_checkError{this, "CheckError", false, "Select events with error status in TileCal status word"};
    Gaudi::Property<bool> m_printOnly{this, "PrintOnly", false, "Only print accepted events, but do not accept anything"};

    Gaudi::Property<std::vector<int>> m_drawer{this, "DrawerToDump", {}, "Drawers for which all channels should be printed"};
    Gaudi::Property<std::vector<int>> m_drawerToCheck{this, "DrawerToCheck", {}, "Drawers for which all checks should be performed"};
    Gaudi::Property<std::vector<int>> m_chanToCheck{this, "ChannelToCheck", {}, "Channels for which all checks should be performed"};

    Gaudi::Property<int> m_maxVerboseCnt{this, "MaxVerboseCnt", 20, "Max number of verbose output lines about drawer off"};
    std::vector<int> m_nDrawerOff;

    Gaudi::Property<std::string> m_infoName{this, "TileInfoName", "TileInfo", "Name of TileInfo object in DetectorStore"};
    const TileInfo* m_tileInfo{nullptr};
    float m_ADCmaxMinusEps = 0.0F;
    float m_ADCmaskValueMinusEps = 0.0F;
};

#endif // TILERECALGS_TILESELECTOR_H
