/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//===================================================================
//  Implementation of ByteStreamSamplingInputSvc
//===================================================================

// Include files.
#include "ByteStreamSamplingInputSvc.h"

#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "ByteStreamCnvSvcBase/ByteStreamAddress.h"
#include "ByteStreamData/ByteStreamMetadata.h"
#include "ByteStreamData/ByteStreamMetadataContainer.h"
#include "PersistentDataModel/DataHeader.h"
#include "StoreGate/StoreGateSvc.h"
#include "eformat/DetectorMask.h"
#include "eformat/HeaderMarker.h"
#include "webdaq/webdaq.hpp"
#include "xAODEventInfo/EventAuxInfo.h"
#include "xAODEventInfo/EventInfo.h"

#include <cstdlib>
#include <exception>
#include <memory>
#include <nlohmann/json.hpp>
#include <random>
#include <sstream>
#include <string>

// when the slot specific state is destroyed, we unsubscribe from
// the sampler
ByteStreamSamplingInputSvc::State::~State() {
  if (!m_subscription.empty() && !m_partition.empty()) {
      // ignore errors, nothing we can do on our side
      webdaq::emon::unsubscribe(m_partition, m_subscription);
  }
}

ByteStreamSamplingInputSvc::ByteStreamSamplingInputSvc(const std::string& name,
                                                       ISvcLocator* svcloc)
    : base_class(name, svcloc),
      m_inputMetaDataStore("StoreGateSvc/InputMetaDataStore", name),
      m_sgSvc("StoreGateSvc", name),
      m_robProvider("ROBDataProviderSvc", name) {}

// Check parameters and setup meta data store
StatusCode ByteStreamSamplingInputSvc::initialize() {
  // check properties
  if (m_partition.empty() && getenv("TDAQ_PARTITION") != 0) {
    m_partition = getenv("TDAQ_PARTITION");
  }

  if (m_partition.empty()) {
    ATH_MSG_ERROR("initialize: No partition name specified");
    return StatusCode::FAILURE;
  }

  if (m_criteria.empty()) {
    ATH_MSG_ERROR("initialize: No subscription criteria");
    return StatusCode::FAILURE;
  }

  if (m_group_id.empty()) {
    // with multiple threads each one subscribes independently
    // if the user has not set a group, we make one up just for this process
    char hostname[HOST_NAME_MAX];
    gethostname(hostname, sizeof(hostname));

    std::random_device rd;
    std::mt19937_64 gen(rd());
    uint64_t random_val = gen();

    // Format as a 16-character zero-padded hex string
    std::string random = std::format("{:016x}", random_val);
    m_group_id = std::format("athena-{}-{}-{}", hostname, getpid(), random);
  }

  //-------------------------------------------------------------------------
  // Setup the InputMetaDataStore
  //-------------------------------------------------------------------------
  ATH_CHECK(m_inputMetaDataStore.retrieve());
  ATH_CHECK(m_robProvider.retrieve());

  // Read run parameters from the partition
  if (m_readDetectorMask) {
    get_runparams();
  }

  ATH_MSG_INFO("initialized for: " << m_partition);

  return StatusCode::SUCCESS;
}

bool ByteStreamSamplingInputSvc::subscribe(State& state) {
  nlohmann::json runParams;
  while (!webdaq::is::get(m_partition, "RunParams", "RunParams", runParams)) {
    ATH_MSG_INFO("No such partition (yet?): " << m_partition
                                              << " -> waiting...");
    sleep(20);
  }

  nlohmann::json criteria;

  try {
    criteria = nlohmann::json::parse(m_criteria);
    criteria["sampler_type"] = m_sampler_type;
    criteria["group_id"] = m_group_id;
    criteria["sampler_keys"] = nlohmann::json(m_sampler_names.value());
  } catch (std::exception& ex) {
    ATH_MSG_ERROR("Subscription criteria cannot be parsed as JSON");
    return false;
  }

  while (true) {
    state.m_subscription = webdaq::emon::subscribe(m_partition, criteria);
    if (!state.m_subscription.empty()) {
      if (m_readDetectorMask) {
        get_runparams();
      }
      state.m_partition = m_partition;
      return true;
    } else {
      ATH_MSG_INFO("Cannot connect to sampler (will wait 20s and retry)");
      sleep(20);
    }
  }
}

// Read previous event should not be called for this version of input svc
const RawEvent* ByteStreamSamplingInputSvc::previousEvent() {
  ATH_MSG_WARNING(
      "previousEvent not implemented for ByteStreamSamplingInputSvc");

  return nullptr;
}

// Read the next event.
const RawEvent* ByteStreamSamplingInputSvc::nextEvent() {

  State& state = *m_slot_specific_data;

  // We only subscribe at the first event, otherwise we block in initialize()
  // the rest of athena
  if (state.m_subscription.empty()) {
    if (!subscribe(state)) {
      ATH_MSG_ERROR("Cannot subscribe");
      return nullptr;
    }
  }

  if (state.m_re) {
    OFFLINE_FRAGMENTS_NAMESPACE::PointerType st = nullptr;
    state.m_re->start(st);
    if (st)
      delete[] st;
    state.m_re.reset();
  }

  while (!state.m_re) {

    std::unique_ptr<uint32_t[]> raw_data;
    size_t length = 0;

    if (webdaq::emon::nextEvent(m_partition, state.m_subscription, raw_data,
                                length, std::chrono::seconds(m_timeout))) {

      OFFLINE_FRAGMENTS_NAMESPACE::DataType* buf =
          new OFFLINE_FRAGMENTS_NAMESPACE::DataType[length];
      memcpy(buf, raw_data.get(),
             length * sizeof(OFFLINE_FRAGMENTS_NAMESPACE::DataType));

      if (buf[0] == eformat::FULL_EVENT) {

        // We got a full event
        state.m_re = std::make_unique<RawEvent>(buf);
        try {
          state.m_re->check_tree();
          ATH_MSG_INFO("nextEvent: Got valid fragment of size:" << length);
        } catch (ers::Issue& ex) {

          // log in any case
          std::stringstream ss;
          ss << ex;
          ATH_MSG_ERROR("nextEvent: Invalid event fragment: " << ss.str());

          if (!m_corrupted_events) {
            delete[] buf;
            state.m_re.reset();
            continue;
          }  // else fall through
        }
        m_robProvider->setNextEvent(Gaudi::Hive::currentContext(),
                                    state.m_re.get());
        m_robProvider->setEventStatus(Gaudi::Hive::currentContext(), 0);

      } else {
        // We got something we didn't expect.
        ATH_MSG_ERROR("nextEvent: Got invalid fragment of unknown type: 0x"
                      << std::hex << buf[0] << std::dec);
        delete[] buf;
        continue;
      }
      ++m_totalEventCounter;
    }

    // generate DataHeader
    DataHeader* Dh = new DataHeader();

    // Declare header primary
    Dh->setStatus(DataHeader::Input);

    // Now add ref to xAOD::EventInfo objects
    CxxUtils::RefCountedPtr<ByteStreamAddress> iop(new ByteStreamAddress(
        ClassID_traits<xAOD::EventInfo>::ID(), "EventInfo", ""));
    StatusCode ioc = m_sgSvc->recordAddress("EventInfo", std::move(iop));
    if (ioc.isSuccess()) {
      const SG::DataProxy* ptmp = m_sgSvc->transientProxy(
          ClassID_traits<xAOD::EventInfo>::ID(), "EventInfo");
      if (ptmp != 0) {
        DataHeaderElement DheEI(ptmp, nullptr, "EventInfo");
        Dh->insert(DheEI);
      }
      // else ATH_MSG_ERROR("Failed to create xAOD::EventInfo proxy " << ptmp);
    }

    // Now add ref to xAOD::EventAuxInfo objects
    CxxUtils::RefCountedPtr<ByteStreamAddress> iopaux(new ByteStreamAddress(
        ClassID_traits<xAOD::EventAuxInfo>::ID(), "EventInfoAux.", ""));
    StatusCode iocaux =
        m_sgSvc->recordAddress("EventInfoAux.", std::move(iopaux));
    if (iocaux.isSuccess()) {
      const SG::DataProxy* ptmpaux = m_sgSvc->transientProxy(
          ClassID_traits<xAOD::EventAuxInfo>::ID(), "EventInfoAux.");
      if (ptmpaux != 0) {
        DataHeaderElement DheEIAux(ptmpaux, nullptr, "EventInfoAux.");
        Dh->insert(DheEIAux);
      }
      // else ATH_MSG_ERROR("Failed to create xAOD::EventAuxInfo proxy " <<
      // ptmpaux);
    }

    // Record new data header.Boolean flags will allow it's deletion in case
    // of skipped events.
    StatusCode rec_sg = m_sgSvc->record<DataHeader>(Dh, "ByteStreamDataHeader",
                                                    true, false, true);
    if (rec_sg != StatusCode::SUCCESS) {
      ATH_MSG_ERROR(
          "Fail to record BS DataHeader in StoreGate. Skipping events?! "
          << rec_sg);
    }
    return state.m_re.get();
  }

  return nullptr;
}

// Get a pointer to the current event.
const RawEvent* ByteStreamSamplingInputSvc::currentEvent() const {
  const State& state = *m_slot_specific_data;
  return state.m_re.get();
}

void ByteStreamSamplingInputSvc::get_runparams() {

  nlohmann::json runParams;
  if (webdaq::is::get(m_partition, "RunParams", "RunParams", runParams)) {

    eformat::helper::DetectorMask mask(runParams["det_mask"].get<uint64_t>());

    auto metadatacont = std::make_unique<ByteStreamMetadataContainer>();
    metadatacont->push_back(std::make_unique<ByteStreamMetadata>(
        runParams["run_number"], 0, 0, runParams["recording_enabled"],
        runParams["trigger_type"], mask.serialize().second,
        runParams["beam_type"], runParams["beam_energy"], "", "",
        runParams["T0_project_tag"], 0, std::vector<std::string>()));
    // Record ByteStreamMetadataContainer in MetaData Store
    if (m_inputMetaDataStore
            ->record(std::move(metadatacont), "ByteStreamMetadata")
            .isFailure()) {
      ATH_MSG_WARNING("Unable to record MetaData in InputMetaDataStore");
    } else {
      ATH_MSG_DEBUG("Recorded MetaData in InputMetaDataStore");
    }

  } else {
    ATH_MSG_ERROR("Cannot get run parameters");
  }
}

StatusCode ByteStreamSamplingInputSvc::finalize() {
  m_inputMetaDataStore.release().ignore();
  m_robProvider.release().ignore();
  m_sgSvc.release().ignore();
  return StatusCode::SUCCESS;
}
