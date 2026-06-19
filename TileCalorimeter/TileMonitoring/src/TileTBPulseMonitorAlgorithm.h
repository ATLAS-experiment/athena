/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TILEMONITORING_TILETBPULSEMONITORALGORITHM_H
#define TILEMONITORING_TILETBPULSEMONITORALGORITHM_H

#include "TileEvent/TileRawChannelContainer.h"
#include "TileEvent/TileDigitsContainer.h"
#include "TileConditions/TileCablingSvc.h"

#include "AthenaMonitoring/AthMonitorAlgorithm.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "StoreGate/ReadHandleKey.h"

class TileInfo;
class TileHWID;

/** @class TileTBPulseMonitorAlgorithm
 *  @brief Class for Tile TB Pulse based monitoring
 */

class TileTBPulseMonitorAlgorithm : public AthMonitorAlgorithm {

  public:

    using AthMonitorAlgorithm::AthMonitorAlgorithm;
    virtual ~TileTBPulseMonitorAlgorithm() = default;
    virtual StatusCode initialize() override;
    virtual StatusCode fillHistograms(const EventContext& ctx) const override;

  private:

    SG::ReadHandleKey<TileRawChannelContainer> m_rawChannelContainerKey{this,
        "TileRawChannelContainer", "TileRawChannelCnt", "Input Tile raw channel container key"};

    SG::ReadHandleKey<TileDigitsContainer> m_digitsContainerKey{this,
        "TileDigitsContainer", "TileDigitsCnt", "Input Tile digits container key"};

    Gaudi::Property<std::string> m_tileInfoName{this,
        "TileInfo", "TileInfo", "Name of TileInfo object in Detector Store"};

    /**
     * @brief Name of Tile cabling service
     */
    ServiceHandle<TileCablingSvc> m_cablingSvc{ this,
        "TileCablingSvc", "TileCablingSvc", "The Tile cabling service"};

    Gaudi::Property<std::vector<int>> m_fragIDs{this,
        "TileFragIDs", {0x100, 0x101, 0x200, 0x201, 0x402}, "Tile Frag IDs of modules to process."};

    std::map<std::string, int> m_pulseGroups;
    std::map<std::string, int> m_pulseProfileGroups;

    const TileHWID* m_tileHWID{nullptr};
    const TileInfo* m_tileInfo{nullptr};
    int m_t0SamplePosition{};
};


#endif // TILEMONITORING_TILETBPULSEMONITORALGORITHM_H
