/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TILEDIGINOISECALIBALG_H
#define TILEDIGINOISECALIBALG_H

// ********************************************************************
//
// NAME:     TileDigiNoiseCalib.h
// PACKAGE:  TileCalib
//
// AUTHOR:   Luca Fiorini (Luca.Fiorini@cern.ch)
// 
// February 2008	
// ********************************************************************

// Gaudi includes
#include "GaudiKernel/ToolHandle.h"

// Athena includes
#include "AthenaBaseComps/AthAlgorithm.h"
#include "xAODEventInfo/EventInfo.h"
#include "StoreGate/ReadHandleKey.h"

// Tile includes
#include "TileConditions/TileCablingService.h"
#include "TileEvent/TileDQstatus.h"
#include "TileEvent/TileDigitsContainer.h"
#include "TileCalibBlobObjs/TileCalibUtils.h"
#include "TileRecUtils/TileRawChannelBuilderFlatFilter.h"

#include <cmath>
#include <vector>
#include <string>
#include <map>
#include <cstdint>

#define NVALS 36

// Forward declaration
class TileHWID;
class TileBeamElemContByteStreamCnv;
class TileOFCorrelation;

class TileDigiNoiseCalibAlg: public AthAlgorithm {
  public:
    TileDigiNoiseCalibAlg(const std::string& name, ISvcLocator* pSvcLocator);
    virtual ~TileDigiNoiseCalibAlg();

    // Functions
    virtual StatusCode initialize() override; //only array initialization is done here
    virtual StatusCode execute(const EventContext& ctx) override;
    virtual StatusCode finalize() override;

  private:

    enum trigtype {
      Unknown = 0
      , Phys = 1
      , Las = 2
      , Ped = 4
      , Cis = 8
    };

    StatusCode FirstEvt_initialize(); // real initialization is done in this method

    StatusCode fillDigits (const TileDQstatus* theDQstatus); //accumulate statistics for pedestal, lfn, hfn, noise_cov calculation
    void finalDigits(); // pedestal, lfn, hfn, noise_cov calculation is done here
    void StoreRunInfo (const TileDQstatus* dqStatus); // called only at the first event. General variables

    inline int digiChannel2PMT(int ros, int chan) {
      return (abs(m_cabling->channel2hole(ros, chan)) - 1);
    }

    // If data should be put in calib mode
    Gaudi::Property<bool> m_calibMode{this, "CalibMode", true};

    // by default we use channels
    Gaudi::Property<bool> m_usePMT{this, "usePMT", false};

    // Toggle usage of average correlation calcualtion or rigorous
    // Default: Rigorous
    Gaudi::Property<bool> m_doAvgCorr{this, "DoAvgCorr", false};
    
    // Toggle usage of robust method to calculate covariance matrix or standard method    
    // Default: use standard                                                              
    Gaudi::Property<bool> m_doRobustCov{this, "DoRobustCov", false};

    SG::ReadHandleKey<TileDQstatus> m_dqStatusKey{this, "TileDQstatus", "TileDQstatus"};
    SG::ReadHandleKey<TileDigitsContainer> m_digitsContainerKey{this,
      "TileDigitsContainer", "TileDigitsCnt", "Tile digits container"};
    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey{this,
      "EventInfo", "EventInfo", "EventInfo key"};

  // Tools / storegate info
    ToolHandle<TileRawChannelBuilderFlatFilter> m_adderFilterAlgTool{this,
      "TileAdderFlatFilter","TileRawChannelBuilderFlatFilter/TileAdderFlatFilter","Tile adder flat filter"};
    TileBeamElemContByteStreamCnv* m_beamCnv{nullptr};
    const TileCablingService* m_cabling{nullptr};
    TileOFCorrelation* m_tileOFCorrelation{nullptr};

    // Identifiers
    const TileID* m_tileID{nullptr};
    const TileHWID* m_tileHWID{nullptr};

    const uint32_t* m_cispar{nullptr};

    // Name of ntuple and file
    Gaudi::Property<std::string> m_ntupleID{this, "NtupleID", "Digi_NoiseCalib"};
    Gaudi::Property<std::string> m_file{this, "FileNamePrefix", "Digi_NoiseCalib"};
    Gaudi::Property<int64_t> m_treeSize{this, "TreeSize", 16000000000LL};
    Gaudi::Property<int> m_nSamples{this, "NSamples", 7};

    // Container Parameters
    std::string m_digitsContainer;
    Gaudi::Property<std::string> m_beamElemContainer{this, "TileBeamElemContainer", "TileBeamElemCnt"};
    Gaudi::Property<std::string> m_dspRawChannelContainer{this, "TileRawChannelContainerDsp", "TileRawChannelCnt"};

    using Tile = TileCalibUtils;

    double (*m_sumPed2)[Tile::MAX_DRAWER][Tile::MAX_CHAN][Tile::MAX_GAIN];
    double (*m_sumRms2)[Tile::MAX_DRAWER][Tile::MAX_CHAN][Tile::MAX_GAIN];
    double (*m_meanAmp)[Tile::MAX_DRAWER][Tile::MAX_CHAN][Tile::MAX_GAIN];
    double (*m_meanAmp_ij)[Tile::MAX_DRAWER][Tile::MAX_CHAN][Tile::MAX_CHAN][Tile::MAX_GAIN];

    // event number
    int m_evtNr{-1};
    int (*m_evt)[Tile::MAX_DRAWER][Tile::MAX_CHAN][Tile::MAX_GAIN];

    // Trigger items
    int m_time{0};
    int m_year{0};
    int m_month{0};
    int m_day{0};
    int m_yday{0};
    int m_hour{0};
    int m_min{0};
    Gaudi::Property<int> m_run{this, "RunNumber", 0};
    int m_trigType{0};
    uint8_t (*m_ros)[Tile::MAX_DRAWER][Tile::MAX_CHAN][Tile::MAX_GAIN];
    uint8_t (*m_drawer)[Tile::MAX_DRAWER][Tile::MAX_CHAN][Tile::MAX_GAIN];
    uint8_t (*m_channel)[Tile::MAX_DRAWER][Tile::MAX_CHAN][Tile::MAX_GAIN];
    bool (*m_gain)[Tile::MAX_DRAWER][Tile::MAX_CHAN][Tile::MAX_GAIN];
    float (*m_ped)[Tile::MAX_DRAWER][Tile::MAX_CHAN][Tile::MAX_GAIN];
    float (*m_lfn)[Tile::MAX_DRAWER][Tile::MAX_CHAN][Tile::MAX_GAIN];
    float (*m_hfn)[Tile::MAX_DRAWER][Tile::MAX_CHAN][Tile::MAX_GAIN];
    float (*m_noise_cov)[Tile::MAX_DRAWER][Tile::MAX_GAIN];
    float (*m_auto_corr)[Tile::MAX_DRAWER][Tile::MAX_CHAN][Tile::MAX_GAIN][NVALS];
};

#endif // TILENOISECALIBALG_H
