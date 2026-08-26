#ifndef ATHENASERVICES_ATHENAREMOTEEVENTLOOPMGR_H
#define ATHENASERVICES_ATHENAREMOTEEVENTLOOPMGR_H

#include <AthenaBaseComps/AthMessaging.h>
#include <AthenaKernel/IAthenaEvtLoopPreSelectTool.h>
#include <AthenaKernel/ICollectionSize.h>
#include <AthenaKernel/IEventExecutionTool.h>
#include <AthenaKernel/IEventSeek.h>
#include <AthenaKernel/Timeout.h>
#include <GaudiKernel/IEvtSelector.h>
#include <GaudiKernel/IScheduler.h>
#include <GaudiKernel/MinimalEventLoopMgr.h>
#include <StoreGate/StoreGateSvc.h>

class AthenaRemoteEventLoopMgr
    : public extends<MinimalEventLoopMgr, IEventSeek, ICollectionSize,
                     IIncidentListener>,
      public Athena::TimeoutMaster,
      public AthMessaging {

  using AthMessaging::msg;
  using AthMessaging::msgLvl;

 public:
  /// Standard Constructor
  AthenaRemoteEventLoopMgr(const std::string& nam, ISvcLocator* svcLoc);

  /// implementation of IService::initialize
  virtual StatusCode initialize() override;
  /// implementation of IService::stop
  virtual StatusCode stop() override;
  /// implementation of IService::finalize
  virtual StatusCode finalize() override;
  /// implementation of IService::reinitialize
  // virtual StatusCode reinitialize() override;
  /// implementation of IService::restart
  // virtual StatusCode restart() override;
  /// implementation of IEventProcessor::nextEvent
  virtual StatusCode nextEvent(int maxevt) override;
  /// implementation of IEventProcessor::executeEvent(EventContext&&)
  virtual StatusCode executeEvent(EventContext&& ctx) override;
  /// implementation of IEventProcessor::executeRun( )
  virtual StatusCode executeRun(int maxevt) override;
  /// implementation of IEventProcessor::stopRun( )
  virtual StatusCode stopRun() override;

  /// Seek to a given event.
  virtual StatusCode seek(int evt) override;
  /// Return the current event count.
  virtual int curEvent() const override;
  /// Return the size of the collection.
  virtual int size() override;
  /// IIncidentListenet interfaces
  virtual void handle(const Incident& inc) override;

 protected:
  /// implementation of IEventProcessor::createEventContext()
  virtual EventContext createEventContext() override;

  /// property update handler:sets up the Pre-selection tools
  void setupPreSelectTools(Gaudi::Details::PropertyBase&);

  typedef ServiceHandle<IIncidentSvc> IIncidentSvc_t;
  typedef ServiceHandle<StoreGateSvc> StoreGateSvc_t;

  typedef IAthenaEvtLoopPreSelectTool tool_type;
  typedef ToolHandleArray<tool_type> tool_store;
  typedef tool_store::const_iterator tool_iterator;
  typedef std::vector<unsigned int> tool_stats;
  typedef tool_stats::const_iterator tool_stats_iterator;

  /// Reference to the incident service
  IIncidentSvc_t m_incidentSvc;
  /// Reference to StoreGateSvc;
  StoreGateSvc_t m_eventStore;  ///< Property

  ///@property List of AthenaEventLoopPreselectTools
  tool_stats m_toolInvoke;  ///< tool called counter
  tool_stats m_toolReject;  ///< tool returns StatusCode::FAILURE counter
  tool_stats m_toolAccept;  ///< tool returns StatusCode::SUCCESS counter
  tool_store m_tools;       ///< internal tool store

  ToolHandle<IEventExecutionTool> m_eventExecutionTool{
      this, "eventExecTool", "PlainEventExecutionTool/PlainEventExecutionTool",
      "Tool that wraps execution of the event"};

 private:
  AthenaRemoteEventLoopMgr() = delete;
  AthenaRemoteEventLoopMgr(const AthenaRemoteEventLoopMgr&) = delete;
  AthenaRemoteEventLoopMgr& operator=(const AthenaRemoteEventLoopMgr&) = delete;

  /// Initialize all algorithms and output streams
  StatusCode initializeAlgorithms();

  bool m_firstRun{true};

  /// Number of events processed
  size_t m_nevt{0};

  bool m_useTools{false};
};

#endif
