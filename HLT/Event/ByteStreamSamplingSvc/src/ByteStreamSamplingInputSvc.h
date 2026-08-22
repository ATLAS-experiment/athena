/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef BYTESTREAMCNVSVC_BYTESTREAMSAMPLINGINPUTSVC_H
#define BYTESTREAMCNVSVC_BYTESTREAMSAMPLINGINPUTSVC_H

/**
   @class ByteStreamSamplingInputSvc
   @brief implements the interface IByteStreamInputSvc for reading events
   from the Phase 2 EventSampling service via the TDAQ proxy.

*/

#include <Gaudi/Property.h>

#include "AthenaBaseComps/AthService.h"
#include "AthenaKernel/SlotSpecificObj.h"
#include "ByteStreamCnvSvcBase/IByteStreamInputSvc.h"
#include "ByteStreamCnvSvcBase/IROBDataProviderSvc.h"
#include "ByteStreamData/RawEvent.h"
#include "GaudiKernel/ServiceHandle.h"

#include <memory>
#include <vector>
#include <nlohmann/json.hpp>

class StoreGateSvc;

class ByteStreamSamplingInputSvc
    : public extends<AthService, IByteStreamInputSvc> {
 public:
  /// Constructors:
  ByteStreamSamplingInputSvc(const std::string& name, ISvcLocator* svcloc);

  virtual StatusCode initialize() override;
  virtual StatusCode finalize() override;

  /// Implementation of the ByteStreamInputSvc interface methods.
  virtual const RawEvent* previousEvent() override;
  virtual const RawEvent* nextEvent() override;

  /// Implementation of the ByteStreamInputSvc interface methods.
  virtual const RawEvent* currentEvent() const override;

 private:
  // our per slot state
  struct State {
    ~State();
    std::unique_ptr<RawEvent> m_re{
        nullptr};                  // current raw event pointer (per slot)
    std::string m_subscription{};  // an opaque string representing our
                                   // subscription identifier
    std::string m_partition;  // copy of partition name, needed in destructore
  };

  bool subscribe(State& state);
  void get_runparams();

  int m_totalEventCounter{0};  //!< event Counter

  // Properties
  Gaudi::Property<std::string> m_partition{
      this, "Partition", "",
      "Partition name, default taken from $TDAQ_PARTITION if not set"};

  Gaudi::Property<std::string> m_sampler_type{
      this, "SamplerType", "eb",
      "Sampler type, eg. 'eb' for event builder (default)"};

  Gaudi::Property<std::vector<std::string>> m_sampler_names{
      this,
      "SamplerNames",
      {},
      "Explicit list samplers, eg. { 'eb-01', 'eb-02' }"};

  Gaudi::Property<std::string> m_group_id{
      this, "GroupID", "",
      "Group ID, clients with the same group ID will receive distinct events"};

  Gaudi::Property<std::string> m_criteria{
      this, "Criteria", "",
      "Event sampling subscription criteria as JSON formatted string"};

  Gaudi::Property<bool> m_readDetectorMask{this, "ReadDetectorMaskFromIS", true,
                                           "Read detector mask from IS"};

  Gaudi::Property<int> m_timeout{
      this, "Timeout", 60, "Event receive timeout in seconds, -1 == infinity"};

  Gaudi::Property<bool> m_corrupted_events{
      this, "ProcessCorruptedEvents", false,
      "Process corrupted events not passing check_tree()"};

  // Slot specific state handler
  SG::SlotSpecificObj<State> m_slot_specific_data;

  ServiceHandle<StoreGateSvc> m_inputMetaDataStore;
  ServiceHandle<StoreGateSvc> m_sgSvc;
  ServiceHandle<IROBDataProviderSvc> m_robProvider;
  nlohmann::json m_subscribe_criteria;
};

#endif
