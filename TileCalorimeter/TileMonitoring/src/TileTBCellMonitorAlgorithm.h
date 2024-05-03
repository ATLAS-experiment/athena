/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TILEMONITORING_TILETBCELLMONITORALGORITHM_H
#define TILEMONITORING_TILETBCELLMONITORALGORITHM_H

#include "TileConditions/TileCablingSvc.h"
#include "TileCalibBlobObjs/TileCalibUtils.h"

#include "CaloEvent/CaloCellContainer.h"

#include "AthenaMonitoring/AthMonitorAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include <array>

class TileInfo;
class TileID;
class TileHWID;

/** @class TileTBCellMonitorAlgorithm
 *  @brief Class for Tile TB Cell based monitoring
 */

class TileTBCellMonitorAlgorithm : public AthMonitorAlgorithm {

  public:

    using AthMonitorAlgorithm::AthMonitorAlgorithm;
    virtual ~TileTBCellMonitorAlgorithm() = default;
    virtual StatusCode initialize() override;
    virtual StatusCode fillHistograms(const EventContext& ctx) const override;

  private:

    /**
     * @brief Name of Tile cabling service
     */
    ServiceHandle<TileCablingSvc> m_cablingSvc{ this,
        "TileCablingSvc", "TileCablingSvc", "The Tile cabling service"};

    SG::ReadHandleKey<CaloCellContainer> m_caloCellContainerKey{this,
        "CaloCellContainer", "AllCalo", "Calo cell container name"};

    Gaudi::Property<std::vector<int>> m_fragIDs{this,
        "TileFragIDs", {0x100, 0x101, 0x200, 0x201, 0x402}, "Tile Frag IDs of modules to process."};

    Gaudi::Property<int> m_TBperiod{this,
        "TBperiod", 2016, "Tile TB period."};

    Gaudi::Property<std::vector<std::string>> m_masked{this,
        "Masked", {}, "Masked channels: 'module gain channel,channel' (channels are separated by comma)"};

    Gaudi::Property<float> m_energyThresholdForTime{this,
        "EnergyThresholdForTime", 100.0F, "Energy threshold for timing in MeV"};

    Gaudi::Property<bool> m_fillHistogramsPerChannel{this,
        "fillHistogramsPerChannel", true, "Fill time and energy histograms per channel"};

    std::map<std::string, int> m_sampleEnergyGroups;
    std::map<std::string, int> m_energyGroups;
    std::map<std::string, int> m_energyDiffGroups;
    std::map<std::string, int> m_energy2VsEnergy1Groups;
    std::map<std::string, int> m_timeGroups;
    std::map<std::string, int> m_timeDiffGroups;
    std::map<std::string, int> m_time2VsTime1Groups;
    std::map<std::string, int> m_channelEnergyGroups;
    std::map<std::string, int> m_channelTimeGroups;

    const TileID* m_tileID{nullptr};
    const TileHWID* m_tileHWID{nullptr};
    const TileCablingService* m_cabling{nullptr};

    std::array<unsigned int, TileCalibUtils::MAX_DRAWERIDX> m_drawerIdxToROS{};
    std::array<unsigned int, TileCalibUtils::MAX_DRAWERIDX> m_drawerIdxToDrawer{};
    std::array<bool, TileCalibUtils::MAX_DRAWERIDX> m_monitoredDrawerIdx{};
    std::array<std::array<unsigned char, TileCalibUtils::MAX_CHAN>, TileCalibUtils::MAX_DRAWERIDX> m_maskedChannels{{}};
};


#endif // TILEMONITORING_TILETBCELLMONITORALGORITHM_H
