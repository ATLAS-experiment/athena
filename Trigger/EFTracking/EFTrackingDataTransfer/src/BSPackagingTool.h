/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef EFTRACKINGDATATRANSFER_BSPackagingTool_H
#define EFTRACKINGDATATRANSFER_BSPackagingTool_H

// Package includes
#include "IPackagingTool.h"

// Framework includes
#include "AthenaBaseComps/AthAlgTool.h"
#include "AthenaKernel/SlotSpecificObj.h"

#include "ByteStreamCnvSvcBase/IROBDataProviderSvc.h"
// STL includes
#include <string>

/**
 * @class BSPackagingTool
 * @brief Packaging tool for transfer of EFTracking ByteStream data.
 *
 * This tool assembles ROB fragments from selected detectors into an
 * OffloadMessage for EFTracking data transfer and unpacks received messages
 * back into the event context.
 */
class BSPackagingTool : public extends<AthAlgTool, IPackagingTool> {
public:
  BSPackagingTool(const std::string& type, const std::string& name, const IInterface* parent);
  virtual ~BSPackagingTool() override;

  virtual StatusCode initialize() override;

  /**
   * @brief Pack selected ROBs into the provided OffloadMessage.
   * @param msg Offload message to fill with packed ROB data.
   * @param context Current event context.
   * The msg will only contain ROBs that that come from specified detectors
   * @return StatusCode::SUCCESS on success.
   */
  virtual StatusCode pack(OffloadMessage& msg, const EventContext& context) const override;

  /**
   * @brief Unpack an OffloadMessage and restore the ROB content in the event context.
   * @param msg Offload message containing packed ROB data.
   * @param context Current event context.
   * The ROBs will be placed in the ROBDataProviderSvc instance, when unpacking all ROBs will unpacked
   * @return StatusCode::SUCCESS on success.
   */
  virtual StatusCode unpack(const OffloadMessage& msg, const EventContext& context) const override;


private:
  ServiceHandle<IROBDataProviderSvc> m_robsSvc{this, "ROBDataProvider", "ROBDataProviderSvc/ROBDataProviderSvc"};

  /// List of detector names whose ROBs should be included in the packed message.
  Gaudi::Property<std::vector<std::string>> m_det{this, "detectors", {}, "detectors from which the ROBs should be packaged"};

  /**
   * @brief Determine whether a ROB should be sent based on its source identifier.
   * @param sourceId ROB source identifier.
   * @return true if the ROB is selected for sending.
   */
  bool isROBToBeSent(eformat::helper::SourceIdentifier sourceId) const;

  /**
   * @brief cashe used wne unpacking 
   * (presumably the ownership in the future can be given to other component)
   */
  // mutable SG::SlotSpecificObj<std::vector<uint32_t>> m_eventsDataCache;
  mutable SG::SlotSpecificObj<RawEvent> m_eventsCache;

  };


#endif // EFTRACKINGDATATRANSFER_BSPackagingTool_H
