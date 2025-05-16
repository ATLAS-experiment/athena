/*
  Copyright (C) 2022, 2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PILEUPMT_PILEUPMTALG_H
#define PILEUPMT_PILEUPMTALG_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "AthenaKernel/IAthRNGSvc.h"
#include <BeamSpotConditionsData/BeamSpotData.h>
#include "PileUpMT/IMinbiasSvc.h"
#include "PileUpTools/IBeamIntensity.h"
#include "PileUpTools/IBeamLuminosity.h"
#include "PileUpTools/IPileUpTool.h"
#include "StoreGate/ActiveStoreSvc.h"
#include "src/ISkipEventIdxSvc.h"
#include "xAODCnvInterfaces/IEventInfoCnvTool.h"
#include "xAODEventInfo/EventInfo.h"
#include "xAODEventInfo/EventAuxInfo.h"
#include "xAODEventInfo/EventInfoContainer.h"

// Example ROOT Includes
// #include "TTree.h"
// #include "TH1D.h"
#include <fmt/chrono.h>
#include <fmt/compile.h>
#include <fmt/format.h>

#include <atomic>
#include <chrono>
#include <cstdio>
#include <string>
#include <string_view>
#include <thread>

class atomic_output {
 private:
  std::FILE* m_file{};
  std::mutex m_mtx{};

 public:
  atomic_output() = default;
  void init(const std::string& filename) {
    using namespace std::chrono;
    std::lock_guard lck{m_mtx};
    if (m_file != nullptr) {
      return;
    }
    m_file = std::fopen(filename.c_str(), "a");
    auto time = fmt::localtime(system_clock::to_time_t(system_clock::now()));
    fmt::print(m_file, ("FILE CREATED ON {:%Y-%m-%d} at {:%H:%M:%S %Z}\n"),
               time, time);
  }
  ~atomic_output() {
    std::lock_guard lck{m_mtx};
    if (m_file != nullptr) {
      std::fclose(m_file);
      m_file = nullptr;
    }
  }

  void print(const fmt::memory_buffer& str) {
    std::lock_guard lck{m_mtx};
    std::fwrite(str.data(), sizeof(char), str.size(), m_file);
    std::fflush(m_file);
  }
};

class PileUpMTAlg : public AthAlgorithm {
 public:
  PileUpMTAlg(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~PileUpMTAlg();

  /// uncomment and implement methods as required

  // IS EXECUTED:
  virtual StatusCode initialize() override;  // once, before any input is loaded
  virtual StatusCode execute() override;     // per event
  virtual StatusCode finalize() override;    // once, after all events processed

  // Make this algorithm clonable
  virtual bool isClonable() const override { return true; }
  // virtual unsigned int cardinality() const override { return m_cardinality; }

  /// Other useful methods provided by base class are:
  /// evtStore()        : ServiceHandle to main event data storegate
  /// inputMetaStore()  : ServiceHandle to input metadata storegate
  /// outputMetaStore() : ServiceHandle to output metadata storegate
  /// histSvc()         : ServiceHandle to output ROOT service (writing
  /// TObjects) currentFile()     : TFile* to the currently open input file
  /// retrieveMetadata(...): See
  /// twiki.cern.ch/twiki/bin/view/AtlasProtected/AthAnalysisBase#ReadingMetaDataInCpp

 private:
  ServiceHandle<IMinbiasSvc> m_lowptMBSvc{this, "LowPtMinbiasSvc", "",
                                          "MinbiasSvc for low pT minbias"};
  ServiceHandle<IMinbiasSvc> m_highptMBSvc{this, "HighPtMinbiasSvc", "",
                                           "MinbiasSvc for high pT minbias"};
  ServiceHandle<IMinbiasSvc> m_cavernMBSvc{this, "CavernMinbiasSvc", "",
                                           "MinbiasSvc for cavern background"};
  ServiceHandle<IMinbiasSvc> m_beamhaloMBSvc{
      this, "BeamHaloMinbiasSvc", "", "MinbiasSvc for beam halo background"};
  ServiceHandle<IMinbiasSvc> m_beamgasMBSvc{
      this, "BeamGasMinbiasSvc", "", "MinbiasSvc for beam gas background"};
  ServiceHandle<IBeamIntensity> m_beamInt{this, "BeamIntSvc", "FlatBM",
                                          "Beam intensity service"};
  ServiceHandle<IBeamLuminosity> m_beamLumi{
      this, "BeamLumiSvc", "LumiProfileSvc", "Beam luminosity service"};
  ServiceHandle<IAthRNGSvc> m_rngSvc{this, "RNGSvc", "AthRNGSvc/PileupRNG",
                                     "RNG service for pile-up digitization"};
  ToolHandle<xAODMaker::IEventInfoCnvTool> m_xAODEICnvTool{
      this, "xAODCnvTool", "xAODMaker::EventInfoCnvTool/EventInfoCnvTool",
      "xAOD EventInfo conversion tool"};
  ToolHandleArray<IPileUpTool> m_puTools{
      this, "PileUpTools", {}, "Pileup tools"};
  Gaudi::Property<bool> m_writeTrace{this, "WriteTrace", false, "Write trace of pileup events used"};
  Gaudi::Property<int> m_skippedHSEvents{this, "SkippedHSEvents", 0,
                                         "Number of skipped HS events"};
  Gaudi::Property<float> m_avgMu{this, "AverageMu", 200, "Average mu"};
  Gaudi::Property<float> m_fracLowPt{this, "FracLowPt", 0.,
                                     "Fraction of pileup that has low pT"};
  Gaudi::Property<float> m_fracHighPt{this, "FracHighPt", 0.,
                                      "Fraction of pileup that has high pT"};
  Gaudi::Property<float> m_numCavern{
      this, "NumCavern", 0., "Number of cavern background events per BC"};
  Gaudi::Property<float> m_numBeamGas{
      this, "NumBeamGas", 0., "Number of beam gas background events per BC"};
  Gaudi::Property<float> m_numBeamHalo{
      this, "NumBeamHalo", 0., "Number of beam halo background events per BC"};
  Gaudi::Property<int> m_BCSpacing{this, "BCSpacing", 25,
                                   "Bunch crossing spacing in ns"};
  Gaudi::Property<int> m_earliestDeltaBC{
      this, "EarliestDeltaBC", -32,
      "Earliest bunch crossing to consider (as delta)"};
  Gaudi::Property<int> m_latestDeltaBC{
      this, "LatestDeltaBC", +6,
      "Latest bunch crossing to consider (as delta)"};
  ServiceHandle<ISkipEventIdxSvc> m_skipEventIdxSvc{
      this, "SkipEvtIdxSvc", "SkipEventIdxSvc",
      "Skipped event index (run / lb num) provider"};
  SG::ReadCondHandleKey<InDet::BeamSpotData> m_beamSpotKey{
      this, "BeamSpotKey", "BeamSpotData", "Beam spot info key"};
  SG::WriteHandleKey<xAOD::EventInfo> m_evtInfoKey{
      this, "EventInfoKey", "OverlayEvent", "Overlay EventInfo key"};
  SG::WriteHandleKey<xAOD::EventInfoContainer> m_evtInfoContKey{""};
  // Utilities
  StatusCode get_ei(StoreGateSvc& sg,
                    std::unique_ptr<const xAOD::EventInfo>& ei,
                    std::unique_ptr<xAOD::EventAuxInfo>& eiAux,
                    bool pileup = false) const;
  inline unsigned int get_BCID(int bc, unsigned int central_BCID) const {
    constexpr int maxBCPerOrbit = 3564;  // FIXME may need update
    return static_cast<unsigned int>((bc + static_cast<int>(central_BCID)) %
                                     maxBCPerOrbit);
  }
  StatusCode add_subevt(const std::vector<std::uint32_t>& bcid,
                        SG::WriteHandle<xAOD::EventInfo>& overlaidEvt,
                        SG::WriteHandle<xAOD::EventInfoContainer>& puCont,
                        ServiceHandle<IMinbiasSvc>& mbSvc,
                        xAOD::EventInfo::PileUpType puType, int bc,
                        const EventContext& ctx, unsigned long subevt_id,
                        std::vector<std::uint64_t>& trace);
  inline static atomic_output m_pileupTrace ATLAS_THREAD_SAFE{};
  inline static bool m_skiptrace_written ATLAS_THREAD_SAFE = false;
};

#endif  //> !PILEUPMT_PILEUPMTALG_H
