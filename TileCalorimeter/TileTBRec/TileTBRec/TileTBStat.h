/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

//****************************************************************************
// Filename : TileTBNtuple.h
// Author   : Paulo da Silva
// Created  : Oct, 2003
//
// DESCRIPTION
// 
//    To create TB beam Ntuple file from Digits container
//
// Properties (JobOption Parameters):
//
//    TileDigitsContainer     string   key value of Digits in TDS 
//    TileBeamElemContainer   string   key of BeamElems in TDS
//
// BUGS:
//  
// History:
//  
//  
//****************************************************************************

/* @class TileTBStat
 @brief Class to produce run statistics. This is used to store run informations in TileCal commissioning database.
 */

#ifndef TileTBStat_H
#define TileTBStat_H

// Gauid includes
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"


// Athena incldues
#include "AthenaBaseComps/AthAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "ByteStreamCnvSvcBase/IROBDataProviderSvc.h"

#include "TileEvent/TileDQstatus.h"
#include "TileEvent/TileLaserObject.h"
#include "TileByteStream/TileHid2RESrcID.h"

class TileID;
class TileHWID;

#include <string>
#include <vector>
#include <map>

#include <inttypes.h>

class StatDouble {
  public:
    StatDouble();
    void addValue(double Value);
    double getMean();
    double getChi2();
    void print(const char* s, bool minMaxOnly = false);
    int Counter;
    double Min, Max, Sum, SumSq;
};

class StatInt {
  public:
    StatInt();
    void addValue(int Value);
    void print(const char* s, bool minMaxOnly = false);
    std::map<int, int> checkOut;
};

class TileTBStat: public AthAlgorithm {
  public:
    //Constructor
    TileTBStat(const std::string& name, ISvcLocator* pSvcLocator);

    //Destructor 
    virtual ~TileTBStat();

    //Gaudi Hooks
    virtual StatusCode initialize() override;
    virtual StatusCode execute() override;
    virtual StatusCode finalize() override;

  private:

    Gaudi::Property<bool> m_printAllEvents{this,
       "PrintAllEvents", false, "Print all events"};
    Gaudi::Property<bool> m_detectDummyFragments{this,
       "DetectDummyFragments", false, "Detect dummy fragments"};
    SG::ReadCondHandleKey<TileHid2RESrcID> m_hid2RESrcIDKey{this,
       "TileHid2RESrcID", "TileHid2RESrcID", "TileHid2RESrcID key"};
    SG::ReadHandleKey<TileDQstatus> m_dqStatusKey{this,
       "TileDQstatus", "TileDQstatus", "Tile DQ status key"};
    SG::ReadHandleKey<TileLaserObject> m_laserObjectKey{this,
       "TileLaserObject", "TileLaserObj", "Tile laser object key"};

   /**
     * @brief Name of ROB data provider service
     */
    ServiceHandle<IROBDataProviderSvc> m_robSvc{this,
       "ROBDataProviderSvc", "ROBDataProviderSvc", "The ROB data provider service"};

    int m_evtNr;
    StatDouble m_laserBoxTemp;
    StatDouble m_laserDiodeTemp;
    StatDouble m_laserBoxHum;
    StatDouble m_gasFlow;
    StatInt m_cisMode;
    StatInt m_cisSamples;
    StatInt m_cisPipeline;
    StatInt m_cisI3Delay;
    StatInt m_cisDAC;
    StatInt m_cisCap;
    StatInt m_cisEvent;
    StatInt m_cisPhase;
    StatInt m_cisCard;
    StatInt m_lasFiltNum;
    StatInt m_lasAmp;
    StatInt m_lasAlphaPos;
    int m_lasStatus, m_checkOn, m_checkOff, m_check0, m_check1, m_check2;
    int m_Alarm, m_Error, m_ShOpen, m_ShClose;

    uint32_t m_runNo;
    uint32_t m_evTime;
    uint32_t m_evtNo;
    int m_trigType;
    uint32_t m_prevTrig;
    uint32_t m_evtBegin;
    uint32_t m_timeBegin;
    uint32_t m_timeStart;
    uint32_t m_timeLast;
    uint32_t m_evtMin;
    uint32_t m_evtMax;
    uint32_t m_calibMode;
    uint32_t m_spillPattern;
    uint32_t m_nSpill;
    uint32_t m_nEvt[5];
    uint32_t m_cisPar[16];
    uint32_t m_cis1st[16];
    uint32_t m_cisBeg[19];
    uint32_t m_cisEnd[19];

    /* the ROD data sub-fragment */
    typedef struct T_RobRodFragMap {
        uint32_t L1type;
        uint32_t ROBid;
        uint32_t RODid;
        std::vector<uint32_t> fragID;
    } T_RobRodFragMap;

    /* the ROD data sub-fragment */
    typedef struct T_RodDataFrag {
        uint32_t size;
        uint32_t id;
        uint32_t data[1];
    } T_RodDataFrag;

    std::vector<T_RobRodFragMap> m_fragMap;
    std::vector<T_RobRodFragMap> m_beamFragMap;

    void find_frag(const uint32_t* data, unsigned int size, T_RobRodFragMap & ROBfrag);

    uint32_t m_nEventsPerTrigger[257];
};

#endif
