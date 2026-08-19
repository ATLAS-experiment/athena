# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

## @file TriggerUnixStandardSetup.py
## @brief py-module to configure the Athena AppMgr for trigger
## @author Werner Wiedenmann <Werner.Wiedenmann@cern.ch>
###############################################################

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
