/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef BYTESTREAMCNVSVC_MCEVENTINFOBYTESTREAMTOOL_H
#define BYTESTREAMCNVSVC_MCEVENTINFOBYTESTREAMTOOL_H

/**
 * @file MCEventInfoByteStreamTool.h
 * @brief Tool for encoding/decoding MC EventInfo to/from ByteStream
 *
 * This tool serializes MC-specific EventInfo fields (mcChannelNumber, mcEventNumber,
 * mcEventWeights, actualInteractionsPerCrossing, averageInteractionsPerCrossing, etc.)
 * into a dedicated ROB fragment for MC ByteStream files (RDO->BS workflow).
 *
 * When reading MC ByteStream files, the EventInfoByteStreamAuxCnv uses this tool
 * to decode the MC EventInfo from the ROB fragment.
 */

#include "AthenaBaseComps/AthAlgTool.h"
#include "ByteStreamData/RawEvent.h"
#include "xAODEventInfo/EventInfo.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "AthenaKernel/SlotSpecificObj.h"
#include "Gaudi/Property.h"

#include <vector>
#include <memory>
#include <cstdint>

/**
 * @class IMCEventInfoByteStreamTool
 * @brief Interface for MCEventInfoByteStreamTool
 */
class IMCEventInfoByteStreamTool : virtual public IAlgTool {
public:
  DeclareInterfaceID(IMCEventInfoByteStreamTool, 1, 0);
  virtual ~IMCEventInfoByteStreamTool() override = default;

  /// Convert xAOD::EventInfo MC fields to ByteStream ROB fragment
  virtual StatusCode convertToBS(std::vector<OFFLINE_FRAGMENTS_NAMESPACE_WRITE::ROBFragment*>& vrobf,
                                 const EventContext& eventContext) = 0;

  /// Decode MC EventInfo from ROB fragment and fill xAOD::EventInfo
  virtual StatusCode convertFromBS(const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment* rob,
                                   xAOD::EventInfo& evtInfo) const = 0;

  /// Return the ROB IDs used by this tool
  virtual const std::vector<uint32_t>& robIds() const = 0;
};

/**
 * @class MCEventInfoByteStreamTool
 * @brief Implementation of MC EventInfo ByteStream encoding/decoding
 *
 * ROB Data Format (version 1.0):
 * - Word 0: Version (0x00010000 = v1.0)
 * - Word 1: mcChannelNumber (uint32_t)
 * - Word 2-3: mcEventNumber (uint64_t, low then high)
 * - Word 4: actualInteractionsPerCrossing (float as uint32_t)
 * - Word 5: averageInteractionsPerCrossing (float as uint32_t)
 * - Word 6: eventTypeBitmask (uint32_t)
 * - Word 7-8: pileUpMixtureIDLowBits (uint64_t, low then high)
 * - Word 9-10: pileUpMixtureIDHighBits (uint64_t, low then high)
 * - Word 11: extendedLevel1ID (uint32_t)
 * - Word 12: backgroundFlags (uint32_t)
 * - Word 13: number of weights (N)
 * - Words 14 to 14+N-1: mcEventWeights (N floats as uint32_t)
 */
class MCEventInfoByteStreamTool : public extends<AthAlgTool, IMCEventInfoByteStreamTool> {
public:
  MCEventInfoByteStreamTool(const std::string& type, const std::string& name, const IInterface* parent);
  virtual ~MCEventInfoByteStreamTool() override = default;

  virtual StatusCode initialize() override;

  /// Convert xAOD::EventInfo MC fields to ByteStream ROB fragment
  virtual StatusCode convertToBS(std::vector<OFFLINE_FRAGMENTS_NAMESPACE_WRITE::ROBFragment*>& vrobf,
                                 const EventContext& eventContext) override;

  /// Decode MC EventInfo from ROB fragment and fill xAOD::EventInfo
  virtual StatusCode convertFromBS(const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment* rob,
                                   xAOD::EventInfo& evtInfo) const override;

  /// Return the ROB IDs used by this tool
  virtual const std::vector<uint32_t>& robIds() const override { return m_robIds.value(); }

  /// Static ROB ID for MC EventInfo (SubDetector=OTHER=0xFF, ModuleId=0x01)
  static constexpr uint32_t MC_EVENTINFO_ROB_ID = 0x00ff0001;

  /// Version word for the data format
  static constexpr uint32_t FORMAT_VERSION = 0x00010000;  // v1.0

  /// Minimum number of words in the ROB data (excluding weights)
  static constexpr uint32_t MIN_DATA_WORDS = 14;

private:
  /// Helper to clear the ByteStream data cache for a given event slot
  void clearCache(const EventContext& eventContext);

  /// Allocate new array of raw ROD words for output ByteStream data
  uint32_t* newRodData(const EventContext& eventContext, size_t size);

  /// Allocate new ROBFragment for output ByteStream data
  OFFLINE_FRAGMENTS_NAMESPACE_WRITE::ROBFragment* newRobFragment(
    const EventContext& eventContext,
    uint32_t source_id,
    uint32_t ndata,
    const uint32_t* data);

  // Properties
  Gaudi::Property<std::vector<uint32_t>> m_robIds {
    this, "ROBIDs", {MC_EVENTINFO_ROB_ID}, "List of ROB IDs for MC EventInfo"};

  SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoReadKey {
    this, "EventInfoReadKey", "", "Read handle key for EventInfo (encoding mode)"};

  // Cache for ByteStream data representation
  struct Cache {
    std::vector<std::unique_ptr<uint32_t[]>> rodData;
    std::vector<std::unique_ptr<OFFLINE_FRAGMENTS_NAMESPACE_WRITE::ROBFragment>> robFragments;
    ~Cache() { clear(); }
    void clear() {
      rodData.clear();
      robFragments.clear();
    }
  };
  SG::SlotSpecificObj<Cache> m_cache;
};

#endif // BYTESTREAMCNVSVC_MCEVENTINFOBYTESTREAMTOOL_H
