/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "ITkStripFrontEnd.h"

#include "InDetIdentifier/SCT_ID.h"
#include "SCT_ReadoutGeometry/SCT_DetectorManager.h"
#include "SCT_ReadoutGeometry/SCT_ModuleSideDesign.h"


// Random number
#include "CLHEP/Random/RandFlat.h"
#include "CLHEP/Random/RandGaussZiggurat.h"  // for RandGaussZiggurat
#include "CLHEP/Random/RandPoisson.h"

#include "CLHEP/Random/RandomEngine.h"

using namespace InDetDD;

// constructor
ITkStripFrontEnd::ITkStripFrontEnd(const std::string& type, const std::string& name, const IInterface* parent)
  : base_class(type, name, parent) {
}

// ----------------------------------------------------------------------
// Initialize
// ----------------------------------------------------------------------
StatusCode ITkStripFrontEnd::initialize() {
  if (m_NoiseOn and (not m_analogueNoiseOn)) {
      ATH_MSG_FATAL("AnalogueNoiseOn/m_analogueNoiseOn should be true if NoiseOn/m_NoiseOn is true.");
      return StatusCode::FAILURE;
    }

  ATH_MSG_DEBUG("ITkStripFrontEnd::initialize()");
  // Get SCT helper
  ATH_CHECK(detStore()->retrieve(m_ITkStripId, "SCT_ID"));
  // Get SCT detector manager
  ATH_CHECK(detStore()->retrieve(m_ITkStripMgr,m_detMgrName));
  
  ATH_CHECK(m_strip_amplifier.retrieve());

  constexpr float fC = 6242.2;
  m_Threshold = m_Threshold * fC;

  // Check configuration. If it is invalid, abort this job.
  if (not (m_data_compression_mode==Level_X1X or
           m_data_compression_mode==Edge_01X or
           m_data_compression_mode==AnyHit_1XX_X1X_XX1)) {
    ATH_MSG_FATAL("m_data_compression_mode = " << m_data_compression_mode
                  << " is invalid. Abort this job!!!");
    return StatusCode::FAILURE;
  }
  if (not (m_data_readout_mode==Condensed or m_data_readout_mode==Expanded)) {
    ATH_MSG_FATAL("m_data_readout_mode = " << m_data_readout_mode
                  << " is invalid. Abort this job!!!");
    return StatusCode::FAILURE;
  }
  if ((m_data_compression_mode==Level_X1X or m_data_compression_mode==AnyHit_1XX_X1X_XX1)
      and m_data_readout_mode==Condensed) {
    ATH_MSG_FATAL("m_data_compression_mode = " << m_data_compression_mode 
                  << (m_data_compression_mode==Level_X1X ? " (Level_X1X)" : " (AnyHit_1XX_X1X_XX1)")
                  << " requires timing information."
                  << " However, m_data_readout_mode = " << m_data_readout_mode
                  << " (Condensed) does not keep timing information. Abort this job!!!");
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}

// ----------------------------------------------------------------------
// Init the class variable  vectors
// ----------------------------------------------------------------------
void ITkStripFrontEnd::initVectors(int strips, ITkStripFrontEndData& data) const {
  data.m_GainFactor.resize(strips);

  data.m_Analogue[0].resize(strips);
  data.m_Analogue[1].resize(strips);
  data.m_Analogue[2].resize(strips);

}


// ----------------------------------------------------------------------
// process the collection of pre digits this will need to go through
// all single-strip pre-digits calculate the amplifier response add noise
// (this could be moved elsewhere later) apply threshold do clustering
// ----------------------------------------------------------------------
void 
ITkStripFrontEnd::process(SiChargedDiodeCollection& collection, CLHEP::HepRandomEngine* /*rndmEngine*/) const {
  // get ITk module side design
  [[maybe_unused]] const SCT_ModuleSideDesign *p_design = static_cast<const SCT_ModuleSideDesign*>(&(collection.design()));

  ITkStripFrontEndData data;

  // Check number of strips in design and from manager(max number of strips on any module)
  // The design value should always be equal or lower than the manager one
  // However, no resising is now done in case of a lower value
  const int strip_max = p_design->cells();

  // Init vectors
  initVectors(strip_max, data);

  // Contains strip hit info, reset to 0 for each wafer processed
  data.m_StripHitsOnWafer.assign(strip_max, 0);

  // data.m_Analogue were cleared in initVectors().

  // Check if collection empty
  if (not collection.empty()) {
    //Commented parts are future Tools to be added
    // Setup gain/offset/noise to the hit and neighbouring strips// Use JO values
    // if (StatusCode::SUCCESS != prepareGainAndOffset(collection, moduleId, rndmEngine, data, strip_max)) {
    //   ATH_MSG_ERROR("\tCan't prepare Gain and Offset");
    // }

    doSignalChargeForHits(collection, data, strip_max);

    doThresholdCheckForRealHits(collection, data, strip_max);

    // if (StatusCode::SUCCESS != doThresholdCheckForCrosstalkHits(collection, data, strip_max)) {
    //   ATH_MSG_ERROR("\tCan't doThresholdCheckForCrosstalkHits");
    // }
  }

  // if (m_NoiseOn) {
  //   if (StatusCode::SUCCESS != randomNoise(collection, moduleId, rndmEngine, data,strip_max)) {
  //     ATH_MSG_ERROR("\tCan't do random noise on wafer?!");
  //   }
  // }

  // // Check for strips above threshold and do clustering
  // if (StatusCode::SUCCESS != doClustering(collection, data,strip_max)) {
  //   ATH_MSG_ERROR("\tCan't cluster the hits?!");
  // }

}


void ITkStripFrontEnd::doSignalChargeForHits(SiChargedDiodeCollection& collection, ITkStripFrontEndData& data, const int& strip_max) const {
  using list_t = SiTotalCharge::list_t;

  // *****************************************************************************
  // Loop over the diodes (strips ) and for each of them define the total signal
  // *****************************************************************************

  // set up number of needed bins depending on the compression mode
  short bin_max = 0;
  if (m_data_readout_mode == Condensed) {
    bin_max = m_data_compression_mode;
  } else {
    bin_max = 3;
  }

  std::vector<float> response(bin_max);

  for (auto& [blub, diode]: collection) {
    // should be const as we aren't trying to change it here - but getReadoutCell() is not a const method...
    unsigned int flagmask = diode.flag() & 0xFE;
    // Get the flag for this diode ( if flagmask = 1 If diode is disconnected/disabled skip it)
    if (!flagmask) { // If the diode is OK (not flagged)
      const SiReadoutCellId &roCell = diode.getReadoutCell();

      if (roCell.isValid()) {
        int strip = roCell.strip();

        [[maybe_unused]] const list_t &ChargesOnStrip = diode.totalCharge().chargeComposition();

        if (m_data_readout_mode == Condensed) {
          // Amplifier response
          // m_sct_amplifier->response(ChargesOnStrip, m_timeOfThreshold, response);
          for (short bin = 0; bin < bin_max; ++bin) {
            data.m_Analogue[bin][strip] += data.m_GainFactor[strip] * response[bin];
          }
          // Add Crosstalk signal for neighboring strip
          // m_sct_amplifier->crosstalk(ChargesOnStrip, m_timeOfThreshold, response);
          for (short bin = 0; bin < bin_max; ++bin) {
            if (strip + 1 < strip_max) {
              data.m_Analogue[bin][strip + 1] += data.m_GainFactor[strip + 1] * response[bin];
            }
            if (strip > 0) {
              data.m_Analogue[bin][strip - 1] += data.m_GainFactor[strip - 1] * response[bin];
            }
          }
        } else { // Expanded
          // Amplifier response
          // m_sct_amplifier->response(ChargesOnStrip, m_timeOfThreshold, response);
          for (short bin = 0; bin < bin_max; ++bin) {
            data.m_Analogue[bin][strip] += data.m_GainFactor[strip] * response[bin];
          }
          // Add Crosstalk signal for neighboring strip
          // m_sct_amplifier->crosstalk(ChargesOnStrip, m_timeOfThreshold, response);
          for (short bin = 0; bin < bin_max; ++bin) {
            if (strip + 1 < strip_max) {
              data.m_Analogue[bin][strip + 1] += data.m_GainFactor[strip + 1] * response[bin];
            }
            if (strip > 0) {
              data.m_Analogue[bin][strip - 1] += data.m_GainFactor[strip - 1] * response[bin];
            }
          }
        }
      } else { // if roCell not valid
        ATH_MSG_WARNING("\t Cannot get the cell ");
      }
    } else {// If diode is disconnected/disabled skip it
      ATH_MSG_WARNING("\tDisabled or disconnected diode (strip)");
    }
  }
}


void ITkStripFrontEnd::doThresholdCheckForRealHits(SiChargedDiodeCollection& collection, ITkStripFrontEndData& data, const int& strip_max) const {
  // **********************************************************************************
  // Flag strips below threshold and flag the threshold check into data.m_StripHitsOnWafer
  // **********************************************************************************

  for (auto& [blub, diode]: collection) {
    const SiReadoutCellId & roCell = diode.getReadoutCell();
    if (roCell.isValid()) {
      int strip = roCell.strip();
      if (strip > -1 and strip < strip_max) {
        if (m_data_readout_mode == Condensed) {
          if ((data.m_Analogue[0][strip] >= m_Threshold or data.m_Analogue[1][strip] < m_Threshold)) {
            SiHelper::belowThreshold(diode, true);   // Below strip diode signal threshold
            data.m_StripHitsOnWafer[strip] = -1;
          } else if (((0x10 & diode.flag()) == 0x10) or ((0x4 & diode.flag()) == 0x4)) {
            // previously a crazy strip number could have screwed things up here.
            data.m_StripHitsOnWafer[strip] = -1;
          } else {
            data.m_StripHitsOnWafer[strip] = 1;
            SiHelper::SetTimeBin(diode, 2, &msg()); // set timebin info
          }
        } else { // Expanded
          int have_hit_bin = 0;
          if (data.m_Analogue[0][strip] >= m_Threshold) {
            have_hit_bin = 4;
          }
          if (data.m_Analogue[1][strip] >= m_Threshold) {
            have_hit_bin += 2;
          }
          if (data.m_Analogue[2][strip] >= m_Threshold) {
            have_hit_bin += 1;
          }
          if (((0x10 & diode.flag()) == 0x10) || ((0x4 & diode.flag()) == 0x4)) {
            // previously a crazy strip number could have screwed things up here.
            data.m_StripHitsOnWafer[strip] = -1;
          } else if (m_data_compression_mode == Level_X1X) { // !< level and expanded mode
            if (have_hit_bin == 2 or have_hit_bin == 3 or have_hit_bin == 6 or have_hit_bin == 7) {
              data.m_StripHitsOnWafer[strip] = 1;
              SiHelper::SetTimeBin(diode, have_hit_bin, &msg());
            } else {
              SiHelper::belowThreshold(diode, true); // Below strip diode signal threshold
              data.m_StripHitsOnWafer[strip] = -1;
            }
          } else if (m_data_compression_mode == Edge_01X) { // !< edge and expanded mode
            if (have_hit_bin == 2 or have_hit_bin == 3) {
              data.m_StripHitsOnWafer[strip] = 1;
              SiHelper::SetTimeBin(diode, have_hit_bin, &msg());
            } else {
              SiHelper::belowThreshold(diode, true); // Below strip diode signal threshold
              data.m_StripHitsOnWafer[strip] = -1;
            }
          } else if (m_data_compression_mode == AnyHit_1XX_X1X_XX1) { // !< any hit mode
            if (have_hit_bin == 0) {
              SiHelper::belowThreshold(diode, true); // Below strip diode signal threshold
              data.m_StripHitsOnWafer[strip] = -1;
            } else {
              data.m_StripHitsOnWafer[strip] = 1;
              if (m_data_readout_mode == Expanded) { // !< check for exp mode or not
                SiHelper::SetTimeBin(diode, have_hit_bin, &msg());
              } else {
                SiHelper::SetTimeBin(diode, 2, &msg());
              }
            }
          }
        }
      }
    }
  }
}
