/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EVENTSELECTORATHENAPOOLSHAREDIO_H
#define EVENTSELECTORATHENAPOOLSHAREDIO_H

/** @file EventSelectorAthenaPoolSharedIO.h
 *  @brief This file contains the class definition for the EventSelectorAthenaPoolSharedIO class.
 *  @author Peter van Gemmeren <gemmeren@anl.gov>
 **/

#include "EventSelectorAthenaPool.h"

#include "AthenaKernel/IAthenaIPCTool.h"
#include "AthenaKernel/IEventShare.h"

/** @class EventSelectorAthenaPoolSharedIO
 *  @brief This class is the EventSelector for event data.
 **/
class EventSelectorAthenaPoolSharedIO :
  public extends<EventSelectorAthenaPool, IEventShare>
{

public: // Constructor and Destructor
   /// Standard Service Constructor
   EventSelectorAthenaPoolSharedIO(const std::string& name, ISvcLocator* pSvcLocator);
   /// Destructor
   virtual ~EventSelectorAthenaPoolSharedIO() = default;

   /// Required of all Gaudi Services
   virtual StatusCode initialize() override;
   virtual StatusCode start() override;
   virtual StatusCode stop() override;
   virtual StatusCode finalize() override;

   //-------------------------------------------------
   // IEventSelector
   /// @param ctxt [IN/OUT] current event context is interated to next event.
   virtual StatusCode next(IEvtSelector::Context& ctxt) const override;
   /// @param ctxt [IN/OUT] current event context is interated to next event.
   /// @param jump [IN] number of events to jump (currently not supported).
   virtual StatusCode next(IEvtSelector::Context& ctxt, int jump) const override;

   //-------------------------------------------------
   // IEventShare
   /// Make this a server.
   virtual StatusCode makeServer(int num) override;

   /// Make this a client.
   virtual StatusCode makeClient(int num) override;

   /// Request to share a given event number.
   /// @param evtnum [IN]  The event number to share.
   virtual StatusCode share(int evtnum) override;

   /// Read the next maxevt events.
   /// @param evtnum [IN]  The number of events to read.
   virtual StatusCode readEvent(int maxevt) override;

   //-------------------------------------------------
   // IIoComponent
   /// Callback method to reinitialize the internal state of the component for I/O purposes (e.g. upon @c fork(2))
   virtual StatusCode io_reinit() override;

protected:
   //-------------------------------------------------
   // ISecondaryEventSelector
   /// Record AttributeList in StoreGate
   virtual StatusCode recordAttributeList() const override;

private: // properties
   ToolHandle<IAthenaIPCTool> m_eventStreamingTool{this, "SharedMemoryTool", "", ""};
   /// Make this instance a Streaming Client during first iteration automatically
   Gaudi::Property<int> m_makeStreamingToolClient{this, "MakeStreamingToolClient", 0};
};

#endif
