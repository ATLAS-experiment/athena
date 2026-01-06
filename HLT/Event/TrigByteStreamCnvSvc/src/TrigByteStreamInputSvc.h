/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGBYTESTREAMINPUTSVC_H
#define TRIGBYTESTREAMINPUTSVC_H

#include "ByteStreamCnvSvcBase/IByteStreamInputSvc.h"
#include "ByteStreamCnvSvcBase/IROBDataProviderSvc.h"
#include "ByteStreamData/RawEvent.h"
#include "AthenaBaseComps/AthService.h"
#include "AthenaKernel/SlotSpecificObj.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "EFInterfaceSvc.h"
#include <memory.h>

// Forward declarations
class StoreGateSvc;

/** @class TrigByteStreamInputSvc
 *  @brief A ByteStreamInputSvc implementation for online use, reading events from hltinterface::DataCollector
 *
 *  The layout and implementation are based on ByteStreamEventStorageInputSvc
 **/
class TrigByteStreamInputSvc : public extends<AthService, IByteStreamInputSvc> {
public:
  /// Standard constructor
  TrigByteStreamInputSvc(const std::string& name, ISvcLocator* svcLoc);
  /// Standard destructor
  virtual ~TrigByteStreamInputSvc();

  // Helper for swapping between the 2 interfaces enum 
  enum class NextEventStatus { OK, NO_EVENT, STOP, ERROR };
  
  // ------------------------- Service methods --------------------------------
  virtual StatusCode initialize() override;
  virtual StatusCode finalize() override;

  // ------------------------- ByteStreamInputSvc methods ----------------------
  virtual const RawEvent* nextEvent() override;
  virtual const RawEvent* previousEvent() override;
  virtual const RawEvent* currentEvent() const override;

private:
  // ------------------------- Service handles ---------------------------------
  ServiceHandle<IROBDataProviderSvc> m_robDataProviderSvc {this, "ROBDataProvider", "ROBDataProviderSvc"};
  ServiceHandle<EFInterfaceSvc> m_efInterfaceSvc {this, "EFInterfaceSvc", "", "Online service managing EF interface with Dataflow"};
  ServiceHandle<StoreGateSvc> m_evtStore {this, "EventStore", "StoreGateSvc"};
  ToolHandle<GenericMonitoringTool> m_monTool {this, "MonTool", "" , "Monitoring tool"};

  // ------------------------- Properties --------------------------------------
  Gaudi::Property<int> m_checkCTPFragmentModuleID {this, "CheckCTPFragmentModuleID", -1,
    "After reading a new event, assert we can retrieve the CTP fragment with Module ID given by this property, "
    "and that has no errors. A value <0 disables the check."};

  // ------------------------- Private data members ----------------------------
  struct EventCache {
    ~EventCache() = default;
    void releaseEvent();
    std::unique_ptr<RawEvent> fullEventFragment {nullptr}; //!< Current event fragment
    std::unique_ptr<uint32_t[]> rawData {nullptr}; //!< Underlying data structure
  };

  SG::SlotSpecificObj<EventCache> m_eventsCache; //!< Cache of RawEvent pointer per event slot
  uint16_t m_maxLB{0}; //!< Maximum lumi block number seen so far, used for monitoring
  bool m_hasEFInterface{false}; //! Flag to select use of legacy or EF interface
};

#endif // TRIGBYTESTREAMINPUTSVC_H
