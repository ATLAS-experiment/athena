/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "TileTBBeamMonitorAlgorithm.h"
#include "TileCalibBlobObjs/TileCalibUtils.h"
#include "TileIdentifier/TileHWID.h"
#include "TileIdentifier/TileTBFrag.h"

#include "StoreGate/ReadHandle.h"

#include <algorithm>

StatusCode TileTBBeamMonitorAlgorithm::initialize() {

  ATH_MSG_INFO("in initialize()");
  ATH_CHECK( AthMonitorAlgorithm::initialize() );

  ATH_CHECK( m_beamElemContainerKey.initialize() );

  ATH_CHECK( m_cablingSvc.retrieve() );
  ATH_CHECK( detStore()->retrieve(m_tileHWID) );

  std::vector<std::string> modules;
  for (int fragID : m_fragIDs) {
    int ros = fragID >> 8;
    int drawer = fragID & 0x3F;
    modules.push_back(TileCalibUtils::getDrawerString(ros, drawer));
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

  using namespace Monitored;

  m_tofGroups = buildToolMap<int>(m_tools, "TOF", N_TOF);
  m_sCounterGroups = buildToolMap<int>(m_tools, "Scounter", N_S_COUNTER);
  m_cherenkovGroups = buildToolMap<int>(m_tools, "Cherenkov", N_CHERENKOV);
  m_muonWallGroups = buildToolMap<int>(m_tools, "MuonWallPMT", N_MUON_WALL_PMT);

  std::vector<std::string> beamChambers{"BC1", "BC2"};
  m_beamChamberGroups = buildToolMap<int>(m_tools, "BeamChamber", beamChambers);

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

  // MUON/MuWall
  float muonWall[N_MUON_WALL_PMT] = {0};

  int tof[16] = {0};
  int btdc[16] = {0};
  int btdcHitsN[16] = {0};

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
          } else {
            errorWrongChannel(frag, channel);
          }

          break;

        case ADDR_ADC_FRAG:

          if(channel >= 0 && channel < 6) {
            if (8 + channel < N_MUON_WALL_PMT) {
              muonWall[8 + channel] = amplitude;
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
          } if (channel < 16) {
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
                muonWall[10] = amplitude;
              } else if (channel < 6) {
                cherenkov[channel - 3] = amplitude;
              } else {
                muonWall[channel - 6] = amplitude;
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
            } else if (channel > 31) {
              errorWrongChannel(frag, channel);
            }
          }
          break;
        }
      }
    }
  }

  for (int counter = 0; counter < N_TOF; ++counter) {
    std::vector<int> counterToTOF{14,15,13};
    auto monAmplitude = Monitored::Scalar<double>("amplitude", tof[counterToTOF[counter]]);
    fill(m_tools[m_tofGroups[counter]], monAmplitude);
  }

  for (int counter = 0; counter < N_S_COUNTER; ++counter) {
    auto monAmplitude = Monitored::Scalar<double>("amplitude", sCounter[counter]);
    fill(m_tools[m_sCounterGroups[counter]], monAmplitude);
  }

  for (int counter = 0; counter < N_CHERENKOV; ++counter) {
    auto monAmplitude = Monitored::Scalar<double>("amplitude", cherenkov[counter]);
    fill(m_tools[m_cherenkovGroups[counter]], monAmplitude);
  }

  for (int pmt = 0; pmt < N_MUON_WALL_PMT; ++pmt) {
    auto monAmplitude = Monitored::Scalar<double>("amplitude", muonWall[pmt]);
    fill(m_tools[m_muonWallGroups[pmt]], monAmplitude);
  }

  for (int row = 0; row < 2; ++row) {
    for (int column = 0; column < 4; ++column) {
      auto monRow = Monitored::Scalar<double>("row", row);
      auto monColumn = Monitored::Scalar<double>("column", column);
      auto monAmplitude = Monitored::Scalar<double>("amplitude", muonWall[8 - (row * 4 + column)]);
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
  fill(m_tools[m_beamChamberGroups.at("BC1")], bc1X);
  fill(m_tools[m_beamChamberGroups.at("BC1")], bc1Y);
  fill(m_tools[m_beamChamberGroups.at("BC1")], bc1X, bc1Y);

  // For BC2:
  auto bc2X = Monitored::Scalar<double>("BC2X", (btdc[5] - btdc[4]) * m_bc2HorizontalSlope + m_bc2HorizontalOffset); // (right - left)
  auto bc2Y = Monitored::Scalar<double>("BC2Y", (btdc[6] - btdc[7]) * m_bc2VerticalSlope + m_bc2VerticalOffset); // (up - down)
  fill(m_tools[m_beamChamberGroups.at("BC2")], bc2X);
  fill(m_tools[m_beamChamberGroups.at("BC2")], bc2Y);
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

  fill("TileTBBeamMonExecuteTime", timer);

  return StatusCode::SUCCESS;
}


void TileTBBeamMonitorAlgorithm::errorWrongChannel(int frag, int channel) const {
  ATH_MSG_ERROR("Wrong channel " << channel
                << " in frag 0x" << MSG::hex << frag << MSG::dec
                << " - " << BeamFragName[frag & 0x1F]);
}
