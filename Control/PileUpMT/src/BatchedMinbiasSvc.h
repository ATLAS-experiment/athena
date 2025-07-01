/* -*- C++ -*- */
/*
  Copyright (C) 2022, 2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PILEUPMT_BATCHEDMINBIASSVC_H
#define PILEUPMT_BATCHEDMINBIASSVC_H

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
#include "ISkipEventIdxSvc.h"
#include "PileUpMT/IMinbiasSvc.h"
#include "PileUpTools/IBeamIntensity.h"
#include "PileUpTools/IBeamLuminosity.h"
#include "StoreGate/StoreGateSvc.h"

class BatchedMinbiasSvc : public extends<AthService, IMinbiasSvc> {
 public:
  /// Constructor
  BatchedMinbiasSvc(const std::string& name, ISvcLocator* svc);
  /// Destructor
  ~BatchedMinbiasSvc();

  /// AthService initialize
  StatusCode initialize() override;

  StatusCode beginHardScatter(const EventContext& ctx) override;
  StoreGateSvc* getMinbias(const EventContext& ctx,
                           std::uint64_t mb_id) override;
  std::size_t getNumForBunch(const EventContext& ctx, int bunch) const override;
  virtual inline
  std::int64_t get_hs_id(const EventContext& ctx) const override {
    return m_skippedHSEvents.value() + ctx.evt();
  }
  StatusCode endHardScatter(const EventContext& ctx) override;

 private:
  using SGHandle = ServiceHandle<StoreGateSvc>;
  using SGHandleArray = std::vector<SGHandle>;
  Gaudi::Property<std::uint64_t> m_seed{this, "Seed", 0,
                                        "Additional seed for PRNGs"};
  Gaudi::Property<bool> m_onDemandMB{
      this, "OnDemandMB", false,
      "Should minbias event contents be read on demand"};
  Gaudi::Property<bool> m_usePoisson{this, "UsePoisson", true,
                                     "Whether to use a Poisson distribution "
                                     "(if False, use a delta distribution)"};
  Gaudi::Property<bool> m_useBeamInt{
      this, "UseBeamInt", true, "Whether to use the beam intensity service"};
  Gaudi::Property<bool> m_useBeamLumi{
      this, "UseBeamLumi", true, "Whether to use the beam luminosity service"};
  Gaudi::Property<int> m_MBBatchSize{
      this, "MBBatchSize", 10000,
      "Number of low pT minbias events to load per batch"};
  Gaudi::Property<int> m_NSimultaneousBatches{
      this, "NSimultaneousBatches", 1,
      "Max number of batches to load simultaneously"};
  Gaudi::Property<int> m_HSBatchSize{
      this, "HSBatchSize", 1,
      "Number of HS events per batch (aka max reuse factor)"};
  Gaudi::Property<int> m_skippedHSEvents{this, "SkippedHSEvents", 0,
                                         "Number of skipped HS events"};
  Gaudi::Property<float> m_nPerBunch{
      this, "AvgMBPerBunch", 0,
      "Average (max) number of minbias events per bunch"};
  Gaudi::Property<int> m_earliestDeltaBC{
      this, "EarliestDeltaBC", -32,
      "Earliest bunch crossing to consider (as delta)"};
  Gaudi::Property<int> m_latestDeltaBC{
      this, "LatestDeltaBC", +6,
      "Latest bunch crossing to consider (as delta)"};
  Gaudi::Property<std::vector<int>> m_actualNHSEventsPerBatch{
      this,
      "actualNHSEventsPerBatch",
      {},
      "Dynamic map of actual number of HS events for each batch, in this run."};
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

  std::vector<std::vector<std::uint64_t>> m_num_mb_by_bunch;
  std::vector<std::vector<std::uint64_t>> m_idx_lists;
  std::map<int, std::unique_ptr<SGHandleArray>> m_cache;
  std::map<int, std::mutex> m_cache_mtxs;  // protects m_cache entries
  // prevents attempting to read multiple batches at once
  std::mutex m_reading_batch_mtx;
  std::deque<std::unique_ptr<SGHandleArray>> m_empty_caches;
  std::mutex m_empty_caches_mtx;  // protects m_empty_caches
  std::vector<std::unique_ptr<std::atomic_int>> m_batch_use_count;
  std::atomic_int m_last_loaded_batch;
  int event_to_batch(std::int64_t hs_id);
  std::size_t calcMBRequired(std::int64_t hs_id, std::size_t slot,
                             unsigned int run, unsigned int lumi,
                             std::uint64_t event);
};

#endif  // PILEUPMT_BATCHEDMINBIASSVC_H
