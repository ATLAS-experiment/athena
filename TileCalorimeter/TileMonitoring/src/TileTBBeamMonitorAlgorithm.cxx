/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TileTBBeamMonitorAlgorithm.h"
#include "TileIdentifier/TileHWID.h"
#include "TileIdentifier/TileTBFrag.h"
#include "TileEvent/TileCell.h"

#include "CaloIdentifier/TileID.h"

#include "StoreGate/ReadHandle.h"

#include <algorithm>

StatusCode TileTBBeamMonitorAlgorithm::initialize() {

  ATH_MSG_INFO("in initialize()");
  ATH_CHECK( AthMonitorAlgorithm::initialize() );

  ATH_CHECK( m_beamElemContainerKey.initialize() );
  ATH_CHECK( m_caloCellContainerKey.initialize(SG::AllowEmpty) );

  ATH_CHECK( m_cablingSvc.retrieve() );
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
  for (const std::string& maskedModuleChannels : m_masked) {

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

  for (unsigned int pmt : m_maskMuonPMTs) {
    if (pmt < m_maskedMuPMTs.size()) {
      m_maskedMuPMTs[pmt] = true;
      ATH_MSG_INFO("Masking Muon Wall PMT: " << pmt);
    }
  }

  using namespace Monitored;

  m_tofGroups = buildToolMap<int>(m_tools, "TOF", N_TOF);
  m_tofDiffGroups = buildToolMap<int>(m_tools, "TOFDiff", m_tofPairs.size());
  m_sCounterGroups = buildToolMap<int>(m_tools, "Scounter", N_S_COUNTER);
  m_cherenkovGroups = buildToolMap<int>(m_tools, "Cherenkov", N_CHERENKOV);
  m_scalerGroups = buildToolMap<int>(m_tools, "Scaler", N_SCALER);
  m_muonWallGroups = buildToolMap<int>(m_tools, "MuonWallPMT", N_MUON_WALL_PMT);

  m_cherenkovVsTOFGroups = buildToolMap<std::vector<int>>(m_tools, "CherenkovVsTOF", N_TOF, N_CHERENKOV);

  std::vector<std::string> beamChambers{"BC1", "BC2"};
  m_beamChamberGroups = buildToolMap<int>(m_tools, "BeamChamber", beamChambers);

  if (!m_caloCellContainerKey.empty()) {
    m_cherenkovVsEnergyGroups = buildToolMap<int>(m_tools, "CherenkovVsEnergy", N_CHERENKOV);
  }

  return StatusCode::SUCCESS;
}


StatusCode TileTBBeamMonitorAlgorithm::fillHistograms( const EventContext& ctx ) const {

  // In case you want to measure the execution time
  auto timer = Monitored::Timer("TIME_execute");

  uint32_t run = GetEventInfo(ctx)->runNumber();

  // TDC/BEAM Items
  int muTag = 0;
  int muHalo = 0;
  int muVeto = 0;

  std::vector<std::reference_wrapper<int>> mu{muTag, muHalo, muVeto};

  int sCounter[N_S_COUNTER] = {0};
  int cherenkov[N_CHERENKOV] = {0};
  int scaler[N_SCALER] = {0};
  int scaler12 = 0;

  // MUON/MuWall
  float muonWall[N_MUON_WALL_PMT] = {0};

  int tof[N_TDC_CHANNELS] = {0};
  int btdc[N_TDC_CHANNELS] = {0};
  for (int i = 0; i < N_TDC_CHANNELS; i += 2) {
    btdc[i] = +0xFFFF;
    btdc[i+1] = -0xFFFF;
  }
  int btdcHitsN[N_TDC_CHANNELS] = {0};

  double totalMuonWallEnergy = 0;

  SG::ReadHandle<TileBeamElemContainer> beamElemContainer(m_beamElemContainerKey, ctx);
  ATH_CHECK( beamElemContainer.isValid() );

  ATH_MSG_VERBOSE("TileRawDataContainer of TileRawDataCollection of TileBeamElem size = " << beamElemContainer->size());

  for (const TileBeamElemCollection* beamElemCollection : *beamElemContainer) {

    // Retreive frag identifiers
    int frag = beamElemCollection->identify();
    ATH_MSG_VERBOSE("TileRawDataCollection of TileBeamElem Id = 0x" << MSG::hex << frag << MSG::dec
                    << " size = " << beamElemCollection->size());

    for (const TileBeamElem* beamElement : *beamElemCollection) {

      ATH_MSG_VERBOSE((std::string) *beamElement);

      HWIdentifier id = beamElement->adc_HWID();
      std::vector<uint32_t> digits = beamElement->get_digits();
      int channel = m_tileHWID->channel(id);
      int nDigits = digits.size();

      if ( nDigits <= 0 ) {
        ATH_MSG_ERROR("Wrong no. of samples (" << nDigits
                      << ") for channel " << channel
                      << " in frag 0x"<< MSG::hex << frag << MSG::dec
                      << " - " << BeamFragName[frag & 0x1F]);
      } else {

        int amplitude = digits[0];

        ATH_MSG_DEBUG("Found channel " << channel
                      << " in frag 0x" << MSG::hex << frag << MSG::dec
                      << " - " << BeamFragName[frag & 0x1F]
                      << " with amp=" << amplitude);

        switch (frag) {

        case MUON_ADC_FRAG:

          if(channel >= 0 && channel < 8) {
            muonWall[channel] = amplitude;
            if (!m_maskedMuPMTs[channel]) {
              totalMuonWallEnergy += amplitude;
            }
          } else {
            errorWrongChannel(frag, channel);
          }

          break;

        case ADDR_ADC_FRAG:

          if(channel >= 0 && channel < 6) {
            if (8 + channel < N_MUON_WALL_PMT) {
              muonWall[8 + channel] = amplitude;
              if (!m_maskedMuPMTs[8 + channel]) {
                totalMuonWallEnergy += amplitude;
              }
            }
          } else {
            errorWrongChannel(frag, channel);
          }

          break;

        case COMMON_TOF_FRAG:
          if (m_TBperiod >= 2022) {
            // The first channels are connected to BC1 and BC2, the last 4 channels are supposed to be TOF
            if (channel > 11) {
              if(channel < 16) {
                tof[channel] = amplitude;
                ATH_MSG_VERBOSE( "TOF: " << channel << " amp: " << amplitude);
              } else {
                errorWrongChannel(frag, channel);
              }
              break;
            }
            // Fall through to case COMMON_TDC1_FRAG to unpack the first channels of BC1 and BC2
            [[fallthrough]]; // silent the warning on fall through
          } else {
            break;
          }
        case COMMON_TDC1_FRAG:

          if ((channel > 11) && (channel < 16) && (run > 2211136)) {
            tof[channel] = amplitude;
            ATH_MSG_VERBOSE( "TOF: " << channel << " amp: " << amplitude);
          } else if (channel < 16) {
            if (m_TBperiod >= 2021) {
              if (btdcHitsN[channel] == 0) {
                btdc[channel] = amplitude;
                ++(btdcHitsN[channel]);
              }
            } else {
              btdc[channel] = amplitude;
            }
          } else errorWrongChannel(frag, channel);
          break;

        case BEAM_ADC_FRAG:

          if (channel >= 0 && channel < 8) {
            if (channel < 3) {
              sCounter[channel] = amplitude;
            } else if (channel < 5) {
              cherenkov[channel - 3] = amplitude;
            } else {
              mu[channel - 5] = amplitude;
            }
          } else {
            errorWrongChannel(frag, channel);
          }
          break;


        case COMMON_ADC1_FRAG:

          if (run > 2211444) {

            if (channel >= 0 && channel < 16) {
              if (channel < 2) {
                sCounter[channel] = amplitude;
              } else if (channel == 2) {
                // Before run #2310000 channel 2 was connected to PMT11 of muon wall.
                // After this run this channel was connected to S3 scintillator.
                if (run < 2310000) {
                  muonWall[10] = amplitude;
                  if (!m_maskedMuPMTs[10]) {
                    totalMuonWallEnergy += amplitude;
                  }
                } else {
                  sCounter[2] = amplitude;
                }
              } else if (channel < 6) {
                cherenkov[channel - 3] = amplitude;
              } else {
                muonWall[channel - 6] = amplitude;
                if (!m_maskedMuPMTs[channel - 6]) {
                  totalMuonWallEnergy += amplitude;
                }
              }
            } else {
              errorWrongChannel(frag, channel);
            }

            break;

          } else {

            if (channel >= 0 && channel < 16) {
              if (channel < 3) {
                sCounter[channel] = amplitude;
              } else if (channel < 6) {
                cherenkov[channel - 3] = amplitude;
              }
            } else {
              errorWrongChannel(frag, channel);
            }

            break;
          }


        case COMMON_ADC2_FRAG:

          if (run < 2211445) {

            if(channel >= 0 && channel < N_MUON_WALL_PMT) {
              muonWall[channel] = amplitude;
              if (!m_maskedMuPMTs[channel]) {
                totalMuonWallEnergy += amplitude;
              }
            } else if (channel > 31) {
              errorWrongChannel(frag, channel);
            }
          }
          break;

        case COMMON_PTN_FRAG:
          if (run > 2310000 && channel < 16) {
            if (channel < N_SCALER) {
              scaler[channel] = amplitude;
            } else if (channel == N_SCALER) {
              scaler12 = amplitude;
            }
          } else {
            errorWrongChannel(frag, channel);
          }
          break;
        }
      }
    }
  }

  auto  monTotalMuonEnergy = Monitored::Scalar<double>("TotalMuonEnergy", totalMuonWallEnergy);
  fill("TileTBTotalMuonEnergy", monTotalMuonEnergy);

  std::vector<int> counterToTOF{14,15,13};
  for (int counter = 0; counter < N_TOF; ++counter) {
    auto monAmplitude = Monitored::Scalar<double>("amplitude", tof[counterToTOF[counter]]);
    fill(m_tools[m_tofGroups[counter]], monAmplitude);
  }

  for (unsigned int pairIdx = 0; pairIdx < m_tofPairs.size(); ++pairIdx) {
    const std::pair<int, int>& tofPair = m_tofPairs[pairIdx];
    int tof1 = tof[counterToTOF[tofPair.first - 1]];
    int tof2 = tof[counterToTOF[tofPair.second - 1]];
    if (tof1 != 0 && tof2 != 0) {
      auto monTOFDiff = Monitored::Scalar<double>("TOFDiff", tof1 - tof2);
      fill(m_tools[m_tofDiffGroups[pairIdx]], monTOFDiff);
    }
  }

  for (int counter = 0; counter < N_S_COUNTER; ++counter) {
    auto monAmplitude = Monitored::Scalar<double>("amplitude", sCounter[counter]);
    fill(m_tools[m_sCounterGroups[counter]], monAmplitude);
  }

  for (int counter = 0; counter < N_CHERENKOV; ++counter) {
    auto monAmplitude = Monitored::Scalar<double>("amplitude", cherenkov[counter]);
    fill(m_tools[m_cherenkovGroups[counter]], monAmplitude);
  }

  auto monCherenkovAmplitude1 = Monitored::Scalar<double>("amplitude1", cherenkov[0]);
  auto monCherenkovAmplitude2 = Monitored::Scalar<double>("amplitude2", cherenkov[1]);
  fill("CherCompare", monCherenkovAmplitude1, monCherenkovAmplitude2);

  for (int tofCounter = 0; tofCounter < N_TOF; ++tofCounter) {
    auto monAmplitudeTOF = Monitored::Scalar<double>("amplitudeTOF", tof[counterToTOF[tofCounter]]);
    for (int cherenkovCounter = 0; cherenkovCounter < N_CHERENKOV; ++cherenkovCounter) {
      auto monAmplitudeCherenkov = Monitored::Scalar<double>("amplitudeCherenkov", cherenkov[cherenkovCounter]);
      fill(m_tools[m_cherenkovVsTOFGroups[tofCounter][cherenkovCounter]], monAmplitudeTOF, monAmplitudeCherenkov);
    }
  }


  if (run > 2310000) {
    for (int counter = 0; counter < N_SCALER; ++counter) {
      auto monCounts = Monitored::Scalar<double>("counts", scaler[counter]);
      fill(m_tools[m_scalerGroups[counter]], monCounts);
    }
    auto monCounts12 = Monitored::Scalar<double>("counts12", scaler12);
    fill("Scaler12", monCounts12);
  }

  for (int pmt = 0; pmt < N_MUON_WALL_PMT; ++pmt) {
    auto monAmplitude = Monitored::Scalar<double>("amplitude", muonWall[pmt]);
    fill(m_tools[m_muonWallGroups[pmt]], monAmplitude);
  }

  for (int row = 0; row < 2; ++row) {
    for (int column = 0; column < 4; ++column) {
      auto monRow = Monitored::Scalar<double>("row", row);
      auto monColumn = Monitored::Scalar<double>("column", column);
      auto monAmplitude = Monitored::Scalar<double>("amplitude", muonWall[7 - (row * 4 + column)]);
      fill("PMTHitMap", monColumn, monRow, monAmplitude);
    }
  }

  // Beam Chamber Coordinates
  // For BC1
  auto bc1X = Monitored::Scalar<double>("BC1X", 0.);
  auto bc1Y = Monitored::Scalar<double>("BC1Y", 0.);
  if (run > 2211444) {
    bc1X = (btdc[8] - btdc[0]) * m_bc1HorizontalSlope + m_bc1HorizontalOffset; // (right - left)
    bc1Y = (btdc[9] - btdc[3]) * m_bc1VerticalSlope + m_bc1VerticalOffset; // (up - down)
  } else {
    bc1X = (btdc[1] - btdc[0]) * m_bc1HorizontalSlope + m_bc1HorizontalOffset;  // (right - left)
    bc1Y = (btdc[2] - btdc[3]) * m_bc1VerticalSlope + m_bc1VerticalOffset; // (up - down)
  }
  fill(m_tools[m_beamChamberGroups.at("BC1")], bc1X, bc1Y);

  // For BC2:
  auto bc2X = Monitored::Scalar<double>("BC2X", (btdc[5] - btdc[4]) * m_bc2HorizontalSlope + m_bc2HorizontalOffset); // (right - left)
  auto bc2Y = Monitored::Scalar<double>("BC2Y", (btdc[6] - btdc[7]) * m_bc2VerticalSlope + m_bc2VerticalOffset); // (up - down)
  fill(m_tools[m_beamChamberGroups.at("BC2")], bc2X, bc2Y);

  // Sum Plots
  // For BC1
  auto bc1Xsum = Monitored::Scalar<double>("BC1Xsum", 0.);
  auto bc1Ysum = Monitored::Scalar<double>("BC1Ysum", 0.);;
  if (run > 2211444) {
    bc1Xsum =(btdc[8] + btdc[0]) * m_bc1HorizontalSlope + m_bc1HorizontalOffset;
    bc1Ysum = (btdc[9] + btdc[3]) * m_bc1VerticalSlope + m_bc1VerticalOffset;
  } else {
    bc1Xsum = (btdc[1] + btdc[0]) * m_bc1HorizontalSlope + m_bc1HorizontalOffset;
    bc1Ysum = (btdc[2] + btdc[3]) * m_bc1VerticalSlope + m_bc1VerticalOffset;
  }
  fill(m_tools[m_beamChamberGroups.at("BC1")], bc1Xsum, bc1Ysum);

  //For BC2
  auto bc2Xsum = Monitored::Scalar<double>("BC2Xsum", (btdc[5] + btdc[4]) * m_bc2HorizontalSlope + m_bc2HorizontalOffset);
  auto bc2Ysum = Monitored::Scalar<double>("BC2Ysum", (btdc[6] + btdc[7]) * m_bc2VerticalSlope + m_bc2VerticalOffset);
  fill(m_tools[m_beamChamberGroups.at("BC2")], bc2Xsum, bc2Ysum);

  //Impact Coordinates
  // For one cell, plot energy total as a function of x Impact -- (xImp, cell_energy)...
  auto xImp = Monitored::Scalar<double>("Ximp", bc2X + (bc2X - bc1X) * m_beamBC2Z / (m_beamBC1Z - m_beamBC2Z));
  auto yImp = Monitored::Scalar<double>("Yimp", bc2Y + (bc2Y - bc1Y) * m_beamBC2Z / (m_beamBC1Z - m_beamBC2Z));
  fill("ImpactProfile", xImp, yImp);


  if (!m_caloCellContainerKey.empty()) {

    SG::ReadHandle<CaloCellContainer> caloCellContainer(m_caloCellContainerKey, ctx);
    ATH_CHECK( caloCellContainer.isValid() );

    if (!caloCellContainer->empty()) {

      double cellEnergy = 0;
      double totalEnergy(0.0);

      for (const CaloCell* cell : *caloCellContainer) {
        if (m_tileID->is_tile(cell->ID())) {
          const TileCell* tile_cell = dynamic_cast<const TileCell*>(cell);
          if (!tile_cell) continue;

          const CaloDetDescrElement* caloDDE = cell->caloDDE();

          IdentifierHash hash1 = caloDDE->onl1();
          IdentifierHash hash2 = caloDDE->onl2();

          double energy = 0.0;

          int gain1 = tile_cell->gain1();

          HWIdentifier channelId1 = m_tileHWID->channel_id(hash1);

          int ros1 = m_tileHWID->ros(channelId1);
          int drawer1 = m_tileHWID->drawer(channelId1);
          int channel1 = m_tileHWID->channel(channelId1);
          int drawerIdx1 = TileCalibUtils::getDrawerIdx(ros1, drawer1);

          if (hash2 == TileHWID::NOT_VALID_HASH) {
            if (m_monitoredDrawerIdx[drawerIdx1]) {
              if (gain1 >= 0 && !((m_maskedChannels[drawerIdx1][channel1] >> gain1) & 1U)) {
                energy = cell->energy();
              }
            }
          } else {

            int gain2 = tile_cell->gain2();

            HWIdentifier channelId2 = m_tileHWID->channel_id(hash2);

            int ros2 = m_tileHWID->ros(channelId2);
            int drawer2 = m_tileHWID->drawer(channelId2);
            int channel2 = m_tileHWID->channel(channelId2);
            int drawerIdx2 = TileCalibUtils::getDrawerIdx(ros2, drawer2);

            if (m_monitoredDrawerIdx[drawerIdx1] || m_monitoredDrawerIdx[drawerIdx2]) {
              if (gain1 < 0 || ((m_maskedChannels[drawerIdx1][channel1] >> gain1) & 1U)) {
                if (gain2 >= 0 && !((m_maskedChannels[drawerIdx2][channel2] >> gain2) & 1U)) {
                  energy = tile_cell->ene2() * 2;
                }
              } else if (gain2 >= 0 && ((m_maskedChannels[drawerIdx2][channel2] >> gain2) & 1U)) {
                if (gain1 >= 0 && !((m_maskedChannels[drawerIdx1][channel1] >> gain1) & 1U)) {
                  energy = tile_cell->ene1() * 2;
                }
              } else {
                energy = cell->energy();
              }
            }
          }

          double energy_pC = energy * 0.001; // keep energy in pC;
          totalEnergy += energy_pC;

          if (energy_pC > cellEnergy)  {
            cellEnergy = energy_pC;
          }
        }
      }

      auto monCellEnergy = Monitored::Scalar<double>("cellEnergy", cellEnergy);
      auto monTotalEnergy = Monitored::Scalar<double>("totalEnergy", totalEnergy);

      for (int counter = 0; counter < N_CHERENKOV; ++counter) {
        auto monAmplitude = Monitored::Scalar<double>("amplitude", cherenkov[counter]);
        fill(m_tools[m_cherenkovVsEnergyGroups[counter]], monTotalEnergy, monAmplitude);
      }

      fill("CellEnergyImpactX", xImp, monCellEnergy);
      fill("CellEnergyImpactY", yImp, monCellEnergy);
      fill("TotalEnergyImpactX", xImp, monTotalEnergy);
      fill("TotalEnergyImpactY", yImp, monTotalEnergy);

      auto monAmplitudeS1 = Monitored::Scalar<double>("amplitude", sCounter[0]);
      fill("ScinCalEnergy", monAmplitudeS1, monTotalEnergy);
    }
  }

  fill("TileTBBeamMonExecuteTime", timer);

  return StatusCode::SUCCESS;
}


void TileTBBeamMonitorAlgorithm::errorWrongChannel(int frag, int channel) const {
  ATH_MSG_ERROR("Wrong channel " << channel
                << " in frag 0x" << MSG::hex << frag << MSG::dec
                << " - " << BeamFragName[frag & 0x1F]);
}
