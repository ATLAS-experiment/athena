/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TILEMONITORING_TILEDIGITSFLXALGORITHM_H
#define TILEMONITORING_TILEDIGITSFLXALGORITHM_H

// Tile includes
#include "TileEvent/TileDigitsContainer.h"


// Atlas includes
#include "AthenaMonitoring/AthMonitorAlgorithm.h"

#include "StoreGate/ReadHandleKey.h"

class TileHWID;

class TileDigitsFlxMonitorAlgorithm : public AthMonitorAlgorithm {

  public:

    using AthMonitorAlgorithm::AthMonitorAlgorithm;
    virtual ~TileDigitsFlxMonitorAlgorithm() = default;
    virtual StatusCode initialize() override;
    virtual StatusCode fillHistograms( const EventContext& ctx ) const override;

  private:

    SG::ReadHandleKey<TileDigitsContainer> m_digitsContainerKeyLegacy{this,
        "TileDigitsContainerLegacy", "TileDigitsCnt", "Tile digits container"};

    SG::ReadHandleKey<TileDigitsContainer> m_digitsContainerKeyFlx{this,
        "TileDigitsContainerFlx", "TileDigitsFlxCnt", "Tile digits container for Felix"};

    const TileHWID* m_tileHWID{nullptr};

    Gaudi::Property<std::vector<int>> m_fragIDsToCompare{this, "TileFragIDsToCompare", {0x201, 0x402}, "Tile Frag IDs of modules to compare."};

    Gaudi::Property<unsigned int> m_firstSample{this, "FirstSample", 0, "First sample to put into histogram"};
    Gaudi::Property<unsigned int> m_lastSample{this,  "LastSample", 15, "Last sample to put into histogram (inclusive)"};
    Gaudi::Property<unsigned int> m_felixOffset{this, "FelixOffset", 0, "Offset for comparison of Felix samples w.r.t. Legacy samples"};
    Gaudi::Property<unsigned int> m_felixScale{this,  "FelixScale",  1, "Scale factor between Felix and Legacy ADC counts"}; // 1 for pedestal run or 4 for physics run

    unsigned int m_firstFelix{};
    unsigned int m_nSamples{};

};
#endif // TILEMONITORING_TILEDIGITSFLXALGORITHM_H
