/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "TileTBMonitorAlgorithm.h"
#include "TileCalibBlobObjs/TileCalibUtils.h"
#include "TileIdentifier/TileHWID.h"
#include "TileEvent/TileCell.h"

#include "CaloIdentifier/TileID.h"
#include "CaloIdentifier/CaloCell_ID.h"

#include "StoreGate/ReadHandle.h"

#include <algorithm>

StatusCode TileTBMonitorAlgorithm::initialize() {

  ATH_MSG_INFO("in initialize()");
  ATH_CHECK( AthMonitorAlgorithm::initialize() );

  ATH_CHECK( m_cablingSvc.retrieve() );

  ATH_CHECK( m_caloCellContainerKey.initialize() );

  ATH_CHECK( detStore()->retrieve(m_tileID) );
  ATH_CHECK( detStore()->retrieve(m_tileHWID) );

  m_monitoredDrawerIdx.resize(TileCalibUtils::MAX_DRAWERIDX, false);

  std::vector<std::string> modules;
  for (int fragID : m_fragIDs) {
    int ros = fragID >> 8;
    int drawer = fragID & 0x3F;
    modules.push_back(TileCalibUtils::getDrawerString(ros, drawer));
    m_monitoredDrawerIdx[TileCalibUtils::getDrawerIdx(ros, drawer)] = true;
  }

  using namespace Monitored;
  m_timeGroups = buildToolMap<int>(m_tools, "TileTBChannelTime", modules);
  m_cellMapGroups = buildToolMap<int>(m_tools, "TileTBCellMap", modules);

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

  m_xCellLB.push_back(m_xCellLB_A.value());
  m_xCellLB.push_back(m_xCellLB_BC.value());
  m_xCellLB.push_back(m_xCellLB_D.value());

  m_yCellLB.push_back(m_yCellLB_A.value());
  m_yCellLB.push_back(m_yCellLB_BC.value());
  m_yCellLB.push_back(m_yCellLB_D.value());

  m_xCellEB.push_back(m_xCellEB_A.value());
  m_xCellEB.push_back(m_xCellEB_BC.value());
  m_xCellEB.push_back(m_xCellEB_D.value());

  m_yCellEB.push_back(m_yCellEB_A.value());
  m_yCellEB.push_back(m_yCellEB_BC.value());
  m_yCellEB.push_back(m_yCellEB_D.value());

  // Sanity check
  std::vector<unsigned int> maxTower{10, 10, 16, 16};
  std::vector<std::reference_wrapper<const std::vector<std::vector<std::vector<double>>>>> xyCells{m_xCellLB, m_yCellLB, m_xCellEB, m_yCellEB};
  for (unsigned int i = 0; i < xyCells.size(); ++i) {
    for (const std::vector<std::vector<double>>& xy : xyCells[i].get()) {
      if (xy.size() != maxTower[i]) {
        std::string properties = ((i % 2 == 0) ?  "xCell" : "yCell");
        properties += (i < 2) ? "LongBarrelSample[A,BC,D]" : "ExtendedBarrelSample[A,BC,D]";
        ATH_MSG_ERROR("Properties " << properties << " should be configured for " << maxTower[i] << " towers");
        return StatusCode::FAILURE;
      }
    }
  }


  const CaloCell_ID* caloID;
  ATH_CHECK( detStore()->retrieve(caloID) );

  unsigned int minCellTower = 1;
  unsigned int maxCellTower = 9;

  std::vector<std::vector<int>> lbCellsD{{}, {0, 2}, {2, 4}, {2, 4}, {1, 4, 6}, {4, 6}, {4, 6}, {6}, {6}};
  for (unsigned int cellTower = minCellTower; cellTower < maxCellTower; ++cellTower) {
    std::vector<IdentifierHash>& cells = m_cellsNearTower[cellTower];
    for (unsigned int cellModule = 0; cellModule < 2; ++cellModule) {
      for (unsigned int cellSample = 0; cellSample < 2; ++cellSample) {
        for (unsigned int tower = cellTower - 1; tower < cellTower + 2; ++tower) {
          Identifier cell_id = m_tileID->cell_id(TileID::BARREL, TileID::NEGATIVE, cellModule, tower, cellSample);
          cells.push_back(caloID->calo_cell_hash(cell_id));
        }
      }

      const std::vector<int>& towersD = lbCellsD[cellTower];
      for (int towerD : towersD) {
        unsigned int side = (towerD == 0) ?  TileID::POSITIVE : TileID::NEGATIVE;
        Identifier cell_id = m_tileID->cell_id(TileID::BARREL, side, cellModule, towerD, TileID::SAMP_D);
        cells.push_back(caloID->calo_cell_hash(cell_id));
      }
    }
  }

  static const std::vector<std::vector<std::vector<int>>> ebCellsNearTower{{{}},
                                                                           {{11, 12}, {9, 10, 11, 12}, {8, 10}},
                                                                           {{11, 12}, {9, 10, 11, 12}, {8, 10}},
                                                                           {{12, 13}, {11, 12}, {10, 12}},
                                                                           {{12, 13, 14}, {11, 12, 13}, {10, 12}},
                                                                           {{13, 14}, {12, 13}, {12}},
                                                                           {{14, 15}, {13, 14}, {12}},
                                                                           {{14, 15}, {13, 14}, {12}},
                                                                           {{15}, {14}, {12}}};
  for (unsigned int cellTower = minCellTower; cellTower < maxCellTower; ++cellTower) {
    std::vector<IdentifierHash>& cells = m_cellsNearTower[cellTower];
    const std::vector<std::vector<int>>& ebCells = ebCellsNearTower[cellTower];
    for (unsigned int cellSample = 0; cellSample < 3; ++cellSample) {
      const std::vector<int>& ebCellsInSample = ebCells[cellSample];
      for (int tower : ebCellsInSample) {
        int section = (tower < 10) ? TileID::GAPDET : TileID::EXTBAR;
        Identifier cell_id = m_tileID->cell_id(section, TileID::NEGATIVE, 2, tower, cellSample);
        cells.push_back(caloID->calo_cell_hash(cell_id));
      }
    }
  }

  for (unsigned int cellTower = minCellTower; cellTower < maxCellTower; ++cellTower) {
    std::vector<IdentifierHash>& cells = m_cellsNearTower[cellTower];
    ATH_MSG_INFO("The are " << cells.size() << " Tile cells near the tower " << cellTower << " in LBC02: ");
    for (IdentifierHash hash : cells) {
      ATH_MSG_INFO("  " << m_tileID->to_string(caloID->cell_id(hash)));
    }
  }

  return StatusCode::SUCCESS;
}


StatusCode TileTBMonitorAlgorithm::fillHistograms( const EventContext& ctx ) const {

  // In case you want to measure the execution time
  auto timer = Monitored::Timer("TIME_execute");

  using Tile = TileCalibUtils;

  SG::ReadHandle<CaloCellContainer> caloCellContainer(m_caloCellContainerKey, ctx);
  ATH_CHECK( caloCellContainer.isValid() );

  if (caloCellContainer->empty()) return StatusCode::SUCCESS;

  const TileCell* cellWithMaxEnergy = nullptr;

  double totalEnergy(0.0);
  bool onlyLBC04(true);
  int nCellsOverThreshold(0);

  double totalEnergyLBA01 = 0.;
  double totalEnergyLBA02 = 0.;
  double totalEnergyLBC01 = 0.;
  double totalEnergyLBC02 = 0.;
  double totalEnergyEBC03 = 0.;

  for (const CaloCell* cell : *caloCellContainer) {
    if (m_tileID->is_tile(cell->ID())) {
      const TileCell* tile_cell = dynamic_cast<const TileCell*>(cell);
      if (!tile_cell) continue;

      const CaloDetDescrElement* caloDDE = cell->caloDDE();

      IdentifierHash hash1 = caloDDE->onl1();
      IdentifierHash hash2 = caloDDE->onl2();

      int side = m_tileID->side(cell->ID());
      int section = m_tileID->section(cell->ID());
      int module = m_tileID->module(cell->ID());
      int tower = m_tileID->tower(cell->ID());
      int sample = m_tileID->sample(cell->ID());

      if ((section == TileID::BARREL) && (side == TileID::NEGATIVE) && (module == 1) && (sample == TileID::SAMP_A)
          && (!cellWithMaxEnergy || cellWithMaxEnergy->energy() < tile_cell->energy())) {
        cellWithMaxEnergy = tile_cell;
      }

      double energy = 0.0;
      double energy_pC(0.0);

      int gain1 = tile_cell->gain1();

      HWIdentifier channelId1 = m_tileHWID->channel_id(hash1);

      int ros1 = m_tileHWID->ros(channelId1);
      int drawer1 = m_tileHWID->drawer(channelId1);
      int chan1 = m_tileHWID->channel(channelId1);
      int drawerIdx1 = TileCalibUtils::getDrawerIdx(ros1, drawer1);

      std::string moduleName1 = TileCalibUtils::getDrawerString(ros1, drawer1);
      int drawerIdx2 = 0;
      std::string moduleName2 = "";

      if (m_monitoredDrawerIdx[drawerIdx1] && tile_cell->ene1() > m_energyThresholdForTime) {
        auto monTime = Monitored::Scalar<float>("time", tile_cell->time1());
        auto monChannel = Monitored::Scalar<float>("channel", chan1);
        fill(m_tools[m_timeGroups.at(moduleName1)], monTime, monChannel);
      }

      if (onlyLBC04 && chan1 > 0 && drawerIdx1 != 87) onlyLBC04 = false;

      if (hash2 == TileHWID::NOT_VALID_HASH) {
        if (!((m_maskedChannels[drawerIdx1][chan1] >> gain1) & 1U)) {
          energy = cell->energy();
        }
      } else {

        int gain2 = tile_cell->gain2();

        HWIdentifier channelId2 = m_tileHWID->channel_id(hash2);

        int ros2 = m_tileHWID->ros(channelId2);
        int drawer2 = m_tileHWID->drawer(channelId2);
        int chan2 = m_tileHWID->channel(channelId2);
        drawerIdx2 = TileCalibUtils::getDrawerIdx(ros2, drawer2);

        moduleName2 = TileCalibUtils::getDrawerString(ros2, drawer2);

        if (m_monitoredDrawerIdx[drawerIdx1] && tile_cell->ene2() > m_energyThresholdForTime) {
          auto monTime = Monitored::Scalar<float>("time", tile_cell->time2());
          auto monChannel = Monitored::Scalar<float>("channel", chan2);
          fill(m_tools[m_timeGroups.at(moduleName2)], monTime, monChannel);
        }

        if ((m_maskedChannels[drawerIdx1][chan1] >> gain1) & 1U) {
          if (!((m_maskedChannels[drawerIdx2][chan2] >> gain2) & 1U)) {
            energy = tile_cell->ene2() * 2;
          }
        } else if ((m_maskedChannels[drawerIdx2][chan2] >> gain2) & 1U) {
          if (!((m_maskedChannels[drawerIdx1][chan1] >> gain1) & 1U)) {
            energy =tile_cell->ene1() * 2;
          }
        } else {
          energy = cell->energy();
        }
      }

      energy_pC = energy * 0.001; // keep energy in pC
      totalEnergy += energy_pC;

      if (section == TileID::BARREL) {
        if (side == TileID::POSITIVE) {
          if      (module == 0) totalEnergyLBA01 += energy_pC;
          else if (module == 1) totalEnergyLBA02 += energy_pC;
        } else if (side == TileID::NEGATIVE) {
          if      (module == 0) totalEnergyLBC01 += energy_pC;
          else if (module == 1) totalEnergyLBC02 += energy_pC;
        }
      } else {
        if (module == 2 && side == TileID::NEGATIVE && sample < TileID::SAMP_E) {
          totalEnergyEBC03 += energy_pC;
        }
      }

      if (side < 0) {
        if (energy > m_cellEnergyThreshold) {
          ++nCellsOverThreshold;
        }
      }

      if (section == TileID::BARREL) {
        if (m_monitoredDrawerIdx[drawerIdx1]) {
          auto monX = Monitored::Collection("x", m_xCellLB[sample][tower]);
          auto monY = Monitored::Collection("y", m_yCellLB[sample][tower]);

          std::vector<double> cellEnergy(m_xCellLB[sample][tower].size(), energy_pC);
          auto monEnergy = Monitored::Collection("energy", cellEnergy);
          if (m_monitoredDrawerIdx[drawerIdx1]) {
            fill(m_tools[m_cellMapGroups.at(moduleName1)], monX, monY, monEnergy);
          }

          if (tower == 0 && sample == TileID::SAMP_D && m_monitoredDrawerIdx[drawerIdx2]) {
            fill(m_tools[m_cellMapGroups.at(moduleName2)], monX, monY, monEnergy);
          }
        }
      } else if (section == TileID::EXTBAR
                 || (section == TileID::GAPDET
                     && (sample == TileID::SAMP_C || sample == TileID::SAMP_D))) {
        if (m_monitoredDrawerIdx[drawerIdx1]) {
          auto monX = Monitored::Collection("x", m_xCellEB[sample][tower]);
          auto monY = Monitored::Collection("y", m_yCellEB[sample][tower]);

          std::vector<double> cellEnergy(m_xCellEB[sample][tower].size(), energy_pC);
          auto monEnergy = Monitored::Collection("energy", cellEnergy);

          fill(m_tools[m_cellMapGroups.at(moduleName1)], monX, monY, monEnergy);
        }
      }
    }
  }

  if (cellWithMaxEnergy && !onlyLBC04) {

    int tower = m_tileID->tower(cellWithMaxEnergy->ID());
    auto monTower = Monitored::Scalar<float>("tower", tower);
    fill("TileTBHotCellA_LBC02", monTower);

    if ((tower > 0) && (tower < 9)) {

      double sumClong = 0.0;
      double sumCtot = 0.0;

      const std::vector<IdentifierHash>& cellsHashes = m_cellsNearTower[tower];
      CaloCellContainer::CellVector cells;
      caloCellContainer->findCellVector(cellsHashes, cells);

      float alpha = (m_beamEnergy < 100000) ? 0.6 : 0.38;
      unsigned int nCells = cells.size();

      if (nCells) {
        double sumCellEnergyAlpha = 0.;
        for (const CaloCell* cell : cells) {
          double energy = cell->energy();
          if(energy >= 0) {
            sumCellEnergyAlpha += std::pow(energy, alpha);
          }
        }

        double avgCellEnergyAlpha = sumCellEnergyAlpha / nCells;

        for (const CaloCell* cell : cells) {
          double energy = cell->energy();
          if (energy >= 0) {
            sumCtot += std::pow( std::pow(energy, alpha) - avgCellEnergyAlpha, 2 );
            int sample = m_tileID->sample(cell->ID());
            if (sample != TileID::SAMP_D) {
              sumClong += energy;
            }
          }
        }

        double Ctot = std::sqrt(sumCtot / nCells) / sumCellEnergyAlpha;
        double Clong = sumClong / m_beamEnergy;

        auto monCtot = Monitored::Scalar<float>("Ctot", Ctot);
        auto monClong = Monitored::Scalar<float>("Clong", Clong);
        fill("TileTBCtot", monCtot);
        fill("TileTBClong", monClong);
        fill("TileTBCtotVsClong", monClong, monCtot);

      }
    }
  }

  if (!onlyLBC04) {

    auto monEnergy = Monitored::Scalar<float>("energy", totalEnergy);
    fill("TileTBTotalEventEnergy", monEnergy);

    auto monCellsOvThr = Monitored::Scalar<float>("nCells", nCellsOverThreshold);
    fill("TileTBCellsNumberVsTotalEnergy", monCellsOvThr, monEnergy);

    std::vector<int> side{0,0,1,1,1};
    auto monSide = Monitored::Collection("side", side);

    std::vector<int> module{0,1,0,1,2};
    auto monModule = Monitored::Collection("module", module);

    std::vector<double> moduleEnergy{totalEnergyLBA01, totalEnergyLBA02,
                                     totalEnergyLBC01, totalEnergyLBC02,
                                     totalEnergyEBC03};
    auto monModEnergy = Monitored::Collection("energy", moduleEnergy);

    fill("TileTBHitMap", monSide, monModule, monModEnergy);
  }


  fill("TileTBMonExecuteTime", timer);

  return StatusCode::SUCCESS;
}
