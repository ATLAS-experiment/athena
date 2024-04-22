/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "TileTBCellMonitorAlgorithm.h"
#include "AthenaKernel/Units.h"
#include "AthenaMonitoringKernel/MonitoredScalar.h"
#include "TileCalibBlobObjs/TileCalibUtils.h"
#include "TileConditions/TileInfo.h"
#include "TileIdentifier/TileHWID.h"
#include "TileEvent/TileCell.h"

#include "CaloIdentifier/TileID.h"
#include "StoreGate/ReadHandle.h"

#include <algorithm>
#include <string>

StatusCode TileTBCellMonitorAlgorithm::initialize() {

  ATH_MSG_INFO("in initialize()");
  ATH_CHECK( AthMonitorAlgorithm::initialize() );

  ATH_CHECK( m_caloCellContainerKey.initialize() );

  ATH_CHECK( m_cablingSvc.retrieve() );
  m_cabling = m_cablingSvc->cablingService();

  ATH_CHECK( detStore()->retrieve(m_tileID) );
  ATH_CHECK( detStore()->retrieve(m_tileHWID) );

  std::vector<std::string> modules;
  for (int fragID : m_fragIDs) {
    int ros = fragID >> 8;
    int drawer = fragID & 0x3F;
    modules.push_back(TileCalibUtils::getDrawerString(ros, drawer));
    m_monitoredDrawerIdx[TileCalibUtils::getDrawerIdx(ros, drawer)] = true;
  }

  std::ostringstream os;
  if ( m_fragIDs.size() != 0) {
    std::sort(m_fragIDs.begin(), m_fragIDs.end());
    for (int fragID : m_fragIDs) {
      unsigned int ros    = fragID >> 8;
      unsigned int drawer = fragID & 0xFF;
      std::string module = TileCalibUtils::getDrawerString(ros, drawer);
      os << " " << module << "/0x" << std::hex << fragID << std::dec;
    }
  } else {
    os << "NONE";
  }

  ATH_MSG_INFO("Monitored modules/frag ID:" << os.str());


  std::map<std::string, unsigned int> roses = { {"AUX", 0}, {"LBA", 1}, {"LBC", 2}, {"EBA", 3}, {"EBC", 4} };
  for (std::string maskedModuleChannels : m_masked) {

    std::string module = maskedModuleChannels.substr(0, 5);
    std::string partition = module.substr(0, 3);
    if (roses.count(partition) != 1) {
      ATH_MSG_WARNING("There no such partition: " << partition << " in module: " << module
		      << " => skip because of bad format: " << maskedModuleChannels);
      continue;
    }

    unsigned int drawer = std::stoi(module.substr(3, 2)) - 1;
    if (drawer >= TileCalibUtils::MAX_DRAWER) {
      ATH_MSG_WARNING("There no such drawer: " << drawer + 1 << " in module: " << module
		      << " => skip because of bad format: " << maskedModuleChannels);
      continue;
    }

    unsigned int ros = roses.at(partition);
    unsigned int drawerIdx = TileCalibUtils::getDrawerIdx(ros, drawer);

    std::string gain = maskedModuleChannels.substr(5,7);
    unsigned int adc = std::stoi(gain);

    if (adc >= TileCalibUtils::MAX_GAIN) {
      ATH_MSG_WARNING("There no such gain: " << gain << " => skip because of bad format: " << maskedModuleChannels);
      continue;
    }

    std::stringstream channels(maskedModuleChannels.substr(7));
    std::string channel;
    while (std::getline(channels, channel, ',')) {
      if (!channel.empty()) {
        unsigned int chan = std::stoi(channel);
        if (chan >= TileCalibUtils::MAX_CHAN) {
          ATH_MSG_WARNING("There no such channel: " << chan << " in channels: " << channels.str()
                          << " => skip because of bad format: " << maskedModuleChannels);
          continue;
        }
        m_maskedChannels[drawerIdx][chan] |= (1U << adc);
        ATH_MSG_INFO(TileCalibUtils::getDrawerString(ros, drawer) << " ch" << chan << (adc ? " HG" : " LG") << ": masked!");
      }
    }

  }

  for (unsigned int ros = 0; ros < TileCalibUtils::MAX_ROS; ++ros) {
    for (unsigned int drawer = 0; drawer < TileCalibUtils::getMaxDrawer(ros); ++drawer) {
      unsigned int drawerIdx = TileCalibUtils::getDrawerIdx(ros, drawer);
      m_drawerIdxToROS[drawerIdx] = ros;
      m_drawerIdxToDrawer[drawerIdx] = drawer;
    }
  }

  using namespace Monitored;

  m_sampleEnergyGroups = buildToolMap<int>(m_tools, "TileSampleEnergy", modules);
  m_energyGroups = buildToolMap<int>(m_tools, "TileCellEnergy", modules);
  m_energyDiffGroups = buildToolMap<int>(m_tools, "TileCellEnergyDiff", modules);
  m_energy2VsEnergy1Groups = buildToolMap<int>(m_tools, "TileCellEnergyLeftVsRightPMT", modules);
  m_timeGroups = buildToolMap<int>(m_tools, "TileCellTime", modules);
  m_timeDiffGroups = buildToolMap<int>(m_tools, "TileCellTimeDiff", modules);
  m_time2VsTime1Groups = buildToolMap<int>(m_tools, "TileCellTimeLeftVsRightPMT", modules);

  if (m_fillHistogramsPerChannel) {
    m_channelEnergyGroups = buildToolMap<int>(m_tools, "TileChannelEnergy", modules);
    m_channelTimeGroups = buildToolMap<int>(m_tools, "TileChannelTime", modules);
  }

  return StatusCode::SUCCESS;
}


StatusCode TileTBCellMonitorAlgorithm::fillHistograms( const EventContext& ctx ) const {

  // In case you want to measure the execution time
  auto timer = Monitored::Timer("TIME_execute");

  using Athena::Units::GeV;

  constexpr int allSamples = TileID::SAMP_E; // To be used to keep total energy from all samples
  constexpr int nSamples = allSamples + 1;

  double sampleEnergy[TileCalibUtils::MAX_DRAWERIDX][nSamples] = {{0}};

  SG::ReadHandle<CaloCellContainer> caloCellContainer(m_caloCellContainerKey, ctx);
  ATH_CHECK( caloCellContainer.isValid() );

  if (!caloCellContainer->empty()) {
    for (const CaloCell* cell : *caloCellContainer) {
       Identifier id = cell->ID();
      if (m_tileID->is_tile(id)) {
        const TileCell* tile_cell = dynamic_cast<const TileCell*>(cell);
        if (!tile_cell) continue;

        int drawer = 0;    // The same for both channels
        int channel1 = -1;
        int channel2 = -1;

        int ros1 = -1;
        int ros2 = -1;

        int drawerIdx1 = -1;
        int drawerIdx2 = -1;

        int gain1 = tile_cell->gain1(); // Gain of first PMT
        int gain2 = tile_cell->gain2(); // Gain of second PMT

        const CaloDetDescrElement* caloDDE = tile_cell->caloDDE();

        IdentifierHash hash1 = caloDDE->onl1();
        if (hash1 != TileHWID::NOT_VALID_HASH) {
          HWIdentifier channel1_id = m_tileHWID->channel_id(hash1);
          channel1 = m_tileHWID->channel(channel1_id);
          drawer = m_tileHWID->drawer(channel1_id);
          ros1 = m_tileHWID->ros(channel1_id);
          drawerIdx1 = TileCalibUtils::getDrawerIdx(ros1, drawer);
        }

        IdentifierHash hash2 = caloDDE->onl2();
        if (hash2 != TileHWID::NOT_VALID_HASH) {
          HWIdentifier channel2_id = m_tileHWID->channel_id(hash2);
          channel2 = m_tileHWID->channel(channel2_id);
          drawer = m_tileHWID->drawer(channel2_id);
          ros2 = m_tileHWID->ros(channel2_id);
          drawerIdx2 = TileCalibUtils::getDrawerIdx(ros2, drawer);
        }

        if (!((drawerIdx1 >= 0 && m_monitoredDrawerIdx[drawerIdx1])
              || (drawerIdx2 >= 0 && m_monitoredDrawerIdx[drawerIdx2]))) continue;

        bool isOkChannel1 = (channel1 > -1 && gain1 != CaloGain::INVALIDGAIN);
        bool isOkChannel2 = (channel2 > -1 && gain2 != CaloGain::INVALIDGAIN);

        bool isMaskedChannel1 = isOkChannel1 && ((m_maskedChannels[drawerIdx1][channel1] >> gain1) & 1U);
        bool isMaskedChannel2 = isOkChannel2 && ((m_maskedChannels[drawerIdx2][channel2] >> gain2) & 1U);

        int sample = m_tileID->sample(id);
        int tower = m_tileID->tower(id);

        bool single_PMT_scin = (sample == TileID::SAMP_E);

        std::string moduleName = TileCalibUtils::getDrawerString(ros1, drawer);
        std::string sampleTowerSuffix = "_" + std::to_string(sample) + "_" + std::to_string(tower);

        // Keep energy in GeV;
        double energy = cell->energy() * (1.0 / GeV);
        double energy1 = tile_cell->ene1() * (1.0 / GeV);
        double energy2 = tile_cell->ene2() * (1.0 / GeV);
        double energyDiff = (single_PMT_scin) ? 0.0 : tile_cell->eneDiff() * (1.0 / GeV);
        double time = cell->time();
        double time1 = tile_cell->time1();
        double time2 = tile_cell->time2();
        double timeDiff = (single_PMT_scin) ? 0.0 : 2. * tile_cell->timeDiff(); // Attention! factor of 2 is needed here

        if (m_fillHistogramsPerChannel) {
          if (isOkChannel1) {
            auto monChannel1Energy = Monitored::Scalar<double>("energy_" + std::to_string(channel1), energy1);
            fill(m_tools[m_channelEnergyGroups.at(moduleName)], monChannel1Energy);
          }
          if (isOkChannel2) {
            auto monChannel2Energy = Monitored::Scalar<double>("energy_" + std::to_string(channel2), energy2);
            fill(m_tools[m_channelEnergyGroups.at(moduleName)], monChannel2Energy);
          }

          if (tile_cell->energy() > m_energyThresholdForTime) {
            if (isOkChannel1) {
              auto monChannel1Time = Monitored::Scalar<double>("time_" + std::to_string(channel1), time1);
              fill(m_tools[m_channelTimeGroups.at(moduleName)], monChannel1Time);
            }
            if (isOkChannel2) {
              auto monChannel2Time = Monitored::Scalar<double>("time_" + std::to_string(channel2), time2);
              fill(m_tools[m_channelTimeGroups.at(moduleName)], monChannel2Time);
            }
          }
        }

        if (sample < TileID::SAMP_E) { // Normal Tile cells with two channels (in TB setup)
          auto monEnergy = Monitored::Scalar<double>("energy" + sampleTowerSuffix, energy);
          fill(m_tools[m_energyGroups.at(moduleName)], monEnergy);

          auto monEnergyDiff = Monitored::Scalar<double>("energyDiff" + sampleTowerSuffix, energyDiff);
          fill(m_tools[m_energyDiffGroups.at(moduleName)], monEnergyDiff);

          auto monEnergy1 = Monitored::Scalar<double>("energy1" + sampleTowerSuffix, energy1);
          auto monEnergy2 = Monitored::Scalar<double>("energy2" + sampleTowerSuffix, energy2);
          fill(m_tools[m_energy2VsEnergy1Groups.at(moduleName)], monEnergy1, monEnergy2);

          if (tile_cell->energy() > m_energyThresholdForTime) {
            auto monTime = Monitored::Scalar<double>("time" + sampleTowerSuffix, time);
            fill(m_tools[m_timeGroups.at(moduleName)], monTime);

            auto monTimeDiff = Monitored::Scalar<double>("timeDiff" + sampleTowerSuffix, timeDiff);
            fill(m_tools[m_timeDiffGroups.at(moduleName)], monTimeDiff);

            auto monTime1 = Monitored::Scalar<double>("time1" + sampleTowerSuffix, time1);
            auto monTime2 = Monitored::Scalar<double>("time2" + sampleTowerSuffix, time2);
            fill(m_tools[m_time2VsTime1Groups.at(moduleName)], monTime1, monTime2);
          }

          if (isMaskedChannel1 && !isMaskedChannel2) {
            energy = energy2 * 2.0;
          } else if (isMaskedChannel2 && !isMaskedChannel1) {
            energy = energy1 * 2.0;
          } else if (isMaskedChannel1 && isMaskedChannel2) {
            energy = 0.0;
          }
          sampleEnergy[drawerIdx1][sample] += energy;
          sampleEnergy[drawerIdx1][allSamples] += energy;
        }
      }
    }


    for (unsigned int drawerIdx = 0; drawerIdx < TileCalibUtils::MAX_DRAWERIDX; ++drawerIdx) {
      if (m_monitoredDrawerIdx[drawerIdx]) {
        unsigned int ros = m_drawerIdxToROS[drawerIdx];
        unsigned int drawer = m_drawerIdxToDrawer[drawerIdx];
        std::string moduleName = TileCalibUtils::getDrawerString(ros, drawer);

        auto monEnergy = Monitored::Scalar<double>("energy", sampleEnergy[drawerIdx][allSamples]);
        auto monEnergyA = Monitored::Scalar<double>("energyA", sampleEnergy[drawerIdx][TileID::SAMP_A]);
        auto monEnergyBC = Monitored::Scalar<double>("energyBC", sampleEnergy[drawerIdx][TileID::SAMP_BC]);
        auto monEnergyD = Monitored::Scalar<double>("energyD", sampleEnergy[drawerIdx][TileID::SAMP_D]);
        fill(m_tools[m_sampleEnergyGroups.at(moduleName)], monEnergy);
        fill(m_tools[m_sampleEnergyGroups.at(moduleName)], monEnergyA, monEnergyBC);
        fill(m_tools[m_sampleEnergyGroups.at(moduleName)], monEnergyD);
      }
    }
  }


  fill("TileTBCellMonExecuteTime", timer);

  return StatusCode::SUCCESS;
}
