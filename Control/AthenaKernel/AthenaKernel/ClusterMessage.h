/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ATHENAKERNEL_CLUSTERMESSAGE_H
#define ATHENAKERNEL_CLUSTERMESSAGE_H

#include <array>
#include <cstdint>
#include <memory>
#include <memory_resource>
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

/** @class Destination
 *  @brief An enum class denoting whether the data is heading to host or device
 * memory
 */
enum class Destination : std::uint8_t { Host = 0, Device, EMPTY };

/** @class ClusterMessage
 *  @brief A class describing a message sent between nodes in a cluster
 */
struct ClusterMessage {

  // Wire message consists of a header and an optional body.
  // If message type is Data, FinalWorkerStatus or WorkerError, payload
  // in header indicates tag used to send body.
  // message type, source, int payload
  using WireMsgHdr = std::array<std::uint32_t, 3>;  // three ints for the three
                                                    // components of the header
  using WireMsgBody = std::array<std::uint32_t, 10>;  // 320 bit max body length
                                                      // (for a DataDescr)
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
    Destination dest = Destination::Host;
    // 0 will always mean CPU memory, but other numbers might depend on the
    // destination rank

    std::size_t evtNumber = 0;
    std::size_t requestNumber = 0;

    std::pmr::memory_resource* allocating_memory_resource =
        nullptr;  // If this was received, we need to keep track of the memory
                  // resource used to allocate memory in order to free it

    // This enforces the invariant that align is a valid alignment for T
    template <typename T>
    DataDescr(T* ptr, std::size_t count = 1)
        : ptr((void*)ptr), len(count * sizeof(T)), align(alignof(T)) {}

    // This constructor is required to send back void*s
    DataDescr(void* ptr, std::size_t len, std::size_t align);

    DataDescr(DataDescr&& rhs) noexcept;

    DataDescr(const DataDescr&) = delete;
    DataDescr& operator=(const DataDescr&) = delete;

    DataDescr(const WireMsgBody& body,
              std::pmr::memory_resource* allocating_memory_resource =
                  std::pmr::new_delete_resource());

    DataDescr& operator=(DataDescr&& rhs) noexcept;

    /// Transfer a received allocation into an owner that retains its
    /// deallocator. The owner preserves the memory resource, byte count and
    /// alignment. The descriptor relinquishes ownership and clears ptr, len and
    /// align.
    /// @return The owner of the received host or device buffer.
    /// @throws std::logic_error if this descriptor only borrows the buffer.
    std::shared_ptr<void> takeOwnership();

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

  ClusterMessage(const WireMsg&,
                 std::pmr::memory_resource* allocatingMemoryResource =
                     std::pmr::new_delete_resource());

  [[nodiscard]] WireMsg wire_msg() const;

  static bool has_body(const WireMsgHdr& header);
};

#include "ClusterMessage.icc"
#endif  // ATHENAKERNEL_CLUSTERMESSAGE_H
