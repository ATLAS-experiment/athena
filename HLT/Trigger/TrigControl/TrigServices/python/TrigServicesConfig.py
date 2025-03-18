# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool

from AthenaCommon.Logging import logging
log = logging.getLogger('TrigServicesConfig')


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


def getHltROBDataProviderSvc(flags, name='ROBDataProviderSvc'):
   '''online ROB data provider service'''
   svc = CompFactory.HltROBDataProviderSvc(name,
      doCostMonitoring = (flags.Trigger.CostMonitoring.doCostMonitoring and
                          flags.Trigger.CostMonitoring.monitorROBs) )

   svc.MonTool = GenericMonitoringTool(flags, 'MonTool', HistPath='HLTFramework/'+name)
   svc.MonTool.defineHistogram('TIME_ROBReserveData', path='EXPERT', type='TH1F',
                               title='Time to reserve ROBs for later retrieval;time [mu s]',
                               xbins=100, xmin=0, xmax=1000)
   svc.MonTool.defineHistogram('NUMBER_ROBReserveData', path='EXPERT', type='TH1F',
                               title='Number of reserved ROBs for later retrieval;number',
                               xbins=100, xmin=0, xmax=500)
   svc.MonTool.defineHistogram('TIME_ROBRequest', path='EXPERT', type='TH1F',
                               title='Time for ROB retrievals;time [mu s]',
                               xbins=400, xmin=0, xmax=200000)
   svc.MonTool.defineHistogram('NUMBER_ROBRequest', path='EXPERT', type='TH1F',
                               title='Number of retrieved ROBs;number',
                               xbins=100, xmin=0, xmax=1000)
   svc.MonTool.defineHistogram('TIME_CollectAllROBs', path='EXPERT', type='TH1F',
                               title='Time for retrieving complete event data;time [mu s]',
                               xbins=400, xmin=0, xmax=200000)
   svc.MonTool.defineHistogram('NUMBER_CollectAllROBs', path='EXPERT', type='TH1F',
                               title='Number of received ROBs for collect call;number',
                               xbins=100, xmin=0, xmax=2500)

   return svc


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
   acc.addService( getHltROBDataProviderSvc(flags) )
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


if __name__=="__main__":
   from TrigPSC.PscDefaultFlags import defaultOnlineFlags
   flags = defaultOnlineFlags()
   flags.lock()

   cfg = ComponentAccumulator()
   cfg.merge( TrigServicesCfg(flags) )
   cfg.wasMerged()
