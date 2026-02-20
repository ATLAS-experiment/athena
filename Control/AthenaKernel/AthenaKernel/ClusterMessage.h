/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ATHENAKERNEL_CLUSTERMESSAGE_H
#define ATHENAKERNEL_CLUSTERMESSAGE_H

#include <array>
#include <optional>
#include <variant>

#include "GaudiKernel/StatusCode.h"
/** @class ClusterMessageType
 *  @brief An enum class defining what type of message this is
 */

enum class ClusterMessageType {
  RequestEvent,
  ProvideEvent,
  EventsDone,
  FinalWorkerStatus,
  WorkerError,
  EmergencyStop,
  Data,
  EMPTY
};

/** @class ClusterMessage
 *  @brief A class describing a message sent between nodes in a cluster
 */
struct ClusterMessage {

  // Wire message consists of a header and an optional body.
  // If message type is Data, FinalWorkerStatus or WorkerError, payload
  // in header indicates tag used to send body.
  // message type, source, int payload
  using WireMsgHdr = std::array<int, 3>; // three ints for the three components of the header
  using WireMsgBody = std::array<int, 10>; // 320 bit max body length (for a DataDescr)
  using WireMsg = std::tuple<WireMsgHdr, std::optional<WireMsgBody>>;

  int source = -1;  // Filled in when it is sent
  ClusterMessageType messageType{ClusterMessageType::EMPTY};

  struct WorkerStatus {
    StatusCode status{};
    int createdEvents = 0;
    int skippedEvents = 0;
    int finishedEvents = 0;
  };

  struct DataDescr {
    void* ptr = nullptr;
    std::size_t len = 0;
    std::size_t align = 0;

    bool received = false;  // set if this DataDescr was received and therefore
                            // owns its memory
    std::size_t evtNumber = 0;
    std::size_t fileNumber = 0;

    template <typename T>
    DataDescr(const T* ptr, std::size_t count = 1)
        : ptr((void*)ptr), len(count * sizeof(T)), align(alignof(T)) {}

    DataDescr(DataDescr&& rhs) noexcept;
    
    DataDescr(const DataDescr&) = delete;
    DataDescr& operator=(const DataDescr&) = delete;

    DataDescr(const WireMsgBody& body);

    DataDescr& operator=(DataDescr&& rhs) noexcept;

    ~DataDescr();
  };

  // A payload is used with:
  // ProvideEvent: an int indicating the event index
  // ProvideStatus and WorkerError: a WorkerStatus
  // Data: DataDescr for miscellaneous data (variable size)
  using Payload_t = std::variant<std::monostate, int, WorkerStatus, DataDescr>;
  Payload_t payload{};

  ClusterMessage();

  ClusterMessage(ClusterMessageType mType);

  ClusterMessage(ClusterMessageType mType, int payload);

  ClusterMessage(ClusterMessageType mType, WorkerStatus payload);

  ClusterMessage(ClusterMessageType mType, DataDescr&& payload);

  ClusterMessage(const WireMsg&);

  [[nodiscard]] WireMsg wire_msg() const;
};

#include "ClusterMessage.icc"
#endif  // ATHENAKERNEL_CLUSTERMESSAGE_H
