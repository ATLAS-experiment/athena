/* -*- C++ -*- */
/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PILEUPMT_ONDEMANDMINBIASSVC_H
#define PILEUPMT_ONDEMANDMINBIASSVC_H

#include <atomic>
#include <deque>
#include <format>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "AthenaBaseComps/AthService.h"
#include "Gaudi/Property.h"
#include "GaudiKernel/IEvtSelector.h"
#include "GaudiKernel/ServiceHandle.h"
#include "PileUpMT/IMinbiasSvc.h"
#include "PileUpTools/IBeamIntensity.h"
#include "PileUpTools/IBeamLuminosity.h"
#include "StoreGate/StoreGateSvc.h"
#include "src/ISkipEventIdxSvc.h"

class OnDemandMinbiasSvc final : public extends<AthService, IMinbiasSvc> {
 public:
  /// Constructor
  OnDemandMinbiasSvc(const std::string& name, ISvcLocator* svc);
  /// Destructor
  ~OnDemandMinbiasSvc() final;

  /// AthService initialize
  StatusCode initialize() final;

  StatusCode beginHardScatter(const EventContext& ctx) override;
  StoreGateSvc* getMinbias(const EventContext& ctx,
                           std::uint64_t mb_id) override;
  std::size_t getNumForBunch(const EventContext& ctx, int bunch) const override;
  inline virtual
  std::int64_t get_hs_id(const EventContext& ctx) const override {
    return m_skippedHSEvents.value() + ctx.evt();
  }

  StatusCode endHardScatter(const EventContext& ctx) override;

 private:
  using SGHandle = ServiceHandle<StoreGateSvc>;

  Gaudi::Property<std::uint64_t> m_seed{this, "Seed", 0,
                                        "Additional seed for PRNGs"};
  Gaudi::Property<bool> m_onDemandMB{
      this, "OnDemandMB", false,
      "Should minbias event contents be read on demand"};
  Gaudi::Property<int> m_skippedHSEvents{this, "SkippedHSEvents", 0,
                                         "Number of skipped HS events"};
  Gaudi::Property<float> m_nPerBunch{
      this, "AvgMBPerBunch", 0.f,
      "Average (max) number of minbias events per bunch"};
  Gaudi::Property<bool> m_usePoisson{this, "UsePoisson", true,
                                     "Whether to use a Poisson distribution "
                                     "(if False, use a delta distribution)"};
  Gaudi::Property<bool> m_useBeamInt{
      this, "UseBeamInt", true, "Whether to use the beam intensity service"};
  Gaudi::Property<bool> m_useBeamLumi{
      this, "UseBeamLumi", true, "Whether to use the beam luminosity service"};
  Gaudi::Property<int> m_earliestDeltaBC{
      this, "EarliestDeltaBC", -32,
      "Earliest bunch crossing to consider (as delta)"};
  Gaudi::Property<int> m_latestDeltaBC{
      this, "LatestDeltaBC", +6,
      "Latest bunch crossing to consider (as delta)"};
  ServiceHandle<ISkipEventIdxSvc> m_skipEventIdxSvc{
      this, "SkipEvtIdxSvc", "SkipEventIdxSvc",
      "Skipped event index (run / lb num) provider"};
  ServiceHandle<IEvtSelector> m_bkgEventSelector{
      this, "BkgEventSelector", {}, "Event selector for minbias events"};
  ServiceHandle<IBeamIntensity> m_beamInt{this, "BeamIntSvc", "FlatBM",
                                          "Beam intensity service"};
  ServiceHandle<IBeamLuminosity> m_beamLumi{
      this, "BeamLumiSvc", "LumiProfileSvc", "Beam luminosity service"};
  ServiceHandle<ActiveStoreSvc> m_activeStoreSvc{
      this, "ActiveStoreSvc", "ActiveStoreSvc", "ActiveStoreSvc"};

  SGHandle m_spare_store{this, "StoreGateSvc",
                         std::format("StoreGateSvc/discards_{}", name()),
                         "StoreGate for discarding events"};
  IEvtSelector::Context* m_bkg_evt_sel_ctx;
  ServiceHandle<IProxyProviderSvc> m_proxyProviderSvc;

  std::vector<std::vector<SGHandle>> m_stores;
  std::vector<std::vector<std::uint64_t>> m_num_mb_by_bunch;
  std::vector<std::vector<std::uint64_t>> m_idx_lists;
  // prevents attempting to read multiple batches at once
  std::mutex m_reading_batch_mtx;
  std::atomic_int64_t m_last_loaded_hs{-1};

  static constexpr std::size_t s_NoSlot =
      std::numeric_limits<std::size_t>::max();  // "slot" to pass when we don't
                                                // have a slot
  std::size_t calcMBRequired(std::int64_t hs_id, std::size_t slot,
                             unsigned int run, unsigned int lumi,
                             std::uint64_t event);
};

#endif  // PILEUPMT_ONDEMANDMINBIASSVC_H
