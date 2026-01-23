/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "TrigT1Run3ZDC.h"

#include <bitset>
#include <fstream>
#include <stdexcept>

#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "ZdcConditions/ZdcLucrodMapRun3.h"
#include "ZdcIdentifier/ZdcID.h"

using json = nlohmann::json;

namespace LVL1 {

//---------------------------------
// initialise()
//---------------------------------

StatusCode TrigT1Run3ZDC::initialize() {
  ATH_CHECK(m_zdcCTPLocation.initialize());
  ATH_CHECK(m_zdcModuleCalibEnergyKey.initialize());
  ATH_CHECK(m_zldContainerName.initialize());
  // Find the full path to filename:
  std::string file = PathResolverFindCalibFile(m_lutFile);
  ATH_MSG_INFO("Reading file " << file);
  std::ifstream fin(file.c_str());
  if (!fin) {
    ATH_MSG_ERROR("Can not read file: " << file);
    return StatusCode::FAILURE;
  }
  json data = json::parse(fin);

  // Will eventually obtain LUTs from COOL, for now obtain them from calibration
  // area A data member to hold the side A LUT values
  std::array<unsigned int, 4096> sideALUTHG =
      data["LucrodHighGain"]["LUTs"]["sideA"];
  // A data member to hold the side C LUT values
  std::array<unsigned int, 4096> sideCLUTHG =
      data["LucrodHighGain"]["LUTs"]["sideC"];
  // A data member to hold the Combined LUT values
  std::array<unsigned int, 256> combLUTHG =
      data["LucrodHighGain"]["LUTs"]["comb"];
  // area A data member to hold the side A LUT values
  std::array<unsigned int, 4096> sideALUTLG =
      data["LucrodLowGain"]["LUTs"]["sideA"];
  // A data member to hold the side C LUT values
  std::array<unsigned int, 4096> sideCLUTLG =
      data["LucrodLowGain"]["LUTs"]["sideC"];
  // A data member to hold the Combined LUT values
  std::array<unsigned int, 256> combLUTLG =
      data["LucrodLowGain"]["LUTs"]["comb"];

  // Access Raw Lucrod Data
  const ZdcID *zdcId = nullptr;
  if (detStore()->retrieve(zdcId).isFailure()) {
    msg(MSG::ERROR)
        << "execute: Could not retrieve ZdcID object from the detector store"
        << endmsg;
    return StatusCode::FAILURE;
  } else {
    msg(MSG::DEBUG) << "execute: retrieved ZdcID" << endmsg;
  }
  m_zdcId = zdcId;

  std::array<std::array<unsigned int, 4>, 2> deriv2ndHGThresh;
  std::array<std::array<unsigned int, 4>, 2> deriv2ndLGThresh;

  for (unsigned int side : {0, 1}) {
    for (unsigned int module : {0, 1, 2, 3}) {
      deriv2ndHGThresh[side][module] = m_negHG2ndDerivThresh;
      deriv2ndLGThresh[side][module] = m_negLG2ndDerivThresh;
    }
  }

  // We disable the EM module in the LG trigger
  //
  deriv2ndLGThresh[0][0] = 4095;
  deriv2ndLGThresh[1][0] = 4095;

  std::cout << "Trigger simulation will use sample indices: " << m_minSampleAna
            << " through " << m_maxSampleAna
            << ", and baselinDelta = " << m_baselineDelta << std::endl;

  // Construct Simulation Objects
  m_triggerSimHGPtr = std::make_shared<ZDCTriggerSimFADC>(
      ZDCTriggerSimFADC(sideALUTHG, sideCLUTHG, combLUTHG, deriv2ndHGThresh,
                        m_minSampleAna, m_maxSampleAna, m_baselineDelta));

  m_triggerSimLGPtr = std::make_shared<ZDCTriggerSimFADC>(
      ZDCTriggerSimFADC(sideALUTLG, sideCLUTLG, combLUTLG, deriv2ndLGThresh,
                        m_minSampleAna, m_maxSampleAna, m_baselineDelta));

  m_hgFADC_ptr =
      std::make_shared<ZDCTriggerSim::FADCInputs>(ZDCTriggerSim::FADCInputs());
  m_lgFADC_ptr =
      std::make_shared<ZDCTriggerSim::FADCInputs>(ZDCTriggerSim::FADCInputs());

  ATH_MSG_DEBUG("TrigT1Run3ZDC initilized");
  return StatusCode::SUCCESS;
}

//----------------------------------------------
// execute() method called once per event
//----------------------------------------------

StatusCode TrigT1Run3ZDC::execute(const EventContext& ctx) const {

  // create uints to hold trigger averages
  unsigned int trigAvgAHG = 0;
  unsigned int trigAvgCHG = 0;
  unsigned int trigAvgALG = 0;
  unsigned int trigAvgCLG = 0;

  // create multi-dim array to store event by event flash ADC values
  std::array<std::array<std::array<unsigned int, 24>, 4>, 2> FADCSamplesHG;
  std::array<std::array<std::array<unsigned int, 24>, 4>, 2> FADCSamplesLG;

  // create vectors to store flattened above multi
  // dim arrays, in order to give the TriggerSimTool
  // data in the proper format
  std::vector<unsigned int> FADCFlattenedHG;
  std::vector<unsigned int> FADCFlattenedLG;

  // access LUCROD data

  // use readhandle to retrieve lucrodCollection
  SG::ReadHandle<ZdcLucrodDataContainer> lucrodCollection(m_zldContainerName,ctx);
  
  for (const ZdcLucrodData *zld : *lucrodCollection) {

    // get lucrod board number
    uint32_t lucrod_id = zld->GetLucrodID();

    // unpack data for each lucrod
    for (size_t i = 0; i < zld->GetChanDataSize(); i++) {

      // create channel object
      const ZdcLucrodChannel &zlc = zld->GetChanData(i);

      // figure out which channel we are reading out (0,1)
      uint16_t lucrod_channel = zlc.id;

      // variable to hold if it is a ZDC (0) or RPD (1) type
      // lucrod
      int type = ZdcLucrodMapRun3::getInstance()->getLucrod(
          lucrod_id)["type"][lucrod_channel];

      // trigger based on ZDC amplitudes, don't use
      // RPD channels
      if (type != 0) {
        continue;
      }

      // retrieve what side, module, and gain we are reading out
      int side = ZdcLucrodMapRun3::getInstance()->getLucrod(
          lucrod_id)["side"][lucrod_channel];
      int module = ZdcLucrodMapRun3::getInstance()->getLucrod(
          lucrod_id)["module"][lucrod_channel];
      int gain = ZdcLucrodMapRun3::getInstance()->getLucrod(
          lucrod_id)["gain"][lucrod_channel];
      // Fill different flash ADC vectors for Low and High
      // gain samples
      if (gain == 0) {
        unsigned int counter = 0;
        for (const auto &sample : zlc.waveform) {
          // fill low gain FADC samples
          FADCSamplesLG.at((side == 1)).at(module).at(counter) = sample;
          counter++;
        }
      } else if (gain == 1) {
        unsigned int counter = 0;
        for (const auto &sample : zlc.waveform) {
          // fill high gain FADC samples
          FADCSamplesHG.at((side == 1)).at(module).at(counter) = sample;
          counter++;
        }
      }
      // retrieve Trig Avg amp for debugging
      // from both LG and HG modules
      if (side * gain * module == 3) {
        trigAvgAHG = zld->GetTrigAvgA();
        trigAvgCHG = zld->GetTrigAvgC();
      } else if (side * module == 3) {
        trigAvgALG = zld->GetTrigAvgA();
        trigAvgCLG = zld->GetTrigAvgC();
      }
    }
  } // end lucrod loop

  // unpack FADC data into one long vector for both
  // high and low gain
  for (int side : {1, 0}) {
    for (int module : {0, 1, 2, 3}) {
      for (const auto sample : FADCSamplesLG.at(side).at(module)) {
        FADCFlattenedLG.push_back(sample);
      }
    }
  }

  for (int side : {1, 0}) {
    for (int module : {0, 1, 2, 3}) {
      for (const auto sample : FADCSamplesHG.at(side).at(module)) {
        FADCFlattenedHG.push_back(sample);
      }
    }
  }

  // set data in Trigger Sim Object
  m_hgFADC_ptr->setData(FADCFlattenedHG);
  m_lgFADC_ptr->setData(FADCFlattenedLG);

  // call ZDCTriggerSim to actually get L1 decisions
  unsigned int wordOutHG = m_triggerSimHGPtr->simLevel1Trig(m_hgFADC_ptr);
  unsigned int wordOutLG = m_triggerSimLGPtr->simLevel1Trig(m_lgFADC_ptr);

  // convert int to bitset for ZDC Triggers
  std::bitset<3> binhg(wordOutHG);

  // convert int to bitset for ZDC ALT Trigggers
  std::bitset<3> binlg(wordOutLG);

  // ZDC L1 items are located on CTPIN SLOT 9
  // Each slot holds 4 connectors that each carry a 32 bit trigger word
  // ZDC HG items are located on Connector 1 (CTPCAL)
  // at bits 25, 26, 27
  // ZDC LG (UCC) items are located on Connector 3 (NIM3)
  // at bits 28, 29, 30
  // for more info see
  // twiki.cern.ch/twiki/bin/view/Atlas/LevelOneCentralTriggerSetup#CTPIN_Slot_9

  // load HG output into trigger word on correct bits
  unsigned int word0 = 0;
  word0 += (binhg[0] << 25);
  word0 += (binhg[1] << 26);
  word0 += (binhg[2] << 27);

  // load LG output into trigger word on correct bits
  unsigned int word1 = 0;
  word1 += (binlg[0] << 28);
  word1 += (binlg[1] << 29);
  word1 += (binlg[2] << 30);

  // form CTP obejct
  SG::WriteHandle<ZdcCTP> zdcCTP = SG::makeHandle(m_zdcCTPLocation,ctx);

  // record CTP object
  ATH_CHECK(zdcCTP.record(std::make_unique<ZdcCTP>(word0, word1)));

  // debug output
  ATH_MSG_DEBUG(
      "Stored ZDC CTP object with words "
      << std::endl
      << std::hex << word0 << " from hgLUTOutput: " << std::dec << wordOutHG
      << " AvgAmpA: " << trigAvgAHG << " C: " << trigAvgCHG << std::endl
      << std::hex << word1 << " from lgLUTOutput: " << std::dec << wordOutLG
      << " AvgAmpA: " << trigAvgALG << " C: " << trigAvgCLG);
  return StatusCode::SUCCESS;
}
} // namespace LVL1
