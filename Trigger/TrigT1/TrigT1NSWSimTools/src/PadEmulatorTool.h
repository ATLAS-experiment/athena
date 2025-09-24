/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef PADEMULATORTOOL_H
#define PADEMULATORTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "Gaudi/Property.h"
#include "GaudiKernel/ConcurrencyFlags.h"
#include "GaudiKernel/ITHistSvc.h"
#include "GaudiKernel/ServiceHandle.h"
#include "MuonCondData/NswDcsDbData.h"
#include "MuonDigitContainer/sTgcDigitContainer.h"
#include "MuonDigitContainer/sTgcDigit.h"
#include "MuonIdHelpers/sTgcIdHelper.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonRDO/NSW_PadTriggerData.h"
#include "MuonRDO/NSW_PadTriggerDataContainer.h"
#include "MuonRDO/NSW_PadTriggerSegment.h"
#include "PadPattern.h"
#include "PadEmulatorCoincidences.h"
#include "PadEmulatorTrigger.h"
#include "PathResolver/PathResolver.h"
#include "TrigT1NSWSimTools/IPadEmulatorTool.h"
#include "TrigT1NSWSimExtras.h"
#include <fstream>

/**
 * @class PadEmulatorTool
 * @brief Tool responsible for evaluating sTGC Pad Trigger coincidences
 *
 * The tool reflects the hardware implementation of the algorithm.
 * List of patterns are loaded before the main algorithm is invoked, masking problematic ones.
 * Then, a list of ROB IDs is created, linked with the number of sectors.
 * If a sector has hits, coincidences are evaluated according to the previously loaded patterns.
 * Triggers undergo some additional checks to remove duplicates, forbidded bandIDs and other filters.
 * The output is saved in the NSW_PadTriggerData RDO, which will be merged in the general trigger RDO
 **/

namespace NSWL1 {

  class PadEmulatorTool : public extends<AthAlgTool, IPadEmulatorTool> {

    public:
      PadEmulatorTool(const std::string& type, const std::string& name, const IInterface* parent);
      virtual ~PadEmulatorTool() override = default;

      virtual StatusCode initialize() override;
      StatusCode loadPatterns(const std::string& pfile);
      StatusCode maskPads();
      StatusCode maskPatterns();
      StatusCode createRobIDs();
      virtual StatusCode attachBranches(MuonVal::MuonTesterTree &tree) override;
      virtual StatusCode emulate(const EventContext& ctx, Muon::NSW_PadTriggerDataContainer* out) const override;

      // Filter functions
      std::vector<uint32_t> getForbiddenBandIDs(const std::vector<PadEmulatorTrigger>& triggers, const bool isLarge) const;
      std::vector<PadEmulatorTrigger> filterPriorityEncoder(const std::vector<PadEmulatorTrigger>& input, const bool isLarge) const;
      std::vector<PadEmulatorTrigger> filterLowBandIDs(const std::vector<PadEmulatorTrigger>& input) const;
      std::vector<PadEmulatorTrigger> filterLastPhiOnly(const std::vector<PadEmulatorTrigger>& input) const;
      std::vector<PadEmulatorTrigger> filterDuplicates(const std::vector<PadEmulatorTrigger>& input) const;

    private:
      Gaudi::Property<bool> m_isMC                 {this, "IsMC",                 true,        "This is MC"};
      Gaudi::Property<bool> m_doNtuple             {this, "DoNtuple",             false,       "Save trigger information into the analysis flat ntuple"};
      Gaudi::Property<unsigned short> m_stretch    {this, "BCstretch",            2,           "Number of BCIDs to stretch pad trigger hits"};
      Gaudi::Property<bool> m_ignoreBCIDs          {this, "IgnoreBCIDs",          false,       "Ignore hit BCIDs"};
      Gaudi::Property<bool> m_lastPhiOnly          {this, "LastPhiOnly",          false,       "Have phi bias in real data"};
      Gaudi::Property<bool> m_bandIDPriorityEncode {this, "bandIDPriorityEncode", true,        "Have band ID bias"};
      Gaudi::Property<bool> m_noDuplicates         {this, "noDuplicates",         false,       "Remove trigger candidate duplicates"};
      Gaudi::Property<std::string> m_triggerLogic  {this, "triggerLogic",         "specific5over8",    "Other options: 3and1, 2and2, 4over8, specific4over8, 5over8, superspecific5over8, 6over8, 2x3over4, 8over8"};

      Gaudi::Property<std::string> m_pattername_L  {this, "PadPatternameL",
        PathResolver::find_calib_file("sTGCPadTriggerSim/patterns_corrected_large_19_12_2023_impr_07_03_2024_forFW.vhdl"), "Name of the vhdl file with pad patterns for LARGE sectors"};
      Gaudi::Property<std::string> m_pattername_S  {this, "PadPatternameS",
        PathResolver::find_calib_file("sTGCPadTriggerSim/patterns_corrected_small_19_12_2023_impr_07_03_2024_forFW.vhdl"), "Name of the vhdl file with pad patterns for SMALL sectors"};

      SG::ReadCondHandleKey<MuonGM::MuonDetectorManager> m_detectorManagerKey {this, "DetectorManagerKey", "MuonDetectorManager", "Key of MuonDetectorManager condition data"};
      ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc {this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
      SG::ReadHandleKey<sTgcDigitContainer> m_sTGCDigitContainerKey = {this, "sTGC_DigitContainerName", "sTGC_DIGITS", "Name of the sTGC digit container"};
      SG::ReadCondHandleKey<NswDcsDbData> m_dcsKey{this, "DCSDataKey", "NswDcsDbData", "Read key for the NSW DCS data"};

      std::vector<uint32_t> m_robIDs;
      std::vector<PadPattern> m_patterns_L, m_patterns_S;
      std::map<std::string, std::vector<std::tuple<uint32_t, uint32_t>>> m_maskedPatterns;

      /*
       * Flat ntuple branches for debug purposes: wheel, sector, hitmask, bandid, phiid, relbcid and other TP quantities
       */
      std::shared_ptr<MuonVal::VectorBranch<int> > m_padTrigger_digits_sector ATLAS_THREAD_SAFE {};
      std::shared_ptr<MuonVal::VectorBranch<int> > m_padTrigger_digits_eta ATLAS_THREAD_SAFE {};
      std::shared_ptr<MuonVal::VectorBranch<int> > m_padTrigger_digits_phi ATLAS_THREAD_SAFE {};
      std::shared_ptr<MuonVal::VectorBranch<int> > m_padTrigger_digits_multilayer ATLAS_THREAD_SAFE {};
      std::shared_ptr<MuonVal::VectorBranch<int> > m_padTrigger_digits_gasgap ATLAS_THREAD_SAFE {};
      std::shared_ptr<MuonVal::VectorBranch<uint32_t> > m_padTrigger_digits_source_id ATLAS_THREAD_SAFE {};
      std::shared_ptr<MuonVal::VectorBranch<uint32_t> > m_padTrigger_digits_pfeb ATLAS_THREAD_SAFE {};
      std::shared_ptr<MuonVal::VectorBranch<uint32_t> > m_padTrigger_digits_pad_channel ATLAS_THREAD_SAFE {};
      std::shared_ptr<MuonVal::VectorBranch<uint32_t> > m_padTrigger_digits_relbcid ATLAS_THREAD_SAFE {};
      std::shared_ptr<MuonVal::VectorBranch<uint8_t> > m_padTrigger_TP_R_id_init ATLAS_THREAD_SAFE {};
      std::shared_ptr<MuonVal::VectorBranch<uint8_t> > m_padTrigger_TP_phi_id_init ATLAS_THREAD_SAFE {};
      std::shared_ptr<MuonVal::VectorBranch<uint32_t> > m_padTrigger_TP_relbcid_init ATLAS_THREAD_SAFE {};
      std::shared_ptr<MuonVal::VectorBranch<uint8_t> > m_padTrigger_TP_R_id ATLAS_THREAD_SAFE {};
      std::shared_ptr<MuonVal::VectorBranch<uint8_t> > m_padTrigger_TP_phi_id ATLAS_THREAD_SAFE {};
      std::shared_ptr<MuonVal::VectorBranch<uint32_t> > m_padTrigger_TP_relbcid ATLAS_THREAD_SAFE {};
      std::shared_ptr<MuonVal::VectorBranch<char> > m_padTrigger_wheel ATLAS_THREAD_SAFE {};
      std::shared_ptr<MuonVal::VectorBranch<uint32_t> > m_padTrigger_sector ATLAS_THREAD_SAFE {};
      std::shared_ptr<MuonVal::VectorBranch<uint32_t> > m_padTrigger_hitmask ATLAS_THREAD_SAFE {};
      std::shared_ptr<MuonVal::VectorBranch<uint32_t> > m_padTrigger_efficiency ATLAS_THREAD_SAFE {};
  };
}
#endif
