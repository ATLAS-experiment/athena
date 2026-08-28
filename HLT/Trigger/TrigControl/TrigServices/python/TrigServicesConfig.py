# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool

from AthenaCommon.Logging import logging
log = logging.getLogger('TrigServicesConfig')


def setDefaultOnlineFlags(flags):
    """Populate flags with the default settings for online running"""

    from AthenaConfiguration.Enums import Format
    flags.Common.isOnline = True
    flags.Input.Files = []
    flags.Input.isMC = False
    flags.Input.Format = Format.BS
    flags.Trigger.doHLT = True  # This distinguishes the HLT setup from online reco (GM, EventDisplay)
    flags.Trigger.Online.isPartition = True  # athenaHLT and partition at P1
    flags.Trigger.EDMVersion = 3
    flags.Trigger.writeBS = True
    flags.Scheduler.CheckDependencies = True
    flags.Scheduler.ShowDataDeps = True
    flags.Scheduler.ShowControlFlow = True
    flags.Scheduler.ShowDataFlow = True
    flags.Scheduler.EnableVerboseViews = True
    flags.Scheduler.AutoLoadUnmetDependencies = False
    flags.Input.FailOnUnknownCollections = True


def getMessageSvc(flags, msgSvcType="TrigMessageSvc"):
   from AthenaCommon.Constants import DEBUG, WARNING

   # set message limit to unlimited when general DEBUG is requested
   msgLimit = -100 if flags.Exec.OutputLevel>DEBUG else 0

   msgsvc = CompFactory.getComp(msgSvcType)(
      "MessageSvc",
      OutputLevel = flags.Exec.OutputLevel,
      Format    = "%t  % F%40W%C%6W%R%e%4W%s%8W%R%T %0W%M",
      ErsFormat = "%S: %M",
      printEventIDLevel = WARNING,

      # Message suppression
      enableSuppression    = False,
      suppressRunningOnly  = True,
      # 0 = no suppression, negative = log-suppression, positive = normal suppression
      # Do not rely on the defaultLimit property, always set each limit separately
      defaultLimit = msgLimit,
      verboseLimit = msgLimit,
      debugLimit   = msgLimit,
      infoLimit    = msgLimit,
      warningLimit = msgLimit,
      errorLimit   = 0,
      fatalLimit   = 0,

      # Message forwarding to ERS
      useErsError = ['*'],
      useErsFatal = ['*'],
      ersPerEventLimit = 2,  # ATR-25214

      # show summary statistics of messages in finalize
      showStats = True,
      statLevel = WARNING
   )
   return msgsvc


def getTrigCOOLUpdateHelper(flags, name='TrigCOOLUpdateHelper'):
   '''Enable COOL folder updates'''

   acc = ComponentAccumulator()

   montool = GenericMonitoringTool(flags, 'MonTool', HistPath='HLTFramework/'+name)
   montool.defineHistogram('TIME_CoolFolderUpdate', path='EXPERT', type='TH1F',
                           title='Time for conditions update;time [ms]',
                           xbins=100, xmin=0, xmax=200)

   cool_helper = CompFactory.TrigCOOLUpdateHelper(
      name,
      MonTool = montool,
      CoolFolderMap = '/TRIGGER/HLT/COOLUPDATE',
      # List of folders that can be updated during the run:
      Folders = ['/Indet/Onl/Beampos',
                 '/TRIGGER/LUMI/HLTPrefLumi',
                 '/TRIGGER/HLT/PrescaleKey'] )

   from IOVDbSvc.IOVDbSvcConfig import addFolders
   acc.merge( addFolders(flags, cool_helper.CoolFolderMap, 'TRIGGER_ONL',
                         className='CondAttrListCollection') )

   acc.setPrivateTools( cool_helper )
   return acc


def getHltEventLoopMgr(flags, name='HltEventLoopMgr'):
   '''online event loop manager'''

   svc = CompFactory.HltEventLoopMgr(
      name,
      setMagFieldFromPtree = flags.Trigger.Online.BFieldAutoConfig
   )

   # Rewrite LVL1 result if L1 simulation and BS-writing is enabled
   if flags.Trigger.doLVL1 and flags.Trigger.writeBS:
      svc.RewriteLVL1 = True
      svc.L1TriggerResultRHKey = 'L1TriggerResult'
      svc.RoIBResultRHKey = 'RoIBResult'

   # Monitoring
   svc.MonTool = GenericMonitoringTool(flags, 'MonTool', HistPath='HLTFramework/'+name)

   svc.MonTool.defineHistogram('TotalTime', path='EXPERT', type='TH1F',
                              title='Total event processing time (all events);Time [ms];Events',
                              xbins=200, xmin=0, xmax=10000)
   svc.MonTool.defineHistogram('TotalTime;TotalTime_extRange', path='EXPERT', type='TH1F',
                              title='Total event processing time (all events);Time [ms];Events',
                              xbins=200, xmin=0, xmax=20000, opt='kCanRebin')

   svc.MonTool.defineHistogram('TotalTimeAccepted', path='EXPERT', type='TH1F',
                              title='Total event processing time (accepted events);Time [ms];Events',
                              xbins=200, xmin=0, xmax=10000)
   svc.MonTool.defineHistogram('TotalTimeAccepted;TotalTimeAccepted_extRange', path='EXPERT', type='TH1F',
                              title='Total event processing time (accepted events);Time [ms];Events',
                              xbins=200, xmin=0, xmax=20000, opt='kCanRebin')

   svc.MonTool.defineHistogram('TotalTimeRejected', path='EXPERT', type='TH1F',
                              title='Total event processing time (rejected events);Time [ms];Events',
                              xbins=200, xmin=0, xmax=10000)
   svc.MonTool.defineHistogram('TotalTimeRejected;TotalTimeRejected_extRange', path='EXPERT', type='TH1F',
                              title='Total event processing time (rejected events);Time [ms];Events',
                              xbins=200, xmin=0, xmax=20000, opt='kCanRebin')

   svc.MonTool.defineHistogram('SlotIdleTime', path='EXPERT', type='TH1F',
                              title='Time between freeing and assigning a scheduler slot;Time [ms];Events',
                              xbins=400, xmin=0, xmax=400)
   svc.MonTool.defineHistogram('SlotIdleTime;SlotIdleTime_extRange', path='EXPERT', type='TH1F',
                              title='Time between freeing and assigning a scheduler slot;Time [ms];Events',
                              xbins=400, xmin=0, xmax=800, opt='kCanRebin')

   svc.MonTool.defineHistogram('TIME_clearStore', path='EXPERT', type='TH1F',
                              title='Time of clearStore() calls;Time [ms];Calls',
                              xbins=200, xmin=0, xmax=50)
   svc.MonTool.defineHistogram('TIME_clearStore;TIME_clearStore_extRange', path='EXPERT', type='TH1F',
                              title='Time of clearStore() calls;Time [ms];Calls',
                              xbins=200, xmin=0, xmax=200, opt='kCanRebin')

   svc.MonTool.defineHistogram('PopSchedulerTime', path='EXPERT', type='TH1F',
                              title='Time spent waiting for a finished event from the Scheduler;Time [ms];drainScheduler() calls',
                              xbins=250, xmin=0, xmax=250)
   svc.MonTool.defineHistogram('PopSchedulerNumEvt', path='EXPERT', type='TH1F',
                              title='Number of events popped out of scheduler at the same time;Time [ms];drainScheduler() calls',
                              xbins=50, xmin=0, xmax=50)

   from TrigSteerMonitor.TrigSteerMonitorConfig import getTrigErrorMonTool
   svc.TrigErrorMonTool = getTrigErrorMonTool(flags)

   if flags.Trigger.CostMonitoring.doCostMonitoring:
      svc.TrigErrorMonTool.TrigCostSvc = CompFactory.TrigCostSvc()

   return svc


def TrigServicesCfg(flags):
   acc = ComponentAccumulator()

   acc.addService( getMessageSvc(flags) )
   acc.addService(CompFactory.ROBDataProviderSvc('ROBDataProviderSvc'))

   cool_helper = acc.popToolsAndMerge( getTrigCOOLUpdateHelper(flags) )

   loop_mgr = getHltEventLoopMgr(flags)
   loop_mgr.CoolUpdateTool = cool_helper

   from TriggerJobOpts.TriggerHistSvcConfig import TriggerHistSvcConfig
   acc.merge( TriggerHistSvcConfig(flags) )

   from TrigOutputHandling.TrigOutputHandlingConfig import HLTResultMTMakerCfg
   loop_mgr.ResultMaker = HLTResultMTMakerCfg(flags)

   from TriggerJobOpts.TriggerByteStreamConfig import ByteStreamReadCfg
   acc.merge(ByteStreamReadCfg(flags))
   loop_mgr.EvtSel = acc.getService('EventSelectorByteStream')
   loop_mgr.OutputCnvSvc = acc.getService('ByteStreamCnvSvc')

   # Rewrite LVL1 result if L1 simulation and BS-writing is enabled
   if flags.Trigger.doLVL1 and flags.Trigger.writeBS:
      from TrigT1ResultByteStream.TrigT1ResultByteStreamConfig import L1TriggerByteStreamEncoderCfg
      acc.merge(L1TriggerByteStreamEncoderCfg(flags))

   from TrigSteerMonitor.TrigSteerMonitorConfig import SchedulerMonSvcCfg
   acc.merge( SchedulerMonSvcCfg(flags) )
   loop_mgr.MonitorScheduler = True

   acc.addService(loop_mgr, primary=True)
   acc.setAppProperty("EventLoop", loop_mgr.name)

   return acc


def commonServicesCfg(flags):
    from AthenaCommon.Constants import INFO
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    from AthenaConfiguration.ComponentFactory import CompFactory

    # Basic services
    cfg = ComponentAccumulator()
    cfg.addService(CompFactory.ClassIDSvc(CLIDDBFiles = ['clid.db','Gaudi_clid.db']))

    cfg.addService(CompFactory.AlgContextSvc(BypassIncidents=True))
    cfg.addAuditor(CompFactory.AlgContextAuditor())

    cfg.addService(CompFactory.StoreGateSvc())
    cfg.addService(CompFactory.StoreGateSvc("DetectorStore"))
    cfg.addService(CompFactory.StoreGateSvc("HistoryStore"))
    cfg.addService(CompFactory.StoreGateSvc("ConditionStore"))

    cfg.addService( CompFactory.SG.HiveMgrSvc(
        "EventDataSvc",
        NSlots = flags.Concurrency.NumConcurrentEvents) )

    cfg.addService( CompFactory.AlgResourcePool(
        OutputLevel = INFO,
        TopAlg=["AthSequencer/AthMasterSeq"]) )

    from AthenaConfiguration.MainServicesConfig import AvalancheSchedulerSvcCfg
    cfg.merge( AvalancheSchedulerSvcCfg(flags, maxParallelismExtra=1) )

    # SGCommitAuditor to sweep new DataObjects at end of Alg execute
    cfg.addAuditor( CompFactory.SGCommitAuditor() )

    # CoreDumpSvc
    cfg.addService( CompFactory.CoreDumpSvc(
        CoreDumpStream = "stdout",
        CallOldHandler = False,  # avoid calling e.g. ROOT signal handler
        FastStackTrace = True,   # first produce a fast stacktrace
        StackTrace = True,       # then produce full stacktrace using gdb
        DumpCoreFile = True,     # also produce core file (if allowed by ulimit -c)
        FatalHandler = 0,        # no extra fatal handler
        KillOnSigInt = True,    # athenaEF runs the event loop in-process (ATR-32990)
        TimeOut = 120e9),        # timeout for stack trace generation changed to 120s (ATR-17112,ATR-25404)
                    create = True )    # always create the service

    # IOVSvc
    cfg.addService( CompFactory.IOVSvc(
        updateInterval = "RUN",
        preLoadData = True,
        preLoadExtensibleFolders = False,  # ATR-19392
        forceResetAtBeginRun = False) )

    # PerfMon
    if flags.PerfMon.doFastMonMT or flags.PerfMon.doFullMonMT:
        from PerfMonComps.PerfMonCompsConfig import PerfMonMTSvcCfg
        cfg.merge( PerfMonMTSvcCfg(flags) )

    from TrigServices.TrigServicesConfig import TrigServicesCfg
    cfg.merge( TrigServicesCfg(flags) )

    # ApplicationMgr properties
    cfg.setAppProperty('AuditAlgorithms', True)
    cfg.setAppProperty('InitializationLoopCheck', False)

    return cfg


if __name__=="__main__":
   from AthenaConfiguration.AllConfigFlags import initConfigFlags

   flags = initConfigFlags()
   setDefaultOnlineFlags(flags)
   flags.lock()

   cfg = ComponentAccumulator()
   cfg.merge( commonServicesCfg(flags) )
   cfg.wasMerged()
